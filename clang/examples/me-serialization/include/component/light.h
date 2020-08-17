#ifndef MOMENTENGINE2_INCLUDE_COMPONENT_LIGHT_H_
#define MOMENTENGINE2_INCLUDE_COMPONENT_LIGHT_H_
#include "component.h"
#include "../alias_types.h"
namespace me {
namespace component {
class Light : public Component {
private:
  [[me::serialized()]]
  [[me::alias("string")]]
  me::alias::FVec3 color_{1.f, 1.f, 1.f};
};
} // namespace component
} // namespace me
#endif // MOMENTENGINE2_INCLUDE_COMPONENT_LIGHT_H_
