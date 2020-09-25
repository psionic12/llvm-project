#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_INDEXMANAGER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_INDEXMANAGER_H_
#include <llvm/ADT/StringRef.h>
#include <map>
#include <string>
#include <unordered_map>
class IndexManager {
public:
  bool emplace(const std::string &Str, unsigned int Index);
  unsigned int operator[](const std::string &Str);
  void clear() {
    StrToIndex.clear();
    Max = 0;
  }
  std::map<unsigned int, llvm::StringRef> &GetOrdered() { return OrderedMap; }

private:
  std::unordered_map<std::string, unsigned int> StrToIndex;
  std::map<unsigned int, llvm::StringRef> OrderedMap;
  unsigned int Max = 0;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_INDEXMANAGER_H_
