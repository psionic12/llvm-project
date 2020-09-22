#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
#include "clang/AST/Type.h"
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
  EntryInfo(SerializableConsumer &Consumer, const clang::Type *Type,
            const clang::NamedDecl *NamedDecl);
  void AddCategory(clang::StringRef category) {
    Categories.emplace_back(category);
  }
  clang::StringRef entryName() {return EntryName;}

private:
  friend class RecordInfo;
  SerializableConsumer &Consumer;
  clang::StringRef TypeName;
  clang::StringRef EntryName;
  std::vector<clang::StringRef> Categories;
  RepeatedKind Repeated = None;
  EntryKind Kind;
};

class RecordInfo {
public:
  RecordInfo(SerializableConsumer &Consumer,
             const clang::CXXRecordDecl *RecordDecl);
  void ParseFields();
  bool isSerializable() const { return Serializable; }
  bool pure() { return Pure; }
  clang::StringRef fullName() { return FullName; }
  std::vector<EntryInfo> &entries() { return Entries; }

private:
  SerializableConsumer &Consumer;
  bool Serializable = false;
  bool Pure = false;
  std::string FullName;
  std::vector<const RecordInfo *> Bases;
  std::vector<EntryInfo> Entries;
  const clang::CXXRecordDecl *RecordDecl;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
