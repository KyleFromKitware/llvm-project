//===--- MacroHeaderGuardStyle.h - clang-tidy -------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_UTILS_MACROHEADERGUARDSTYLE_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_UTILS_MACROHEADERGUARDSTYLE_H

#include "HeaderGuardStyle.h"

namespace clang::tidy::utils {
class MacroHeaderGuardStyle : public HeaderGuardStyle {
public:
  MacroHeaderGuardStyle(readability::HeaderGuardCheck *Check)
      : HeaderGuardStyle(Check) {}

  /// Ensure that the provided header guard is a non-reserved identifier.
  std::string sanitizeHeaderGuard(StringRef Guard);

  /// Returns ``true`` if the check should suggest inserting a trailing comment
  /// on the ``#endif`` of the header guard. It will use the same name as
  /// returned by ``HeaderGuardCheck::getHeaderGuard``.
  virtual bool shouldSuggestEndifComment(StringRef Filename);
  /// Returns a replacement for the ``#endif`` line with a comment mentioning
  /// \p HeaderGuard. The replacement should start with ``endif``.
  virtual std::string formatEndIf(StringRef HeaderGuard);
  /// Gets the canonical header guard for a file.
  virtual std::string getHeaderGuard(StringRef Filename,
                                     StringRef OldGuard = StringRef()) = 0;

  void onHeaderGuard(Preprocessor *PP, StringRef FileName, const FileEntry *FE,
                     SourceLocation IfndefHash, SourceLocation Ifndef,
                     SourceLocation IfndefToken, SourceLocation DefineHash,
                     const Token &Define, SourceLocation EndIfHash,
                     SourceLocation EndIf) override;
  void onGuardlessHeader(
      Preprocessor *PP, StringRef FileName, const FileEntry *FE,
      SourceLocation StartLoc,
      const std::vector<std::tuple<SourceLocation, Token, const MacroInfo *>>
          &Macros) override;

private:
  bool wouldFixEndifComment(Preprocessor *PP, StringRef FileName,
                            SourceLocation EndIf, StringRef HeaderGuard,
                            size_t *EndIfLenPtr = nullptr);
  std::string
  checkHeaderGuardDefinition(Preprocessor *PP, SourceLocation Ifndef,
                             SourceLocation Define, SourceLocation EndIf,
                             StringRef FileName, StringRef CurHeaderGuard,
                             std::vector<FixItHint> &FixIts);
  void checkEndifComment(Preprocessor *PP, StringRef FileName,
                         SourceLocation EndIf, StringRef HeaderGuard,
                         std::vector<FixItHint> &FixIts);
};
} // namespace clang::tidy::utils

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_UTILS_MACROHEADERGUARDSTYLE_H
