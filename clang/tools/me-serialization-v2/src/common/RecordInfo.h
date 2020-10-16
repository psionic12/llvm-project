#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
#include "clang/AST/Type.h"
#include <me/s11n/field_coder.h>
#include <unordered_map>
class SerializableConsumer;
enum EntryKind {
  TypeBool,
  TypeChar,
  TypeUchar,
  TypeFloat,
  TypeDouble,
  TypeInt16,
  TypeUint16,
  TypeInt32,
  TypeUint32,
  TypeInt64,
  TypeUint64,
  TypeUnknown,
};

enum RepeatedKind {
  None,
  Array,
  STLVectorOrList,
  STLForwardList,
};

class EntryInfo {
public:
  EntryInfo() = default;
  EntryInfo(SerializableConsumer *Consumer, const clang::Type *Type,
            const clang::NamedDecl *NamedDecl);
  void setID(uint32_t ID) { this->EntryID = ID; }
  clang::StringRef entryName() const { return EntryName; }
  // called when this record appears in database.
  void setNotNew() { IsNew = false; }
  bool isNew() const { return IsNew; }
  void toObjFile(std::stringstream &SS) const;

private:
  friend class RecordInfo;
  template <typename T, typename Enable> friend struct me::s11n::TypeCoder;
  SerializableConsumer *Consumer;
  // 0
  std::string EntryName;
  // 1
  uint32_t EntryID = 0;
  // 2
  bool IsNew = true;
};

class RecordInfo {
public:
  RecordInfo() = default;
  RecordInfo(SerializableConsumer *Consumer,
             const clang::CXXRecordDecl *RecordDecl);
  void parseFields();
  bool isSerializable() const { return Serializable; }
  bool pure() const { return Pure; }
  bool hasID() const { return HasID; }
  void setID(uint32_t ID) {
    RecordID = ID;
    HasID = true;
  }
  uint32_t getID() const { return RecordID; }
  bool isPolymorphic() const { return Polymorphic; }
  // called when this record appears in database.
  void setNotNew() { IsNew = false; }
  bool isNew() const { return IsNew; }
  clang::StringRef fullName() const { return FullName; }
  std::unordered_map<std::string, EntryInfo> &entries() { return Entries; }
  const std::unordered_map<std::string, EntryInfo> &entries() const {
    return Entries;
  }
  std::string toObj() const;

private:
  template <typename T, typename Enable> friend struct me::s11n::TypeCoder;
  SerializableConsumer *Consumer;
  bool Serializable = false;
  bool Pure = false;
  // 0
  bool HasID = false;
  // 1
  std::string FullName;
  // 2
  std::unordered_map<std::string, EntryInfo> Entries;
  const clang::CXXRecordDecl *RecordDecl;
  // 3
  uint32_t RecordID = 0;
  // 4
  bool Polymorphic = false;
  // 5
  bool IsNew = true;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
