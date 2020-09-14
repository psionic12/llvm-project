#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_FOO_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_FOO_H_
#include <cstdint>
#include <me/s11n/built_in_type_coder.h>
#include <string>
#include <vector>
struct Foo {
  template <typename T, typename Enable> friend struct me::serialization::Coder;

public:
  int f1 = -1;
  unsigned int f2 = 2;
  float f3 = 0.3f;
  double f4 = 0.4;
  uint8_t f5 = 5;
  int16_t f6 = 6;
  uint32_t f7 = 7;
  int64_t f8 = 8;
  int f9[4] = {9, 99, 999, 9999};
  int32_t f10[4] = {10, 100, 1000, 10000};
  std::vector<unsigned int> f11 = {11, 111, 1111, 11111};
  std::vector<uint32_t> f12 = {12, 1212, 121212, 12121212};
  std::string f13 = "f13";
  float f14[4] = {1.0, 0.1, 0.01, 0.001};
  char f15[5] = "skip";
  std::unique_ptr<int> f16 = std::make_unique<int>(1);
};

#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_FOO_H_
