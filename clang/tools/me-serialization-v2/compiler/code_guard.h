#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_CODEGUARD_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_CODEGUARD_H_
#include <fstream>
#include <llvm/ADT/StringRef.h>
#include "record_info.h"
class CodeGuard {
public:
  CodeGuard(std::fstream &Out) : Out(Out) {}

protected:
  std::fstream &Out;
};

class HeaderGuardCoder : CodeGuard {
public:
  HeaderGuardCoder(std::fstream &Out, llvm::StringRef InFile);
  ~HeaderGuardCoder();

private:
  std::string Str;
};

class IncludeCoder : CodeGuard {
public:
  IncludeCoder(std::fstream &Out, llvm::StringRef Infile);
};

class NamespaceCoder : CodeGuard {
public:
  NamespaceCoder(std::fstream &Out, llvm::StringRef Namespace);
  ~NamespaceCoder();

private:
  llvm::StringRef Namespace;
};

class RecordCoder : CodeGuard {
  RecordCoder(std::fstream &Out, RecordInfo& RecordInfo);
};

#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_CODEGUARD_H_
