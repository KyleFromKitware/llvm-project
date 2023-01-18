//===--- HeaderGuardCheck.cpp - clang-tidy --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "HeaderGuardCheck.h"
#include "../ClangTidyModuleRegistry.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Path.h"

namespace clang::tidy::readability {

/// canonicalize a path by removing ./ and ../ components.
static std::string cleanPath(StringRef Path) {
  SmallString<256> Result = Path;
  llvm::sys::path::remove_dots(Result, true);
  return std::string(Result.str());
}

namespace {
class HeaderGuardCheckPPCallbacks : public PPCallbacks {
public:
  HeaderGuardCheckPPCallbacks(Preprocessor *PP, HeaderGuardCheck *Check,
                              utils::HeaderGuardStyle *Style)
      : PP(PP), Check(Check), Style(Style) {}

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID) override {
    // Record all files we enter. We'll need them to diagnose headers without
    // guards.
    SourceManager &SM = PP->getSourceManager();
    if (Reason == EnterFile && FileType == SrcMgr::C_User) {
      if (const FileEntry *FE = SM.getFileEntryForID(SM.getFileID(Loc))) {
        std::string FileName = cleanPath(FE->getName());
        Files[FileName] = FE;
      }
    }
  }

  void Ifndef(SourceLocation HashLoc, SourceLocation Loc,
              const Token &MacroNameTok, const MacroDefinition &MD) override {
    if (MD)
      return;

    // Record #ifndefs that succeeded. We also need the Location of the Name.
    Ifndefs[MacroNameTok.getIdentifierInfo()] =
        std::make_tuple(HashLoc, Loc, MacroNameTok.getLocation());
  }

  void MacroDefined(SourceLocation HashLoc, const Token &MacroNameTok,
                    const MacroDirective *MD) override {
    // Record all defined macros. We store the whole token to get info on the
    // name later.
    Macros.emplace_back(HashLoc, MacroNameTok, MD->getMacroInfo());
  }

  void Endif(SourceLocation HashLoc, SourceLocation Loc,
             SourceLocation IfLoc) override {
    // Record all #endif and the corresponding #ifs (including #ifndefs).
    EndIfs[IfLoc] = std::make_pair(HashLoc, Loc);
  }

  void EndOfMainFile() override {
    // Now that we have all this information from the preprocessor, use it!
    SourceManager &SM = PP->getSourceManager();

    for (const auto &MacroEntry : Macros) {
      const MacroInfo *MI = std::get<2>(MacroEntry);

      // We use clang's header guard detection. This has the advantage of also
      // emitting a warning for cases where a pseudo header guard is found but
      // preceded by something blocking the header guard optimization.
      if (!MI->isUsedForHeaderGuard())
        continue;

      const FileEntry *FE =
          SM.getFileEntryForID(SM.getFileID(MI->getDefinitionLoc()));
      std::string FileName = cleanPath(FE->getName());
      Files.erase(FileName);

      // See if we should check and fix this header guard.
      if (!Style->shouldFixIfHeaderGuard(FileName))
        continue;

      // Look up Locations for this guard.
      std::tuple<SourceLocation, SourceLocation, SourceLocation> Ifndef =
          Ifndefs[std::get<1>(MacroEntry).getIdentifierInfo()];
      std::pair<SourceLocation, SourceLocation> EndIf =
          EndIfs[std::get<1>(Ifndef)];

      Style->onHeaderGuard(PP, FileName, FE, std::get<0>(Ifndef),
                           std::get<1>(Ifndef), std::get<2>(Ifndef),
                           std::get<0>(MacroEntry), std::get<1>(MacroEntry),
                           EndIf.first, EndIf.second);
    }

    // Emit warnings for headers that are missing guards.
    checkGuardlessHeaders();
    clearAllState();
  }

  /// Looks for files that were visited but didn't have a header guard.
  /// Emits a warning with fixits suggesting adding one.
  void checkGuardlessHeaders() {
    // Look for header files that didn't have a header guard. Emit a warning and
    // fix-its to add the guard.
    // TODO: Insert the guard after top comments.
    for (const auto &FE : Files) {
      StringRef FileName = FE.getKey();
      if (!Style->shouldFixIfNoHeaderGuard(FileName))
        continue;

      SourceManager &SM = PP->getSourceManager();
      FileID FID = SM.translateFile(FE.getValue());
      SourceLocation StartLoc = SM.getLocForStartOfFile(FID);
      if (StartLoc.isInvalid())
        continue;

      Style->onGuardlessHeader(PP, FileName, FE.getValue(), StartLoc, Macros);
    }
  }

private:
  void clearAllState() {
    Macros.clear();
    Files.clear();
    Ifndefs.clear();
    EndIfs.clear();
  }

  std::vector<std::tuple<SourceLocation, Token, const MacroInfo *>> Macros;
  llvm::StringMap<const FileEntry *> Files;
  std::map<const IdentifierInfo *,
           std::tuple<SourceLocation, SourceLocation, SourceLocation>>
      Ifndefs;
  std::map<SourceLocation, std::pair<SourceLocation, SourceLocation>> EndIfs;

  Preprocessor *PP;
  HeaderGuardCheck *Check;
  utils::HeaderGuardStyle *Style;
};
} // namespace

void HeaderGuardCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "Style", StyleName);
}

void HeaderGuardCheck::registerPPCallbacks(const SourceManager &SM,
                                           Preprocessor *PP,
                                           Preprocessor *ModuleExpanderPP) {
  Style = createHeaderGuardStyle();
  if (Style)
    PP->addPPCallbacks(
        std::make_unique<HeaderGuardCheckPPCallbacks>(PP, this, Style.get()));
}

std::unique_ptr<utils::HeaderGuardStyle>
HeaderGuardCheck::createHeaderGuardStyle() {
  utils::HeaderGuardStyleFactories StyleFactories;
  for (ClangTidyModuleRegistry::entry E : ClangTidyModuleRegistry::entries()) {
    E.instantiate()->addHeaderGuardStyleFactories(StyleFactories);
  }

  auto It = StyleFactories.find(StyleName);
  if (It == StyleFactories.end()) {
    std::string MsgStr;
    llvm::raw_string_ostream Msg(MsgStr);
    Msg << "no such header guard style: '" << StyleName << "'";
    configurationDiag(Msg.str(), DiagnosticIDs::Error);
    return nullptr;
  }

  return It->second(this);
}
} // namespace clang::tidy::readability
