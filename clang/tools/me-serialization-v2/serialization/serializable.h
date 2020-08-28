#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_SERIALIZABLE_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_SERIALIZABLE_H_
#include "built_in_types_helper.h"
#include <vector>
namespace me {
namespace serialization {
class Serializable {
public:
  virtual std::vector<char> Serialize() = 0;
};

} // namespace serialization
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_V2_SERIALIZATION_SERIALIZABLE_H_
