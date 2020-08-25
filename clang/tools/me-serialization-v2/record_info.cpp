#include "record_info.h"

#include "serializable_generator.h"
#include "clang/AST/Attr.h"
#include <cstdarg>
void LogWarning(const clang::Decl *Decl, const char *Format, ...) {
  const auto &source_manager = Decl->getASTContext().getSourceManager();
  Decl->getLocation().dump(source_manager);
  va_list Argptr;
  va_start(Argptr, Format);
  vfprintf(stderr, Format, Argptr);
  va_end(Argptr);
}

void LogError(const clang::Decl *Decl, const char *format, ...) {
  va_list Argptr;
  va_start(Argptr, format);
  LogWarning(Decl, format, Argptr);
  va_end(Argptr);
  std::terminate();
}
EntryInfo::EntryInfo(SerializableConsumer &Consumer, const clang::Type *Type,
                     clang::StringRef Name)
    : Consumer(Consumer), EntryName(Name) {
  if (Consumer.isDeriveFrom<SerializableConsumer::SerializableListClassName>(
      Type->getAsCXXRecordDecl())) {
    Repeated = true;
    
  }
  if (const auto *BuiltinType = Type->getAs<clang::BuiltinType>()) {
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
        LogError(Type->getAsCXXRecordDecl(),
                 "unsupported built-in type : %s", TypeName.data());
    }
  } else if (Consumer.isDeriveFrom<SerializableConsumer::SerializableClassName>(
      Type->getAsCXXRecordDecl())) {
    Kind = TypePointer;}
  else {
    LogError(Type->getAsCXXRecordDecl(), "type is not a Serializable type");
  }
}
RecordInfo::RecordInfo(SerializableConsumer &Consumer,
                       const clang::CXXRecordDecl *RecordDecl)
    : Consumer(Consumer) {
  if (Consumer.isDeriveFrom<SerializableConsumer::SerializableClassName>(
          RecordDecl)) {
    Serializable = true;
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
          LogError(Method, "getter cannot have parameters");
        }
        auto QualReturnType = Method->getDeclaredReturnType();
        if (QualReturnType.isConstQualified()) {
          LogError(Method, "the return type of getter cannot be const");
        }
        if (!QualReturnType->isReferenceType()) {
          LogError(Method, "the return type of getter must be reference");
        }

        Entries.emplace_back(Consumer, QualReturnType.getTypePtr(),
                        Method->getName());
      }
    }

    // parse fields
    for (auto *field : RecordDecl->fields()) {
      if (auto *serialized_attr = field->getAttr<clang::MESerializedAttr>()) {
      }
    }

    // parse all base classes
    for (const auto &cxx_base_specifier : RecordDecl->bases()) {
      if (cxx_base_specifier.isVirtual()) {
        LogWarning(RecordDecl, "Do not support virtual inheritance, ignore");
      } else {
        const auto *base_decl =
            RecordDecl->getTypeForDecl()->getAsCXXRecordDecl();
        if(Consumer.)
      }
    }
  }
}
