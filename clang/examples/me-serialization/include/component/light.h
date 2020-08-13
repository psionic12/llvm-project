#ifndef MOMENTENGINE2_INCLUDE_COMPONENT_LIGHT_H_
#define MOMENTENGINE2_INCLUDE_COMPONENT_LIGHT_H_
#include "component.h"
#include <glm/glm.hpp>
namespace me {
namespace component {
class Light : public Component {
private:
  glm::vec3 color_{1.f, 1.f, 1.f};
};
} // namespace component
} // namespace me
#endif // MOMENTENGINE2_INCLUDE_COMPONENT_LIGHT_H_
