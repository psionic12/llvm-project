#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_CODEDE_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_CODEDE_H_
#include "built_in_types_helper.h"
#include <algorithm>
#include <cstdint>
#include <list>
#include <string>
#include <type_traits>
#include <vector>
#ifndef ME_LITTLE_ENDIAN
#define ME_LITTLE_ENDIAN true
#endif
namespace me {
namespace serialization {
template <std::size_t Size, std::size_t Index = 0>
std::enable_if_t<(Size <= 1) || (Index >= Size), void>
ReverseEndian(const uint8_t *s, uint8_t *t) {}

template <std::size_t Size, std::size_t Index = 0>
std::enable_if_t<(Size > 1) && (Index < Size), void>
ReverseEndian(const uint8_t *s, uint8_t *t) {
  t[Index] = s[Size - 1 - Index];
  ReverseEndian<Size, Index + 1>(t, s);
}

template <std::size_t Size, bool LittleEndian = true> struct EndianHelper {
  static uint8_t *Save(uint8_t *data, uint8_t *target) {
    std::copy(data, data + Size, target);
    return target + Size;
  }
  static uint8_t *SaveRacked(uint8_t *data, uint8_t *target,
                              std::size_t size) {
    std::copy(data, data + (Size * size), target);
    return target + (Size * size);
  }
  static void Load(uint8_t *data, const uint8_t *target) {
    std::copy(target, target + Size, data);
  }
};
// if the target platform is big endian, we should convert the byte order first
template <std::size_t Size> struct EndianHelper<Size, false> {
  static uint8_t *Save(uint8_t *data, uint8_t *target) {
    ReverseEndian<Size>(data, target);
    return target + Size;
  }
  static uint8_t *SaveRacked(uint8_t *data, uint8_t *target,
                              std::size_t size) {
    for (int i = 0; i < size; i++) {
      ReverseEndian<Size>(data, target);
      target += Size;
    }
    return target + (Size * size);
  }
  static void Load(uint8_t *data, const uint8_t *target) {
    ReverseEndian<Size>(target, data);
  }
};

constexpr inline uint32_t MakeTag(int field_number, WireType type) {
  return static_cast<uint32_t>((static_cast<uint32_t>(field_number) << 3) |
                               static_cast<uint32_t>(type));
}

// varint encoder, used for unsigned arithmetic type
template <typename T>
std::enable_if_t<std::is_unsigned<T>::value, uint8_t *>
WriteDataToArrayByType(T value, uint8_t *ptr) {
  if (value < 0x80) {
    ptr[0] = static_cast<uint8_t>(value);
    return ptr + 1;
  }
  ptr[0] = static_cast<uint8_t>(value | 0x80);
  value >>= 7;
  if (value < 0x80) {
    ptr[1] = static_cast<uint8_t>(value);
    return ptr + 2;
  }
  ptr++;
  do {
    *ptr = static_cast<uint8_t>(value | 0x80);
    value >>= 7;
    ++ptr;
  } while (/*unlikely*/ (value >= 0x80));
  *ptr++ = static_cast<uint8_t>(value);
  return ptr;
}

// zig-zag varint encoder, used for signed arithmetic type
template <typename T>
std::enable_if_t<std::is_signed<T>::value, uint8_t *>
WriteDataToArrayByType(T value, uint8_t *ptr) {
  typedef typename std::make_unsigned<T>::type UnsignedT;
  // convert to zig zag value
  std::make_unsigned<T> zig_zag_value = (static_cast<UnsignedT>(value) << 1) ^
                                        static_cast<UnsignedT>(value >> 31);
  // now call to unsigned version of WriteWireTypeToArray
  return WriteDataToArrayByType(zig_zag_value, ptr);
}

// fixed size encoder
template <typename T>
std::enable_if_t<WireTypeWrapper<T>::type == WireType::BIT_32 ||
                     WireTypeWrapper<T>::type == WireType::BIT_64,
                 uint8_t *>
WriteDataToArrayByType(T value, uint8_t *ptr) {
  return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::Save(value, ptr);
}

inline uint8_t *WriteTagToArray(int field_number, WireType type,
                                uint8_t *target) {
  return WriteDataToArrayByType(MakeTag(field_number, type), target);
}

template <typename T>
uint8_t *WriteFieldToArray(int index, T value, uint8_t *target) {
  target = WriteTagToArray(index, WireTypeWrapper<T>::wire_type, target);
  target = WriteDataToArrayByType(value, target);
  return target;
}

// encoder for arrays which element is varint
template <typename T, std::size_t N>
std::enable_if_t<WireTypeWrapper<T>::type == WireType::VARINT, uint8_t *>
WriteRepeatedFieldToArray(int index, const T (&value)[N], uint8_t *target) {
  target = WriteTagToArray(index, WireTypeWrapper<T>::wire_type, target);
  target = WriteDataToArrayByType(N, target);
  for (int i = 0; i < N; i++) {
    target = WriteDataToArrayByType(value[i], target);
  }
}

// encoder for arrays which element is fixed
template <typename T, std::size_t N>
std::enable_if_t<WireTypeWrapper<T>::type == WireType::BIT_32 ||
                     WireTypeWrapper<T>::type == WireType::BIT_64,
                 uint8_t *>
WriteRepeatedFieldToArray(int index, const T (&value)[N], uint8_t *target) {
  target = WriteTagToArray(index, WireTypeWrapper<T>::wire_type, target);
  target = WriteDataToArrayByType(N, target);
  return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::SaveRacked(value, target,
                                                               N);
}

// encoder for vectors which element is varint
template <typename T, typename... Ts, std::vector<T, Ts...>>
std::enable_if_t<WireTypeWrapper<T>::type == WireType::VARINT, uint8_t *>
WriteRepeatedFieldToArray(int index, const std::vector<T, Ts...> &v,
                          uint8_t *target) {
  target = WriteTagToArray(index, WireTypeWrapper<T>::wire_type, target);
  target = WriteDataToArrayByType(v.size(), target);
  for (auto i : v) {
    target = WriteDataToArrayByType(i, target);
  }
}

// encoder for vectors which element is fixed
template <typename T, typename... Ts, std::vector<T, Ts...>>
std::enable_if_t<WireTypeWrapper<T>::type == WireType::BIT_32 ||
                     WireTypeWrapper<T>::type == WireType::BIT_64,
                 uint8_t *>
WriteRepeatedFieldToArray(int index, const std::vector<T, Ts...> &v,
                          uint8_t *target) {
  target = WriteTagToArray(index, WireTypeWrapper<T>::wire_type, target);
  target = WriteDataToArrayByType(v.size(), target);
  return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::SaveRacked(v.data(), target,
                                                               v.size());
}

} // namespace serialization
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_CODEDE_H_
