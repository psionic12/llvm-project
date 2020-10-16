#ifndef S11N_INCLUDE_ME_S11N_UTILS_LOG2_FLOOR_HELPER_H
#define S11N_INCLUDE_ME_S11N_UTILS_LOG2_FLOOR_HELPER_H
#include <cstdint>
#include <type_traits>
namespace me {
namespace s11n {
template <typename T, std::size_t SIZE> struct DeferBits {
  constexpr static std::size_t value = SIZE;
};
#if defined(__GNUC__)
template <typename T>
static typename std::enable_if<DeferBits<T, sizeof(std::size_t)>::value == 8,
                               unsigned int>::type
Log2FloorNonZero(T value) {
  return 63 ^ static_cast<uint64_t>(__builtin_clzll(value));
}
template <typename T>
static typename std::enable_if<DeferBits<T, sizeof(std::size_t)>::value == 4,
                               unsigned int>::type
Log2FloorNonZero(T value) {
  return 31 ^ static_cast<uint32_t>(__builtin_clz(value));
}
#elif defined(_MSC_VER)
template <typename T>
static typename std::enable_if<DeferBits<T, sizeof(std::size_t)>::value == 8,
                               unsigned int>::type
Log2FloorNonZero(T value) {
  unsigned long where;
  _BitScanReverse64(&where, value);
  return where;
}
template <typename T>
static typename std::enable_if<DeferBits<T, sizeof(std::size_t)>::value == 4,
                               unsigned int>::type
Log2FloorNonZero(T value) {
  unsigned long where;
  _BitScanReverse(&where, value);
  return where;
}
#else
template <typename T, int SIZE>
typename std::enable_if<SIZE == 0, unsigned int>::type
Log2FloorNonZero(T value) {
  return 0;
}

template <typename T, int SIZE = sizeof(T) * 8>
typename std::enable_if<(SIZE > 0), unsigned int>::type
Log2FloorNonZero(T value) {
  unsigned int log = 0;
  unsigned int shift = SIZE / 2;
  T rest = value >> shift;
  if (rest != 0) {
    log += shift;
    log += Log2FloorNonZero<T, SIZE / 2>(rest);
  } else {
    log += Log2FloorNonZero<T, SIZE / 2>(value);
  }
  return log;
}
#endif
// Constexpr verison Log2FloorNonZero.
// Cannot use constexpr function, C++ 11 do not support
template <std::size_t Value, std::size_t Shift = (sizeof(std::size_t) * 8 / 2)>
constexpr typename std::enable_if<Value == 0 || Shift == 0, std::size_t>::type
Log2FloorNonZero() {
  return 0;
}
template <std::size_t Value, std::size_t Shift = (sizeof(std::size_t) * 8 / 2)>
constexpr
    typename std::enable_if<Value != 0 && Shift != 0 && ((Value >> Shift) == 0),
                            std::size_t>::type
    Log2FloorNonZero();
template <std::size_t Value, std::size_t Shift = (sizeof(std::size_t) * 8 / 2)>
constexpr
    typename std::enable_if<Value != 0 && Shift != 0 && ((Value >> Shift) != 0),
                            std::size_t>::type
    Log2FloorNonZero() {
  return Shift + Log2FloorNonZero<(Value >> Shift), Shift / 2>();
}
template <std::size_t Value, std::size_t Shift>
constexpr
    typename std::enable_if<Value != 0 && Shift != 0 && ((Value >> Shift) == 0),
                            std::size_t>::type
    Log2FloorNonZero() {
  return Log2FloorNonZero<Value, Shift / 2>();
}
} // namespace s11n
} // namespace me
#endif // S11N_INCLUDE_ME_S11N_UTILS_LOG2_FLOOR_HELPER_H
