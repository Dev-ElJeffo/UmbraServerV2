#pragma once

#include "database/Models.hpp"
#include <map>
#include <mutex>
#include <cstdint>

namespace Umbra {
namespace Zone {

struct PlayerState {
  uint64_t playerId;
  float posX, posY, posZ;
  float rotX, rotY, rotZ;
  uint32_t health;
  bool isDead = false;
};

class PlayerManager {
 public:
  explicit PlayerManager(uint32_t maxPlayers);
  
  bool addPlayer(const Database::Player& player);
  bool removePlayer(uint64_t playerId);
  void update(float deltaTime);
  
  bool updatePosition(uint64_t playerId, float x, float y, float z);
  uint32_t getPlayerCount() const;

 private:
  uint32_t maxPlayers_;
  std::map<uint64_t, PlayerState> players_;
  mutable std::mutex mutex_;
};

}  // namespace Zone
}  // namespace Umbra

