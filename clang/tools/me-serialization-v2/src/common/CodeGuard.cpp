#include "CodeGuard.h"
#include <fmt/core.h>
#include <sstream>
HeaderGuardCoder::HeaderGuardCoder(std::fstream &Out, llvm::StringRef InFile)
    : CodeGuard(Out) {
  std::stringstream SS;
  const char *C = InFile.data();
  for (std::size_t i = 0; i < InFile.size(); i++) {
    char NC;
    if (C[i] >= 'a' && C[i] <= 'z') {
      NC = C[i] - 32;
    } else if ((C[i] < 'A' || C[i] > 'Z') && (C[i] < '0' || C[i] > '9')) {
      NC = '_';
    } else {
      NC = C[i];
    }
    SS << NC;
  }
  Str = SS.str();
  Out << fmt::format("#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_{}_H_\n",
                     Str);
  Out << fmt::format("#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_{}_H_\n",
                     Str);
  Out << fmt::format("#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_{}_H_\n",
                     Str);
}
HeaderGuardCoder::~HeaderGuardCoder() {
  Out << fmt::format("#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_{}_H_\n",
                     Str);
}
IncludeCoder::IncludeCoder(std::fstream &Out, llvm::StringRef Infile)
    : CodeGuard(Out) {
  Out << fmt::format("#include \"{}\"\n", Infile.data());
  Out << fmt::format("#include \"me/s11n/field_coder.h\"\n");
  Out << fmt::format("#include <cstdint>\n");
}
NamespaceCoder::NamespaceCoder(std::fstream &Out, llvm::StringRef Namespace)
    : CodeGuard(Out), Namespace(Namespace) {
  Out << fmt::format("namespace {} {{\n");
}
NamespaceCoder::~NamespaceCoder() {
  Out << fmt::format("}} // namespace {}\n", Namespace.data());
}