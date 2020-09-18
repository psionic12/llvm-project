#include "indexer.h"
#include <fstream>
#include <iostream>
#include <regex>

bool EntryIndexer::parse(std::fstream &In) {
  std::string S;
  while (In) {
    if (!parseClass(In))
      return false;
  }
  return true;
}
void EntryIndexer::refresh(std::fstream &Out) {
  for (const auto &Pair : ClassId) {
    const auto &ClassName = Pair.first;
    for (const auto &Entry : Pair.second) {
      Out << ClassName << "::" << Entry.first << "=" << Entry.second << "\n";
    }
  }
}
bool EntryIndexer::parseClass(std::fstream &In) {
  if (In.peek() == '+') {
    int i = 0;

  } else {
    return false;
  }
}
bool EntryIndexer::parseField(std::fstream &In) {}
bool EntryIndexer::parseIdentifier(std::fstream &In) {
  std::stringstream ss;
  char c = In.peek();
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
    ss << In.get();
    c = In.peek();
    while ((c >= 0 && c <= 9) || (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') || c == '_') {
      ss << In.get();
      c = In.peek();
    }
    return true;
  }
  return false;
}
