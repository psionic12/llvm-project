#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_BASE_DERIVE_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_BASE_DERIVE_H_
#include <string>
class Base {
public:
  std::string msg = "base";
  virtual std::string Test() { return msg; }
};
class DerivedOne : Base {
public:
  std::string msg = "DerivedOne";
  std::string Test() override { return msg; }
};
class DerivedTwo : Base {
public:
  std::string msg = "DerivedOne";
  std::string Test() override { return msg; }
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_BASE_DERIVE_H_
