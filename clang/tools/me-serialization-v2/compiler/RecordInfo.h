#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
#include "clang/AST/Type.h"
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
  EntryInfo(SerializableConsumer *Consumer, const clang::Type *Type,
            const clang::NamedDecl *NamedDecl);
  void addCategory(clang::StringRef category) {
    Categories.emplace_back(category);
  }
  void setID(uint32_t ID) { this->RecordID = ID; }
  clang::StringRef entryName() { return EntryName; }
  // called when this record appears in database.
  void setNotNew() { IsNew = false; }
  bool isNew() { return IsNew; }
  void toObjFile(std::stringstream &SS);

private:
  friend class RecordInfo;
  SerializableConsumer *Consumer;
  std::string EntryName;
  std::vector<std::string> Categories;
  uint32_t RecordID = 0;
  bool IsNew = true;
};

class RecordInfo {
public:
  RecordInfo(SerializableConsumer *Consumer,
             const clang::CXXRecordDecl *RecordDecl);
  void parseFields();
  bool isSerializable() const { return Serializable; }
  bool pure() { return Pure; }
  void setID(uint32_t ID) { RecordID = ID; }
  bool isPolymorphic() { return Polymorphic; }
  // called when this record appears in database.
  void setNotNew() { IsNew = false; }
  bool isNew() { return IsNew; }
  clang::StringRef fullName() { return FullName; }
  std::unordered_map<std::string, EntryInfo> &entries() { return Entries; }
  std::string toObjFile();

private:
  SerializableConsumer *Consumer;
  bool Serializable = false;
  bool Pure = false;
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
