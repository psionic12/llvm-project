#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include "SerializableConsumer.h"
static llvm::cl::OptionCategory serializable_category("serialization options");
static llvm::cl::extrahelp
    common_help(clang::tooling::CommonOptionsParser::HelpMessage);
static llvm::cl::opt<std::string> out_dir("o",
                                          llvm::cl::desc("Specify output dir"),
                                          llvm::cl::value_desc("dir"),
                                          llvm::cl::init("."),
                                          llvm::cl::cat(serializable_category));



int main(int argc, const char **argv) {
  clang::tooling::CommonOptionsParser options_parser(argc, argv,
                                                     serializable_category);
  clang::tooling::ClangTool tool(options_parser.getCompilations(),
                                 options_parser.getSourcePathList());
  tool.run(
      clang::tooling::newFrontendActionFactory<SerializableGenerationAction>()
          .get());
}