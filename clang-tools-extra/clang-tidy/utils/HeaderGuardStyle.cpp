//===--- HeaderGuardStyle.cpp - clang-tidy ----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "HeaderGuardStyle.h"
#include "../readability/HeaderGuardCheck.h"
#include "../utils/FileExtensionsUtils.h"

namespace clang::tidy::utils {
bool HeaderGuardStyle::shouldFixIfHeaderGuard(StringRef Filename) {
  return true;
}

bool HeaderGuardStyle::shouldFixIfNoHeaderGuard(StringRef FileName) {
  return utils::isFileExtension(FileName, Check->getHeaderFileExtensions());
}

void HeaderGuardStyleFactories::registerStyleFactory(StringRef Name,
                                                     StyleFactory Factory) {
  Factories.insert_or_assign(Name, std::move(Factory));
}
} // namespace clang::tidy::utils
