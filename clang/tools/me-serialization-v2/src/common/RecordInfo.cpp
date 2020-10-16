#include "RecordInfo.h"
#include "../compiler/SerializableConsumer.h"
#include "CodeGuard.h"
#include "me/s11n/field_coder.h"
#include "clang/AST/Attr.h"
#include <fmt/core.h>
#include <fstream>

class ParseErrorException {};
EntryInfo::EntryInfo(SerializableConsumer *Consumer, const clang::Type *Type,
                     const clang::NamedDecl *NamedDecl)
    : Consumer(Consumer), EntryName(NamedDecl->getName()) {}
void EntryInfo::toObjFile(std::stringstream &SS) const {}
RecordInfo::RecordInfo(SerializableConsumer *Consumer,
                       const clang::CXXRecordDecl *RecordDecl)
    : Consumer(Consumer), RecordDecl(RecordDecl) {
  FullName = RecordDecl->getQualifiedNameAsString();

  // parse all base classes
  for (const auto &CXXBaseSpecifier : RecordDecl->bases()) {
    if (CXXBaseSpecifier.isVirtual()) {
      Consumer->LogWarning(RecordDecl,
                           "Do not support virtual inheritance, ignore");
      return;
    } else {
      const auto *BaseDecl = RecordDecl->getTypeForDecl()->getAsCXXRecordDecl();
      const RecordInfo &BaseInfo = Consumer->getRecord(BaseDecl);
      if (BaseInfo.Serializable) {
        Serializable = true;
        return;
      }
    }
  }
  for (const auto *Method : RecordDecl->methods()) {
    if (Method->getAttr<clang::MESerializedAttr>()) {
      Serializable = true;
      return;
    }
  }
  for (auto *Field : RecordDecl->fields()) {
    if (Field->getAttr<clang::MESerializedAttr>()) {
      Serializable = true;
      return;
    }
  }
}
void RecordInfo::parseFields() {
  if (!Consumer)
    return;

  // not a serializable, not need further parsing.
  if (!Serializable)
    return;

  // parse all methods
  for (const auto *Method : RecordDecl->methods()) {
    if (Method->isPure()) {
      // this is an "abstract" class, no need to generate serialization code
      // for it
      Pure = true;
    }
    if (auto *SerializedAttr = Method->getAttr<clang::MESerializedAttr>()) {
      // only support reference getter style
      if (Method->param_size() > 0) {
        Consumer->LogError(Method, "getter cannot have parameters");
      }
      auto QualReturnType = Method->getDeclaredReturnType();
      if (QualReturnType.isConstQualified()) {
        Consumer->LogError(Method, "the return type of getter cannot be const");
      }
      if (!QualReturnType->isReferenceType()) {
        Consumer->LogError(Method,
                           "the return type of getter must be reference");
      }
      if (Consumer->HasError())
        return;
      EntryInfo Entry(Consumer, QualReturnType.getTypePtr(), Method);
      for (const auto &Category : SerializedAttr->categories()) {
        Entry.addCategory(Category);
      }
      Entries.emplace(Entry.EntryName, std::move(Entry));
    }
  }

  // parse fields
  for (auto *Field : RecordDecl->fields()) {
    if (auto *SerializedAttr = Field->getAttr<clang::MESerializedAttr>()) {
      const auto FieldType = Field->getType();
      if (FieldType.isConstQualified()) {
        Consumer->LogError(Field, "serialized type cannot be const");
      }
      if (FieldType->isReferenceType()) {
        Consumer->LogError(Field, "serialized type cannot be a reference type");
      }
      if (FieldType->isPointerType()) {
        Consumer->LogError(Field, "serialized type cannot be a pointer type");
      }
      if (Consumer->HasError())
        return;
      EntryInfo Entry(Consumer, FieldType.getTypePtr(), Field);
      for (const auto &Category : SerializedAttr->categories()) {
        Entry.addCategory(Category);
      }
      Entries.emplace(Entry.EntryName, std::move(Entry));
    }
  }
}
