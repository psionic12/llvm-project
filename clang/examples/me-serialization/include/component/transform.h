#ifndef MOMENTENGINE2_INCLUDE_COMPONENT_TRANSFORM_H_
#define MOMENTENGINE2_INCLUDE_COMPONENT_TRANSFORM_H_
#include "component.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
namespace me {
namespace component {
class Transform : public Component {
public:
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;
};
} // namespace component
} // namespace me
#endif // MOMENTENGINE2_INCLUDE_COMPONENT_TRANSFORM_H_
