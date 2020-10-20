#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMPILER_DIAGMSG_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMPILER_DIAGMSG_H_
#include "clang/Basic/Diagnostic.h"
namespace clang {
namespace s11n {
constexpr char warn_attribute_wrong_decl_type_str[] =
    "cannot attach attribute to decl: %0";
} // namespace s11n
} // namespace clang
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMPILER_DIAGMSG_H_
