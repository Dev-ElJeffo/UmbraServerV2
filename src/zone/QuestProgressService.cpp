#include "zone/QuestProgressService.hpp"
#include "zone/PartyShare.hpp"
#include "core/Logger.hpp"
#include <sstream>
#include <string>

namespace Umbra {
namespace Zone {

QuestProgressService::QuestProgressService(std::shared_ptr<Database::MySQLConnector> db,
                                           uint32_t zoneId)
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
        "[QuestProgressService] kill ignorado por zone mismatch player={} npcInstance={} "
        "npcZone={} serverZone={}",
        killerPlayerId, npcInstanceId, zoneId, zoneId_);
    return;
  }

  const auto recipients = collectPartyShareRecipients(killerPlayerId, shareRadiusUu_,
                                                      movementServer_, resolvePartyMembers_);
  std::ostringstream ids;
  for (size_t i = 0; i < recipients.size(); ++i) {
    if (i) ids << ',';
    ids << recipients[i];
    creditKillForPlayer(recipients[i], npcInstanceId, npcTemplateId);
  }
  Core::Logger::getInstance().info(
      "[QuestProgressService] quest kill share killer={} npcInstance={} npcTemplate={} "
      "recipients=[{}] radius={:.0f}",
      killerPlayerId, npcInstanceId, npcTemplateId, ids.str(), shareRadiusUu_);
}

void QuestProgressService::creditKillForPlayer(uint32_t playerId, uint32_t npcInstanceId,
                                               uint32_t npcTemplateId) {
  if (playerId == 0) return;

  const std::string playerStr = std::to_string(playerId);
  const std::string templateStr = std::to_string(npcTemplateId);
  const std::string instanceStr = std::to_string(npcInstanceId);

  const std::string updateSql = R"(
    UPDATE player_quest_objectives pqo
    INNER JOIN player_quests pq ON pq.player_quest_id = pqo.player_quest_id
      AND pq.player_id = ? AND pq.status = 'active'
    INNER JOIN quest_objectives qo ON qo.objective_id = pqo.objective_id
      AND qo.objective_type = 'kill'
    SET
      pqo.is_completed = IF(
        pqo.current_count + 1 >= GREATEST(1, CAST(JSON_UNQUOTE(JSON_EXTRACT(qo.params_json, '$.required_count')) AS SIGNED)),
        1,
        0
      ),
      pqo.current_count = LEAST(
        pqo.current_count + 1,
        GREATEST(1, CAST(JSON_UNQUOTE(JSON_EXTRACT(qo.params_json, '$.required_count')) AS SIGNED))
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
        "[QuestProgressService] falha SQL kill player={} npcInstance={} npcTemplate={}", playerId,
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
    return;
  }

  promoteReadyQuests(playerId);
  Core::Logger::getInstance().info(
      "[QuestProgressService] kill creditado player={} npcInstance={} npcTemplate={} count={} "
      "completed={}",
      playerId, npcInstanceId, npcTemplateId, checkRows[0][0], checkRows[0][1]);
}

void QuestProgressService::promoteReadyQuests(uint32_t playerId) {
  const std::string playerStr = std::to_string(playerId);
  const std::string promoteSql = R"(
    UPDATE player_quests pq
    SET pq.status = 'ready'
    WHERE pq.player_id = ?
      AND pq.status = 'active'
      AND EXISTS (
        SELECT 1 FROM player_quest_objectives pqo
        WHERE pqo.player_quest_id = pq.player_quest_id
      )
      AND NOT EXISTS (
        SELECT 1 FROM player_quest_objectives pqo
        WHERE pqo.player_quest_id = pq.player_quest_id AND pqo.is_completed = 0
      )
      AND NOT EXISTS (
        SELECT 1 FROM quest_objectives qo
        WHERE qo.quest_id = pq.quest_id
          AND qo.objective_id NOT IN (
            SELECT pqo2.objective_id FROM player_quest_objectives pqo2
            WHERE pqo2.player_quest_id = pq.player_quest_id
          )
      )
  )";
  db_->executePreparedInsert(promoteSql, {playerStr});
}

}  // namespace Zone
}  // namespace Umbra
