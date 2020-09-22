#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_CODEGENERATOR_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_CODEGENERATOR_H_
#include "RecordDatabase.h"
#include "RecordInfo.h"
#include <clang/AST/AST.h>
#include <llvm/ADT/StringRef.h>
#include <string>
class SingleHeaderGenerator {
public:
  SingleHeaderGenerator(
      llvm::StringRef InFile, clang::DiagnosticsEngine &Diags,
      std::unordered_map<const clang::CXXRecordDecl *, RecordInfo> &Cache);
  ~SingleHeaderGenerator();

private:
  void recordGen(std::fstream &Out, RecordInfo& RecordInfo);
  llvm::StringRef InFile;
  std::string FileName;
  RecordDatabase Database;
  std::unordered_map<const clang::CXXRecordDecl *, RecordInfo> &Cache;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_CODEGENERATOR_H_
