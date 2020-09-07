#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPE_CODER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPE_CODER_H_
#include "built_in_types_helper.h"
#include "port.h"
#include <cstdint>
#include <string>
#include <vector>
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
// if the target platform is little endian, copy directly
template <std::size_t Size, bool LittleEndian = true> struct EndianHelper {
  static uint8_t *Save(uint8_t *data, uint8_t *target) {
    std::copy(data, data + Size, target);
    return target + Size;
  }
  static uint8_t *SavePacked(uint8_t *data, uint8_t *target, std::size_t size) {
    std::copy(data, data + (Size * size), target);
    return target + (Size * size);
  }
  static const uint8_t *Load(uint8_t *data, const uint8_t *target) {
    std::copy(target, target + Size, data);
    return target + Size;
  }
  static const uint8_t *LoadPacked(uint8_t *data, const uint8_t *target) {
    std::copy(data, data + Size, target);
    return target + Size;
  }
};
// helper to avoid static_assert trigger directly
template <typename> struct deferred_false : std::false_type {};
// if the target platform is big endian, we should convert the byte order first
template <std::size_t Size> struct EndianHelper<Size, false> {
  static uint8_t *Save(uint8_t *data, uint8_t *target) {
    ReverseEndian<Size>(data, target);
    return target + Size;
  }
  static uint8_t *SavePacked(uint8_t *data, uint8_t *target, std::size_t size) {
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
template <typename T, typename Enable = void> struct Coder {
  static uint8_t *Write(const T value, uint8_t *ptr) {
    static_assert(deferred_false<T>::value,
                  "fall through default coder, unsupported type.");
    return nullptr;
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    static_assert(deferred_false<T>::value,
                  "fall through default coder, unsupported type.");
    return nullptr;
  }
};
// varint coder, used for unsigned arithmetic type
template <typename T>
struct Coder<
    T, typename std::enable_if_t<WireTypeWrapper<T>::type == WireType::VARINT &&
                                 std::is_unsigned<T>::value>> {
  static uint8_t *Write(T value, uint8_t *ptr) {
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
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    uint32_t res32 = ptr[0];
    if (!(res32 & 0x80)) {
      out = res32;
      return ptr + 1;
    }
    uint32_t byte1 = ptr[1];
    res32 += (byte1 - 1) << 7;
    if (!(byte1 & 0x80)) {
      out = res32;
      return ptr + 2;
    }
    // VarintParseSlow64 in protobuf code
    uint64_t res64 = res32;
    for (std::uint32_t i = 2; i < 10; i++) {
      uint64_t byte2 = static_cast<uint8_t>(ptr[i]);
      res64 += (byte2 - 1) << (7 * i);
      if (/*likely*/ (byte2 < 128)) {
        out = res64;
        return ptr + i + 1;
      }
    }
    out = 0;
    return nullptr;
  }
};
// zig-zag varint encoder, used for signed arithmetic type
template <typename T>
struct Coder<
    T, typename std::enable_if_t<WireTypeWrapper<T>::type == WireType::VARINT &&
                                 std::is_signed<T>::value>> {
  typedef typename std::make_unsigned<T>::type UnsignedT;
  static uint8_t *Write(T value, uint8_t *ptr) {
    // convert to zig zag value
    UnsignedT zig_zag_value = (static_cast<UnsignedT>(value) << 1) ^
                              static_cast<UnsignedT>(value >> 31);
    // now call to unsigned version of WriteWireTypeToArray
    return Coder<UnsignedT>::Write(zig_zag_value, ptr);
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    UnsignedT zig_zag_value;
    auto temp = Coder<UnsignedT>::Read(zig_zag_value, ptr);
    // convert from zig zag value to normal value
    out = static_cast<T>((zig_zag_value >> 1) ^ (~(zig_zag_value & 1) + 1));
    return temp;
  }
};
// fixed size encoder
template <typename T>
struct Coder<T, typename std::enable_if_t<
                    WireTypeWrapper<T>::type == WireType::BIT_32 ||
                    WireTypeWrapper<T>::type == WireType::BIT_64>> {
  static uint8_t *Write(T value, uint8_t *ptr) {
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::Save((uint8_t *)&value,
                                                           ptr);
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::Load((uint8_t *)&out,
                                                           ptr);
  }
};
// encoder for arrays which element is varint
template <typename T, std::size_t Size>
struct Coder<T[Size], typename std::enable_if_t<WireTypeWrapper<T>::type ==
                                                WireType::VARINT>> {
  static uint8_t *Write(const T value[], uint8_t *ptr) {
    // write size
    ptr = Coder<decltype(Size)>::Write(Size, ptr);

    for (int i = 0; i < Size; i++) {
      ptr = Coder<T>::Write(value[i], ptr);
    }
    return ptr;
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    std::size_t size;
    ptr = Coder<decltype(Size)>::Read(size, ptr);
    for (int i = 0; i < Size; i++) {
      ptr = Coder<T>::Read(out[i], ptr);
    }
    return ptr;
  }
};
// encoder for arrays which element is fixed
template <typename T, std::size_t Size>
struct Coder<T[Size], typename std::enable_if_t<
                          WireTypeWrapper<T>::type == WireType::BIT_32 ||
                          WireTypeWrapper<T>::type == WireType::BIT_64>> {
  static uint8_t *Write(const T value[], uint8_t *ptr) {
    // write size
    ptr = Coder<decltype(Size)>::Write(Size, ptr);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::SavePacked(value, ptr,
                                                                 Size);
  }
  static const uint8_t *Read(T out[], const uint8_t *ptr) {
    std::size_t size;
    ptr = Coder<decltype(Size)>::Read(size, ptr);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::LoadPacked(out, ptr,
                                                                 Size);
  }
};
// encoder for vectors which element is varint
template <typename T, typename... Ts>
struct Coder<
    std::vector<T, Ts...>,
    typename std::enable_if_t<WireTypeWrapper<T>::type == WireType::VARINT>> {
  static uint8_t *Write(const T &v, uint8_t *ptr) {
    // write size
    ptr = Coder<decltype(v.size())>::Write(v.size(), ptr);
    for (auto i : v) {
      ptr = Coder<T>::Write(i, ptr);
    }
    return ptr;
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    std::size_t size;
    ptr = Coder<decltype(size)>::Read(size, ptr);
    for (auto i : out) {
      ptr = Coder<T>::Read(i, ptr);
    }
    return ptr;
  }
};
// encoder for vectors which element is fixed
template <typename T, typename... Ts>
struct Coder<
    std::vector<T, Ts...>,
    typename std::enable_if_t<WireTypeWrapper<T>::type == WireType::BIT_32 ||
                              WireTypeWrapper<T>::type == WireType::BIT_64>> {
  static uint8_t *Write(const T &v, uint8_t *ptr) {
    ptr = Coder<decltype(v.size())>::Write(v.size(), ptr);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::SavePacked(v.data(), ptr,
                                                                 v.size());
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    std::size_t size;
    ptr = Coder<decltype(size)>::Read(size, ptr);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::LoadPacked(
        out.data(), ptr, out.size());
  }
};
// encoder for char[]
template <std::size_t Size> struct Coder<char[Size]> {
  static uint8_t *Write(const char value[], uint8_t *ptr) {
    // TODO add protobuf UTF8 validation for debugging

    // write size
    ptr = Coder<decltype(Size)>::Write(Size, ptr);
    // write data
    std::copy(value, value + Size, ptr);
    return ptr + Size;
  }
  static const uint8_t *Read(char out[], const uint8_t *ptr) {
    std::size_t size;
    ptr = Coder<decltype(size)>::Read(size, ptr);
    std::copy(ptr, ptr + Size, out);
    return ptr + Size;
  }
};
// coder for std::string
template <> struct Coder<std::string> {
  static uint8_t *Write(const std::string &s, uint8_t *ptr) {
    ptr = Coder<decltype(s.size())>::Write(s.size(), ptr);
    std::copy(s.begin(), s.begin() + s.size(), ptr);
    return ptr + s.size();
  }
  static const uint8_t *Read(std::string &out, const uint8_t *ptr) {
    std::size_t size;
    ptr = Coder<decltype(size)>::Read(size, ptr);
    std::copy(ptr, ptr + size, &out[0]);
    return ptr + size;
  }
};
} // namespace serialization
} // namespace me

#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPE_CODER_H_
