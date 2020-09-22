#include <../database/record_database.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include "iostream"
#include <fmt/core.h>
int main() {
  clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts(new clang::DiagnosticOptions);
  clang::TextDiagnosticPrinter Client(llvm::errs(), DiagOpts.get());
  clang::DiagnosticsEngine Diags(new clang::DiagnosticIDs, DiagOpts, &Client, false);
  RecordDatabase database(Diags);
  if(!database.parse("example_record_database.txt")) {
    return 0;
  }
  database.save();
  std::cout << fmt::format("The answer is {}.", 42);
}