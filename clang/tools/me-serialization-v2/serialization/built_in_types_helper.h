#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPES_HELPER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPES_HELPER_H_
namespace me {
namespace serialization {
enum class TypeSize : int {
  k1 = 0,
  k2 = 1,
  k4 = 2,
  k8 = 3,
};
#define TypeSpec(type, type_size)                                              \
  template <> struct Type<type> { constexpr static TypeSize size = type_size; }

template <typename T> struct Type {};
TypeSpec(bool, TypeSize::k1);
TypeSpec(char, TypeSize::k1);
TypeSpec(unsigned char, TypeSize::k1);
TypeSpec(signed char, TypeSize::k1);
TypeSpec(float, TypeSize::k4);
TypeSpec(double, TypeSize::k8);
TypeSpec(long double, TypeSize::k8);
TypeSpec(short, TypeSize::k2);
TypeSpec(int, TypeSize::k4);
TypeSpec(long, TypeSize::k8);
TypeSpec(long long, TypeSize::k8);
TypeSpec(unsigned short, TypeSize::k2);
TypeSpec(unsigned int, TypeSize::k4);
TypeSpec(unsigned long, TypeSize::k8);
TypeSpec(unsigned long long, TypeSize::k8);
} // namespace serialization
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPES_HELPER_H_
