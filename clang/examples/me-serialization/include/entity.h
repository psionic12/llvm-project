#ifndef MOMENTENGINE2_INCLUDE_ENTITY_H_
#define MOMENTENGINE2_INCLUDE_ENTITY_H_
#include <memory>
#include <vector>
#include "component/component.h"
namespace me {
class Entity {
private:
  [[me::serialized("prefab")]][[me::list("string", "component::Component", "begin", "emplace_back")]]
  std::vector<component::Component> components_;
};
}//namespace me
#endif //MOMENTENGINE2_INCLUDE_ENTITY_H_
