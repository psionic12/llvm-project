#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMMON_RECORDINFO_MES11N_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMMON_RECORDINFO_MES11N_H_
#include "RecordInfo.h"
#include <me/s11n/field_coder.h>
template <> struct me::s11n::TypeCoder<EntryInfo> {
  static uint8_t *Write(const EntryInfo &value, uint8_t *ptr) {
    ptr = Encode(SizeCache<>::Get(value), ptr);
    ptr = EncodeField(0, value.EntryName, ptr);
    ptr = EncodeField(1, value.EntryID, ptr);
    ptr = EncodeField(2, value.IsNew, ptr);
    return ptr;
  }
  static const uint8_t *Read(EntryInfo &out, const uint8_t *ptr) {
    std::size_t total_size;
    const uint8_t * begin = ptr;
    ptr = Decode(total_size, ptr);
    const uint8_t *ptr_end = begin + total_size;
    while (ptr < ptr_end) {
      uint32_t index;
      ptr = Decode(index, ptr);
      uint8_t tag;
      ptr = Decode(tag, ptr);
      switch (index) {
      case 0:
        ptr = Decode(out.EntryName, ptr);
        break;
      case 1:
        ptr = Decode(out.EntryID, ptr);
        break;
      case 2:
        ptr = Decode(out.IsNew, ptr);
        break;
      default: {
        ptr = SkipUnknownField(tag, ptr);
      }
      }
    }
    return ptr;
  }
  static std::size_t PayloadSize(const EntryInfo &value) {
    std::size_t payload_size = 0;
    payload_size += FieldCapacity<0>(value.EntryName);
    payload_size += FieldCapacity<1>(value.EntryID);
    payload_size += FieldCapacity<2>(value.IsNew);
    SizeCache<>::Set(value, payload_size);
    return payload_size;
  }
  static std::size_t Size(const EntryInfo &value) {
    auto size = PayloadSize(value);
    size += Capacity(size);
    return size;
  }
};
template <> struct me::s11n::TypeCoder<RecordInfo> {
  static uint8_t *Write(const RecordInfo &value, uint8_t *ptr) {
    ptr = Encode(SizeCache<>::Get(value), ptr);
    ptr = EncodeField(0, value.HasID, ptr);
    ptr = EncodeField(1, value.FullName, ptr);
    ptr = EncodeField(2, value.Entries, ptr);
    ptr = EncodeField(3, value.RecordID, ptr);
    ptr = EncodeField(4, value.Polymorphic, ptr);
    ptr = EncodeField(5, value.IsNew, ptr);
    return ptr;
  }
  static const uint8_t *Read(RecordInfo &out, const uint8_t *ptr) {
    std::size_t total_size;
    const uint8_t * begin = ptr;
    ptr = Decode(total_size, ptr);
    const uint8_t *ptr_end = begin + total_size;
    while (ptr < ptr_end) {
      uint32_t index;
      ptr = Decode(index, ptr);
      uint8_t tag;
      ptr = Decode(tag, ptr);
      switch (index) {
      case 0:
        ptr = Decode(out.HasID, ptr);
        break;
      case 1:
        ptr = Decode(out.FullName, ptr);
        break;
      case 2:
        ptr = Decode(out.Entries, ptr);
        break;
      case 3:
        ptr = Decode(out.RecordID, ptr);
        break;
      case 4:
        ptr = Decode(out.Polymorphic, ptr);
        break;
      case 5:
        ptr = Decode(out.IsNew, ptr);
        break;
      default: {
        ptr = SkipUnknownField(tag, ptr);
      }
      }
    }
    return ptr;
  }
  static std::size_t PayloadSize(const RecordInfo &value) {
    std::size_t payload_size = 0;
    payload_size += FieldCapacity<0>(value.HasID);
    payload_size += FieldCapacity<1>(value.FullName);
    payload_size += FieldCapacity<2>(value.Entries);
    payload_size += FieldCapacity<3>(value.RecordID);
    payload_size += FieldCapacity<4>(value.Polymorphic);
    payload_size += FieldCapacity<5>(value.IsNew);
    SizeCache<>::Set(value, payload_size);
    return payload_size;
  }
  static std::size_t Size(const RecordInfo &value) {
    auto size = PayloadSize(value);
    size += Capacity(size);
    return size;
  }
};
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SRC_COMMON_RECORDINFO_MES11N_H_
