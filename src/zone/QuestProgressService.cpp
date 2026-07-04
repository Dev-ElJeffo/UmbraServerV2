#include "zone/QuestProgressService.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace Zone {

QuestProgressService::QuestProgressService(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId)
    : db_(std::move(db)), zoneId_(zoneId) {}

void QuestProgressService::onNpcKilled(uint32_t killerPlayerId, uint32_t npcInstanceId,
                                       uint32_t npcTemplateId, uint32_t zoneId) {
  if (!db_ || !db_->isConnected() || killerPlayerId == 0) {
    return;
  }
  if (npcTemplateId == 0 && npcInstanceId == 0) {
    return;
  }
  if (zoneId > 0 && zoneId_ > 0 && zoneId != zoneId_ && zoneId != 0) {
    Core::Logger::getInstance().debug(
        "[QuestProgressService] kill ignorado por zone mismatch player={} npcInstance={} npcZone={} serverZone={}",
        killerPlayerId, npcInstanceId, zoneId, zoneId_);
    return;
  }

  const std::string playerStr = std::to_string(killerPlayerId);
  const std::string templateStr = std::to_string(npcTemplateId);
  const std::string instanceStr = std::to_string(npcInstanceId);

  const std::string updateSql = R"(
    UPDATE player_quest_objectives pqo
    INNER JOIN player_quests pq ON pq.player_quest_id = pqo.player_quest_id
      AND pq.player_id = ? AND pq.status = 'active'
    INNER JOIN quest_objectives qo ON qo.objective_id = pqo.objective_id
      AND qo.objective_type = 'kill'
    SET
      pqo.current_count = LEAST(
        pqo.current_count + 1,
        GREATEST(1, CAST(JSON_UNQUOTE(JSON_EXTRACT(qo.params_json, '$.required_count')) AS SIGNED))
      ),
      pqo.is_completed = IF(
        pqo.current_count + 1 >= GREATEST(1, CAST(JSON_UNQUOTE(JSON_EXTRACT(qo.params_json, '$.required_count')) AS SIGNED)),
        1,
        pqo.is_completed
      )
    WHERE pqo.is_completed = 0
      AND (
        (? > 0 AND CAST(JSON_UNQUOTE(JSON_EXTRACT(qo.params_json, '$.npc_template_id')) AS UNSIGNED) = ?)
        OR (? > 0 AND CAST(JSON_UNQUOTE(JSON_EXTRACT(qo.params_json, '$.npc_instance_id')) AS UNSIGNED) = ?)
      )
  )";

  if (!db_->executePreparedInsert(updateSql,
                                  {playerStr, templateStr, templateStr, instanceStr, instanceStr})) {
    Core::Logger::getInstance().warn(
        "[QuestProgressService] falha SQL kill player={} npcInstance={} npcTemplate={}", killerPlayerId,
        npcInstanceId, npcTemplateId);
    return;
  }

  const auto checkRows = db_->executePreparedQuery(
      R"(
        SELECT pqo.current_count, pqo.is_completed, q.quest_id
        FROM player_quest_objectives pqo
        INNER JOIN player_quests pq ON pq.player_quest_id = pqo.player_quest_id
          AND pq.player_id = ? AND pq.status IN ('active', 'ready')
        INNER JOIN quest_objectives qo ON qo.objective_id = pqo.objective_id
          AND qo.objective_type = 'kill'
        INNER JOIN quests q ON q.quest_id = pq.quest_id
        WHERE (
          (? > 0 AND CAST(JSON_UNQUOTE(JSON_EXTRACT(qo.params_json, '$.npc_template_id')) AS UNSIGNED) = ?)
          OR (? > 0 AND CAST(JSON_UNQUOTE(JSON_EXTRACT(qo.params_json, '$.npc_instance_id')) AS UNSIGNED) = ?)
        )
        ORDER BY pqo.is_completed DESC, pqo.current_count DESC
        LIMIT 1
      )",
      {playerStr, templateStr, templateStr, instanceStr, instanceStr});

  if (checkRows.empty()) {
    Core::Logger::getInstance().warn(
        "[QuestProgressService] kill sem objetivo ativo player={} npcInstance={} npcTemplate={} "
        "(quest não aceita, objetivo com template/instance errado, ou já completo)",
        killerPlayerId, npcInstanceId, npcTemplateId);
    return;
  }

  promoteReadyQuests(killerPlayerId);
  Core::Logger::getInstance().info(
      "[QuestProgressService] kill creditado player={} npcInstance={} npcTemplate={} count={} completed={}",
      killerPlayerId, npcInstanceId, npcTemplateId, checkRows[0][0], checkRows[0][1]);
}

void QuestProgressService::promoteReadyQuests(uint32_t playerId) {
  const std::string playerStr = std::to_string(playerId);
  const std::string promoteSql = R"(
    UPDATE player_quests pq
    SET pq.status = 'ready'
    WHERE pq.player_id = ?
      AND pq.status = 'active'
      AND NOT EXISTS (
        SELECT 1 FROM player_quest_objectives pqo
        WHERE pqo.player_quest_id = pq.player_quest_id AND pqo.is_completed = 0
      )
  )";
  db_->executePreparedInsert(promoteSql, {playerStr});
}

}  // namespace Zone
}  // namespace Umbra
