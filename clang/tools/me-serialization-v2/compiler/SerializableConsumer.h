#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZABLE_GENERATOR_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZABLE_GENERATOR_H_
#include "CodeGenerator.h"
#include "RecordDatabase.h"
#include "RecordInfo.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
class SerializableVisitor
    : public clang::RecursiveASTVisitor<SerializableVisitor> {
public:
  SerializableVisitor(SerializableConsumer &Consumer) : Consumer(Consumer) {}
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *Declaration);

private:
  SerializableConsumer &Consumer;
};

class SerializableConsumer : public clang::ASTConsumer {
public:
  SerializableConsumer(
      clang::CompilerInstance &Compiler, llvm::StringRef InFile,
      std::unordered_map<std::string, RecordDatabase> &ChangedDatabase,
      llvm::StringRef OutDir);
  void LogWarning(const clang::Decl *Decl, const char *Format, ...);

  void LogError(const clang::Decl *Decl, const char *format, ...);

  virtual void HandleTranslationUnit(clang::ASTContext &Context);
  const clang::PrintingPolicy &getPrintingPolicy() {
    return Context->getPrintingPolicy();
  }
  const RecordInfo &getRecord(const clang::CXXRecordDecl *Decl) {
    // God damn I want to use try_emplace.
    const auto &Result = Cache.find(Decl);
    if (Result == Cache.end()) {
      return Cache.emplace(Decl, RecordInfo(*this, Decl)).first->second;
    } else {
      return Result->second;
    }
  }
  bool HasError() { return HasErrors; }

private:
  bool shouldParse();
  static std::set<std::string> HeaderExtensions;
  std::unordered_map<const clang::CXXRecordDecl *, RecordInfo> Cache;
  SerializableVisitor Visitor;
  clang::ASTContext *Context;
  bool HasErrors = false;
  clang::StringRef InFile;
  std::string DatabaseFile;
  std::string ObjFile;
  clang::DiagnosticsEngine &Diags;
  std::unordered_map<std::string, RecordDatabase> &ChangedDatabase;
  clang::StringRef OutDir;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZABLE_GENERATOR_H_
