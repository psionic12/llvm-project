#include "record_database.h"
#include <clang/Basic/FileManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/Support/VirtualFileSystem.h>
clang::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem>
RecordDatabase::getFS() {
  static clang::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> FS(
      new llvm::vfs::InMemoryFileSystem);
  return FS;
}
clang::FileManager &RecordDatabase::getFileManager() {
  static clang::FileManager Files(clang::FileSystemOptions(), getFS());
  return Files;
}
clang::DiagnosticsEngine &RecordDatabase::diag() {
  static clang::DiagnosticsEngine Diagnostics(new clang::DiagnosticIDs,
                                              new clang::DiagnosticOptions);
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
  getFS()->addFileNoOwn(Path, 0, Code.get());
  auto File = getFileManager().getFile(Path);
  auto FileID = getSourceManager().createFileID(
      File ? *File : nullptr, clang::SourceLocation(), clang::SrcMgr::C_User);
  clang::Lexer Lexer(FileID, Code.get(), getSourceManager(),
                     clang::LangOptions());

  Tokens.clear();
  while (true) {
    Tokens.emplace_back();
    Lexer.LexFromRawLexer(Tokens.back());
    if (Tokens.back().getKind() == clang::tok::eof)
      break;
  }

  std::size_t i = 0;
  while (i < Tokens.size()) {
    if (!parseClass(i)) {
      return false;
    }
  }
  return true;
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

  while (parseField(Cursor, Class)) {
  }
  Cursor--;

  if (!expectedToken(Cursor, clang::tok::r_brace)) {
    return false;
  }
  return true;
}
bool RecordDatabase::parseFullName(size_t &Cursor, std::string &Name) {
  if (!parseIdentifier(Cursor, Name, true))
    return false;
  while (expectedToken(Cursor, clang::tok::coloncolon)) {
    if (!parseIdentifier(Cursor, Name, true)) {
      return false;
    }
  }
  Cursor--;
  return true;
}
bool RecordDatabase::expectedToken(size_t &Cursor, clang::tok::TokenKind Kind) {
  clang::Token Token = Tokens[Cursor++];
  if (!Token.is(Kind)) {
    diag().Report(Token.getLocation(), mes11n_err_expected_token) << Kind;
    return false;
  }
  return true;
}
bool RecordDatabase::parseIndex(size_t &Cursor, uint32_t &Index) {
  long l = 0;
  if (!expectedToken(Cursor, clang::tok::l_paren)) {
    return false;
  }
  clang::Token Token = Tokens[Cursor++];
  if (Token.is(clang::tok::numeric_constant)) {
    auto str = clang::Lexer::getSpelling(Token, getSourceManager(),
                                         clang::LangOptions());
    for (char c : str) {
      if (c < 0 || c > 9) {
        // not a integer
        diag().Report(Token.getLocation(), mes11n_err_invalid_index) << str;
        return -1;
      }
    }
    l = std::stol(str);
    if (l <= 0) {
      diag().Report(Token.getLocation(), mes11n_err_invalid_index)
          << std::to_string(l);
      return false;
    }
    if (l > 0xFFFFFFFF) {
      diag().Report(Token.getLocation(), mes11n_err_index_max)
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
    diag().Report(Token.getLocation(), mes11n_err_expected_token)
        << "identifier";
    return false;
  }
  std::string Str = clang::Lexer::getSpelling(Token, getSourceManager(),
                                              clang::LangOptions());
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
