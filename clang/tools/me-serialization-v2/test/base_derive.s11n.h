#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_BASE_DERIVE_S11N_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_BASE_DERIVE_S11N_H_
#include "base_derive.h"
#include "me/s11n/field_coder.h"
namespace me {
namespace serialization {

template <> struct Coder<Base> {
  static uint8_t *Write(const Base& value, uint8_t *ptr) {
    ptr = WriteRaw(Size(value), ptr);
    ptr = WriteField(1, value.msg, ptr);
    return ptr;
  }
  static const uint8_t *Read(Base &out, const uint8_t *ptr) {
    std::size_t total_size;
    ptr = ReadRaw(total_size, ptr);
    const uint8_t *ptr_end = ptr + total_size;
    while (ptr < ptr_end) {
      uint32_t index;
      ptr = ReadRaw(index, ptr);
      uint8_t tag;
      ptr = ReadRaw(tag, ptr);
      switch (index) {
      case 0:
        // TODO should not have 0 index
        break;
      case 1:
        // TODO unlikely tag check
        ptr = ReadRaw(out.msg, ptr);
        break;
      default: {
        ptr = SkipUnknown(tag, ptr);
      }
      }
    }
    return ptr;
  }
  static std::size_t Size(const Base& value) {
    std::size_t size = 0;
    size += FieldSize<1>(value.msg);
    size += SizeRaw(size);
    return size;
  }
};
template <> struct Coder<DerivedOne> {
  static uint8_t *Write(const DerivedOne& value, uint8_t *ptr) {
    ptr = WriteRaw(Size(value), ptr);
    ptr = WriteField(1, value.msg, ptr);
    return ptr;
  }
  static const uint8_t *Read(DerivedOne &out, const uint8_t *ptr) {
    std::size_t total_size;
    ptr = ReadRaw(total_size, ptr);
    const uint8_t *ptr_end = ptr + total_size;
    while (ptr < ptr_end) {
      uint32_t index;
      ptr = ReadRaw(index, ptr);
      uint8_t tag;
      ptr = ReadRaw(tag, ptr);
      switch (index) {
      case 0:
        // TODO should not have 0 index
        break;
      case 1:
        // TODO unlikely tag check
        ptr = ReadRaw(out.msg, ptr);
        break;
      default: {
        ptr = SkipUnknown(tag, ptr);
      }
      }
    }
    return ptr;
  }
  static std::size_t Size(const DerivedOne& value) {
    std::size_t size = 0;
    size += FieldSize<1>(value.msg);
    size += SizeRaw(size);
    return size;
  }
};
template <> struct Coder<DerivedTwo> {
  static uint8_t *Write(const DerivedTwo& value, uint8_t *ptr) {
    ptr = WriteRaw(Size(value), ptr);
    ptr = WriteField(1, value.msg, ptr);
    return ptr;
  }
  static const uint8_t *Read(DerivedTwo &out, const uint8_t *ptr) {
    std::size_t total_size;
    ptr = ReadRaw(total_size, ptr);
    const uint8_t *ptr_end = ptr + total_size;
    while (ptr < ptr_end) {
      uint32_t index;
      ptr = ReadRaw(index, ptr);
      uint8_t tag;
      ptr = ReadRaw(tag, ptr);
      switch (index) {
      case 0:
        // TODO should not have 0 index
        break;
      case 1:
        // TODO unlikely tag check
        ptr = ReadRaw(out.msg, ptr);
        break;
      default: {
        ptr = SkipUnknown(tag, ptr);
      }
      }
    }
    return ptr;
  }
  static std::size_t Size(const DerivedTwo& value) {
    std::size_t size = 0;
    size += FieldSize<1>(value.msg);
    size += SizeRaw(size);
    return size;
  }
};
} // namespace serialization
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_BASE_DERIVE_S11N_H_
