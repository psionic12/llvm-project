#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_INDEXER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_INDEXER_H_
#include <map>
#include <set>
#include <unordered_map>
class EntryIndexer {
public:
  bool parse(std::fstream &In);
  void refresh(std::fstream &Out);

  class FullNameMap {
  public:
    void emplace(const std::string &Str, unsigned int Index) {
      StrToIndex.emplace(Str, Index);
      if (Index > Max)
        Max = Index;
    }
    unsigned int operator[](const std::string &Str) {
      auto pair = StrToIndex.emplace(Str, Max + 1);
      if (pair.second) {
        Max += 1;
      }
      return pair.first->second;
    }

  private:
    std::unordered_map<std::string, unsigned int> StrToIndex;
    unsigned int Max = 0;
  };

private:
  FullNameMap Classes;
  std::unordered_map<unsigned int, FullNameMap> ClassesToFields;
  bool parseClass(std::fstream &In);
  bool parseField(std::fstream &In);
  bool parseIdentifier(std::fstream &In);
  bool parseColonColon(std::fstream &In);
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_INDEXER_H_
