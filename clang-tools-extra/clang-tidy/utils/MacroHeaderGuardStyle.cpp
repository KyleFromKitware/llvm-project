//===--- MacroHeaderGuardStyle.cpp - clang-tidy -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MacroHeaderGuardStyle.h"
#include "../readability/HeaderGuardCheck.h"
#include "../utils/FileExtensionsUtils.h"
#include "clang/Lex/Preprocessor.h"

namespace clang::tidy::utils {

void MacroHeaderGuardStyle::onHeaderGuard(
    Preprocessor *PP, StringRef FileName, const FileEntry *FE,
    SourceLocation IfndefHash, SourceLocation Ifndef,
    SourceLocation IfndefToken, SourceLocation DefineHash, const Token &Define,
    SourceLocation EndIfHash, SourceLocation EndIf) {
  // If the macro Name is not equal to what we can compute, correct it in
  // the #ifndef and #define.
  StringRef CurHeaderGuard = Define.getIdentifierInfo()->getName();
  std::vector<FixItHint> FixIts;
  std::string NewGuard =
      checkHeaderGuardDefinition(PP, IfndefToken, Define.getLocation(), EndIf,
                                 FileName, CurHeaderGuard, FixIts);

  // Now look at the #endif. We want a comment with the header guard. Fix it
  // at the slightest deviation.
  checkEndifComment(PP, FileName, EndIf, NewGuard, FixIts);

  // Bundle all fix-its into one warning. The message depends on whether we
  // changed the header guard or not.
  if (!FixIts.empty()) {
    if (CurHeaderGuard != NewGuard) {
      Check->diag(Ifndef, "header guard does not follow preferred style")
          << FixIts;
    } else {
      Check->diag(EndIf, "#endif for a header guard should reference the "
                         "guard macro in a comment")
          << FixIts;
    }
  }
}

bool MacroHeaderGuardStyle::wouldFixEndifComment(Preprocessor *PP,
                                                 StringRef FileName,
                                                 SourceLocation EndIf,
                                                 StringRef HeaderGuard,
                                                 size_t *EndIfLenPtr) {
  if (!EndIf.isValid())
    return false;
  const char *EndIfData = PP->getSourceManager().getCharacterData(EndIf);
  size_t EndIfLen = std::strcspn(EndIfData, "\r\n");
  if (EndIfLenPtr)
    *EndIfLenPtr = EndIfLen;

  StringRef EndIfStr(EndIfData, EndIfLen);
  EndIfStr = EndIfStr.substr(EndIfStr.find_first_not_of("#endif \t"));

  // Give up if there's an escaped newline.
  size_t FindEscapedNewline = EndIfStr.find_last_not_of(' ');
  if (FindEscapedNewline != StringRef::npos &&
      EndIfStr[FindEscapedNewline] == '\\')
    return false;

  bool IsLineComment =
      EndIfStr.consume_front("//") ||
      (EndIfStr.consume_front("/*") && EndIfStr.consume_back("*/"));
  if (!IsLineComment)
    return shouldSuggestEndifComment(FileName);

  return EndIfStr.trim() != HeaderGuard;
}

/// Look for header guards that don't match the preferred style. Emit
/// fix-its and return the suggested header guard (or the original if no
/// change was made.
std::string MacroHeaderGuardStyle::checkHeaderGuardDefinition(
    Preprocessor *PP, SourceLocation Ifndef, SourceLocation Define,
    SourceLocation EndIf, StringRef FileName, StringRef CurHeaderGuard,
    std::vector<FixItHint> &FixIts) {
  std::string CPPVar = getHeaderGuard(FileName, CurHeaderGuard);
  CPPVar = sanitizeHeaderGuard(CPPVar);
  std::string CPPVarUnder = CPPVar + '_';

  // Allow a trailing underscore if and only if we don't have to change the
  // endif comment too.
  if (Ifndef.isValid() && CurHeaderGuard != CPPVar &&
      (CurHeaderGuard != CPPVarUnder ||
       wouldFixEndifComment(PP, FileName, EndIf, CurHeaderGuard))) {
    FixIts.push_back(FixItHint::CreateReplacement(
        CharSourceRange::getTokenRange(
            Ifndef, Ifndef.getLocWithOffset(CurHeaderGuard.size())),
        CPPVar));
    FixIts.push_back(FixItHint::CreateReplacement(
        CharSourceRange::getTokenRange(
            Define, Define.getLocWithOffset(CurHeaderGuard.size())),
        CPPVar));
    return CPPVar;
  }
  return std::string(CurHeaderGuard);
}

/// Checks the comment after the #endif of a header guard and fixes it
/// if it doesn't match \c HeaderGuard.
void MacroHeaderGuardStyle::checkEndifComment(Preprocessor *PP,
                                              StringRef FileName,
                                              SourceLocation EndIf,
                                              StringRef HeaderGuard,
                                              std::vector<FixItHint> &FixIts) {
  size_t EndIfLen;
  if (wouldFixEndifComment(PP, FileName, EndIf, HeaderGuard, &EndIfLen)) {
    FixIts.push_back(FixItHint::CreateReplacement(
        CharSourceRange::getCharRange(EndIf, EndIf.getLocWithOffset(EndIfLen)),
        formatEndIf(HeaderGuard)));
  }
}

void MacroHeaderGuardStyle::onGuardlessHeader(
    Preprocessor *PP, StringRef FileName, const FileEntry *FE,
    SourceLocation StartLoc,
    const std::vector<std::tuple<SourceLocation, Token, const MacroInfo *>>
        &Macros) {
  SourceManager &SM = PP->getSourceManager();
  std::string CPPVar = getHeaderGuard(FileName);
  CPPVar = sanitizeHeaderGuard(CPPVar);
  std::string CPPVarUnder = CPPVar + '_'; // Allow a trailing underscore.
  // If there's a macro with a name that follows the header guard convention
  // but was not recognized by the preprocessor as a header guard there must
  // be code outside of the guarded area. Emit a plain warning without
  // fix-its.
  // FIXME: Can we move it into the right spot?
  bool SeenMacro = false;
  for (const auto &MacroEntry : Macros) {
    StringRef Name = std::get<1>(MacroEntry).getIdentifierInfo()->getName();
    SourceLocation DefineLoc = std::get<1>(MacroEntry).getLocation();
    if ((Name == CPPVar || Name == CPPVarUnder) &&
        SM.isWrittenInSameFile(StartLoc, DefineLoc)) {
      Check->diag(DefineLoc, "code/includes outside of area guarded by "
                             "header guard; consider moving it");
      SeenMacro = true;
      break;
    }
  }

  if (SeenMacro)
    return;

  Check->diag(StartLoc, "header is missing header guard")
      << FixItHint::CreateInsertion(
             StartLoc, "#ifndef " + CPPVar + "\n#define " + CPPVar + "\n\n")
      << FixItHint::CreateInsertion(SM.getLocForEndOfFile(SM.translateFile(FE)),
                                    shouldSuggestEndifComment(FileName)
                                        ? "\n#" + formatEndIf(CPPVar) + "\n"
                                        : "\n#endif\n");
}

std::string MacroHeaderGuardStyle::sanitizeHeaderGuard(StringRef Guard) {
  // Only reserved identifiers are allowed to start with an '_'.
  return Guard.drop_while([](char C) { return C == '_'; }).str();
}

bool MacroHeaderGuardStyle::shouldSuggestEndifComment(StringRef FileName) {
  return utils::isFileExtension(FileName, Check->getHeaderFileExtensions());
}

std::string MacroHeaderGuardStyle::formatEndIf(StringRef HeaderGuard) {
  return "endif // " + HeaderGuard.str();
}
} // namespace clang::tidy::utils
