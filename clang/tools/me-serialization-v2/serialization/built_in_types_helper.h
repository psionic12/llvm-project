#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPES_HELPER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPES_HELPER_H_
#include <cstdint>
#include <type_traits>
namespace me {
template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};
namespace serialization {
#define STRONG_TYPEDEF(_base, _type)                                           \
  class _type {                                                                \
  public:                                                                      \
    _type() = default;                                                         \
    _type(const _base v) : value(v) {}                                         \
    _type(const _type &v) = default;                                           \
    _type &operator=(const _type &rhs) = default;                              \
    _type &operator=(const _base &rhs) {                                       \
      value = rhs;                                                             \
      return *this;                                                            \
    }                                                                          \
    operator const _base &() const { return value; }                           \
    _type &operator++() {                                                      \
      ++value;                                                                 \
      return *this;                                                            \
    }                                                                          \
    _type operator++(int) {                                                    \
      _type tmp(*this);                                                        \
      tmp.value += 1;                                                          \
      return tmp;                                                              \
    }                                                                          \
    _type operator--(int) {                                                    \
      _type tmp(*this);                                                        \
      tmp.value -= 1;                                                          \
      return tmp;                                                              \
    }                                                                          \
    _type &operator--() {                                                      \
      --value;                                                                 \
      return *this;                                                            \
    }                                                                          \
                                                                               \
  private:                                                                     \
    _base value;                                                               \
  };
// no 8 and 16 bit int yet, because protobuf do not support that
STRONG_TYPEDEF(uint32_t, uint32)
STRONG_TYPEDEF(uint64_t, uint64)
STRONG_TYPEDEF(int32_t, int32)
STRONG_TYPEDEF(int64_t, int64)
enum class WireType : int {
  VARINT = 0,
  BIT_64 = 1,
  LENGTH_DELIMITED = 2,
  BIT_32 = 5,
};

template <typename T> struct WireTypeWrapper {};
template <> struct WireTypeWrapper<bool> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<char> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<unsigned char> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<signed char> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<float> {
  constexpr static WireType type = WireType::BIT_32;
  constexpr static int size = 32;
};
template <> struct WireTypeWrapper<double> {
  constexpr static WireType type = WireType::BIT_64;
  constexpr static int size = 64;
};
template <> struct WireTypeWrapper<long double> {
  constexpr static WireType type = WireType::BIT_64;
  constexpr static int size = 64;
};
template <> struct WireTypeWrapper<short> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<int> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<long> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<long long> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<unsigned short> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<unsigned int> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<unsigned long> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<unsigned long long> {
  constexpr static WireType type = WireType::VARINT;
};
template <> struct WireTypeWrapper<uint32> {
  constexpr static WireType type = WireType::BIT_32;
  constexpr static int size = 32;
};
template <> struct WireTypeWrapper<int32> {
  constexpr static WireType type = WireType::BIT_32;
  constexpr static int size = 32;
};
template <> struct WireTypeWrapper<int64> {
  constexpr static WireType type = WireType::BIT_64;
  constexpr static int size = 64;
};
template <> struct WireTypeWrapper<uint64> {
  constexpr static WireType type = WireType::BIT_64;
  constexpr static int size = 64;
};
} // namespace serialization
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPES_HELPER_H_
