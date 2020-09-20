#include "record_database.h"
#include <clang/Basic/FileManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <llvm/Support/VirtualFileSystem.h>
RecordDatabase::RecordDatabase(clang::DiagnosticsEngine &Diags)
    : InMemFS(new llvm::vfs::InMemoryFileSystem), Diags(Diags),
      FileMgr(clang::FileSystemOptions(), InMemFS), SM(Diags, FileMgr) {
  LangOpts.CPlusPlus = true;
}
std::unique_ptr<clang::Lexer>
RecordDatabase::loadData(clang::StringRef InFile) {
  llvm::SmallString<128> Path(InFile);
  llvm::sys::path::replace_extension(Path, ".db");
  auto DataOrError = llvm::MemoryBuffer::getFileAsStream(Path);
  if (std::error_code EC = DataOrError.getError()) {
    llvm::errs() << EC.message() << "\n";
    return nullptr;
  }
  std::unique_ptr<llvm::MemoryBuffer> Code = std::move(DataOrError.get());
  if (Code->getBufferSize() == 0)
    return nullptr;
  InMemFS->addFileNoOwn(Path, 0, Code.get());
  auto File = FileMgr.getFile(Path);
  auto FileID = SM.createFileID(File ? *File : nullptr, clang::SourceLocation(),
                                clang::SrcMgr::C_User);
  return std::make_unique<clang::Lexer>(FileID, Code.get(), SM, LangOpts);
}
bool RecordDatabase::parse(llvm::StringRef InFile) {

  auto LexerPtr = loadData(InFile);
  // file does not exist or is empty, just skip parsing
  if (!LexerPtr)
    return true;

  auto &Lexer = *LexerPtr;
  Classes.clear();
  ClassesToFields.clear();
  Tokens.clear();

  Diags.getClient()->BeginSourceFile(LangOpts);
  while (true) {
    Tokens.emplace_back();
    Lexer.LexFromRawLexer(Tokens.back());
    if (Tokens.back().getKind() == clang::tok::eof)
      break;
  }

  std::size_t i = 0;
  bool success = true;
  while (i < Tokens.size() - 1) {
    if (!parseClass(i)) {
      success = false;
      break;
    }
  }
  Diags.getClient()->EndSourceFile();
  return success;
}
bool RecordDatabase::parseClass(size_t &Cursor) {
  uint32_t Index;
  std::string Name;
  if (!parseIndex(Cursor, Index)) {
    return false;
  }
  if (!parseFullName(Cursor, Name)) {
    return false;
  }
  if (!expectedToken(Cursor, clang::tok::l_brace)) {
    return false;
  }
  Classes.emplace(Name, Index);
  auto Class = ClassesToFields[Index];

  while (expectedToken(Cursor, clang::tok::l_paren, true)) {
    Cursor--;
    if (!parseField(Cursor, Class)) {
      return false;
    }
  }

  if (!expectedToken(Cursor, clang::tok::r_brace)) {
    return false;
  }
  return true;
}
bool RecordDatabase::parseFullName(size_t &Cursor, std::string &Name) {
  if (!parseIdentifier(Cursor, Name, true))
    return false;
  while (expectedToken(Cursor, clang::tok::coloncolon, true)) {
    if (!parseIdentifier(Cursor, Name, true)) {
      return false;
    }
  }
  return true;
}
bool RecordDatabase::expectedToken(size_t &Cursor, clang::tok::TokenKind Kind,
                                   bool CanFail) {
  clang::Token Token = Tokens[Cursor];
  if (Token.is(Kind)) {
    Cursor++;
    return true;
  } else {
    if (!CanFail) {
      Diags.Report(Token.getLocation(), clang::diag::mes11n_err_expected_token)
          << Kind;
    }
    return false;
  }
}
bool RecordDatabase::parseIndex(size_t &Cursor, uint32_t &Index) {
  long l = 0;
  if (!expectedToken(Cursor, clang::tok::l_paren)) {
    return false;
  }
  clang::Token Token = Tokens[Cursor++];
  if (Token.is(clang::tok::numeric_constant)) {
    auto str = clang::Lexer::getSpelling(Token, SM, LangOpts);
    for (char c : str) {
      if (c < '0' || c > '9') {
        // not a integer
        Diags.Report(Token.getLocation(), clang::diag::mes11n_err_invalid_index)
            << str;
        return -1;
      }
    }
    l = std::stol(str);
    if (l <= 0) {
      Diags.Report(Token.getLocation(), clang::diag::mes11n_err_invalid_index)
          << std::to_string(l);
      return false;
    }
    if (l > 0xFFFFFFFF) {
      Diags.Report(Token.getLocation(), clang::diag::mes11n_err_index_max)
          << std::to_string(l);
      return false;
    }
  }
  if (!expectedToken(Cursor, clang::tok::r_paren)) {
    return false;
  }
  Index = l;
  return true;
}
bool RecordDatabase::parseIdentifier(size_t &Cursor, std::string &Name,
                                     bool Append) {
  clang::Token Token = Tokens[Cursor++];
  if (!Token.isAnyIdentifier()) {
    Diags.Report(Token.getLocation(), clang::diag::mes11n_err_expected_token)
        << "identifier";
    return false;
  }
  std::string Str = clang::Lexer::getSpelling(Token, SM, LangOpts);
  if (Append) {
    Name.append(Str);
  } else {
    Name = Str;
  };
  return true;
}
bool RecordDatabase::parseField(size_t &Cursor,
                                RecordDatabase::FullNameMap &Class) {
  uint32_t Index;
  std::string Name;
  if (!parseIndex(Cursor, Index)) {
    return false;
  }
  if (!parseIdentifier(Cursor, Name, false)) {
    return false;
  }
  Class.emplace(Name, Index);
  return true;
}
void RecordDatabase::save() {

}
