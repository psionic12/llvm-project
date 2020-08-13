#ifndef MOMENTENGINE2_INCLUDE_SCENE_H_
#define MOMENTENGINE2_INCLUDE_SCENE_H_
#include "entity.h"
namespace me {
class Scene {
private:
  [[me::serialized("prefab")]][[me::list("string", "begin", "emplace_back")]]
  std::vector<Entity> entities_;
};
} // namespace me
#endif // MOMENTENGINE2_INCLUDE_SCENE_H_
