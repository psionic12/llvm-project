#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_CODEDE_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_CODEDE_H_
#include "built_in_types_helper.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <type_traits>
#ifndef __ME_LittleEndianTarget__
#define __ME_LittleEndianTarget__ true
#endif
namespace me {
namespace serialization {
thread_local std::string error_msg;
template <typename T, std::size_t Index = 0>
std::enable_if_t<Index == sizeof(T), void>
_ReverseEndianInner(uint8_t *t, const uint8_t *s) {}

template <typename T, std::size_t Index = 0>
std::enable_if_t<Index != sizeof(T), void>
_ReverseEndianInner(uint8_t *t, const uint8_t *s) {
  t[Index] = s[sizeof(T) - 1 - Index];
  _ReverseEndianInner<T, Index + 1>(t, s);
}
template <typename T> T ReverseEndian(T source) {
  T target;
  _ReverseEndianInner<T>((uint8_t *)&target, (uint8_t *)&source);
  return target;
}

uint8_t *WriteIndexToArray(uint16_t index, uint8_t *target);
uint8_t *WriteTagToArray(bool repeated, TypeSize size, uint8_t *target);
// in some platform the real type size is smaller than we defined (such as
// a 2 bytes int), we need to fill 0 to the rest bytes.
template <typename T>
std::enable_if<sizeof(T) < Type<T>::size, uint8_t *>
FillZeroIf(uint8_t *target) {
  uint8_t *end = target + Type<T>::size - sizeof(T);
  std::copy(target, end, 0);
  return end;
}
template <typename T>
std::enable_if<sizeof(T) == Type<T>::size, uint8_t *>
FillZeroIf(uint8_t *target) {
  return target;
}

template <bool LittleEndian = true> struct EncodeHelper {
  template <typename T>
  static uint8_t *EncodeTypeToArray(T t, uint8_t *target) {
    std::copy(&t, &t + sizeof(T), target);
    target = target + sizeof(T);
    return FillZeroIf<T>(target);
  }
};
// if the target platform is big endian, we should convert the byte order first
template <> struct EncodeHelper<false> {
  template <typename T>
  static uint8_t *EncodeTypeToArray(T t, uint8_t *target) {
    T nt = ReverseEndian(t);
    return EncodeHelper<true>::EncodeTypeToArray(nt, target);
  }
};
template <typename T>
uint8_t *WriteTypeToArray(int index, T t, uint8_t *target) {
  target = WriteIndexToArray(index, target);
  target = WriteTagToArray(false, Type<T>::size, target);
  target =
      EncodeHelper<>::EncodeTypeToArray(static_cast<uint8_t *>(&t), target);
  return target;
}
template <typename T>
uint8_t *WriteRepeatTypeToArray(int index, T &t, uint8_t *target) {
  target = WriteIndexToArray(index, target);
  target = WriteTagToArray(true, Type<typename T::value_type>::size, target);
  if (/*unlikely*/ t.size() >
      (1 >> (static_cast<int>(Type<uint32_t>::size)) * 8)) {
    error_msg = "repeated entry has size larger than MAX_UINT_32";
    return nullptr;
  }
  target = EncodeHelper<>::EncodeTypeToArray((uint32_t)t.size(), target);
  for (auto value : t) {
    target = EncodeHelper<>::EncodeTypeToArray(value, target);
  }
  return target;
}
} // namespace serialization
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_CODEDE_H_
