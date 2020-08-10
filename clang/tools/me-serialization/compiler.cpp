#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/Support/CommandLine.h"

#include "me_attributes.h"

llvm::cl::OptionCategory serializable_category("serialization options");
llvm::cl::extrahelp commond_help(clang::tooling::CommonOptionsParser::HelpMessage);
llvm::cl::opt<std::string> builtin_includes("builtin", llvm::cl::desc("path of Clang builtin includes"),
                                            llvm::cl::cat(serializable_category));
llvm::cl::opt<std::string> out_dir("o", llvm::cl::desc("Specify output dir"),
                                   llvm::cl::value_desc("dir"),
                                   llvm::cl::init("./"),
                                   llvm::cl::cat(serializable_category));
llvm::cl::opt<bool> readable_class_info("readable", llvm::cl::desc("generate readable class info"));
clang::ast_matchers::DeclarationMatcher record_matcher =
    clang::ast_matchers::cxxRecordDecl(clang::ast_matchers::hasDefinition()).bind("Record");

bool IsSerializedField(const clang::FieldDecl *field) {
  if (auto *category_attr = field->getAttr<clang::MECategoryAttr>()) {
    for(const auto& category : category_attr->categories()) {
      if (category == PREFAB || category == SAVING) {
        return true;
      }
      return true;
    }
  }
  return false;
}

void HandleRecord(const clang::CXXRecordDecl *record) {
    record->dump();
}

class RecordMatcherCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override {
    if (const auto *record = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("Record")) {
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
  clang::tooling::CommonOptionsParser options_parser(argc, argv, serializable_category);
  clang::tooling::ClangTool tool(options_parser.getCompilations(),
                                 options_parser.getSourcePathList());

  RecordMatcherCallback callback;
  clang::ast_matchers::MatchFinder finder;
  finder.addMatcher(record_matcher, &callback);
  return tool.run(clang::tooling::newFrontendActionFactory(&finder).get());
}