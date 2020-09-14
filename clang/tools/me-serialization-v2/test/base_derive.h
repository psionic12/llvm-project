#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_BASE_DERIVE_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_BASE_DERIVE_H_
#include <string>
class Base {
public:
  std::string msg = "Base";
  virtual std::string Test() { return msg; }
};
class DerivedOne : public Base {
public:
  std::string msg = "DerivedOne";
  std::string Test() override { return msg; }
};
class DerivedTwo : public Base {
public:
  std::string msg = "DerivedTwo";
  std::string Test() override { return msg; }
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_BASE_DERIVE_H_
