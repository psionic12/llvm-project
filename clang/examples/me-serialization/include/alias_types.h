#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_INCLUDE_ALIAS_TYPES_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_INCLUDE_ALIAS_TYPES_H_
#include <glm/glm.hpp>
namespace me {
namespace alias {
struct FVec3 : public glm::vec3 {
  [[me::serialized()]] float &x() { return glm::vec3::x; }
  [[me::serialized()]] float &y() { return glm::vec3::y; }
  [[me::serialized()]] float &z() { return glm::vec3::z; }
};
} // namespace alias
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_INCLUDE_ALIAS_TYPES_H_
