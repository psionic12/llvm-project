#include <fstream>
#include <iostream>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <google/protobuf/text_format.h>
#include <llvm/Support/CommandLine.h>

#include "me_attributes.h"
#include "record_info.pb.h"

llvm::cl::OptionCategory serializable_category("serialization options");
llvm::cl::extrahelp
    common_help(clang::tooling::CommonOptionsParser::HelpMessage);
llvm::cl::opt<std::string>
    builtin_includes("builtin",
                     llvm::cl::desc("path of Clang builtin includes"),
                     llvm::cl::cat(serializable_category));
llvm::cl::opt<std::string> out_dir("o", llvm::cl::desc("Specify output dir"),
                                   llvm::cl::value_desc("dir"),
                                   llvm::cl::init("./"),
                                   llvm::cl::cat(serializable_category));
llvm::cl::opt<bool>
    readable_class_info("readable",
                        llvm::cl::desc("generate readable class info"));
clang::ast_matchers::DeclarationMatcher record_matcher =
    clang::ast_matchers::cxxRecordDecl(clang::ast_matchers::hasDefinition())
        .bind("Record");

const std::unordered_map<std::string, Category> category_map{
    {PREFAB, Category::Prefab},
    {SAVE, Category::Save},
};

const std::unordered_map<std::string, ProtoKind> str_to_kind_map{
    {"double", ProtoKind::TypeDouble},
    {"float", ProtoKind::TypeFloat},
    {"int64", ProtoKind::TypeInt64},
    {"uint64", ProtoKind::TypeUint64},
    {"int32", ProtoKind::TypeInt32},
    {"fixed64", ProtoKind::TypeFixed64},
    {"fixed32", ProtoKind::TypeFixed32},
    {"bool", ProtoKind::TypeBool},
    {"string", ProtoKind::TypeString},
    {"bytes", ProtoKind::TypeBytes},
    {"uint32", ProtoKind::TypeUint32},
    {"sfixed32", ProtoKind::TypeSfixed32},
    {"sfixed64", ProtoKind::TypeSfixed64},
    {"sint32", ProtoKind::TypeSint32},
    {"sint64", ProtoKind::TypeSint64},
};

void TerminateIf(bool condition, const char *err, const clang::Decl* decl) {
  if (condition) {
    const auto &source_manager = decl->getASTContext().getSourceManager();
    decl->getLocation().dump(source_manager);
    std::cerr << err << std::endl;
    std::terminate();
  }
}


ProtoKind StringToKind(const std::string &str, const clang::Decl* decl) {
  const auto &it = str_to_kind_map.find(str);
  TerminateIf(it == str_to_kind_map.end(), (std::string("alias type unknown: ") + str).c_str() , decl);
  return it->second;
}

bool IsSerializedField(const clang::FieldDecl *field) {
  if (auto *category_attr = field->getAttr<clang::MECategoryAttr>()) {
    for (const auto &category : category_attr->categories()) {
      if (category == PREFAB || category == SAVE) {
        return true;
      } else {
        return false;
      }
    }
  }
  return false;
}

std::string GetFullName(const clang::CXXRecordDecl *record) {
  std::string nd_str;
  for (const clang::DeclContext *dc = record->getParent(); dc;
       dc = dc->getParent()) {
    if (const auto *nd = llvm::dyn_cast<clang::NamespaceDecl>(dc)) {
      nd_str += nd->getNameAsString();
      nd_str += "_";
    } else {
      break;
    }
  }
  return nd_str += record->getNameAsString();
}

void CheckRecord(const clang::CXXRecordDecl *record) {
  TerminateIf(record->getNumVBases() > 0,
              "serialization tool currently do not support virtual-inheritance",
              record);
  TerminateIf(record->getNumBases() > 1,
              "serialization tool currently do not support multi-inheritance",
              record);
}

void CheckField(const clang::FieldDecl *field) {
  auto type = field->getType();
  TerminateIf(type.isConstQualified(), "serialized type cannot be const",
              field);
  TerminateIf((type->isPointerType() || type->isReferenceType()),
              "serialized type cannot be a pointer or reference", field);
}

ProtoKind GetProtoKind(const clang::BuiltinType *builtin_type) {
  switch (builtin_type->getKind()) {
  case clang::BuiltinType::Kind::Bool: {
    return ProtoKind::TypeBool;
  }
  case clang::BuiltinType::Kind::Int: {
    return ProtoKind::TypeInt32;
  }
  case clang::BuiltinType::Kind::UInt: {
    return ProtoKind::TypeUint32;
  }
  case clang::BuiltinType::Kind::Float: {
    return ProtoKind::TypeFloat;
  }
  case clang::BuiltinType::Kind::Double: {
    return ProtoKind::TypeDouble;
  }
  default:
    std::cerr << "unsupported type: " << builtin_type->getTypeClassName();
    std::terminate();
  }
}

bool ParseRecordDetails(const clang::CXXRecordDecl *record,
                        ProtoFileInfo &proto_file_info) {
  // parse parents
  bool has_serializable_field = false;
  CheckRecord(record);
  if (record->getNumBases() == 1) {
    auto *parent = record->bases_begin()->getType()->getAsCXXRecordDecl();
    has_serializable_field |= ParseRecordDetails(parent, proto_file_info);
    proto_file_info.add_ancestors(GetFullName(parent));
  }

  // parse fields
  for (auto *field : record->fields()) {
    CheckField(field);
    if (auto *category_attr = field->getAttr<clang::MECategoryAttr>()) {
      has_serializable_field |= true;

      // create a field
      auto *proto_field = proto_file_info.add_fields();
      proto_field->set_name(field->getNameAsString());

      // add all categories
      for (const auto &category_str_ref : category_attr->categories()) {
        const auto &category = category_map.find(category_str_ref.str());
        if (category != category_map.end()) {
          proto_field->add_categories(category->second);
        }
      }

      auto *alias_attr = field->getAttr<clang::MEAliasAttr>();
      auto *list_attr = field->getAttr<clang::MEListAttr>();
      TerminateIf((alias_attr && list_attr),
                  "cannot mark a field both as alias and as list", field);
      std::string field_type_name;
      if(auto* r = field->getType().getTypePtr()->getAsCXXRecordDecl()) {
        field_type_name = GetFullName(r);
      } else {
        field_type_name = field->getType().getAsString();
      }

      // check if the field is an alias
      if (alias_attr) {
        auto *alias_field = proto_field->mutable_alias_field();
        const auto &alias_type_name = alias_attr->getAliasType().str();
        alias_field->set_alias_kind(StringToKind(alias_type_name, field));
        alias_field->set_original_type_name(field_type_name);
      }
      // check if the field is a list
      else if (list_attr) {
        auto *list_field = proto_field->mutable_list_field();
        const auto &alias_type_name = list_attr->getAliasElementType().str();
        list_field->set_alias_kind(StringToKind(alias_type_name, field));
        list_field->set_original_type_name(field_type_name);
        list_field->set_iterator(list_attr->getListIterator().str());
        list_field->set_emplacer(list_attr->getListEmplacer().str());
      }
      // check if the field is a built-in type
      else if (const auto *built_in =
                   field->getType().getTypePtr()->getAs<clang::BuiltinType>()) {
        auto *basic_field = proto_field->mutable_basic_field();
        basic_field->set_kind(GetProtoKind(built_in));
      } else {
        std::cerr << "unknown field type, do you forget to add attribute?"
                  << std::endl;
      }
    }
  }
  return has_serializable_field;
}

void HandleRecord(const clang::CXXRecordDecl *record) {
  ProtoFileInfo info;
  std::string full_name = GetFullName(record);
  info.set_full_class_name(full_name);
  TerminateIf(!ParseRecordDetails(record, info),
              std::string("target \"" + full_name +
                          "\" do not have any field to be serialized")
                  .c_str(),
              record);
  if (readable_class_info) {
    std::fstream out(out_dir + full_name + ".info.txt",
                     std::ios::out | std::ios::trunc);
    std::string data;
    google::protobuf::TextFormat::PrintToString(info, &data);
    out << data;
  } else {
    std::fstream out(out_dir + full_name + ".info",
                     std::ios::binary | std::ios::out | std::ios::trunc);
    if (!info.SerializeToOstream(&out)) {
      std::cerr << "failed to write class info" << std::endl;
    }
  }
}

class RecordMatcherCallback
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  void
  run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override {
    if (const auto *record =
            Result.Nodes.getNodeAs<clang::CXXRecordDecl>("Record")) {
      for (auto *field : record->fields()) {
        if (IsSerializedField(field)) {
          HandleRecord(record);
          break;
        }
      }
    }
  }
};

int main(int argc, const char **argv) {
  clang::tooling::CommonOptionsParser options_parser(argc, argv,
                                                     serializable_category);
  clang::tooling::ClangTool tool(options_parser.getCompilations(),
                                 options_parser.getSourcePathList());

  RecordMatcherCallback callback;
  clang::ast_matchers::MatchFinder finder;
  finder.addMatcher(record_matcher, &callback);
  return tool.run(clang::tooling::newFrontendActionFactory(&finder).get());
}