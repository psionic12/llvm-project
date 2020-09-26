#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_FOO_S11N_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_FOO_S11N_H_
#include "foo.h"
#include "me/s11n/field_coder.h"
#include <cstdint>
namespace me {
namespace s11n {

template <> struct Coder<Foo> {
  static uint8_t *Write(const Foo &value, uint8_t *ptr) {
    std::size_t size = Size(value);
    if (size == 0)
      return ptr;
    ptr = WriteRaw(Size(value), ptr);
    ptr = WriteField(1, value.f1, ptr);
    ptr = WriteField(2, value.f2, ptr);
    ptr = WriteField(3, value.f3, ptr);
    ptr = WriteField(4, value.f4, ptr);
    ptr = WriteField(5, value.f5, ptr);
    ptr = WriteField(6, value.f6, ptr);
    ptr = WriteField(7, value.f7, ptr);
    ptr = WriteField(8, value.f8, ptr);
    ptr = WriteField(9, value.f9, ptr);
    ptr = WriteField(10, value.f10, ptr);
    ptr = WriteField(11, value.f11, ptr);
    ptr = WriteField(12, value.f12, ptr);
    ptr = WriteField(13, value.f13, ptr);
    ptr = WriteField(14, value.f14, ptr);
    ptr = WriteField(15, value.f15, ptr);
    //    ptr = WriteField(16, value.f16, ptr);
    return ptr;
  }
  static const uint8_t *Read(Foo &out, const uint8_t *ptr) {
    std::size_t total_size;
    const uint8_t * begin = ptr;
    ptr = ReadRaw(total_size, ptr);
    const uint8_t *ptr_end = begin + total_size;
    while (ptr < ptr_end) {
      uint32_t index;
      ptr = ReadRaw(index, ptr);
      uint8_t tag;
      ptr = ReadRaw(tag, ptr);
      switch (index) {
      case 0:

        break;
      case 1:
        // TODO unlikely tag check
        ptr = ReadRaw(out.f1, ptr);
        break;
      case 2:
        ptr = ReadRaw(out.f2, ptr);
        break;
      case 3:
        ptr = ReadRaw(out.f3, ptr);
        break;
      case 4:
        ptr = ReadRaw(out.f4, ptr);
        break;
      case 5:
        ptr = ReadRaw(out.f5, ptr);
        break;
      case 6:
        ptr = ReadRaw(out.f6, ptr);
        break;
      case 7:
        ptr = ReadRaw(out.f7, ptr);
        break;
      case 8:
        ptr = ReadRaw(out.f8, ptr);
        break;
      case 9:
        ptr = ReadRaw(out.f9, ptr);
        break;
      case 10:
        ptr = ReadRaw(out.f10, ptr);
        break;
      case 11:
        ptr = ReadRaw(out.f11, ptr);
        break;
      case 12:
        ptr = ReadRaw(out.f12, ptr);
        break;
      case 13:
        ptr = ReadRaw(out.f13, ptr);
        break;
      case 14:
        ptr = ReadRaw(out.f14, ptr);
        break;
        // skip test
        //      case 15:
        //        ptr = ReadRaw(out.f15, ptr);
        //        break;
        //      case 16:
        //        ptr = ReadRaw(out.f16, ptr);
        //        break;
      default: {
        ptr = SkipUnknown(tag, ptr);
      }
      }
    }
    return ptr;
  }
  static std::size_t Size(const Foo &value) {
    std::size_t size = 0;
    size += FieldSize<1>(value.f1);
    size += FieldSize<2>(value.f2);
    size += FieldSize<3>(value.f3);
    size += FieldSize<4>(value.f4);
    size += FieldSize<5>(value.f5);
    size += FieldSize<6>(value.f6);
    size += FieldSize<7>(value.f7);
    size += FieldSize<8>(value.f8);
    size += FieldSize<9>(value.f9);
    size += FieldSize<10>(value.f10);
    size += FieldSize<11>(value.f11);
    size += FieldSize<12>(value.f12);
    size += FieldSize<13>(value.f13);
    size += FieldSize<14>(value.f14);
    size += FieldSize<15>(value.f15);
    //    size += FieldSize<16>(value.f16);
    size += SizeRaw(size);
    return size;
  }
};
} // namespace s11n
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_TEST_FOO_S11N_H_
