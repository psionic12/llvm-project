#include <fstream>
#include <iostream>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/cpp/cpp_generator.h>
#include <google/protobuf/text_format.h>

#include "record_info.pb.h"

static llvm::cl::OptionCategory serializable_category("serialization options");
static llvm::cl::extrahelp
    common_help(clang::tooling::CommonOptionsParser::HelpMessage);
static llvm::cl::opt<std::string>
    builtin_includes("builtin",
                     llvm::cl::desc("path of Clang builtin includes"),
                     llvm::cl::cat(serializable_category));
static llvm::cl::opt<std::string> out_dir("o",
                                          llvm::cl::desc("Specify output dir"),
                                          llvm::cl::value_desc("dir"),
                                          llvm::cl::init("./"),
                                          llvm::cl::cat(serializable_category));
static llvm::cl::opt<bool>
    readable_class_info("readable",
                        llvm::cl::desc("generate readable class info"));
static clang::ast_matchers::DeclarationMatcher record_matcher =
    clang::ast_matchers::cxxRecordDecl(clang::ast_matchers::hasDefinition())
        .bind("Record");

static const std::unordered_map<std::string, ProtoKind> str_to_kind_map{
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

static constexpr const char *KindToString(ProtoKind kind) {
  switch (kind) {
  case ProtoKind::TypeDouble:
    return "double";
  case ProtoKind::TypeFloat:
    return "float";
  case ProtoKind::TypeInt64:
    return "int64";
  case ProtoKind::TypeUint64:
    return "uint64";
  case ProtoKind::TypeInt32:
    return "int32";
  case ProtoKind::TypeFixed64:
    return "fixed64";
  case ProtoKind::TypeFixed32:
    return "fixed32";
  case ProtoKind::TypeBool:
    return "bool";
  case ProtoKind::TypeString:
    return "string";
  case ProtoKind::TypeBytes:
    return "bytes";
  case ProtoKind::TypeUint32:
    return "uint32";
  case ProtoKind::TypeSfixed32:
    return "sfixed32";
  case ProtoKind::TypeSfixed64:
    return "sfixed64";
  case ProtoKind::TypeSint32:
    return "sint32";
  case ProtoKind::TypeSint64:
    return "sint64";
  default:
    std::cerr << "parse proto kind failed";
    std::terminate();
  }
}

static std::vector<std::string> generated_protobuf_files;

void TerminateIf(bool condition, const char *err, const clang::Decl *decl) {
  if (condition) {
    const auto &source_manager = decl->getASTContext().getSourceManager();
    decl->getLocation().dump(source_manager);
    std::cerr << err << std::endl;
    std::terminate();
  }
}

ProtoKind StringToKind(const std::string &str, const clang::Decl *decl) {
  const auto &it = str_to_kind_map.find(str);
  TerminateIf(it == str_to_kind_map.end(),
              (std::string("alias type unknown: ") + str).c_str(), decl);
  return it->second;
}

void GetFullName(const clang::Type *type, FullName *full_name,
                 const clang::PrintingPolicy &policy) {
  // inner lambda to get namespace
  std::function<void(const clang::NamespaceDecl *)> GetNameSpace =
      [&GetNameSpace, &full_name](const clang::NamespaceDecl *decl) {
        if (const auto *parent = llvm::dyn_cast_or_null<clang::NamespaceDecl>(
                decl->getParent())) {
          GetNameSpace(parent);
        }
        full_name->add_strings(decl->getNameAsString());
      };

  // if the type has a RecordDecl to refer to
  if (const auto *decl = type->getAsCXXRecordDecl()) {
    // if the decl has parent and the parent is a namespace decl
    if (const auto *namespace_decl =
            llvm::dyn_cast_or_null<clang::NamespaceDecl>(decl->getParent())) {
      // recursive to get the namespace name
      GetNameSpace(namespace_decl);
    }
    // the type name is the record name
    full_name->add_strings(decl->getNameAsString());
  } else if (const auto *built_in_type = type->getAs<clang::BuiltinType>()) {
    full_name->add_strings(built_in_type->getNameAsCString(policy));
  }
}

std::string NamespaceAsString(const FullName &full_name,
                              const std::string &replacement) {
  std::string s;
  auto iterator = full_name.strings().begin();
  while (iterator != full_name.strings().end() - 2) {
    s += *iterator;
    s += replacement;
    iterator++;
  }
  s += *iterator;
  return s;
}

std::string FullNameAsString(const FullName &full_name,
                             const std::string &replacement) {
  std::string s;
  auto iterator = full_name.strings().begin();
  while (iterator != full_name.strings().end() - 1) {
    s += *iterator;
    s += replacement;
    iterator++;
  }
  s += *iterator;
  return s;
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

const clang::Type *GetFieldType(const clang::CXXMethodDecl *method_decl) {

  TerminateIf(method_decl->param_size() > 0, "getter cannot have parameters",
              method_decl);
  auto *type =
      method_decl->getDeclaredReturnType()->getAs<clang::LValueReferenceType>();
  TerminateIf(!type, "getter must return reference type", method_decl);
  return type->getPointeeType().getTypePtr();
}

const clang::Type *GetFieldType(const clang::FieldDecl *field_decl) {
  return field_decl->getType().getTypePtr();
}

void HandleField(const clang::NamedDecl *decl, ProtoFileInfo &proto_file_info,
                 clang::MESerializedAttr *serialized_attr,
                 const clang::Type *field_type) {
  // create a field
  auto *proto_field = proto_file_info.add_fields();
  proto_field->set_name(decl->getNameAsString());

  // add all categories
  for (const auto &category_str_ref : serialized_attr->categories()) {
    proto_field->add_categories(category_str_ref.str());
  }

  auto *alias_attr = decl->getAttr<clang::MEAliasAttr>();
  auto *list_attr = decl->getAttr<clang::MEListAttr>();
  TerminateIf((alias_attr && list_attr),
              "cannot mark a field both as alias and as list", decl);

  const clang::PrintingPolicy &policy =
      decl->getASTContext().getPrintingPolicy();

  // check if the field is an alias
  if (alias_attr) {
    auto *alias_field = proto_field->mutable_alias_field();
    const auto &alias_type_name = alias_attr->getAliasType().str();
    proto_field->set_kind(StringToKind(alias_type_name, decl));
    GetFullName(field_type, alias_field->mutable_original_type_name(), policy);
    alias_field->mutable_original_type_name();
  }
  // check if the field is a list
  else if (list_attr) {
    auto *list_field = proto_field->mutable_list_field();
    const auto &alias_type_name = list_attr->getAliasElementType().str();
    proto_field->set_kind(StringToKind(alias_type_name, decl));
    GetFullName(field_type, list_field->mutable_original_type_name(), policy);
    list_field->set_iterator(list_attr->getListIterator().str());
    list_field->set_emplacer(list_attr->getListEmplacer().str());
  }
  // check if the field is a built-in type
  else {
    // TODO add default alias type
    const auto *built_in = field_type->getAs<clang::BuiltinType>();
    TerminateIf(!built_in,
                "unknown field type, do you forget to add attribute?", decl);
    proto_field->set_kind(GetProtoKind(built_in));
  }
}

bool ParseRecordDetails(const clang::CXXRecordDecl *record,
                        ProtoFileInfo &proto_file_info) {
  // parse parents
  bool has_serializable_field = false;
  CheckRecord(record);
  if (record->getNumBases() == 1) {
    const auto *type = record->bases_begin()->getType().getTypePtr();
    auto *parent = type->getAsCXXRecordDecl();
    has_serializable_field |= ParseRecordDetails(parent, proto_file_info);
    GetFullName(type, proto_file_info.add_ancestors(),
                record->getASTContext().getPrintingPolicy());
  }

  // parse fields
  for (auto *field : record->fields()) {
    if (auto *serialized_attr = field->getAttr<clang::MESerializedAttr>()) {
      CheckField(field);
      has_serializable_field |= true;
      HandleField(field, proto_file_info, serialized_attr, GetFieldType(field));
    }
  }

  // parse alias getters
  for (auto *method : record->methods()) {
    if (auto *serialized_attr = method->getAttr<clang::MESerializedAttr>()) {
      has_serializable_field |= true;
      HandleField(method, proto_file_info, serialized_attr,
                  GetFieldType(method));
    }
  }
  return has_serializable_field;
}

void GenerateRecordInfo(const ProtoFileInfo &proto_file_info,
                        const std::string &file_name) {

  if (readable_class_info) {
    std::fstream out(out_dir + "/" + file_name + ".info.txt",
                     std::ios::out | std::ios::trunc);
    if (!out) {
      std::cerr << "cannot create " + file_name + ".info.txt";
    }
    std::string data;
    google::protobuf::TextFormat::PrintToString(proto_file_info, &data);
    out << data;
    out.close();
  } else {
    std::fstream out(out_dir + "/" + file_name + ".info",
                     std::ios::binary | std::ios::out | std::ios::trunc);
    if (!out) {
      std::cerr << "cannot create " + file_name + ".info.txt";
    }
    if (!proto_file_info.SerializeToOstream(&out)) {
      std::cerr << "failed to write class info" << std::endl;
    }
    out.close();
  }
}

void GenerateProtobuf(const ProtoFileInfo &proto_file_info,
                      const std::string &file_name) {
  std::string path = out_dir + "/" + file_name + ".proto";
  std::fstream out(path, std::ios::out | std::ios::trunc);
  if (!out) {
    std::cerr << "cannot create " + file_name + ".proto";
  }

  out << "syntax = \"proto3\";" << std::endl;
  out << "package " << NamespaceAsString(proto_file_info.full_class_name(), ".")
      << ";" << std::endl;

  out << "message " << *proto_file_info.full_class_name().strings().rbegin()
      << " {" << std::endl;
  int i = 1;
  for (const auto &field : proto_file_info.fields()) {
    out << "\t";
    if (field.has_list_field()) {
      out << "repeated ";
    }
    out << KindToString(field.kind()) << " " << field.name() << " = " << i
        << ";" << std::endl;
    i++;
  }
  out << "}" << std::endl;
  out.close();
  generated_protobuf_files.push_back(std::move(path));
}

void ProtoToCpp() {
  namespace gpc = google::protobuf::compiler;
  gpc::CommandLineInterface cli;
  cli.AllowPlugins("protoc-");

  gpc::cpp::CppGenerator cpp_generator;
  cli.RegisterGenerator("--cpp_out", "--cpp_opt", &cpp_generator,
                        "Generate C++ header and source.");
  char const**argv = new char const*[generated_protobuf_files.size() + 5];
  int argc = 0;
  argv[argc++] = "protoc";
  argv[argc++] = "-I";
  argv[argc++] = out_dir.data();
  argv[argc++] = "--cpp_out";
  argv[argc++] = out_dir.data();
  for (auto& str : generated_protobuf_files) {
    argv[argc++] = str.data();
  }

  cli.Run(argc, argv);
}

void HandleRecord(const clang::CXXRecordDecl *record) {
  // TODO check if target is newer than dependency
  ProtoFileInfo info;
  GetFullName(record->getTypeForDecl(), info.mutable_full_class_name(),
              record->getASTContext().getPrintingPolicy());
  std::string full_cpp_name = FullNameAsString(info.full_class_name(), "::");
  std::string file_name = FullNameAsString(info.full_class_name(), "-");
  TerminateIf(!ParseRecordDetails(record, info),
              std::string("target \"" + full_cpp_name +
                          "\" do not have any field to be serialized")
                  .c_str(),
              record);
  std::cout << "class " + full_cpp_name + " scanned." << std::endl;
  GenerateRecordInfo(info, file_name);
  GenerateProtobuf(info, file_name);
}

class RecordMatcherCallback
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  void
  run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override {
    if (const auto *record =
            Result.Nodes.getNodeAs<clang::CXXRecordDecl>("Record")) {
      for (auto *field : record->fields()) {
        if (field->getAttr<clang::MESerializedAttr>() != nullptr) {
          HandleRecord(record);
          break;
        }
      }
      for (auto *method : record->methods()) {
        if (method->getAttr<clang::MESerializedAttr>() != nullptr) {
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
  tool.run(clang::tooling::newFrontendActionFactory(&finder).get());
  ProtoToCpp();
}