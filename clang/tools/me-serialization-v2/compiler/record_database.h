#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_RECORD_DATABASE_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_RECORD_DATABASE_H_
#include <clang/Basic/FileManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <unordered_map>
enum {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, NOWERROR,      \
             SHOWINSYSHEADER, CATEGORY)                                        \
  ENUM,
#define PARSESTART
#include "DiagnosticS11nKinds.inc"
#undef DIAG
  NUM_BUILTIN_PARSE_DIAGNOSTICS
};
class RecordDatabase {
public:
  bool parse(llvm::StringRef InFile);

private:
  class FullNameMap {
  public:
    void emplace(const std::string &Str, unsigned int Index) {
      StrToIndex.emplace(Str, Index);
      if (Index > Max)
        Max = Index;
    }
    unsigned int operator[](const std::string &Str) {
      auto pair = StrToIndex.emplace(Str, Max + 1);
      if (pair.second) {
        Max += 1;
      }
      return pair.first->second;
    }

  private:
    std::unordered_map<std::string, unsigned int> StrToIndex;
    unsigned int Max = 0;
  };
  static clang::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> getFS();
  static clang::FileManager &getFileManager();
  static clang::DiagnosticsEngine &diag();
  static clang::SourceManager &getSourceManager();
  bool parseClass(size_t& Cursor);
  bool parseFullName(size_t &Cursor, std::string &Name);
  bool expectedToken(size_t &Cursor, clang::tok::TokenKind Kind);
  bool parseIndex(size_t &Cursor, uint32_t &Index);
  bool parseIdentifier(size_t &Cursor, std::string &Name, bool Append);
  bool parseField(size_t &Cursor, FullNameMap& Class);
  std::vector<clang::Token> Tokens;
  FullNameMap Classes;
  std::unordered_map<unsigned int, FullNameMap> ClassesToFields;
};

#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_RECORD_DATABASE_H_
