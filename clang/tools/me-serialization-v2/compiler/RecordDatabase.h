#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_RECORDDATABASE_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_RECORDDATABASE_H_
#include "IndexManager.h"
#include "RecordInfo.h"
#include <clang/Basic/FileManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <unordered_map>
class RecordDatabase {
public:
  RecordDatabase(clang::DiagnosticsEngine &Diags, IndexManager &ClassesMgr,
                 std::unordered_map<std::string, RecordInfo> RecordInfos);
  bool parse(llvm::StringRef InFile);
  void save();

private:
  bool parseClass(size_t &Cursor);
  bool parseFullName(size_t &Cursor, std::string &Name);
  bool expectedToken(size_t &Cursor, clang::tok::TokenKind Kind,
                     bool CanFail = false);
  bool parseIndex(size_t &Cursor, uint32_t &Index);
  bool parseIdentifier(size_t &Cursor, std::string &Name, bool Append);
  bool parseField(size_t &Cursor, IndexManager &Class);
  bool loadData(clang::StringRef InFile);
  IndexManager &getIndicesByName(llvm::StringRef Name) {
    return Classes[Name.str()];
  }
  IndexManager& ClassesMgr;
  std::string codeGen();
  clang::LangOptions LangOpts;
  std::unique_ptr<llvm::MemoryBuffer> Code;
  clang::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> InMemFS;
  clang::DiagnosticsEngine &Diags;
  clang::FileManager FileMgr;
  clang::SourceManager SM;
  clang::FileID FileID;
  std::unique_ptr<clang::Lexer> LexerPtr;
  std::vector<clang::Token> Tokens;
  std::unordered_map<std::string, IndexManager> Classes;
  std::map<long, clang::StringRef> LocalClassIdCopies;
  unsigned err_expected_token;
  unsigned err_invalid_index;
  unsigned err_index_max;
  unsigned err_duplicated_index;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_RECORDDATABASE_H_
