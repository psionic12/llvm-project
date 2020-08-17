#ifndef MOMENTENGINE2_INCLUDE_COMPONENT_TRANSFORM_H_
#define MOMENTENGINE2_INCLUDE_COMPONENT_TRANSFORM_H_
#include "component.h"
#include "../alias_types.h"
namespace me {
namespace component {
class Transform : public Component {
public:
  [[me::serialized()]][[me::alias("string")]]me::alias::FVec3 position;
  [[me::serialized()]][[me::alias("string")]]me::alias::FQuat rotation;
  [[me::serialized()]][[me::alias("string")]]me::alias::FVec3 scale;
};
} // namespace component
} // namespace me
#endif // MOMENTENGINE2_INCLUDE_COMPONENT_TRANSFORM_H_
