#ifndef S11N_INCLUDE_ME_S11N_TYPE_CODER_TYPE_CODER_H
#define S11N_INCLUDE_ME_S11N_TYPE_CODER_TYPE_CODER_H
#include "builtin_types_helper.h"
#include "utils/common.h"
#include "utils/endianness_helper.h"
#include "utils/log2_floor_helper.h"
#include "utils/port.h"
#include "utils/size_cache.h"
#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <map>
namespace me {
namespace s11n {
// all unsupported types will fall into this Coder
template <typename T, typename Enable = void> struct TypeCoder {
  static uint8_t *Write(const T &value, uint8_t *ptr) {
    static_assert(DeferredFalse<T>::value, "type do not support write");
    return nullptr;
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    static_assert(DeferredFalse<T>::value, "type do not support read");
    return nullptr;
  }
  static std::size_t Size(const T &value) {
    static_assert(DeferredFalse<T>::value, "type do not support size");
    return 0;
  }
};

template <typename T>
inline static uint8_t *Encode(const T &value, uint8_t *ptr) {
  return TypeCoder<T>::Write(value, ptr);
}
template <typename T>
inline static const uint8_t *Decode(T &out, const uint8_t *ptr) {
  return TypeCoder<T>::Read(out, ptr);
}
template <typename T> inline static std::size_t Capacity(const T &value) {
  return TypeCoder<T>::Size(value);
}
template <typename T, std::size_t SIZE>
inline static uint8_t *Encode(const T (&value)[SIZE], uint8_t *ptr) {
  return TypeCoder<T[SIZE]>::Write(value, ptr);
}
template <typename T, std::size_t SIZE>
inline static const uint8_t *Decode(T (&out)[SIZE], const uint8_t *ptr) {
  return TypeCoder<T[SIZE]>::Read(out, ptr);
}
template <typename T, std::size_t SIZE>
inline static constexpr std::size_t Capacity(const T (&value)[SIZE]) {
  return TypeCoder<T[SIZE]>::Size(value);
}

// more specific unsupported types

// pointers are not supported
template <typename T> struct TypeCoder<T *> {
  static void CompileError() {
    static_assert(
        DeferredFalse<T>::value,
        "pointer is not supported, serializable should owns something, not "
        "pointers something, use std::unique ptr instead.");
  }
  static uint8_t *Write(const T &value, uint8_t *ptr) {
    CompileError();
    return nullptr;
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    CompileError();
    return nullptr;
  }
  static std::size_t Size(const T &value) {
    CompileError();
    return 0;
  }
};

// varint coder, used for unsigned arithmetic type
template <typename T>
struct TypeCoder<T, typename std::enable_if<GraininessWrapper<T>::type ==
                                                Graininess::VARINT &&
                                            std::is_unsigned<T>::value>::type> {
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
    out = res64;
    return ptr + 2;
  }
  static std::size_t Size(const T &value) {
    // same as VarintSizeXX in protobuf
    std::size_t log2value = Log2FloorNonZero(value | 0x1);
    // division to multiplication optimize
    return static_cast<std::size_t>((log2value * 9 + 73) / 64);
  }
  template <std::size_t SIZE> static constexpr std::size_t Size() {
    return static_cast<std::size_t>((Log2FloorNonZero<SIZE>() * 9 + 73) / 64);
  }
  template <std::size_t SIZE> static constexpr std::size_t ConstexprSize() {
    return static_cast<std::size_t>((Log2FloorNonZero<SIZE>() * 9 + 73) / 64);
  }
};

// zig-zag varint encoder, used for signed arithmetic type
template <typename T>
struct TypeCoder<T, typename std::enable_if<GraininessWrapper<T>::type ==
                                                Graininess::VARINT &&
                                            std::is_signed<T>::value>::type> {
  typedef typename std::make_unsigned<T>::type UnsignedT;
  static inline constexpr UnsignedT ZigZagValue(T value) {
    return (static_cast<UnsignedT>(value) << 1) ^
           static_cast<UnsignedT>(value >> 31);
  }
  static uint8_t *Write(T value, uint8_t *ptr) {
    // convert to zig zag value
    UnsignedT zig_zag_value = ZigZagValue(value);
    // now call to unsigned version of WriteGraininessToArray
    return TypeCoder<UnsignedT>::Write(zig_zag_value, ptr);
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    UnsignedT zig_zag_value;
    auto temp = TypeCoder<UnsignedT>::Read(zig_zag_value, ptr);
    // convert from zig zag value to normal value
    out = static_cast<T>((zig_zag_value >> 1) ^ (~(zig_zag_value & 1) + 1));
    return temp;
  }
  static std::size_t Size(const T &value) {
    UnsignedT zig_zag_value = ZigZagValue(value);
    return TypeCoder<UnsignedT>::Size(zig_zag_value);
  }
  template <std::size_t SIZE> static constexpr std::size_t ConstexprSize() {
    return TypeCoder<UnsignedT>::template ConstexprSize<ZigZagValue(SIZE)>();
  }
};

// enum type encoder
template <typename T>
struct TypeCoder<T, typename std::enable_if<std::is_enum<T>::value>::type> {
  static uint8_t *Write(const T &value, uint8_t *ptr) {
    return TypeCoder<unsigned>::Write(static_cast<unsigned>(value), ptr);
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    unsigned index;
    ptr = TypeCoder<unsigned>::Read(index, ptr);
    out = static_cast<T>(index);
    return ptr;
  }
  static constexpr std::size_t Size(T value) {
    return TypeCoder<unsigned>::Size(static_cast<unsigned>(value));
  }
};

// fixed size encoder
template <typename T>
struct TypeCoder<
    T, typename std::enable_if<(GraininessWrapper<T>::type < /* less than*/
                                Graininess::VARINT)>::type> {
  static uint8_t *Write(T value, uint8_t *ptr) {
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::Save((uint8_t *)&value,
                                                           ptr);
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::Load((uint8_t *)&out,
                                                           ptr);
  }
  static constexpr std::size_t Size(T value) { return sizeof(T); }
};

// encoder for arrays which element is varint
template <typename T, std::size_t SIZE>
struct TypeCoder<T[SIZE], typename std::enable_if<GraininessWrapper<T>::type ==
                                                  Graininess::VARINT>::type> {
  static uint8_t *Write(const T (&value)[SIZE], uint8_t *ptr) {
    // write size
    ptr = Encode(SizeCache<>::Get(value), ptr);
    for (std::size_t i = 0; i < SIZE; i++) {
      ptr = Encode(value[i], ptr);
    }
    return ptr;
  }
  static const uint8_t *Read(T (&out)[SIZE], const uint8_t *ptr) {
    std::size_t size;
    ptr = Decode(size, ptr);
    for (std::size_t i = 0; i < SIZE; i++) {
      ptr = Decode(out[i], ptr);
    }
    return ptr;
  }
  static std::size_t PayloadSize(const T (&value)[SIZE]) {
    std::size_t payload_size = Capacity(value[0]);
    for (std::size_t i = 1; i < SIZE; i++) {
      payload_size += Capacity(value[i]);
    }
    SizeCache<>::Set(value, payload_size);
    return payload_size;
  }
  static std::size_t Size(const T (&value)[SIZE]) {
    auto payload_size = PayloadSize(value);
    return payload_size + Capacity(payload_size);
  }
};

// encoder for arrays which element is fixed
template <typename T, std::size_t SIZE>
struct TypeCoder<T[SIZE], typename std::enable_if<(GraininessWrapper<T>::type <
                                                   Graininess::VARINT)>::type> {
  static uint8_t *Write(const T (&value)[SIZE], uint8_t *ptr) {
    // write size
    ptr = Encode(SIZE * sizeof(T), ptr);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::SavePacked(
        (const uint8_t *)&value[0], ptr, SIZE);
  }
  static const uint8_t *Read(T (&out)[SIZE], const uint8_t *ptr) {
    std::size_t size;
    ptr = Decode(size, ptr);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::LoadPacked((uint8_t *)out,
                                                                 ptr, SIZE);
  }
  static constexpr std::size_t Size(const T (&value)[SIZE]) {
    return TypeCoder<decltype(
               SIZE)>::template ConstexprSize<SIZE * TypeCoder<T>::Size(0)>() +
           (SIZE * TypeCoder<T>::Size(0));
  }
};

// encoder for vectors which element is varint
template <typename T, typename... TS>
struct TypeCoder<std::vector<T, TS...>,
                 typename std::enable_if<(GraininessWrapper<T>::type >=
                                          Graininess::VARINT)>::type> {
  static uint8_t *Write(const std::vector<T> &v, uint8_t *ptr) {
    // write size
    ptr = Encode(SizeCache<>::Get(v), ptr);
    for (auto i : v) {
      ptr = TypeCoder<T>::Write(i, ptr);
    }
    return ptr;
  }
  static const uint8_t *Read(std::vector<T> &out, const uint8_t *ptr) {
    std::size_t size;
    ptr = Decode(size, ptr);
    out.resize(0);
    out.reserve(size); // allocate mininum space
    auto end = ptr + size;
    while (ptr != end) {
      out.emplace_back();
      ptr = Decode(out.back(), ptr);
    }
    return ptr;
  }
  static std::size_t PayloadSize(const std::vector<T> &v) {
    std::size_t payload_size = 0;
    for (auto e : v) {
      payload_size += Capacity(e);
    }
    SizeCache<>::Set(v, payload_size);
    return payload_size;
  }
  static std::size_t Size(const std::vector<T> &v) {
    auto payload_size = PayloadSize(v);
    return payload_size + Capacity(payload_size);
  }
};

// encoder for vectors which element is fixed
template <typename T, typename... TS>
struct TypeCoder<std::vector<T, TS...>,
                 typename std::enable_if<(GraininessWrapper<T>::type <
                                          Graininess::VARINT)>::type> {
  static uint8_t *Write(const std::vector<T> &v, uint8_t *ptr) {
    ptr = Encode(SizeCache<>::Get(v), ptr);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::SavePacked(
        (const uint8_t *)&v[0], ptr, v.size());
  }
  static const uint8_t *Read(std::vector<T> &out, const uint8_t *ptr) {
    std::size_t size;
    ptr = Decode(size, ptr);
    auto counts = size / sizeof(T);
    out.clear();
    out.resize(counts);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::LoadPacked(
        (uint8_t *)out.data(), ptr, counts);
  }
  static std::size_t PayloadSize(const std::vector<T> &v) {
    std::size_t payload_size = v.size() * Capacity(v[0]);
    SizeCache<>::Set(v, payload_size);
    return payload_size;
  }
  static std::size_t Size(const std::vector<T> &v) {
    auto payload_size = PayloadSize(v);
    return payload_size + Capacity(payload_size);
  }
};

// coder for std::string
template <> struct TypeCoder<std::string> {
  static uint8_t *Write(const std::string &s, uint8_t *ptr) {
    ptr = Encode(s.size(), ptr);
    std::copy(s.begin(), s.end(), ptr);
    return ptr + s.size();
  }
  static const uint8_t *Read(std::string &out, const uint8_t *ptr) {
    std::size_t size;
    ptr = Decode(size, ptr);
    out.clear();
    out.resize(size);
    std::copy(ptr, ptr + size, &out[0]);
    return ptr + size;
  }
  static std::size_t Size(const std::string &s) {
    auto string_size = s.size();
    return Capacity(string_size) + string_size;
  }
};

// coder for std::pair
template <typename T1, typename T2> struct TypeCoder<std::pair<T1, T2>> {
  static uint8_t *Write(const std::pair<T1, T2> &value, uint8_t *ptr) {
    ptr = Encode(SizeCache<>::Get(value), ptr);
    ptr = Encode(value.first, ptr);
    ptr = Encode(value.second, ptr);
    return ptr;
  }
  static const uint8_t *Read(std::pair<T1, T2> &out, const uint8_t *ptr) {
    ptr = SkipVarint(ptr);
    ptr = Decode(out.first, ptr);
    ptr = Decode(out.second, ptr);
    return ptr;
  }
  static std::size_t PayloadSize(const std::pair<T1, T2> &value) {
    std::size_t payload_size = Capacity(value.first) + Capacity(value.second);
    SizeCache<>::Set(value, payload_size);
    return payload_size;
  }
  static std::size_t Size(const std::pair<T1, T2> &value) {
    auto payload_size = PayloadSize(value);
    return payload_size + Capacity(payload_size);
  }
};

// coder for maps
template <template <typename, typename, typename...> class MAP, typename KEY,
          typename VALUE, typename... TS>
struct TypeCoder<
    MAP<KEY, VALUE, TS...>,
    typename std::enable_if<
        is_specialization<MAP<KEY, VALUE, TS...>, std::unordered_map>::value ||
        is_specialization<MAP<KEY, VALUE, TS...>, std::map>::value>::type> {
  static uint8_t *Write(const MAP<KEY, VALUE, TS...> &map, uint8_t *ptr) {
    ptr = Encode(SizeCache<>::Get(map), ptr);
    for (const std::pair<KEY, VALUE> &pair : map) {
      ptr = Encode(pair, ptr);
    }
    return ptr;
  }
  static const uint8_t *Read(MAP<KEY, VALUE, TS...> &out, const uint8_t *ptr) {
    std::size_t size;
    ptr = Decode(size, ptr);
    std::pair<KEY, VALUE> pair;
    out.clear();
    auto end = ptr + size;
    while (ptr < end) {
      ptr = Decode(pair, ptr);
      out.emplace(pair);
    }
    return ptr;
  }
  static std::size_t PayloadSize(const MAP<KEY, VALUE, TS...> &map) {
    std::size_t payload_size = 0;
    for (auto e : map) {
      payload_size += Capacity(e);
    }
    SizeCache<>::Set(map, payload_size);
    return payload_size;
  }
  static std::size_t Size(const MAP<KEY, VALUE, TS...> &map) {
    auto payload_size = PayloadSize(map);
    return payload_size + Capacity(payload_size);
  }
};
} // namespace s11n
} // namespace me
#endif // S11N_INCLUDE_ME_S11N_TYPE_CODER_TYPE_CODER_H
