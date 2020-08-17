#ifndef LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_INCLUDE_ALIAS_TYPES_H_
#define LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_INCLUDE_ALIAS_TYPES_H_
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
namespace me {
namespace alias {
struct FVec3 : public glm::vec3 {
  FVec3(float x, float y, float z) : glm::vec3(x, y, z) {}
  [[me::serialized()]] float &x() { return glm::vec3::x; }
  [[me::serialized()]] float &y() { return glm::vec3::y; }
  [[me::serialized()]] float &z() { return glm::vec3::z; }
};
struct FQuat : public glm::quat {
  FQuat(float x, float y, float z, float w) : glm::quat(x, y, z, w) {}
  [[me::serialized()]] float &x() { return glm::quat::x; }
  [[me::serialized()]] float &y() { return glm::quat::y; }
  [[me::serialized()]] float &z() { return glm::quat::z; }
  [[me::serialized()]] float &w() { return glm::quat::w; }
};
} // namespace alias
} // namespace me
#endif // LLVM_CLANG_TOOLS_ME_SERIALIZATION_EXAMPLE_INCLUDE_ALIAS_TYPES_H_
