#include "RecordInfo.h"
#include "CodeGuard.h"
#include "SerializableConsumer.h"
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
                       const clang::CXXRecordDecl *RecordDecl,
                       clang::StringRef IncludeFile)
    : Consumer(Consumer), RecordDecl(RecordDecl), IncludeFile(IncludeFile) {
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
std::string RecordInfo::toObj() const {
  std::string String;
  std::size_t Size = 0;
  Size += me::s11n::SizeRaw(HasID);
  Size += me::s11n::SizeRaw(FullName);
  Size += me::s11n::SizeRaw(Entries);
  Size += me::s11n::SizeRaw(RecordID);
  Size += me::s11n::SizeRaw(Polymorphic);
  Size += me::s11n::SizeRaw(IsNew);
  Size += me::s11n::SizeRaw(IncludeFile);
  Size += me::s11n::SizeRaw(Size);

  String.resize(Size);
  uint8_t *Ptr = reinterpret_cast<uint8_t *>(&String[0]);
  Ptr = me::s11n::WriteRaw(Size, Ptr);
  Ptr = me::s11n::WriteField(0, HasID, Ptr);
  Ptr = me::s11n::WriteField(1, FullName, Ptr);
  Ptr = me::s11n::WriteField(2, Entries, Ptr);
  Ptr = me::s11n::WriteField(3, RecordID, Ptr);
  Ptr = me::s11n::WriteField(4, Polymorphic, Ptr);
  Ptr = me::s11n::WriteField(5, IsNew, Ptr);
  Ptr = me::s11n::WriteField(6, IncludeFile, Ptr);
  return String;
}
RecordInfo::RecordInfo(const uint8_t *Ptr) {
  std::size_t Size;
  const uint8_t *Begin = Ptr;
  Ptr = me::s11n::ReadRaw(Size, Ptr);
  const uint8_t *End = Begin + Size;
  while (Ptr < End) {
    uint32_t Index;
    Ptr = me::s11n::ReadRaw(Index, Ptr);
    uint8_t tag;
    Ptr = me::s11n::ReadRaw(tag, Ptr);
    switch (Index) {
    case 0:
      Ptr = me::s11n::ReadRaw(HasID, Ptr);
    case 1:
      Ptr = me::s11n::ReadRaw(FullName, Ptr);
      break;
    case 2:
      Ptr = me::s11n::ReadRaw(Entries, Ptr);
      break;
    case 3:
      Ptr = me::s11n::ReadRaw(RecordID, Ptr);
      break;
    case 4:
      Ptr = me::s11n::ReadRaw(Polymorphic, Ptr);
      break;
    case 5:
      Ptr = me::s11n::ReadRaw(IsNew, Ptr);
      break;
    case 6:
      Ptr = me::s11n::ReadRaw(IncludeFile, Ptr);
      break;
    default: {
      Ptr = me::s11n::SkipUnknown(tag, Ptr);
    }
    }
  }
}
