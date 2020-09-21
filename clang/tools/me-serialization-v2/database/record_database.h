#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_RECORD_DATABASE_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_RECORD_DATABASE_H_
#include <clang/Basic/FileManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <unordered_map>
class FullNameMap {
public:
  void emplace(const std::string &Str, unsigned int Index) {
    clang::StringRef Name = StrToIndex.emplace(Str, Index).first->first;
    OrderedMap.emplace(Index, Name);
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
  void clear() {
    StrToIndex.clear();
    Max = 0;
  }
  std::map<unsigned int, clang::StringRef>& GetOrdered() {
    return OrderedMap;
  }
private:
  std::unordered_map<std::string, unsigned int> StrToIndex;
  std::map<unsigned int, clang::StringRef> OrderedMap;
  unsigned int Max = 0;
};
class RecordDatabase {
public:
  RecordDatabase(clang::DiagnosticsEngine& Diags);
  bool parse(llvm::StringRef InFile);
  void save();
  std::pair<FullNameMap&, unsigned int> getClassByName(const std::string& Name);

private:

  bool parseClass(size_t &Cursor);
  bool parseFullName(size_t &Cursor, std::string &Name);
  bool expectedToken(size_t &Cursor, clang::tok::TokenKind Kind,
                     bool CanFail = false);
  bool parseIndex(size_t &Cursor, uint32_t &Index);
  bool parseIdentifier(size_t &Cursor, std::string &Name, bool Append);
  bool parseField(size_t &Cursor, FullNameMap &Class);
  bool loadData(clang::StringRef InFile);
  std::string CodeGen();
  clang::LangOptions LangOpts;
  std::unique_ptr<llvm::MemoryBuffer> Code;
  clang::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> InMemFS;
  clang::DiagnosticsEngine& Diags;
  clang::FileManager FileMgr;
  clang::SourceManager SM;
  clang::FileID FileID;
  std::unique_ptr<clang::Lexer> LexerPtr;
  std::vector<clang::Token> Tokens;
  FullNameMap Classes;
  std::unordered_map<unsigned int, FullNameMap> ClassesToFields;
};

#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_COMPILER_RECORD_DATABASE_H_
