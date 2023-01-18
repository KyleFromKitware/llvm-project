//===--- HeaderGuardStyle.h - clang-tidy ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_UTILS_HEADERGUARDSTYLE_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_UTILS_HEADERGUARDSTYLE_H

#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/StringMap.h"

namespace clang {
class MacroInfo;

namespace tidy {
namespace readability {
class HeaderGuardCheck;
} // namespace readability

namespace utils {
class HeaderGuardStyle {
public:
  HeaderGuardStyle(readability::HeaderGuardCheck *Check) : Check(Check) {}
  virtual ~HeaderGuardStyle() = default;

  /// Returns ``true`` if the check should suggest fixing a header file if it
  /// has an existing header guard.
  virtual bool shouldFixIfHeaderGuard(StringRef Filename);

  /// Returns ``true`` if the check should suggest fixing a header file if it
  /// does not have an existing header guard.
  virtual bool shouldFixIfNoHeaderGuard(StringRef Filename);

  /// @brief Called when a header guard is detected.
  /// @param PP Preprocessor.
  /// @param FileName Name of the file.
  /// @param FE FileEntry.
  /// @param IfndefHash Location of '#' in '#ifndef'.
  /// @param Ifndef Location of 'ifndef' in '#ifndef'.
  /// @param IfndefToken Location of macro token in '#ifndef'.
  /// @param DefineHash Location of '#' in '#define'.
  /// @param Define Location of macro token in '#define'.
  /// @param EndIfHash Location of '#' in '#endif'.
  /// @param EndIf Location of 'endif' in '#endif'.
  virtual void onHeaderGuard(Preprocessor *PP, StringRef FileName,
                             const FileEntry *FE, SourceLocation IfndefHash,
                             SourceLocation Ifndef, SourceLocation IfndefToken,
                             SourceLocation DefineHash, const Token &Define,
                             SourceLocation EndIfHash,
                             SourceLocation EndIf) = 0;

  /// @brief Called when a header with no header guard is detected.
  /// @param PP Preprocessor.
  /// @param FileName Name of the file.
  /// @param FE FileEntry.
  /// @param StartLoc Location of the start of the file.
  /// @param Macros List of macros in this file. Contains location of '#', macro
  /// token, and macro info.
  virtual void onGuardlessHeader(
      Preprocessor *PP, StringRef FileName, const FileEntry *FE,
      SourceLocation StartLoc,
      const std::vector<std::tuple<SourceLocation, Token, const MacroInfo *>>
          &Macros) = 0;

protected:
  readability::HeaderGuardCheck *Check;
};

/// A collection of \c HeaderGuardStyle factories.
///
/// All clang-tidy modules register their header guard style factories with an
/// instance of this object.
class HeaderGuardStyleFactories {
public:
  using StyleFactory = std::function<std::unique_ptr<HeaderGuardStyle>(
      readability::HeaderGuardCheck *Check)>;

  /// Registers style \p Factory with name \p Name.
  ///
  /// For all style that have constructors that take a HeaderGuardCheck*, use \c
  /// registerStyle.
  void registerStyleFactory(llvm::StringRef Name, StyleFactory Factory);

  /// Registers the \c StyleType with the name \p Name.
  ///
  /// This method should be used for all \c HeaderGuardStyles whose constructors
  /// take one \c HeaderGuardCheck * parameter.
  ///
  /// For example, if you have a header guard style like:
  /// \code
  /// class MyStyle : public HeaderGuardStyle {
  ///   bool shouldFixIfHeaderGuard(StringRef Filename) override {
  ///     ..
  ///   }
  /// };
  /// \endcode
  /// you can register it with:
  /// \code
  /// class MyModule : public ClangTidyModule {
  ///   void addHeaderGuardStyleFactories(HeaderGuardStyleFactories &Factories)
  ///   override {
  ///     Factories.registerStyle<MyStyle>("myproject-my-style");
  ///   }
  /// };
  /// \endcode
  template <typename StyleType> void registerStyle(llvm::StringRef StyleName) {
    registerStyleFactory(StyleName, [](readability::HeaderGuardCheck *Check) {
      return std::make_unique<StyleType>(Check);
    });
  }

  typedef llvm::StringMap<StyleFactory> FactoryMap;
  FactoryMap::const_iterator begin() const { return Factories.begin(); }
  FactoryMap::const_iterator end() const { return Factories.end(); }
  FactoryMap::const_iterator find(StringRef Name) const {
    return Factories.find(Name);
  }
  bool empty() const { return Factories.empty(); }

private:
  FactoryMap Factories;
};
} // namespace utils
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_UTILS_HEADERGUARDSTYLE_H
