#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
#include "clang/AST/Type.h"
#include "indexer.h"
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
  void ToCpp(std::fstream& Out, EntryIndexer::EntryMap &EntryMap);
private:
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
  void ToCpp(std::fstream& Out, EntryIndexer& Indexer);
  bool isSerializable() const {
    return Serializable;
  }
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
