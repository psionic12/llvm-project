#include "record_database.h"
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/Support/VirtualFileSystem.h>
clang::LangOptions &RecordDatabase::langopts() {
  static clang::LangOptions LangOpts;
  LangOpts.CPlusPlus=true;
  return LangOpts; }
clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> RecordDatabase::diagOpts() {
  static clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts;
  return DiagOpts;
}
clang::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem>
RecordDatabase::fs() {
  static clang::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> FS(
      new llvm::vfs::InMemoryFileSystem);
  return FS;
}
clang::FileManager &RecordDatabase::getFileManager() {
  static clang::FileManager Files(clang::FileSystemOptions(), fs());
  return Files;
}
clang::DiagnosticsEngine &RecordDatabase::diag() {

  static clang::DiagnosticsEngine Diagnostics(
      new clang::DiagnosticIDs, diagOpts(),
      new clang::TextDiagnosticPrinter(llvm::errs(), diagOpts().get()));
  return Diagnostics;
}
clang::SourceManager &RecordDatabase::getSourceManager() {
  static clang::SourceManager Sources(diag(), getFileManager());
  return Sources;
}
bool RecordDatabase::parse(llvm::StringRef InFile) {
  llvm::SmallString<128> Path(InFile);
  llvm::sys::path::replace_extension(Path, ".db");
  auto DataOrError = llvm::MemoryBuffer::getFileAsStream(Path);
  if (std::error_code EC = DataOrError.getError()) {
    llvm::errs() << EC.message() << "\n";
    return false;
  }
  std::unique_ptr<llvm::MemoryBuffer> Code = std::move(DataOrError.get());
  if (Code->getBufferSize() == 0)
    return false;
  fs()->addFileNoOwn(Path, 0, Code.get());
  auto File = getFileManager().getFile(Path);
  auto FileID = getSourceManager().createFileID(
      File ? *File : nullptr, clang::SourceLocation(), clang::SrcMgr::C_User);

  diag().getClient()->BeginSourceFile(langopts());
  clang::Lexer Lexer(FileID, Code.get(), getSourceManager(),
                     langopts());

  Tokens.clear();
  while (true) {
    Tokens.emplace_back();
    Lexer.LexFromRawLexer(Tokens.back());
    if (Tokens.back().getKind() == clang::tok::eof)
      break;
  }

  std::size_t i = 0;
  bool success = true;
  while (i < Tokens.size()) {
    if (!parseClass(i)) {
      success = false;
      break;
    }
  }
  diag().getClient()->EndSourceFile();
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

  while (parseField(Cursor, Class));

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
    if(!CanFail) {
      diag().Report(Token.getLocation(), clang::diag::mes11n_err_expected_token) << Kind;
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
    auto str = clang::Lexer::getSpelling(Token, getSourceManager(),
                                         langopts());
    for (char c : str) {
      if (c < '0' || c > '9') {
        // not a integer
        diag().Report(Token.getLocation(), clang::diag::mes11n_err_invalid_index) << str;
        return -1;
      }
    }
    l = std::stol(str);
    if (l <= 0) {
      diag().Report(Token.getLocation(), clang::diag::mes11n_err_invalid_index)
          << std::to_string(l);
      return false;
    }
    if (l > 0xFFFFFFFF) {
      diag().Report(Token.getLocation(), clang::diag::mes11n_err_index_max)
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
    diag().Report(Token.getLocation(), clang::diag::mes11n_err_expected_token)
        << "identifier";
    return false;
  }
  std::string Str = clang::Lexer::getSpelling(Token, getSourceManager(),
                                              langopts());
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
