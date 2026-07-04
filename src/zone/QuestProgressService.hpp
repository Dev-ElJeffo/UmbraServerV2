#pragma once

#include "database/MySQLConnector.hpp"
#include <cstdint>
#include <memory>

namespace Umbra {
namespace Zone {

class QuestProgressService {
public:
  explicit QuestProgressService(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId);

  /** Incrementa progresso de kill quando um NPC morre na zone. */
  void onNpcKilled(uint32_t killerPlayerId, uint32_t npcInstanceId, uint32_t npcTemplateId,
                   uint32_t zoneId);

private:
  void promoteReadyQuests(uint32_t playerId);

  std::shared_ptr<Database::MySQLConnector> db_;
  uint32_t zoneId_ = 0;
};

}  // namespace Zone
}  // namespace Umbra
