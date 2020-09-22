#include "SerializableConsumer.h"
#include "CodeGuard.h"
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
std::set<std::string> SerializableConsumer::HeaderExtensions{
    ".h", ".hh", ".hpp", ".hxx", ".inc"};
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
}
SerializableConsumer::SerializableConsumer(clang::CompilerInstance &Compiler,
                                           llvm::StringRef InFile)
    : Visitor(*this), Context(&Compiler.getASTContext()),
      HeaderGen(InFile, Compiler.getDiagnostics(), Cache) {

  llvm::StringRef PathExt = llvm::sys::path::extension(InFile);
  if (HeaderExtensions.find(PathExt.str()) == HeaderExtensions.end()) {
    llvm::errs() << "Error: " << InFile << " seems not a header file\n";
    std::terminate();
  }
}
