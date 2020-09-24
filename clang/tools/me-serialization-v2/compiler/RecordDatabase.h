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
  RecordDatabase(clang::DiagnosticsEngine &Diags);
  bool parse(llvm::StringRef InFile,std::unordered_map<std::string, RecordInfo&>& RecordInfos);
  void save();

private:
  bool parseClass(size_t &Cursor, std::unordered_map<std::string, RecordInfo &> &RecordInfos);
  bool parseFullName(size_t &Cursor, std::string &Name);
  bool expectedToken(size_t &Cursor, clang::tok::TokenKind Kind,
                     bool CanFail = false);
  bool parseIndex(size_t &Cursor, uint32_t &Index);
  bool parseIdentifier(size_t &Cursor, std::string &Name, bool Append);
  bool parseField(size_t &Cursor, RecordInfo &RecordInfo);
  bool loadData(clang::StringRef InFile);
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

  unsigned err_expected_token;
  unsigned err_invalid_index;
  unsigned err_index_max;
  unsigned err_duplicated_index;
  unsigned err_not_polymorphic;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_RECORDDATABASE_H_
