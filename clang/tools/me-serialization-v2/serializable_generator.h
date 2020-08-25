#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZABLE_GENERATOR_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZABLE_GENERATOR_H_
#include "record_info.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
class SerializableVisitor
    : public clang::RecursiveASTVisitor<SerializableVisitor> {
public:
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *Declaration);
};

class SerializableConsumer : public clang::ASTConsumer {
public:
  SerializableConsumer(clang::ASTContext *Context, llvm::StringRef InFile)
      : Context(Context), InFile(InFile) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
  class SerializableClassName {
  public:
    static constexpr const char name[] = "me::serialization::Serializable";
  };
  class SerializableListClassName {
  public:
    static constexpr const char name[] = "me::serialization::SerializableList";
  };
  template <typename T> bool isType(const clang::CXXRecordDecl *RecordDecl) {
    static const clang::CXXRecordDecl *Target = nullptr;
    if (Target != nullptr) {
      return RecordDecl == Target;
    } else {
      if (RecordDecl->getQualifiedNameAsString() == T::name) {
        Target = RecordDecl;
        return true;
      } else {
        return false;
      }
    }
  }
  template <typename T>
  bool isDeriveFrom(const clang::CXXRecordDecl *RecordDecl) {
    if (!RecordDecl) return false;
    if (isType<T>(RecordDecl))
      return true;
    else {
      for (const auto &Base : RecordDecl->bases()) {
        if (const auto *BaseDecl =
                Base.getType().getTypePtr()->getAsCXXRecordDecl()) {
          isDeriveFrom<T>(BaseDecl);
        } else {
          return true;
        }
      }
    }
  }
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

private:
  std::unordered_map<const clang::CXXRecordDecl *, RecordInfo> Cache;
  SerializableVisitor Visitor;
  clang::ASTContext *Context;
  llvm::StringRef InFile;
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
