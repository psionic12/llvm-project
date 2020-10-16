#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_LINKER_OBJECTDECODER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_LINKER_OBJECTDECODER_H_
#include "common/RecordInfo.h"
std::unordered_map<std::string, std::vector<RecordInfo>>
getObjFiles(clang::StringRef OutDir);

std::string checkValidObjectName(clang::StringRef FileName);
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_LINKER_OBJECTDECODER_H_
