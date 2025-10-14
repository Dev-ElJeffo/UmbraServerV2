#pragma once

#include <map>
#include <mutex>
#include <cstdint>
#include <string>

namespace Umbra {
namespace Zone {

struct Entity {
  uint64_t id;
  std::string type;
  float posX, posY, posZ;
  uint32_t health;
  bool active = true;
};

class EntitySystem {
 public:
  uint64_t spawnEntity(const Entity& entity);
  bool despawnEntity(uint64_t entityId);
  void update(float deltaTime);
  
  size_t getEntityCount() const;

 private:
  std::map<uint64_t, Entity> entities_;
  std::atomic<uint64_t> nextEntityId_;
  mutable std::mutex mutex_;
};

}  // namespace Zone
}  // namespace Umbra

