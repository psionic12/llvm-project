#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_INDEXER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_INDEXER_H_
#include <unordered_map>
class EntryIndexer {
public:
  typedef std::unordered_map<std::string, unsigned int> EntryMap;
  void parse(std::fstream &In);
  void refresh(std::fstream &Out);
  EntryMap &getEntryMap(const std::string &ClassName) {
    return RecordMap[ClassName];
  }
private:
  std::unordered_map<std::string, EntryMap> RecordMap;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_INDEXER_H_
