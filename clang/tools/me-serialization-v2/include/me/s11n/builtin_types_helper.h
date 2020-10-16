#ifndef S11N_INCLUDE_ME_S11N_BUILTIN_TYPES_HELPER_H
#define S11N_INCLUDE_ME_S11N_BUILTIN_TYPES_HELPER_H
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
namespace me {
namespace s11n {
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
  }
// strong typedef for xintxx_t, used to call fixed type size coder.
// no 8 and 16 bit int yet, because protobuf do not support that.
// this is used for serialization compiler to treat xintxx_t,
// do not use it in you code.
STRONG_TYPEDEF(uint8_t, strong_uint8);
STRONG_TYPEDEF(int8_t, strong_int8);
STRONG_TYPEDEF(uint16_t, strong_uint16);
STRONG_TYPEDEF(int16_t, strong_int16);
STRONG_TYPEDEF(uint32_t, strong_uint32);
STRONG_TYPEDEF(int32_t, strong_int32);
STRONG_TYPEDEF(uint64_t, strong_uint64);
STRONG_TYPEDEF(int64_t, strong_int64);

enum class Graininess : int {
  BIT_8 = 0,
  BIT_16 = 1,
  BIT_32 = 2,
  BIT_64 = 3,
  VARINT = 4,
  LENGTH_DELIMITED = 5,
};
template <typename T> struct GraininessWrapper {
  constexpr static Graininess type = Graininess::LENGTH_DELIMITED;
};
#define GraininessDef(_type, _graininess)                                      \
  template <> struct GraininessWrapper<_type> {                                \
    constexpr static Graininess type = _graininess;                            \
  }
GraininessDef(bool, Graininess::VARINT);
GraininessDef(char, Graininess::BIT_8);
GraininessDef(unsigned char, Graininess::BIT_8);
GraininessDef(signed char, Graininess::BIT_8);
GraininessDef(float, Graininess::BIT_32);
GraininessDef(double, Graininess::BIT_64);
GraininessDef(long double, Graininess::BIT_64);
GraininessDef(short, Graininess::VARINT);
GraininessDef(int, Graininess::VARINT);
GraininessDef(long, Graininess::VARINT);
GraininessDef(long long, Graininess::VARINT);
GraininessDef(unsigned short, Graininess::VARINT);
GraininessDef(unsigned int, Graininess::VARINT);
GraininessDef(unsigned long, Graininess::VARINT);
GraininessDef(unsigned long long, Graininess::VARINT);
GraininessDef(strong_uint8, Graininess::BIT_8);
GraininessDef(strong_int8, Graininess::BIT_8);
GraininessDef(strong_uint16, Graininess::BIT_16);
GraininessDef(strong_int16, Graininess::BIT_16);
GraininessDef(strong_uint32, Graininess::BIT_32);
GraininessDef(strong_int32, Graininess::BIT_32);
GraininessDef(strong_int64, Graininess::BIT_64);
GraininessDef(strong_uint64, Graininess::BIT_64);
template <typename T>
struct GraininessWrapper<std::shared_ptr<T>> {
  // do not support shard_ptr
};
template <typename T>
struct GraininessWrapper<T*> {
  // do not support pointers
};
} // namespace serialization
} // namespace me
#endif // S11N_INCLUDE_ME_S11N_BUILTIN_TYPES_HELPER_H
