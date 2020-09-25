#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_FIELD_CODER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_FIELD_CODER_H_
#include "built_in_type_coder.h"
#include "built_in_types_helper.h"
#include <cstdint>
namespace me {
namespace s11n {
inline constexpr uint8_t MakeTag(bool rtti, Graininess type) {
  uint8_t tag = static_cast<uint8_t>(type);
  tag |= static_cast<uint8_t>(rtti) << 7;
  return tag;
}
constexpr Graininess GetGraininess(uint8_t tag) {
  return static_cast<Graininess>(tag &= 0b111);
}
constexpr bool HasRtti(uint8_t tag) { return tag >> 7; }
template <typename T>
uint8_t *WriteField(const uint32_t index, const T& value, uint8_t *ptr) {
    ptr = Coder<uint32_t>::Write(index, ptr);
    constexpr uint8_t tag = MakeTag(false, GraininessWrapper<T>::type);
    ptr = Coder<uint8_t>::Write(tag, ptr);
    ptr = WriteRaw(value, ptr);
  return ptr;
}
// array type
template <typename T, std::size_t SIZE>
uint8_t *WriteField(const int index, const T (&value)[SIZE], uint8_t *ptr) {
  ptr = Coder<uint32_t>::Write(index, ptr);
  constexpr uint8_t tag = MakeTag(false, Graininess::LENGTH_DELIMITED);
  ptr = Coder<uint8_t>::Write(tag, ptr);
  ptr = WriteRaw(value, ptr);
  return ptr;
}

// unique_ptr type
template <typename T, typename... TS>
uint8_t *WriteField(const int index, const std::unique_ptr<T, TS...>& value,
                    uint8_t *ptr) {
  if (value != nullptr) {
    ptr = Coder<uint32_t>::Write(index, ptr);
    constexpr uint8_t tag = MakeTag(true, GraininessWrapper<T>::type);
    ptr = Coder<uint8_t>::Write(tag, ptr);
    ptr = WriteRaw(value, ptr);
  }
  return ptr;
}

template <std::uint32_t INDEX, typename T>
constexpr std::size_t FieldSize(const T& value) {
    return Coder<uint32_t>::ConstexprSize<INDEX>() // index size
           + 1                                     // tag size
           + SizeRaw(value);
}
template <std::uint32_t INDEX, typename T, std::size_t SIZE>
constexpr std::size_t FieldSize(const T (&value)[SIZE]) {
  return Coder<uint32_t>::ConstexprSize<INDEX>() // index size
         + 1                                     // tag size
         + SizeRaw(value);
}

inline const uint8_t *SkipVarint(const uint8_t *ptr) {
  for (std::uint64_t i = 0; i < 10; i++) {
    if (/*likely*/ (static_cast<uint8_t>(ptr[i]))) {
      ptr += (i + 1);
      break;
    }
  }
  return ptr;
}
inline const uint8_t *SkipUnknown(uint8_t tag, const uint8_t *ptr) {
  if (HasRtti(tag)) {
    ptr = SkipVarint(ptr);
  }
  switch (GetGraininess(tag)) {
  case Graininess::BIT_8: {
    ptr += 1;
    break;
  }
  case Graininess::BIT_16: {
    ptr += 2;
    break;
  }
  case Graininess::BIT_32: {
    ptr += 4;
    break;
  }
  case Graininess::BIT_64: {
    ptr += 8;
    break;
  }
  case Graininess::VARINT: {
    ptr = SkipVarint(ptr);
    break;
  }
  case Graininess::LENGTH_DELIMITED: {
    uint64_t size;
    ptr = ReadRaw(size, ptr);
    ptr += size;
    break;
  }
  }
  return ptr;
}
} // namespace serialization
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_FIELD_CODER_H_
