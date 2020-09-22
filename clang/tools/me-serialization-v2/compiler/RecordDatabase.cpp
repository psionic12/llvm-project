#include "RecordDatabase.h"
#include <clang/Basic/FileManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/Core/Replacement.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <sstream>
RecordDatabase::RecordDatabase(clang::DiagnosticsEngine &Diags)
    : InMemFS(new llvm::vfs::InMemoryFileSystem), Diags(Diags),
      FileMgr(clang::FileSystemOptions(), InMemFS), SM(Diags, FileMgr) {
  LangOpts.CPlusPlus = true;
}
bool RecordDatabase::loadData(clang::StringRef InFile) {
  llvm::SmallString<128> Path(InFile);
  llvm::sys::path::replace_extension(Path, ".db");

  auto FDOrErr = llvm::sys::fs::openNativeFileForReadWrite(
      Path, llvm::sys::fs::CD_OpenAlways, llvm::sys::fs::OF_None);
  if (!FDOrErr) {
    llvm::errs() << FDOrErr.takeError() << "\n";
    return false;
  }
  uint64_t FileSize;
  if (llvm::sys::fs::file_size(Path, FileSize)) {
    llvm::errs() << "failed to get file size"
                 << "\n";
    return false;
  }
  llvm::sys::fs::file_t FD = *FDOrErr;
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> DataOrError =
      llvm::MemoryBuffer::getOpenFile(FD, Path, FileSize);
  llvm::sys::fs::closeFile(FD);

  if (std::error_code EC = DataOrError.getError()) {

    llvm::errs() << EC.message() << "\n";
    LexerPtr = nullptr;
    return true;
  }
  Code = std::move(DataOrError.get());
  InMemFS->addFileNoOwn(Path, 0, Code.get());
  auto File = FileMgr.getFile(Path);
  FileID = SM.createFileID(File ? *File : nullptr, clang::SourceLocation(),
                           clang::SrcMgr::C_User);
  LexerPtr = std::make_unique<clang::Lexer>(FileID, Code.get(), SM, LangOpts);
  return true;
}
bool RecordDatabase::parse(llvm::StringRef InFile) {

  if (!loadData(InFile)) {
    return false;
  }
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
  auto &Class = ClassesToFields[Index];

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
  if (!parseIdentifier(Cursor, Name, false))
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
    Name.append("::");
    Name.append(Str);
  } else {
    Name = Str;
  };
  return true;
}
bool RecordDatabase::parseField(size_t &Cursor,
                                FullNameMap &Class) {
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
  // replace the whole file for simplicity.
  std::string NewCode = CodeGen();
  clang::StringRef Ref = NewCode;
  clang::tooling::Replacement Replacement(SM, SM.getLocForStartOfFile(FileID),
                                          SM.getFileIDSize(FileID), Ref);
  clang::tooling::Replacements Replaces;
  if (auto Error = Replaces.add(Replacement)) {
    llvm::errs() << Error << "\n";
  }
  clang::Rewriter Rewrite(SM, LangOpts);
  clang::tooling::applyAllReplacements(Replaces, Rewrite);
  Rewrite.overwriteChangedFiles();
}
std::string RecordDatabase::CodeGen() {
  std::stringstream NewCode;
  for (const auto &pair : Classes.GetOrdered()) {
    clang::StringRef Name = pair.second;
    const auto ID = pair.first;
    NewCode << "(" << ID << ")" << Name.data() << "{\n";
    auto &Fields = ClassesToFields[ID];
    for (const auto &pair2 : Fields.GetOrdered()) {
      clang::StringRef FieldName = pair2.second;
      const auto FieldID = pair2.first;
      NewCode << "(" << FieldID << ")" << FieldName.data() << "\n";
    }
    NewCode << "}\n";
  }
  return NewCode.str();
}
std::pair<FullNameMap &, unsigned int>
RecordDatabase::getClassByName(const std::string &Name) {
  auto ID = Classes[Name];
  auto &Class = ClassesToFields[ID];
  return {Class, ID};
}
