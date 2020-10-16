#ifndef S11N_INCLUDE_ME_S11N_FIELD_CODER_H
#define S11N_INCLUDE_ME_S11N_FIELD_CODER_H
#include "type_coder.h"
namespace me {
namespace s11n {
/*  0     | 1   2   3   4   5  | 6   7   8   */
/*  rtti  | reserved           | graininess  */
template <bool RTTI, Graininess Type> constexpr uint8_t MakeTag() {
  return static_cast<uint8_t>(Type) | static_cast<uint8_t>(RTTI) << 7;
}

template <uint8_t Tag> constexpr Graininess GetGraininess() {
  return static_cast<Graininess>(Tag & 0b111);
}

inline Graininess GetGraininess(uint8_t tag) {
  return static_cast<Graininess>(tag & 0b111);
}

template <uint8_t Tag> constexpr bool HasRtti() { return Tag >> 7; }
inline bool HasRtti(uint8_t tag) { return tag >> 7; }

template <typename T>
uint8_t *EncodeField(const uint32_t index, const T &value, uint8_t *ptr) {
  ptr = Encode(index, ptr);
  constexpr uint8_t tag = MakeTag<false, GraininessWrapper<T>::type>();
  ptr = Encode(tag, ptr);
  ptr = Encode(value, ptr);
  return ptr;
}
// array type
template <typename T, std::size_t SIZE>
uint8_t *EncodeField(const uint32_t index, const T (&value)[SIZE], uint8_t *ptr) {
  ptr = Encode(index, ptr);
  constexpr uint8_t tag = MakeTag<false, Graininess::LENGTH_DELIMITED>();
  ptr = Encode(tag, ptr);
  ptr = Encode(value, ptr);
  return ptr;
}

// unique_ptr type
template <typename T, typename... TS>
uint8_t *EncodeField(const uint32_t index, const std::unique_ptr<T, TS...> &value,
                     uint8_t *ptr) {
  ptr = Encode(index, ptr);
  constexpr uint8_t tag = MakeTag<true, GraininessWrapper<T>::type>();
  ptr = Encode(tag, ptr);
  ptr = Encode(value, ptr);
  return ptr;
}

template <std::uint32_t INDEX, typename T>
constexpr std::size_t FieldCapacity(const T &value) {
  return TypeCoder<uint32_t>::ConstexprSize<INDEX>() // index size
         + 1                                         // tag size
         + Capacity(value);
}
template <std::uint32_t INDEX, typename T, std::size_t SIZE>
constexpr std::size_t FieldCapacity(const T (&value)[SIZE]) {
  return TypeCoder<uint32_t>::ConstexprSize<INDEX>() // index size
         + 1                                         // tag size
         + Capacity(value);
}
inline const uint8_t *SkipUnknownField(uint8_t tag, const uint8_t *ptr) {
  if (HasRtti(tag)) {
    std::size_t picker_id;
    ptr = Decode(picker_id, ptr);
    if (picker_id == 0) {
      return ptr;
    }
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
    ptr = Decode(size, ptr);
    ptr += size;
    break;
  }
  }
  return ptr;
}
} // namespace s11n
} // namespace me
#endif // S11N_INCLUDE_ME_S11N_FIELD_CODER_H
