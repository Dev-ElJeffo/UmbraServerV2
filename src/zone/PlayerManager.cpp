#include "PlayerManager.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace Zone {

PlayerManager::PlayerManager(uint32_t maxPlayers)
    : maxPlayers_(maxPlayers) {
}

bool PlayerManager::addPlayer(const Database::Player& player) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (players_.size() >= maxPlayers_) {
    return false;
  }
  
  PlayerState state;
  state.playerId = player.id;
  state.posX = player.posX;
  state.posY = player.posY;
  state.posZ = player.posZ;
  state.health = player.health;
  
  players_[player.id] = state;
  
  Core::Logger::getInstance().info("Player {} joined zone", player.id);
  return true;
}

bool PlayerManager::removePlayer(uint64_t playerId) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = players_.find(playerId);
  if (it != players_.end()) {
    players_.erase(it);
    Core::Logger::getInstance().info("Player {} left zone", playerId);
    return true;
  }
  
  return false;
}

void PlayerManager::update(float deltaTime) {
  std::lock_guard<std::mutex> lock(mutex_);
  // TODO: Update player states
}

bool PlayerManager::updatePosition(uint64_t playerId, float x, float y, float z) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = players_.find(playerId);
  if (it != players_.end()) {
    it->second.posX = x;
    it->second.posY = y;
    it->second.posZ = z;
    return true;
  }
  
  return false;
}

uint32_t PlayerManager::getPlayerCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return static_cast<uint32_t>(players_.size());
}

}  // namespace Zone
}  // namespace Umbra

