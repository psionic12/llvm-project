#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_FIELD_CODER_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_FIELD_CODER_H_
#include "built_in_type_coder.h"
#include "built_in_types_helper.h"
#include <cstdint>
namespace me {
namespace serialization {
constexpr inline uint8_t MakeTag(bool rtti, Graininess type) {
  uint8_t tag = static_cast<uint8_t>(type);
  tag |= static_cast<uint8_t>(rtti) << 7;
  return tag;
}
} // namespace serialization
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_FIELD_CODER_H_
