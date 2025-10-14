#include "EntitySystem.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace Zone {

uint64_t EntitySystem::spawnEntity(const Entity& entity) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  uint64_t id = ++nextEntityId_;
  Entity newEntity = entity;
  newEntity.id = id;
  
  entities_[id] = newEntity;
  
  Core::Logger::getInstance().debug("Spawned entity {} of type {}", id, entity.type);
  return id;
}

bool EntitySystem::despawnEntity(uint64_t entityId) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = entities_.find(entityId);
  if (it != entities_.end()) {
    entities_.erase(it);
    Core::Logger::getInstance().debug("Despawned entity {}", entityId);
    return true;
  }
  
  return false;
}

void EntitySystem::update(float deltaTime) {
  std::lock_guard<std::mutex> lock(mutex_);
  // TODO: Update entity AI and behaviors
}

size_t EntitySystem::getEntityCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return entities_.size();
}

}  // namespace Zone
}  // namespace Umbra

