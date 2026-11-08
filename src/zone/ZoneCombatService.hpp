#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "zone/MovementProtocol.hpp"

namespace Umbra {
namespace Database {
class MySQLConnector;
}
namespace Zone {

class MovementServer;

struct RespawnResult {
  bool success = false;
  std::string error;
  PlayerRespawnPayload payload;
};

class ZoneCombatService {
 public:
  explicit ZoneCombatService(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId);

  /** memorySaysDead: Zone já marcou isDead (vitals 0) — não depender do write-behind do MySQL.
   *  preferredMaxHealth/Mana: HP/MP buffados (equip/classe); 0 = fallback na coluna players.max_*. */
  RespawnResult processRespawn(uint32_t playerId, uint32_t zoneId, const std::string& spawnKey,
                               bool memorySaysDead = false, int32_t preferredMaxHealth = 0,
                               int32_t preferredMaxMana = 0);
  void tickActiveDots(MovementServer* movementServer);

 private:
  std::shared_ptr<Database::MySQLConnector> db_;
  uint32_t zoneId_;
  float dotAccumulator_ = 0.0f;
  /** Backoff: se o último poll de active_dots veio vazio, não reconsultar a cada 250ms. */
  std::chrono::steady_clock::time_point nextDotsPollAt_{};

  bool applyVitalsInDb(uint32_t sourcePlayerId, uint32_t targetPlayerId,
                       int32_t deltaHealth, int32_t deltaMana,
                       int32_t& outNewHealth, int32_t& outMaxHealth,
                       int32_t& outNewMana, int32_t& outMaxMana,
                       bool& outIsDead);
};

}  // namespace Zone
}  // namespace Umbra
