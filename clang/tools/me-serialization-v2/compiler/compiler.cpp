#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include <fstream>

#include "SerializableConsumer.h"
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

int main(int argc, const char **argv) {
  clang::tooling::CommonOptionsParser options_parser(argc, argv,
                                                     serializable_category);
  clang::tooling::ClangTool tool(options_parser.getCompilations(),
                                 options_parser.getSourcePathList());
  tool.run(
      clang::tooling::newFrontendActionFactory<SerializableGenerationAction>()
          .get());

  // handle all mes11n.obj files
  std::vector<RecordInfo> RecordInfos;
  std::error_code EC;
  llvm::sys::fs::directory_iterator Begin(OutDir, EC, false);
  llvm::sys::fs::directory_iterator End;
  while (Begin != End) {
    auto &Item = *Begin;
    if (Item.type() == llvm::sys::fs::file_type::regular_file &&
        llvm::sys::path::extension(Item.path()) == "mes11n.obj") {
      auto FileSize = Item.status()->getSize();
      std::unique_ptr<const uint8_t> Buffer(new uint8_t[FileSize]);
      std::fstream Obj(Item.path().c_str(), std::ios::in);
      Obj.read((char *)Buffer.get(), FileSize);
      const uint8_t *End = Buffer.get() + FileSize;
      const uint8_t *Ptr = Buffer.get();
      while (Ptr < End) {
        RecordInfos.emplace_back(Ptr);
      }
    }
  }
}