#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_INCLUDE_ME_S11N_ENDIANNESS_HELPER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_INCLUDE_ME_S11N_ENDIANNESS_HELPER_H_
namespace me {
namespace s11n {
template <std::size_t Size, std::size_t Index = 0>
std::enable_if_t<(Size <= 1) || (Index >= Size), void>
ReverseEndian(const uint8_t *s, uint8_t *t) {}

template <std::size_t Size, std::size_t Index = 0>
std::enable_if_t<(Size > 1) && (Index < Size), void>
ReverseEndian(const uint8_t *s, uint8_t *t) {
  t[Index] = s[Size - 1 - Index];
  ReverseEndian<Size, Index + 1>(t, s);
}
// if the target platform is little endian, copy directly
template <std::size_t Size, bool LittleEndian = true> struct EndianHelper {
  static uint8_t *Save(const uint8_t *data, uint8_t *target) {
    std::copy(data, data + Size, target);
    return target + Size;
  }
  static uint8_t *SavePacked(const uint8_t *data, uint8_t *target,
                             std::size_t size) {
    std::copy(data, data + (Size * size), target);
    return target + (Size * size);
  }
  static const uint8_t *Load(uint8_t *data, const uint8_t *target) {
    std::copy(target, target + Size, data);
    return target + Size;
  }
  static const uint8_t *LoadPacked(uint8_t *data, const uint8_t *target,
                                   std::size_t size) {
    std::copy(target, target + (Size * size), data);
    return target + (Size * size);
  }
};
// if the target platform is big endian, we should convert the byte order first
template <std::size_t Size> struct EndianHelper<Size, false> {
  static uint8_t *Save(uint8_t *data, uint8_t *target) {
    ReverseEndian<Size>(data, target);
    return target + Size;
  }
  static uint8_t *SavePacked(uint8_t *data, uint8_t *target, std::size_t size) {
    for (std::size_t i = 0; i < size; i++) {
      ReverseEndian<Size>(data, target);
      target += Size;
    }
    return target + (Size * size);
  }
  static void Load(uint8_t *data, const uint8_t *target) {
    ReverseEndian<Size>(target, data);
  }
};
} // namespace serialization
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_INCLUDE_ME_S11N_ENDIANNESS_HELPER_H_
