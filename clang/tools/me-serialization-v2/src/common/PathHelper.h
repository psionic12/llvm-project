#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMMON_PATHHELPER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMMON_PATHHELPER_H_
#include "llvm/ADT/StringRef.h"
#include <string>

static std::string convertPathToSingleName(llvm::StringRef Path) {
  std::string FileName;
  for (auto c : Path) {
    if (c == '/' || c == '\\') {
      FileName.push_back('$');
    } else if (c == '$') {
      FileName.append("$$");
    } else {
      FileName.push_back(c);
    }
  }
  return FileName;
}
static std::string convertSingleNameToPath(llvm::StringRef FileName) {
  std::string Path;
  for (unsigned int I = 0; I < FileName.size(); I++) {
    if (FileName[I] == '$') {
      if (FileName[I + 1] == '$') {
        I += 1;
        Path.push_back('$');
      } else {
        Path.push_back('/');
      }
    } else {
      Path.push_back(FileName[I]);
    }
  }
  return Path;
}

static std::string relative(llvm::StringRef Base, llvm::StringRef Path) {
  if (Base.size() >= Path.size())
    return {};
  llvm::Optional<std::size_t> PC;
  unsigned I = 0;
  for (; I < Base.size(); I++) {
    if (Base[I] == Path[I]) {
      if (Base[I] == '\\' || Base[I] == '/') {
        PC = I;
      }
      continue;
    } else {
      break;
    }
  }
  // check if base is not end with '/'
  if (Base.back() != '\\' || Base.back() != '/') {
    if (Path[I] == '/' || Path[I] == '\\') {
      PC = I;
    }
  }
  if (PC) {
    return Path.substr(PC.getValue() + 1, Path.size() - PC.getValue()).str();
  } else {
    return {};
  }
}

#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMMON_PATHHELPER_H_
