#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_EXAMPLE_1_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_EXAMPLE_1_H_
#include "../me_attributes.h"
class A {
private:
  [[me::category(PREFAB)]][[me::type(Type="int", Getter="xxx", Setter="xxx")]]
  int i = 0;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_EXAMPLE_1_H_
