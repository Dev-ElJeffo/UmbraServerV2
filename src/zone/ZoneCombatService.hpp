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
  // DOT/HOT de players rodam em memória no CombatCoreEngine (active_dots é só write-behind).

 private:
  std::shared_ptr<Database::MySQLConnector> db_;
  uint32_t zoneId_;
};

}  // namespace Zone
}  // namespace Umbra
