#include "ObjectDecoder.h"
#include "common/PostfixNames.h"
#include "common/RecordInfo.mes11n.h"
#include <fstream>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <me/s11n/utils/simple_buffer.h>
std::unordered_map<std::string, std::vector<RecordInfo>>
getObjFiles(clang::StringRef OutDir) {
  // handle all mes11n.obj files
  me::s11n::SimpleBuffer Buffer;
  std::unordered_map<std::string, std::vector<RecordInfo>> RecordMap;
  std::error_code EC;
  llvm::sys::fs::directory_iterator Begin(OutDir, EC, false);
  llvm::sys::fs::directory_iterator End;
  while (Begin != End) {
    const auto &Item = *Begin;
    if (Item.type() != llvm::sys::fs::file_type::regular_file)
      continue;
    std::string HeaderName = checkValidObjectName(Item.path());
    if (!HeaderName.empty()) {
      auto FileSize = Item.status()->getSize();
      Buffer.ReCapacity(FileSize);
      auto RecordInfos = RecordMap[HeaderName];
      std::fstream Obj(Item.path().c_str(), std::ios::in);
      Obj.read((char *)Buffer.Data(), FileSize);
      const uint8_t *Ptr = Buffer.Data();
      while (Ptr < Buffer.End()) {
        RecordInfos.emplace_back();
        me::s11n::Decode(RecordInfos.back(), Ptr);
      }
    }
  }
  return RecordMap;
}
std::string checkValidObjectName(clang::StringRef FileName) {
  std::string s;
  auto FileNameSize = FileName.size();
  constexpr auto PostfixSize = sizeof(OBJECT_POSTFIX);
  if (FileNameSize < PostfixSize)
    return s;
  auto Offsite = FileNameSize - PostfixSize + 1;
  const auto *const Ptr = FileName.data() + Offsite;
  if (memcmp(Ptr, OBJECT_POSTFIX, PostfixSize - 1) != 0) {
    return s;
  } else {
    s = FileName.str();
    s = s.substr(0, Offsite);
    s += ".h";
    return s;
  }
}
