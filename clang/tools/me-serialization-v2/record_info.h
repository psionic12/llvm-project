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
  TypePointer,
};
class EntryInfo {
public:
  EntryInfo(SerializableConsumer& Consumer, const clang::Type* Type, clang::StringRef Name);
  void AddCategory(clang::StringRef category) {
    Categories.emplace_back(category);
  }
private:
  SerializableConsumer& Consumer;
  clang::StringRef TypeName;
  clang::StringRef EntryName;
  std::vector<clang::StringRef> Categories;
  bool Repeated = false;
  EntryKind Kind;
};

class RecordInfo {
public:
  RecordInfo(SerializableConsumer& Consumer, const clang::CXXRecordDecl* RecordDecl);
  void AddBase(const RecordInfo *Base) { Bases.push_back(Base); }
private:
  SerializableConsumer& Consumer;
  bool Serializable = false;
  bool Pure = false;
  std::vector<const RecordInfo *> Bases;
  std::vector<EntryInfo> Entries;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_RECORD_INFO_H_
