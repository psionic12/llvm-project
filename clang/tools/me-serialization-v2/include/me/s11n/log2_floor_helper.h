#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_LOG2_FLOOR_HELPER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_LOG2_FLOOR_HELPER_H_
#include <cstdint>
#include <type_traits>
namespace me {
namespace s11n {
struct Log2FloorHelper {
  template <typename U, std::size_t SIZE> struct Bits {
    constexpr static std::size_t value = SIZE;
  };
#if defined(__GNUC__)
  template <typename T>
  static std::enable_if_t<Bits<T, sizeof(std::size_t)>::value == 8,
                          unsigned int>
  Log2Floor(T value) {
    return 63 ^ static_cast<uint64_t>(__builtin_clzll(value));
  }
  template <typename T>
  static std::enable_if_t<Bits<T, sizeof(std::size_t)>::value == 4,
                          unsigned int>
  Log2Floor(T value) {
    return 31 ^ static_cast<uint32_t>(__builtin_clz(value));
  }
#elif defined(_MSC_VER)
  template <typename T>
  static std::enable_if_t<Bits<T, sizeof(std::size_t)>::value == 8,
                          unsigned int>
  Log2Floor(T value) {
    unsigned long where;
    _BitScanReverse64(&where, value);
    return where;
  }
  template <typename T>
  static std::enable_if_t<Bits<T, sizeof(std::size_t)>::value == 4,
                          unsigned int>
  Log2Floor(T value) {
    unsigned long where;
    _BitScanReverse(&where, value);
    return where;
  }
#else
  template <typename T, int SIZE>
  std::enable_if_t<SIZE == 0, unsigned int> Log2FloorDefault(T value) {
    return 0;
  }

  template <typename T, int SIZE = sizeof(T) * 8>
  std::enable_if_t<(SIZE > 0), unsigned int> Log2FloorDefault(T value) {
    unsigned int log = 0;
    unsigned int shift = SIZE / 2;
    T rest = value >> shift;
    if (rest != 0) {
      log += shift;
      log += Log2FloorDefault<T, SIZE / 2>(rest);
    } else {
      log += Log2FloorDefault<T, SIZE / 2>(value);
    }
    return log;
  }
#endif
  // We need constexpr parameter overloading!!!
  constexpr static std::size_t Log2FloorConstexpr(const std::size_t value) {
    int log = 0;
    std::size_t v = value;
    for (int i = sizeof(std::size_t) * 8 / 2; i > 0; i /= 2) {
      std::size_t x = v >> i;
      if (x != 0) {
        v = x;
        log += i;
      }
    }
    return log;
  }
};
} // namespace serialization
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_LOG2_FLOOR_HELPER_H_
