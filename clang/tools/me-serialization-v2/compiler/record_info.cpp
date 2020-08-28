#include "record_info.h"

#include "serializable_generator.h"
#include "clang/AST/Attr.h"
#include <fstream>

class ParseErrorException {};
EntryInfo::EntryInfo(SerializableConsumer &Consumer, const clang::Type *Type,
                     const clang::NamedDecl *NamedDecl)
    : Consumer(Consumer), EntryName(NamedDecl->getName()) {
  const auto *ElementType = Type;
  // check if Type is vector<T>
  if (const auto *TST =
          ElementType->getAs<clang::TemplateSpecializationType>()) {
    clang::StringRef TemlateName = TST->getTemplateName()
                                       .getAsQualifiedTemplateName()
                                       ->getDecl()
                                       ->getName();
    if (TemlateName == "std::vector" || TemlateName == "std::list" ||
        TemlateName == "std::forward_list") {
      // template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
      const clang::TemplateArgument &Arg = TST->getArg(0);
      ElementType = Arg.getAsType().getTypePtr();
      if (TemlateName == "std::vector" || TemlateName == "std::list")
        Repeated = STLVectorOrList;
      else if (TemlateName == "std::forward_list")
        Repeated = STLForwardList;
    }
  }

  // check if Type is vector<unique_ptr<T>>
  if (const auto *TST =
          ElementType->getAs<clang::TemplateSpecializationType>()) {
    clang::StringRef TemlateName = TST->getTemplateName()
                                       .getAsQualifiedTemplateName()
                                       ->getDecl()
                                       ->getName();
    if (TemlateName == "std::unique_ptr" || TemlateName == "std::shared_ptr") {
      // template<typename _Tp, (typename _Alloc = std::allocator<_Tp>)? >
      const clang::TemplateArgument &Arg = TST->getArg(0);
      ElementType = Arg.getAsType().getTypePtr();
    }
  }

  if (const auto *BuiltinType = ElementType->getAs<clang::BuiltinType>()) {
    BuiltinType->isInteger();
    TypeName = BuiltinType->getName(Consumer.getPrintingPolicy());
    switch (BuiltinType->getKind()) {
    case clang::BuiltinType::Bool:
      Kind = TypeBool;
      break;
    case clang::BuiltinType::Char_U:
    case clang::BuiltinType::UChar:
      Kind = TypeUchar;
      break;
    case clang::BuiltinType::Char_S:
    case clang::BuiltinType::SChar:
      Kind = TypeChar;
      break;
    case clang::BuiltinType::Float:
      Kind = TypeFloat;
      break;
    case clang::BuiltinType::Double:
    case clang::BuiltinType::LongDouble:
      Kind = TypeDouble;
      break;
    case clang::BuiltinType::Short:
      Kind = TypeInt16;
      break;
    case clang::BuiltinType::Int:
      Kind = TypeInt32;
      break;
    case clang::BuiltinType::Long:
    case clang::BuiltinType::LongLong:
      Kind = TypeInt64;
      break;
    case clang::BuiltinType::UShort:
      Kind = TypeUint16;
      break;
    case clang::BuiltinType::UInt:
      Kind = TypeUint32;
      break;
    case clang::BuiltinType::ULong:
    case clang::BuiltinType::ULongLong:
      Kind = TypeUint64;
      break;
    default:
      Consumer.LogError(NamedDecl, "unsupported built-in type : %s",
                        TypeName.data());
      Kind = TypeUnknown;
    }
  } else {
    const auto *RecordDecl = ElementType->getAsCXXRecordDecl();
    if (RecordDecl && Consumer.getRecord(RecordDecl).isSerializable()) {
      Kind = TypePointer;
      TypeName = RecordDecl->getName();
    } else {
      Kind = TypeUnknown;
      Consumer.LogError(ElementType->getAsCXXRecordDecl(),
                        "type is not a Serializable type");
    }
  }
}
RecordInfo::RecordInfo(SerializableConsumer &Consumer,
                       const clang::CXXRecordDecl *RecordDecl)
    : Consumer(Consumer), RecordDecl(RecordDecl) {
  FullName = RecordDecl->getQualifiedNameAsString();
  if (RecordDecl == Consumer.serializableDecl()) {
    Serializable = true;
  }

  // parse all base classes
  for (const auto &CXXBaseSpecifier : RecordDecl->bases()) {
    if (CXXBaseSpecifier.isVirtual()) {
      Consumer.LogWarning(RecordDecl,
                          "Do not support virtual inheritance, ignore");
    } else {
      const auto *BaseDecl = RecordDecl->getTypeForDecl()->getAsCXXRecordDecl();
      const RecordInfo &BaseInfo = Consumer.getRecord(BaseDecl);
      if (BaseInfo.Serializable)
        Serializable = true;
    }
  }
}
void RecordInfo::ParseFields() {
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
        Consumer.LogError(Method, "getter cannot have parameters");
      }
      auto QualReturnType = Method->getDeclaredReturnType();
      if (QualReturnType.isConstQualified()) {
        Consumer.LogError(Method, "the return type of getter cannot be const");
      }
      if (!QualReturnType->isReferenceType()) {
        Consumer.LogError(Method,
                          "the return type of getter must be reference");
      }
      if (Consumer.HasError())
        return;
      EntryInfo Entry(Consumer, QualReturnType.getTypePtr(), Method);
      for (const auto &Category : SerializedAttr->categories()) {
        Entry.AddCategory(Category);
      }
      Entries.push_back(std::move(Entry));
    }
  }

  // parse fields
  for (auto *Field : RecordDecl->fields()) {
    if (auto *SerializedAttr = Field->getAttr<clang::MESerializedAttr>()) {
      const auto FieldType = Field->getType();
      if (FieldType.isConstQualified()) {
        Consumer.LogError(Field, "serialized type cannot be const");
      }
      if (FieldType->isReferenceType()) {
        Consumer.LogError(Field, "serialized type cannot be a reference type");
      }
      if (FieldType->isPointerType()) {
        Consumer.LogError(Field, "serialized type cannot be a pointer type");
      }
      if (Consumer.HasError())
        return;
      EntryInfo Entry(Consumer, FieldType.getTypePtr(), Field);
      for (const auto &Category : SerializedAttr->categories()) {
        Entry.AddCategory(Category);
      }
      Entries.push_back(std::move(Entry));
    }
  }
}
void EntryInfo::ToCpp(std::fstream &Out, EntryIndexer::EntryMap &EntryMap) {
  // get the index number
  auto index = EntryMap[EntryName.str()];
  if (index == 0) {
    // no index, add new one
    index = EntryMap.size() + 1;
    EntryMap[EntryName.str()] = index;
  }

  Out << "// " << (Repeated ? "repeated" : "") << TypeName.str() << " "
      << EntryName.str() << " = " << index << "\n";
  if (!Repeated) {
    switch (Kind) {
    case TypeFloat:
    case TypeDouble:
      Out << "if (" << EntryName.str() << " < 0 || " << EntryName.str()
          << " > 0) {\n";
      break;
    default:
      Out << "if (" << EntryName.str() << " != 0) {\n";
    }
  } else {
    Out << "if (" << EntryName.str() << ".size() > 0) {\n";
  }

  Out << "}\n";
}
void RecordInfo::ToCpp(std::fstream &Out, EntryIndexer &Indexer) {
  if (!isSerializable())
    return;
  if (Pure)
    return;

  Out << "std::vector<char> " << FullName << "::Serialize() {\n";
  auto &EntryMap = Indexer.getEntryMap(FullName);
  for (auto &Entry : Entries) {
    Entry.ToCpp(Out, EntryMap);
  }
  Out << "}\n";
}
