#include "serializable_generator.h"

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
bool SerializableVisitor::VisitCXXRecordDecl(
    clang::CXXRecordDecl *Declaration) {

  if (IsDeriveFrom<SerializableClassName>(Declaration)) {
    // this decl has already handled
    if (TryGetCachedRecord(Declaration))
      return true;

    RecordInfo info;

    // parse all methods
    for (const auto *method : Declaration->methods()) {
      if (method->isPure()) {
        // this is an "abstract" class, no need to generate serialization code
        // for it
        info.SetToInvalid();
        goto done_handling;
      } else if (auto *serialized_attr =
          method->getAttr<clang::MESerializedAttr>()) {
        // only support reference getter style
        if (method->param_size() > 0) {
          LogError(method, "getter cannot have parameters");
        }
        auto qual_return_type = method->getDeclaredReturnType();
        if (qual_return_type.isConstQualified()) {
          LogError(method, "the return type of getter cannot be const");
        }
        if (!qual_return_type->isReferenceType()) {
          LogError(method, "the return type of getter must be reference");
        }
      }
    }

    // parse fields
    for (auto *field : Declaration->fields()) {
      if (auto *serialized_attr = field->getAttr<clang::MESerializedAttr>()) {


      }
    }

    // parse all base classes
    for (const auto &cxx_base_specifier : Declaration->bases()) {
      if (cxx_base_specifier.isVirtual()) {
        LogWarning(Declaration, "Do not support virtual inheritance, ignore");
      } else {
        const auto *base_decl =
            Declaration->getTypeForDecl()->getAsCXXRecordDecl();
        if (const auto *record = TryGetCachedRecord(base_decl)) {
          info.AddBase(record);
          return;
        }
      }
    }
    done_handling:
    CachedRecords.try_emplace(Declaration, std::move(info));
  }
  return true;
}
EntryKind SerializableVisitor::TypeToEntryKind(const clang::Type *Type) {
  Type->getAsCXXRecordDecl();
  if (const auto *Builtin_type = Type->getAs<clang::BuiltinType>()) {
    Builtin_type->isInteger();
    switch (Builtin_type->getKind()) {
    case clang::BuiltinType::Bool:
      return TypeBool;
    case clang::BuiltinType::Char_U:
    case clang::BuiltinType::UChar:
      return TypeUchar;
    case clang::BuiltinType::Char_S:
    case clang::BuiltinType::SChar:
      return TypeChar;
    case clang::BuiltinType::Float:
      return TypeFloat;
    case clang::BuiltinType::Double:
    case clang::BuiltinType::LongDouble:
      return TypeDouble;
    case clang::BuiltinType::Short:
      return TypeInt16;
    case clang::BuiltinType::Int:
      return TypeInt32;
    case clang::BuiltinType::Long:
    case clang::BuiltinType::LongLong:
      return TypeInt64;
    case clang::BuiltinType::UShort:
      return TypeUint16;
    case clang::BuiltinType::UInt:
      return TypeUint32;
    case clang::BuiltinType::ULong:
    case clang::BuiltinType::ULongLong:
      return TypeUint64;
    default:
      LogError(Type->getAsCXXRecordDecl(), "unsported built-in type");
    }
  } else if (IsDeriveFrom<SerializableClassName>(Type->getAsCXXRecordDecl())) {
    return TypePointer;
  } else {
    LogError(Type->getAsCXXRecordDecl(), "type is not a Serializable type");
  }
}
const RecordInfo *
SerializableVisitor::TryGetCachedRecord(const clang::CXXRecordDecl *target) {
  const auto &result = CachedRecords.find(target);
  return result != CachedRecords.end() ? &result->second : nullptr;
}
