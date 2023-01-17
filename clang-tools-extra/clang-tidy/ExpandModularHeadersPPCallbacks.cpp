//===- ExpandModularHeadersPPCallbacks.h - clang-tidy -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ExpandModularHeadersPPCallbacks.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Serialization/ASTReader.h"
#include <optional>

#define DEBUG_TYPE "clang-tidy"

namespace clang::tooling {

class ExpandModularHeadersPPCallbacks::FileRecorder {
public:
  /// Records that a given file entry is needed for replaying callbacks.
  void addNecessaryFile(const FileEntry *File) {
    // Don't record modulemap files because it breaks same file detection.
    if (!(File->getName().endswith("module.modulemap") ||
          File->getName().endswith("module.private.modulemap") ||
          File->getName().endswith("module.map") ||
          File->getName().endswith("module_private.map")))
      FilesToRecord.insert(File);
  }

  /// Records content for a file and adds it to the FileSystem.
  void recordFileContent(const FileEntry *File,
                         const SrcMgr::ContentCache &ContentCache,
                         llvm::vfs::InMemoryFileSystem &InMemoryFs) {
    // Return if we are not interested in the contents of this file.
    if (!FilesToRecord.count(File))
      return;

    // FIXME: Why is this happening? We might be losing contents here.
    std::optional<StringRef> Data = ContentCache.getBufferDataIfLoaded();
    if (!Data)
      return;

    InMemoryFs.addFile(File->getName(), /*ModificationTime=*/0,
                       llvm::MemoryBuffer::getMemBufferCopy(*Data));
    // Remove the file from the set of necessary files.
    FilesToRecord.erase(File);
  }

  /// Makes sure we have contents for all the files we were interested in. Ideally
  /// `FilesToRecord` should be empty.
  void checkAllFilesRecorded() {
    LLVM_DEBUG({
      for (auto FileEntry : FilesToRecord)
        llvm::dbgs() << "Did not record contents for input file: "
                     << FileEntry->getName() << "\n";
    });
  }

private:
  /// A set of files whose contents are to be recorded.
  llvm::DenseSet<const FileEntry *> FilesToRecord;
};

ExpandModularHeadersPPCallbacks::ExpandModularHeadersPPCallbacks(
    CompilerInstance *CI,
    IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem> OverlayFS)
    : Recorder(std::make_unique<FileRecorder>()), Compiler(*CI),
      InMemoryFs(new llvm::vfs::InMemoryFileSystem),
      Sources(Compiler.getSourceManager()),
      // Forward the new diagnostics to the original DiagnosticConsumer.
      Diags(new DiagnosticIDs, new DiagnosticOptions,
            new ForwardingDiagnosticConsumer(Compiler.getDiagnosticClient())),
      LangOpts(Compiler.getLangOpts()) {
  // Add a FileSystem containing the extra files needed in place of modular
  // headers.
  OverlayFS->pushOverlay(InMemoryFs);

  Diags.setSourceManager(&Sources);

  LangOpts.Modules = false;

  auto HSO = std::make_shared<HeaderSearchOptions>();
  *HSO = Compiler.getHeaderSearchOpts();

  HeaderInfo = std::make_unique<HeaderSearch>(HSO, Sources, Diags, LangOpts,
                                               &Compiler.getTarget());

  auto PO = std::make_shared<PreprocessorOptions>();
  *PO = Compiler.getPreprocessorOpts();

  PP = std::make_unique<clang::Preprocessor>(PO, Diags, LangOpts, Sources,
                                              *HeaderInfo, ModuleLoader,
                                              /*IILookup=*/nullptr,
                                              /*OwnsHeaderSearch=*/false);
  PP->Initialize(Compiler.getTarget(), Compiler.getAuxTarget());
  InitializePreprocessor(*PP, *PO, Compiler.getPCHContainerReader(),
                         Compiler.getFrontendOpts());
  ApplyHeaderSearchOptions(*HeaderInfo, *HSO, LangOpts,
                           Compiler.getTarget().getTriple());
}

ExpandModularHeadersPPCallbacks::~ExpandModularHeadersPPCallbacks() = default;

Preprocessor *ExpandModularHeadersPPCallbacks::getPreprocessor() const {
  return PP.get();
}

void ExpandModularHeadersPPCallbacks::handleModuleFile(
    serialization::ModuleFile *MF) {
  if (!MF)
    return;
  // Avoid processing a ModuleFile more than once.
  if (VisitedModules.count(MF))
    return;
  VisitedModules.insert(MF);

  // Visit all the input files of this module and mark them to record their
  // contents later.
  Compiler.getASTReader()->visitInputFiles(
      *MF, true, false,
      [this](const serialization::InputFile &IF, bool /*IsSystem*/) {
        Recorder->addNecessaryFile(IF.getFile());
      });
  // Recursively handle all transitively imported modules.
  for (auto *Import : MF->Imports)
    handleModuleFile(Import);
}

void ExpandModularHeadersPPCallbacks::parseToLocation(SourceLocation Loc) {
  // Load all source locations present in the external sources.
  for (unsigned I = 0, N = Sources.loaded_sloc_entry_size(); I != N; ++I) {
    Sources.getLoadedSLocEntry(I, nullptr);
  }
  // Record contents of files we are interested in and add to the FileSystem.
  for (auto It = Sources.fileinfo_begin(); It != Sources.fileinfo_end(); ++It) {
    Recorder->recordFileContent(It->getFirst(), *It->getSecond(), *InMemoryFs);
  }
  Recorder->checkAllFilesRecorded();

  if (!StartedLexing) {
    StartedLexing = true;
    PP->Lex(CurrentToken);
  }
  while (!CurrentToken.is(tok::eof) &&
         Sources.isBeforeInTranslationUnit(CurrentToken.getLocation(), Loc)) {
    PP->Lex(CurrentToken);
  }
}

void ExpandModularHeadersPPCallbacks::FileChanged(
    SourceLocation Loc, FileChangeReason Reason,
    SrcMgr::CharacteristicKind FileType, FileID PrevFID = FileID()) {
  if (!EnteredMainFile) {
    EnteredMainFile = true;
    PP->EnterMainSourceFile();
  }
}

void ExpandModularHeadersPPCallbacks::InclusionDirective(
    SourceLocation DirectiveLoc, const Token &IncludeToken,
    StringRef IncludedFilename, bool IsAngled, CharSourceRange FilenameRange,
    OptionalFileEntryRef IncludedFile, StringRef SearchPath,
    StringRef RelativePath, const Module *Imported,
    SrcMgr::CharacteristicKind FileType) {
  if (Imported) {
    serialization::ModuleFile *MF =
        Compiler.getASTReader()->getModuleManager().lookup(
            Imported->getASTFile());
    handleModuleFile(MF);
  }
  parseToLocation(DirectiveLoc);
}

void ExpandModularHeadersPPCallbacks::EndOfMainFile() {
  while (!CurrentToken.is(tok::eof))
    PP->Lex(CurrentToken);
}

// Handle all other callbacks.
// Just parse to the corresponding location to generate the same callback for
// the PPCallbacks registered in our custom preprocessor.
void ExpandModularHeadersPPCallbacks::Ident(SourceLocation HashLoc,
                                            SourceLocation Loc, StringRef) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaDirective(
    PragmaIntroducer Introducer) {
  parseToLocation(Introducer.Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaComment(PragmaIntroducer Introducer,
                                                    SourceLocation Loc,
                                                    const IdentifierInfo *,
                                                    StringRef) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaDetectMismatch(
    PragmaIntroducer Introducer, SourceLocation Loc, StringRef, StringRef) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaDebug(PragmaIntroducer Introducer,
                                                  SourceLocation Loc,
                                                  StringRef) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaMessage(PragmaIntroducer Introducer,
                                                    SourceLocation Loc,
                                                    StringRef,
                                                    PragmaMessageKind,
                                                    StringRef) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaDiagnosticPush(
    PragmaIntroducer Introducer, SourceLocation Loc, StringRef) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaDiagnosticPop(
    PragmaIntroducer Introducer, SourceLocation Loc, StringRef) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaDiagnostic(
    PragmaIntroducer Introducer, SourceLocation Loc, StringRef, diag::Severity,
    StringRef) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::HasInclude(SourceLocation Loc, StringRef,
                                                 bool, OptionalFileEntryRef,
                                                 SrcMgr::CharacteristicKind) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaOpenCLExtension(
    PragmaIntroducer Introducer, SourceLocation NameLoc, const IdentifierInfo *,
    SourceLocation StateLoc, unsigned) {
  // FIXME: Figure out whether it's the right location to parse to.
  parseToLocation(NameLoc);
}
void ExpandModularHeadersPPCallbacks::PragmaWarning(PragmaIntroducer Introducer,
                                                    SourceLocation Loc,
                                                    PragmaWarningSpecifier,
                                                    ArrayRef<int>) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaWarningPush(
    PragmaIntroducer Introducer, SourceLocation Loc, int) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaWarningPop(
    PragmaIntroducer Introducer, SourceLocation Loc) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaAssumeNonNullBegin(
    PragmaIntroducer Introducer, SourceLocation Loc) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::PragmaAssumeNonNullEnd(
    PragmaIntroducer Introducer, SourceLocation Loc) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::MacroExpands(const Token &MacroNameTok,
                                                   const MacroDefinition &,
                                                   SourceRange Range,
                                                   const MacroArgs *) {
  // FIXME: Figure out whether it's the right location to parse to.
  parseToLocation(Range.getBegin());
}
void ExpandModularHeadersPPCallbacks::MacroDefined(SourceLocation HashLoc,
                                                   const Token &MacroNameTok,
                                                   const MacroDirective *MD) {
  parseToLocation(MD->getLocation());
}
void ExpandModularHeadersPPCallbacks::MacroUndefined(
    SourceLocation HashLoc, const Token &, const MacroDefinition &,
    const MacroDirective *Undef) {
  if (Undef)
    parseToLocation(Undef->getLocation());
}
void ExpandModularHeadersPPCallbacks::Defined(const Token &MacroNameTok,
                                              const MacroDefinition &,
                                              SourceRange Range) {
  // FIXME: Figure out whether it's the right location to parse to.
  parseToLocation(Range.getBegin());
}
void ExpandModularHeadersPPCallbacks::SourceRangeSkipped(
    SourceRange Range, SourceLocation EndifLoc) {
  // FIXME: Figure out whether it's the right location to parse to.
  parseToLocation(EndifLoc);
}
void ExpandModularHeadersPPCallbacks::If(SourceLocation HashLoc,
                                         SourceLocation Loc, SourceRange,
                                         ConditionValueKind) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::Elif(SourceLocation HashLoc,
                                           SourceLocation Loc, SourceRange,
                                           ConditionValueKind, SourceLocation) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::Ifdef(SourceLocation HashLoc,
                                            SourceLocation Loc, const Token &,
                                            const MacroDefinition &) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::Ifndef(SourceLocation HashLoc,
                                             SourceLocation Loc, const Token &,
                                             const MacroDefinition &) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::Else(SourceLocation HashLoc,
                                           SourceLocation Loc, SourceLocation) {
  parseToLocation(Loc);
}
void ExpandModularHeadersPPCallbacks::Endif(SourceLocation HashLoc,
                                            SourceLocation Loc,
                                            SourceLocation) {
  parseToLocation(Loc);
}

} // namespace clang::tooling
