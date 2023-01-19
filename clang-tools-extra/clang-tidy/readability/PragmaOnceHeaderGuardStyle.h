//===--- PragmaOnceHeaderGuardStyle.h - clang-tidy --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_READABILITY_PRAGMAONCEHEADERGUARDSTYLE_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_READABILITY_PRAGMAONCEHEADERGUARDSTYLE_H

#include "../utils/HeaderGuardStyle.h"

namespace clang::tidy::readability {

/// Header guard style that suggests the use of #pragma once.
class PragmaOnceHeaderGuardStyle : public utils::HeaderGuardStyle {
public:
  PragmaOnceHeaderGuardStyle(HeaderGuardCheck *Check)
      : HeaderGuardStyle(Check) {}

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
};
} // namespace clang::tidy::readability

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_READABILITY_PRAGMAONCEHEADERGUARDSTYLE_H
