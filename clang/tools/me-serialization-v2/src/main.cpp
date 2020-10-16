#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include <fstream>

#include "common/CodeGenerator.h"
#include "compiler/SerializableConsumer.h"
static llvm::cl::OptionCategory serializable_category("serialization options");
static llvm::cl::extrahelp
    CommonHelper(clang::tooling::CommonOptionsParser::HelpMessage);
static llvm::cl::opt<std::string> OutDir("o",
                                         llvm::cl::desc("Specify output dir"),
                                         llvm::cl::value_desc("dir"),
                                         llvm::cl::init("."),
                                         llvm::cl::cat(serializable_category));
// saved as changed databases, update them with new indices later
static std::unordered_map<std::string, RecordDatabase> ChangedDatabase;

class SerializableGenerationAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(new SerializableConsumer(
        Compiler, InFile, ChangedDatabase, llvm::StringRef()));
  }
};



CoreCppGenerator CreateCore() {
  auto RecordInfos = getObjFiles();
  CoreCppGenerator Core;
  for (auto &RecordInfo : RecordInfos) {
    Core.registerClass(RecordInfo);
  }
  return Core;
}

int main(int argc, const char **argv) {
  clang::tooling::CommonOptionsParser options_parser(argc, argv,
                                                     serializable_category);
  clang::tooling::ClangTool tool(options_parser.getCompilations(),
                                 options_parser.getSourcePathList());
  tool.run(
      clang::tooling::newFrontendActionFactory<SerializableGenerationAction>()
          .get());
}