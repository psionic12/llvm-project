#include "code_guard.h"
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
  Out << "#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_" << Str << "_H_"
      << std::endl;
  Out << "#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_" << Str << "_H_"
      << std::endl;
}
HeaderGuardCoder::~HeaderGuardCoder() {
  Out << "#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_" << Str << "_H_"
      << std::endl;
}
IncludeCoder::IncludeCoder(std::fstream &Out, llvm::StringRef Infile)
    : CodeGuard(Out) {
  Out << "#include \"" << Infile.data() << "\"" << std::endl;
  Out << "#include \"me/s11n/field_coder.h\"" << std::endl;
  Out << "#include <cstdint>" << std::endl;
}
NamespaceCoder::NamespaceCoder(std::fstream &Out, llvm::StringRef Namespace)
    : CodeGuard(Out), Namespace(Namespace) {
  Out << "namespace " << Namespace.data() << " {" << std::endl;
}
NamespaceCoder::~NamespaceCoder() {
  Out << "} // namespace " << Namespace.data() << std::endl;
}
RecordCoder::RecordCoder(std::fstream &Out, RecordInfo &RecordInfo)
    : CodeGuard(Out) {
  Out << "template <> struct Coder<" << RecordInfo.FullName << "> {"
      << std::endl;
  Out << "static uint8_t *Write(const " << RecordInfo.FullName
      << " &value, uint8_t *ptr) {" << std::endl;
}