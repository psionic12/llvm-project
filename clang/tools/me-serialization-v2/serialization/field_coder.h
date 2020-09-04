#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_FIELD_CODER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_FIELD_CODER_H_
#include "built_in_type_coder.h"
#include "built_in_types_helper.h"
#include <cstdint>
namespace me {
namespace serialization {
constexpr inline uint32_t MakeTag(int field_number, WireType type) {
  return static_cast<uint32_t>((static_cast<uint32_t>(field_number) << 3) |
                               static_cast<uint32_t>(type));
}
inline uint8_t *WriteTagToArray(int field_number, WireType type,
                                uint8_t *target) {
  auto tag = MakeTag(field_number, type);
  return Coder<decltype(tag)>::Write(tag, target);
}
} // namespace serialization
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_FIELD_CODER_H_
