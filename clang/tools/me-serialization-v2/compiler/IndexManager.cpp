#include "IndexManager.h"
bool IndexManager::emplace(const std::string &Str, unsigned int Index) {
  const auto &Pair = StrToIndex.emplace(Str, Index);
  bool success = Pair.second;
  llvm::StringRef Name = Pair.first->first;
  OrderedMap.emplace(Index, Name);
  if (Index > Max)
    Max = Index;
  return success;
}
unsigned int IndexManager::operator[](const std::string &Str) {
  auto pair = StrToIndex.emplace(Str, Max + 1);
  if (pair.second) {
    Max += 1;
  }
  return pair.first->second;
}