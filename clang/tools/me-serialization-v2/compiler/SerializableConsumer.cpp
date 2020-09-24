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
  if (!shouldParse())
    return;

  // parse header
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  // now parse the fields of each record
  for (auto &Record : Cache) {
    Record.second.parseFields();
  }
  // got errors when parsing, do not continue
  if (HasErrors)
    return;

  std::unordered_map<std::string, RecordInfo &> RecordInfoMap;
  for (auto &Pair : Cache) {
    auto &RecordInfo = Pair.second;
    RecordInfoMap.emplace(RecordInfo.fullName(), RecordInfo);
  }

  // parse database
  auto &Database = ChangedDatabase.emplace(InFile, Diags).first->second;
  Database.parse(DatabaseFile, RecordInfoMap);
}
SerializableConsumer::SerializableConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile,
    std::unordered_map<std::string, RecordDatabase> &ChangedDatabase)
    : Visitor(*this), Context(&Compiler.getASTContext()), InFile(InFile),
      Diags(Compiler.getDiagnostics()), ChangedDatabase(ChangedDatabase) {

  // check if input file is header
  llvm::StringRef PathExt = llvm::sys::path::extension(InFile);
  if (HeaderExtensions.find(PathExt.str()) == HeaderExtensions.end()) {
    llvm::errs() << "Error: " << InFile << " seems not a header file\n";
    std::terminate();
  }
  llvm::SmallString<128> Buffer(InFile);

  // get correspond database
  llvm::sys::path::replace_extension(Buffer, "mes11n.db");
  DatabaseFile = Buffer.str().str();

  // get correspond mes11n.obj
  Buffer.assign(InFile);
  llvm::sys::path::replace_extension(Buffer, "mes11n.obj");
  ObjFile = Buffer.str().str();
}
bool SerializableConsumer::shouldParse() {
  // is head and database of of date?
  llvm::sys::fs::file_status file_status;
  llvm::sys::fs::status(InFile, file_status);
  auto LastInFile = file_status.getLastModificationTime();
  llvm::sys::fs::status(DatabaseFile, file_status);
  auto LastDatabaseFile = file_status.getLastModificationTime();
  llvm::sys::fs::status(ObjFile, file_status);
  auto LastObjFile = file_status.getLastModificationTime();
  return LastInFile < LastObjFile && LastDatabaseFile < LastObjFile;
}
