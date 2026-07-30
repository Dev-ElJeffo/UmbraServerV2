#pragma once

#include "database/MySQLConnector.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace Umbra {
namespace Zone {

class MovementServer;

class QuestProgressService {
public:
  explicit QuestProgressService(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId);

  void setMovementServer(MovementServer* movement) { movementServer_ = movement; }
  void setResolvePartyMembers(std::function<std::vector<uint32_t>(uint32_t)> cb) {
    resolvePartyMembers_ = std::move(cb);
  }
  void setShareRadiusUu(float radiusUu) { shareRadiusUu_ = radiusUu; }

  /** Incrementa progresso de kill (party in-range) quando um NPC morre na zone. */
  void onNpcKilled(uint32_t killerPlayerId, uint32_t npcInstanceId, uint32_t npcTemplateId,
                   uint32_t zoneId);

private:
  void creditKillForPlayer(uint32_t playerId, uint32_t npcInstanceId, uint32_t npcTemplateId);
  void promoteReadyQuests(uint32_t playerId);

  std::shared_ptr<Database::MySQLConnector> db_;
  uint32_t zoneId_ = 0;
  MovementServer* movementServer_ = nullptr;
  std::function<std::vector<uint32_t>(uint32_t)> resolvePartyMembers_;
  float shareRadiusUu_ = 5000.f;
};

}  // namespace Zone
}  // namespace Umbra
