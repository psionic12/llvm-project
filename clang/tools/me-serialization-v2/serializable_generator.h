#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZABLE_GENERATOR_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZABLE_GENERATOR_H_
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
class SerializableVisitor
    : public clang::RecursiveASTVisitor<SerializableVisitor> {
public:
  explicit SerializableVisitor(clang::ASTContext *Context,
                               llvm::StringRef InFile)
      : Context(Context), InFile(InFile) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *Declaration);

private:
  static constexpr char kSerializable[] = "me::serialization::Serializable";
  static constexpr char kSerializableList[] =
      "me::serialization::SerializableList";
  const clang::ASTContext *Context;
  const llvm::StringRef InFile;
  const clang::CXXRecordDecl *SerializableDecl = nullptr;
  const clang::CXXRecordDecl *SerializableListDecl = nullptr;
  template <char T[]> bool IsType(const clang::CXXRecordDecl *RecordDecl) {
    static const clang::CXXRecordDecl *Target = nullptr;
    if (Target != nullptr) {
      return RecordDecl == Target;
    } else {
      if (RecordDecl->getQualifiedNameAsString() == T) {
        Target = RecordDecl;
        return true;
      } else {
        return false;
      }
    }
  }
  template <char T[]>
  bool IsDeriveFrom(const clang::CXXRecordDecl *RecordDecl) {
    if (IsType<T>())
      return true;
    else {
      for (const auto &Base : RecordDecl->bases()) {
        if (const auto *BaseDecl =
                Base.getType().getTypePtr()->getAsRecordDecl()) {
          IsDeriveFrom<T>(BaseDecl);
        } else {
          return true;
        }
      }
    }
  }
};

class SerializableConsumer : public clang::ASTConsumer {
public:
  explicit SerializableConsumer(clang::ASTContext *Context,
                                llvm::StringRef InFile)
      : Visitor(Context, InFile) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  SerializableVisitor Visitor;
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
