#ifndef MOMENTENGINE2_INCLUDE_COMPONENT_CAMERA_H_
#define MOMENTENGINE2_INCLUDE_COMPONENT_CAMERA_H_
#include "component.h"
namespace me {
namespace component {
class Camera : public Component {
public:
  [[me::serialized("prefab")]]
  float fov_ = 45.0f;
};
} // namespace component
} // namespace me
#endif // MOMENTENGINE2_INCLUDE_COMPONENT_CAMERA_H_
