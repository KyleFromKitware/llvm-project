//===--- PragmaOnceHeaderGuardStyle.cpp - clang-tidy ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "PragmaOnceHeaderGuardStyle.h"
#include "HeaderGuardCheck.h"
#include "clang/Lex/Preprocessor.h"

namespace clang::tidy::readability {
namespace {
CharSourceRange goToLineEnd(Preprocessor *PP, SourceLocation StartLoc,
                            SourceLocation EndLoc) {
  SourceManager &SM = PP->getSourceManager();
  FileID FID = SM.getFileID(EndLoc);
  StringRef BufData = SM.getBufferData(FID);
  const char *EndData = BufData.begin() + SM.getFileOffset(EndLoc);
  Lexer Lex(EndLoc, PP->getLangOpts(), BufData.begin(), EndData, BufData.end());
  // FIXME: this is a bit hacky to get ReadToEndOfLine work.
  Lex.setParsingPreprocessorDirective(true);
  Lex.ReadToEndOfLine();
  return CharSourceRange::getCharRange(
      StartLoc, SM.getLocForStartOfFile(FID).getLocWithOffset(
                    Lex.getCurrentBufferOffset()));
}
} // namespace

void PragmaOnceHeaderGuardStyle::onHeaderGuard(
    Preprocessor *PP, StringRef FileName, const FileEntry *FE,
    SourceLocation IfndefHash, SourceLocation Ifndef,
    SourceLocation IfndefToken, SourceLocation DefineHash, const Token &Define,
    SourceLocation EndIfHash, SourceLocation EndIf) {
  if (!Ifndef.isValid())
    return;

  std::vector<FixItHint> FixIts;

  HeaderSearch &HeaderInfo = PP->getHeaderSearchInfo();

  HeaderFileInfo &Info = HeaderInfo.getFileInfo(FE);

  CharSourceRange IfndefSrcRange = goToLineEnd(PP, IfndefHash, IfndefToken);
  CharSourceRange DefineSrcRange =
      goToLineEnd(PP, DefineHash, Define.getLocation());
  CharSourceRange EndifSrcRange = goToLineEnd(PP, EndIfHash, EndIf);

  if (Info.isPragmaOnce)
    FixIts.push_back(FixItHint::CreateRemoval(IfndefSrcRange));
  else
    FixIts.push_back(
        FixItHint::CreateReplacement(IfndefSrcRange, "#pragma once\n"));

  FixIts.push_back(FixItHint::CreateRemoval(DefineSrcRange));
  FixIts.push_back(FixItHint::CreateRemoval(EndifSrcRange));

  Check->diag(IfndefSrcRange.getBegin(), "use #pragma once") << FixIts;
}

void PragmaOnceHeaderGuardStyle::onGuardlessHeader(
    Preprocessor *PP, StringRef FileName, const FileEntry *FE,
    SourceLocation StartLoc,
    const std::vector<std::tuple<SourceLocation, Token, const MacroInfo *>>
        &Macros) {
  HeaderSearch &HeaderInfo = PP->getHeaderSearchInfo();

  HeaderFileInfo &Info = HeaderInfo.getFileInfo(FE);
  if (Info.isPragmaOnce)
    return;

  Check->diag(StartLoc, "use #pragma once")
      << FixItHint::CreateInsertion(StartLoc, "#pragma once\n");
}
} // namespace clang::tidy::readability
