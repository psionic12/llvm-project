#include <../compiler/RecordDatabase.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <gtest/gtest.h>
class DatabaseTest : public testing::Test {};

TEST_F(DatabaseTest, parser_test) {
  clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts(
      new clang::DiagnosticOptions);
  clang::TextDiagnosticPrinter Client(llvm::errs(), DiagOpts.get());
  clang::DiagnosticsEngine Diags(new clang::DiagnosticIDs, DiagOpts, &Client,
                                 false);
  RecordDatabase database(Diags);
}