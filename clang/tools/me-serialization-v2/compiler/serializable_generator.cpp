#include "serializable_generator.h"
#include <clang/Basic/FileManager.h>
#include <cstdarg>
#include <fstream>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
bool SerializableVisitor::VisitCXXRecordDecl(
    clang::CXXRecordDecl *Declaration) {
  Consumer.getRecord(Declaration);
  return true;
}
void SerializableConsumer::LogWarning(const clang::Decl *Decl,
                                      const char *Format, ...) {
  const auto &source_manager = Decl->getASTContext().getSourceManager();
  Decl->getLocation().dump(source_manager);
  va_list Argptr;
  va_start(Argptr, Format);
  vfprintf(stderr, Format, Argptr);
  va_end(Argptr);
}
void SerializableConsumer::LogError(const clang::Decl *Decl, const char *format,
                                    ...) {
  va_list Argptr;
  va_start(Argptr, format);
  LogWarning(Decl, format, Argptr);
  va_end(Argptr);
  HasErrors = true;
}
void SerializableConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  // now parse the fields of each record
  for (auto &Record : Cache) {
    Record.second.ParseFields();
  }
  // got errors when parsing, do not continue
  if (HasErrors)
    return;

  // now generator codes for all records
  std::fstream Cpp((InFile + ".cpp").str(), std::ios::out | std::ios::trunc);
  for (auto &Record : Cache) {
    Record.second.ToCpp(Cpp, Indexer);
  }
  Cpp.close();
  // update index file
  std::fstream IndexFile((InFile + ".index").str(),
                         std::ios::out | std::ios::trunc);
  Indexer.refresh(IndexFile);
  IndexFile.close();
}
SerializableConsumer::SerializableConsumer(clang::CompilerInstance &Compiler,
                                           llvm::StringRef InFile)
    : Visitor(*this), Context(&Compiler.getASTContext()), InFile(InFile) {
  auto& VFS = Compiler.getVirtualFileSystem();
  llvm::SmallString<128> Path(InFile);
  llvm::sys::path::replace_extension(Path, ".db");
  if (!VFS.exists(Path)) {
    Compiler.getFileManager().
  }
  clang::SourceManager &SM = Compiler.getSourceManager();

  auto FileEntry = Compiler.getFileManager().getFile(Path);
  clang::FileID FileId = SM.createFileID(
      FileEntry ? *FileEntry : nullptr, clang::SourceLocation(), clang::SrcMgr::C_User);

                             std::fstream IndexFile((InFile + ".index").str());
  Indexer.parse(IndexFile);
  IndexFile.close();
}
