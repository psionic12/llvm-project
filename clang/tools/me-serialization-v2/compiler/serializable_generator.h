#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZABLE_GENERATOR_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZABLE_GENERATOR_H_
#include "record_info.h"
#include "indexer.h"
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

class SerializableClassFinder
    : public clang::RecursiveASTVisitor<SerializableVisitor> {
public:
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *Declaration) {
    if (Declaration->getQualifiedNameAsString() ==
        "me::serialization::Serializable") {
      SerializableDecl = Declaration;
      // class found, do not continue
      return false;
    }
    return true;
  }
  const clang::CXXRecordDecl *getDecl() { return SerializableDecl; }

private:
  const clang::CXXRecordDecl *SerializableDecl = nullptr;
};

class SerializableConsumer : public clang::ASTConsumer {
public:
  SerializableConsumer(clang::ASTContext *Context, llvm::StringRef InFile);
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
  const clang::CXXRecordDecl *serializableDecl() { return SerializableDecl; }
  bool HasError() { return HasErrors; }

private:
  std::unordered_map<const clang::CXXRecordDecl *, RecordInfo> Cache;
  SerializableVisitor Visitor;
  SerializableClassFinder Finder;
  clang::ASTContext *Context;
  llvm::StringRef InFile;
  const clang::CXXRecordDecl *SerializableDecl = nullptr;
  bool HasErrors = false;
  EntryIndexer Indexer;
};

class SerializableGenerationAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new SerializableConsumer(&Compiler.getASTContext(), InFile));
  }
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZABLE_GENERATOR_H_
