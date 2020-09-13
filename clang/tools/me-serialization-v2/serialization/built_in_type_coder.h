#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPE_CODER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPE_CODER_H_
#include "built_in_types_helper.h"
#include "log2_floor_helper.h"
#include "port.h"
#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
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
// helper to avoid static_assert trigger directly
template <typename> struct DeferredFalse : std::false_type {};
template <typename T, typename Enable = void> struct Coder {
  static uint8_t *Write(const T value, uint8_t *ptr) {
    static_assert(DeferredFalse<T>::value,
                  "fall through default coder, unsupported type.");
    return nullptr;
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    static_assert(DeferredFalse<T>::value,
                  "fall through default coder, unsupported type.");
    return nullptr;
  }
  static std::size_t Size(const T value) {
    static_assert(DeferredFalse<T>::value,
                  "fall through default coder, unsupported type.");
    return 0;
  }
};

struct CoderWrapper {
  static std::unordered_map<std::size_t, std::unique_ptr<CoderWrapper>>
      IdToCoderMap;
  static std::unordered_map<std::type_index, std::size_t> CppIdToIdMap;
  virtual uint8_t *Write(const void *value, uint8_t *ptr) = 0;
  virtual const uint8_t *Read(void *out, const uint8_t *ptr) = 0;
  virtual std::size_t Size(const void *value) = 0;
};

template <typename T> struct CoderWrapperImpl : public CoderWrapper {
  uint8_t *Write(const void *value, uint8_t *ptr) override {
    return Coder<T>::Write(*reinterpret_cast<T *>(value), ptr);
  }
  const uint8_t *Read(void *out, const uint8_t *ptr) override {
    return Coder<T>::Read(*reinterpret_cast<T *>(value), ptr);
  }
  size_t Size(const void *value) override {
    return Coder<T>::Size(*reinterpret_cast<T *>(value), ptr);
  }
};

// pointers are not supported
template <typename T> struct Coder<T *> {
  static uint8_t *Write(const T value, uint8_t *ptr) {
    static_assert(
        DeferredFalse<T>::value,
        "pointer is not supported, serializable should owns something, not "
        "pointers something, use std::unique ptr instead.");
    return nullptr;
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    static_assert(
        DeferredFalse<T>::value,
        "pointer is not supported, serializable should owns something, not "
        "pointers something, use std::unique ptr instead.");
    return nullptr;
  }
  static std::size_t Size(const T value) {
    static_assert(
        DeferredFalse<T>::value,
        "pointer is not supported, serializable should owns something, not "
        "pointers something, use std::unique ptr instead.");
    return 0;
  }
};
// shared_ptrs are not supported
template <typename T> struct Coder<std::shared_ptr<T>> {
  static uint8_t *Write(const T value, uint8_t *ptr) {
    static_assert(
        DeferredFalse<T>::value,
        "shared_ptr is not supported, serializable should owns something, not "
        "shares something.");
    return nullptr;
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    static_assert(
        DeferredFalse<T>::value,
        "shared_ptr is not supported, serializable should owns something, not "
        "shares something.");
    return nullptr;
  }
  static std::size_t Size(const T value) {
    static_assert(
        DeferredFalse<T>::value,
        "shared_ptr is not supported, serializable should owns something, not "
        "shares something.");
    return 0;
  }
};
// varint coder, used for unsigned arithmetic type
template <typename T>
struct Coder<T, typename std::enable_if_t<GraininessWrapper<T>::type ==
                                              Graininess::VARINT &&
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
  static std::size_t Size(const T value) {
    // same as VarintSizeXX in protobuf
    std::size_t log2value = Log2FloorHelper::Log2Floor(value | 0x1);
    // division to multiplication optimize
    return static_cast<std::size_t>((log2value * 9 + 73) / 64);
  }
  // We need passing constexpr parameter feature!!!
  template <std::size_t SIZE> static constexpr std::size_t ConstexprSize() {
    constexpr std::size_t log2value = Log2FloorHelper::Log2FloorConstexpr(SIZE);
    return static_cast<std::size_t>((log2value * 9 + 73) / 64);
  }
};
// zig-zag varint encoder, used for signed arithmetic type
template <typename T>
struct Coder<T, typename std::enable_if_t<GraininessWrapper<T>::type ==
                                              Graininess::VARINT &&
                                          std::is_signed<T>::value>> {
  typedef typename std::make_unsigned<T>::type UnsignedT;
  static inline constexpr UnsignedT ZigZagValue(T value) {
    return (static_cast<UnsignedT>(value) << 1) ^
           static_cast<UnsignedT>(value >> 31);
  }
  static uint8_t *Write(T value, uint8_t *ptr) {
    // convert to zig zag value
    UnsignedT zig_zag_value = ZigZagValue(value);
    // now call to unsigned version of WriteGraininessToArray
    return Coder<UnsignedT>::Write(zig_zag_value, ptr);
  }
  static const uint8_t *Read(T &out, const uint8_t *ptr) {
    UnsignedT zig_zag_value;
    auto temp = Coder<UnsignedT>::Read(zig_zag_value, ptr);
    // convert from zig zag value to normal value
    out = static_cast<T>((zig_zag_value >> 1) ^ (~(zig_zag_value & 1) + 1));
    return temp;
  }
  static std::size_t Size(const T value) {
    // TODO efficiency problem, Size and Write/Read calculate zig zag value
    // twice
    UnsignedT zig_zag_value = ZigZagValue(value);
    return Coder<UnsignedT>::Size(zig_zag_value);
  }
  template <std::size_t SIZE> static constexpr std::size_t ConstexprSize() {
    constexpr UnsignedT zig_zag_value = ZigZagValue(SIZE);
    return Coder<UnsignedT>::template ConstexprSize<zig_zag_value>();
  }
};
// fixed size encoder
template <typename T>
    struct Coder < T,
    typename std::enable_if_t<GraininessWrapper<T>::type<Graininess::VARINT>> {
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
// unique_ptr Type
template <typename T, typename... Ts> struct Coder<std::unique_ptr<T, Ts...>> {
  static uint8_t *Write(const std::unique_ptr<T, Ts...> unique_ptr,
                        uint8_t *ptr) {
    // TODO do I need to check the existence?
    ptr = WriteRaw(CoderWrapper::CppIdToIdMap[typeid(T)]);
    ptr = WriteRaw(*unique_ptr);
    return ptr;
  }
  static const uint8_t *Read(std::unique_ptr<T, Ts...> &unique_ptr,
                             const uint8_t *ptr) {
    std::size_t id;
    ptr = ReadRaw(id, ptr);
    void *out;
    ptr = CoderWrapper::IdToCoderMap[id]->Read(out, ptr);
    unique_ptr.reset(reinterpret_cast<T>(out));
    return ptr;
  }
  static std::size_t Size(const std::unique_ptr<T, Ts...> unique_ptr) {
    return 0;
  }
};
// encoder for arrays which element is varint
template <typename T, std::size_t SIZE>
struct Coder<T[SIZE], typename std::enable_if_t<GraininessWrapper<T>::type ==
                                                Graininess::VARINT>> {
  static uint8_t *Write(const T (&value)[SIZE], uint8_t *ptr) {
    // write size
    ptr = Coder<decltype(SIZE)>::Write(SIZE, ptr);

    for (std::size_t i = 0; i < SIZE; i++) {
      ptr = Coder<T>::Write(value[i], ptr);
    }
    return ptr;
  }
  static const uint8_t *Read(T (&out)[SIZE], const uint8_t *ptr) {
    std::size_t size;
    ptr = Coder<decltype(SIZE)>::Read(size, ptr);
    for (std::size_t i = 0; i < SIZE; i++) {
      ptr = Coder<T>::Read(out[i], ptr);
    }
    return ptr;
  }
  static std::size_t Size(const T (&value)[SIZE]) {
    std::size_t total_size =
        Coder<decltype(SIZE)>::template ConstexprSize<SIZE>();
    for (std::size_t i = 0; i < SIZE; i++) {
      total_size += Coder<T>::Size(value[i]);
    }
    return total_size;
  }
};
// encoder for arrays which element is fixed
template <typename T, std::size_t SIZE>
    struct Coder < T[SIZE],
    typename std::enable_if_t<GraininessWrapper<T>::type<Graininess::VARINT>> {
  static uint8_t *Write(const T (&value)[SIZE], uint8_t *ptr) {
    // write size
    ptr = Coder<decltype(SIZE)>::Write(SIZE, ptr);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::SavePacked(
        (const uint8_t *)&value[0], ptr, SIZE);
  }
  static const uint8_t *Read(T (&out)[SIZE], const uint8_t *ptr) {
    std::size_t size;
    ptr = Coder<decltype(SIZE)>::Read(size, ptr);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::LoadPacked((uint8_t *)out,
                                                                 ptr, SIZE);
  }
  static constexpr std::size_t Size(const T (&value)[SIZE]) {
    return Coder<decltype(SIZE)>::template ConstexprSize<SIZE>() +
           (SIZE * Coder<T>::Size(value[0]));
  }
};
// encoder for vectors which element is varint
template <typename T, typename... Ts>
struct Coder<std::vector<T, Ts...>,
             typename std::enable_if_t<GraininessWrapper<T>::type ==
                                       Graininess::VARINT>> {
  static uint8_t *Write(const std::vector<T> &v, uint8_t *ptr) {
    // write size
    ptr = Coder<decltype(v.size())>::Write(v.size(), ptr);
    for (auto i : v) {
      ptr = Coder<T>::Write(i, ptr);
    }
    return ptr;
  }
  static const uint8_t *Read(std::vector<T> &out, const uint8_t *ptr) {
    std::size_t size;
    ptr = Coder<decltype(size)>::Read(size, ptr);
    out.resize(size);
    for (auto &i : out) {
      ptr = Coder<T>::Read(i, ptr);
    }
    return ptr;
  }
  static std::size_t Size(const std::vector<T> &v) {
    auto vector_size = v.size();
    std::size_t total_size = Coder<decltype(vector_size)>::Size(vector_size);
    for (auto i : v) {
      total_size += Coder<T>::Size(i);
    }
    return total_size;
  }
};
// encoder for vectors which element is fixed
template <typename T, typename... Ts>
    struct Coder < std::vector<T, Ts...>,
    typename std::enable_if_t<GraininessWrapper<T>::type<Graininess::VARINT>> {
  static uint8_t *Write(const std::vector<T> &v, uint8_t *ptr) {
    ptr = Coder<decltype(v.size())>::Write(v.size(), ptr);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::SavePacked(
        (const uint8_t *)&v[0], ptr, v.size());
  }
  static const uint8_t *Read(std::vector<T> &out, const uint8_t *ptr) {
    std::size_t size;
    ptr = Coder<decltype(size)>::Read(size, ptr);
    out.resize(size);
    return EndianHelper<sizeof(T), ME_LITTLE_ENDIAN>::LoadPacked(
        (uint8_t *)out.data(), ptr, out.size());
  }
  static std::size_t Size(const std::vector<T> &v) {
    auto vector_size = v.size();
    return Coder<decltype(vector_size)>::Size(vector_size) +
           (vector_size * Coder<T>::Size(v));
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
    out.resize(size);
    std::copy(ptr, ptr + size, &out[0]);
    return ptr + size;
  }
  static std::size_t Size(const std::string &s) {
    auto string_size = s.size();
    return Coder<decltype(string_size)>::Size(string_size) + s.size();
  }
};
template <typename T> static uint8_t *WriteRaw(const T value, uint8_t *ptr) {
  return Coder<T>::Write(value, ptr);
}
template <typename T>
static const uint8_t *ReadRaw(T &out, const uint8_t *ptr) {
  return Coder<T>::Read(out, ptr);
}
template <typename T> static std::size_t SizeRaw(const T value) {
  return Coder<T>::Size(value);
}
template <typename T, std::size_t SIZE>
static uint8_t *WriteRaw(const T (&value)[SIZE], uint8_t *ptr) {
  return Coder<T[SIZE]>::Write(value, ptr);
}
template <typename T, std::size_t SIZE>
static const uint8_t *ReadRaw(T (&out)[SIZE], const uint8_t *ptr) {
  return Coder<T[SIZE]>::Read(out, ptr);
}
template <typename T, std::size_t SIZE>
static constexpr std::size_t SizeRaw(const T (&value)[SIZE]) {
  return Coder<T[SIZE]>::Size(value);
}

} // namespace serialization
} // namespace me

#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_BUILT_IN_TYPE_CODER_H_
