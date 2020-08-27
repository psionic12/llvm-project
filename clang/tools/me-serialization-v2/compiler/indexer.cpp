#include "indexer.h"
#include <fstream>
#include <iostream>
#include <regex>

void EntryIndexer::parse(std::fstream &In) {
  std::string S;
  std::regex Regex(
      "([0-9a-zA-Z_](::[0-9a-zA-Z_])*)(::[0-9a-zA-Z_])=([1-9][0-9]*)");
  std::smatch Matches;
  while (std::getline(In, S)) {
    if (std::regex_search(S, Matches, Regex)) {
      const std::string &ClassName = Matches[1];
      const std::string &FieldName = Matches[3];
      const unsigned int Index = std::stoi(Matches[4]);
      RecordMap[ClassName][FieldName] = Index;
    }
  }
}
void EntryIndexer::refresh(std::fstream &Out) {
  for (const auto &Pair : RecordMap) {
    const auto &ClassName = Pair.first;
    for (const auto &Entry : Pair.second) {
      Out << ClassName << "::" << Entry.first << "=" << Entry.second << "\n";
    }
  }
}
