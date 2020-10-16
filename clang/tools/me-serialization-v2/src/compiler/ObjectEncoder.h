#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMPILER_OBJECTENCODER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMPILER_OBJECTENCODER_H_
#include "../common/RecordInfo.mes11n.h"
struct ObjectEncoder {
  void Encode(llvm::StringRef OutDir, llvm::StringRef FullPath,
              llvm::StringRef RelativePath,
              std::unordered_map<std::string, RecordInfo &> Records) {

  }
  uint8_t *EncodeStringRef(llvm::StringRef StringRef, uint8_t* Ptr) {
    Ptr = me::s11n::Encode(StringRef.size(), Ptr);
    std::copy(StringRef.begin(), StringRef.end(), Ptr);
    return ptr + StringRef.size();
  }
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMPILER_OBJECTENCODER_H_
