//===--- HeaderGuardCheck.h - clang-tidy ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_READABILITY_HEADERGUARDCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_READABILITY_HEADERGUARDCHECK_H

#include "../ClangTidyCheck.h"
#include "../utils/FileExtensionsUtils.h"
#include "../utils/HeaderGuardStyle.h"

namespace clang::tidy::readability {

/// Finds and fixes header guards.
/// The check supports these options:
///   - `Style`: the name of a header guard style to use. The only available
///     option is "llvm". "llvm" by default.
class HeaderGuardCheck : public ClangTidyCheck {
public:
  HeaderGuardCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context),
        RawStringHeaderFileExtensions(Options.getLocalOrGlobal(
            "HeaderFileExtensions", utils::defaultHeaderFileExtensions())),
        StyleName(Options.getLocalOrGlobal("Style", "llvm")) {
    utils::parseFileExtensions(RawStringHeaderFileExtensions,
                               HeaderFileExtensions,
                               utils::defaultFileExtensionDelimiters());
  }
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;
  void registerPPCallbacks(const SourceManager &SM, Preprocessor *PP,
                           Preprocessor *ModuleExpanderPP) override;
  utils::FileExtensionsSet &getHeaderFileExtensions() {
    return HeaderFileExtensions;
  }

protected:
  virtual std::unique_ptr<utils::HeaderGuardStyle> createHeaderGuardStyle();

  std::string RawStringHeaderFileExtensions;
  utils::FileExtensionsSet HeaderFileExtensions;
  std::string StyleName;
  std::unique_ptr<utils::HeaderGuardStyle> Style;
};

} // namespace clang::tidy::readability

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_READABILITY_HEADERGUARDCHECK_H
