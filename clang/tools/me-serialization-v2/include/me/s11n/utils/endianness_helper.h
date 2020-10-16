#ifndef S11N_INCLUDE_ME_S11N_UTILS_ENDIANNESS_HELPER_H
#define S11N_INCLUDE_ME_S11N_UTILS_ENDIANNESS_HELPER_H
#include <cstdint>
#include <cstring>
#include <type_traits>
namespace me {
namespace s11n {
// endian helper, serialize data with desired endianness
template <std::size_t Size, std::size_t Index = 0>
typename std::enable_if<(Index >= Size), void>::type
ReverseEndian(const uint8_t *s, uint8_t *t) {}

template <std::size_t Size, std::size_t Index = 0>
typename std::enable_if<(Index < Size), void>::type
ReverseEndian(const uint8_t *s, uint8_t *t) {
  t[Index] = s[Size - 1 - Index];
  ReverseEndian<Size, Index + 1>(s, t);
}
// if the target platform is little endian, copy directly
template <std::size_t Size, bool LittleEndian = true> struct EndianHelper {
  static uint8_t *Save(const uint8_t *data, uint8_t *target) {
    memcpy(target, data, Size);
    return target + Size;
  }
  static uint8_t *SavePacked(const uint8_t *data, uint8_t *target,
                             std::size_t counts) {
    memcpy(target, data, counts * Size);
    return target + counts * Size;
  }
  static const uint8_t *Load(uint8_t *data, const uint8_t *target) {
    memcpy(data, target, Size);
    return target + Size;
  }
  static const uint8_t *LoadPacked(uint8_t *data, const uint8_t *target,
                                   std::size_t counts) {
    memcpy(data, target, counts * Size);
    return target + counts * Size;
  }
};
// if the target platform is big endian, we should convert the byte order first
template <std::size_t Size> struct EndianHelper<Size, false> {
  static uint8_t *Save(const uint8_t *data, uint8_t *target) {
    ReverseEndian<Size>(data, target);
    return target + Size;
  }
  static uint8_t *SavePacked(const uint8_t *data, uint8_t *target,
                             std::size_t counts) {
    for (std::size_t i = 0; i < counts; i++) {
      ReverseEndian<Size>(data + (i * Size), target + (i * Size));
    }
    return target + (Size * counts);
  }
  static void Load(uint8_t *data, const uint8_t *target) {
    ReverseEndian<Size>(target, data);
  }
  static const uint8_t *LoadPacked(uint8_t *data, const uint8_t *target,
                                   std::size_t counts) {
    for (std::size_t i = 0; i < counts; i++) {
      ReverseEndian<Size>(target + (i * Size), data + (i * Size));
    }
    return target + counts * Size;
  }
};
} // namespace s11n
} // namespace me
#endif // S11N_INCLUDE_ME_S11N_UTILS_ENDIANNESS_HELPER_H
