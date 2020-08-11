#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_EXAMPLE_1_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_EXAMPLE_1_H_
#include "../me_attributes.h"
#include <vector>
class A {
private:
  [[me::category(PREFAB)]][[me::alias("int64")]]
  int i = 0;

  [[me::category(PREFAB, SAVE)]] [[me::list("int64", "xxx", "yyy")]]
  std::vector<int> v;
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_EXAMPLE_1_H_
