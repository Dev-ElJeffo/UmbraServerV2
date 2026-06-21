#include "zone/ZoneCombatService.hpp"
#include "zone/MovementServer.hpp"
#include "database/MySQLConnector.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <cmath>

namespace Umbra {
namespace Zone {

namespace {

uint8_t dotTypeToByte(const std::string& dotType) {
  if (dotType == "HEAL") return 2;
  if (dotType == "MANA") return 3;
  return 1;
}

}  // namespace

ZoneCombatService::ZoneCombatService(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId)
    : db_(std::move(db)), zoneId_(zoneId) {}

bool ZoneCombatService::applyVitalsInDb(uint32_t sourcePlayerId, uint32_t targetPlayerId,
                                        int32_t deltaHealth, int32_t deltaMana,
                                        int32_t& outNewHealth, int32_t& outMaxHealth,
                                        int32_t& outNewMana, int32_t& outMaxMana,
                                        bool& outIsDead) {
  if (!db_ || !db_->isConnected()) return false;

  const std::string tid = std::to_string(targetPlayerId);
  auto healthOpt = db_->executePreparedScalar("SELECT health FROM players WHERE id = ? LIMIT 1", {tid});
  auto manaOpt = db_->executePreparedScalar("SELECT mana FROM players WHERE id = ? LIMIT 1", {tid});
  if (!healthOpt || !manaOpt) return false;

  int32_t curHealth = std::stoi(*healthOpt);
  int32_t curMana = std::stoi(*manaOpt);
  outMaxHealth = std::max(1, curHealth);
  outMaxMana = std::max(1, curMana);

  auto maxHOpt = db_->executePreparedScalar(
      "SELECT COALESCE(MAX(health), 100) FROM players WHERE id = ?", {tid});
  auto maxMOpt = db_->executePreparedScalar(
      "SELECT COALESCE(MAX(mana), 50) FROM players WHERE id = ?", {tid});
  if (maxHOpt && !maxHOpt->empty()) {
    try { outMaxHealth = std::max(1, std::stoi(*maxHOpt)); } catch (...) {}
  }
  if (maxMOpt && !maxMOpt->empty()) {
    try { outMaxMana = std::max(1, std::stoi(*maxMOpt)); } catch (...) {}
  }

  outNewHealth = std::max(0, std::min(outMaxHealth, curHealth + deltaHealth));
  outNewMana = std::max(0, std::min(outMaxMana, curMana + deltaMana));
  outIsDead = (outNewHealth <= 0);

  if (outIsDead) {
    db_->executePreparedInsert(
        "UPDATE players SET health = ?, mana = ?, is_dead = 1, last_death_at = CURRENT_TIMESTAMP WHERE id = ?",
        {std::to_string(outNewHealth), std::to_string(outNewMana), tid});
  } else {
    db_->executePreparedInsert(
        "UPDATE players SET health = ?, mana = ?, is_dead = 0 WHERE id = ?",
        {std::to_string(outNewHealth), std::to_string(outNewMana), tid});
  }

  (void)sourcePlayerId;
  return true;
}

RespawnResult ZoneCombatService::processRespawn(uint32_t playerId, uint32_t zoneId,
                                                const std::string& spawnKey) {
  RespawnResult result;
  if (!db_ || !db_->isConnected()) {
    result.error = "database_unavailable";
    return result;
  }

  const std::string pid = std::to_string(playerId);
  auto deadOpt = db_->executePreparedScalar("SELECT is_dead FROM players WHERE id = ? LIMIT 1", {pid});
  auto healthOpt = db_->executePreparedScalar("SELECT health FROM players WHERE id = ? LIMIT 1", {pid});
  if (!deadOpt || !healthOpt) {
    result.error = "player_not_found";
    return result;
  }

  const bool isDead = (std::stoi(*deadOpt) != 0);
  const int health = std::stoi(*healthOpt);
  if (!isDead && health > 0) {
    result.error = "NOT_DEAD";
    return result;
  }

  if (zoneId == 0) zoneId = zoneId_;

  std::string key = spawnKey.empty() ? "cidade_inicial" : spawnKey;
  auto rows = db_->executePreparedQuery(
      "SELECT spawn_key, pos_x, pos_y, pos_z, yaw FROM spawn_points "
      "WHERE zone_id = ? AND (spawn_key = ? OR is_default = 1) "
      "ORDER BY (spawn_key = ?) DESC, is_default DESC LIMIT 1",
      {std::to_string(zoneId), key, key});

  float px = 0.f, py = 0.f, pz = 200.f, yaw = 0.f;
  if (!rows.empty() && rows[0].size() >= 5) {
    try {
      px = std::stof(rows[0][1]);
      py = std::stof(rows[0][2]);
      pz = std::stof(rows[0][3]);
      yaw = std::stof(rows[0][4]);
    } catch (...) {}
  }

  auto maxHOpt = db_->executePreparedScalar("SELECT max_health FROM players WHERE id = ?", {pid});
  auto maxMOpt = db_->executePreparedScalar("SELECT max_mana FROM players WHERE id = ?", {pid});
  int32_t maxHealth = maxHOpt ? std::max(1, std::stoi(*maxHOpt)) : 100;
  int32_t maxMana = maxMOpt ? std::max(1, std::stoi(*maxMOpt)) : 50;

  db_->executePreparedInsert(
      "UPDATE players SET health = ?, mana = ?, is_dead = 0, pos_x = ?, pos_y = ?, pos_z = ?, current_zone = ? WHERE id = ?",
      {std::to_string(maxHealth), std::to_string(maxMana),
       std::to_string(px), std::to_string(py), std::to_string(pz),
       std::to_string(zoneId), pid});

  result.success = true;
  result.payload.playerId = playerId;
  result.payload.x = px;
  result.payload.y = py;
  result.payload.z = pz;
  result.payload.yaw = yaw;
  result.payload.currentHealth = maxHealth;
  result.payload.maxHealth = maxHealth;
  result.payload.currentMana = maxMana;
  result.payload.maxMana = maxMana;
  return result;
}

void ZoneCombatService::tickActiveDots(MovementServer* movementServer) {
  if (!db_ || !db_->isConnected() || !movementServer) return;

  auto rows = db_->executePreparedQuery(
      "SELECT dot_id, target_player_id, source_player_id, dot_type, tick_value, tick_interval_ms, ticks_remaining "
      "FROM active_dots WHERE next_tick_at <= NOW(3) LIMIT 50",
      {});

  for (const auto& row : rows) {
    if (row.size() < 7) continue;

    uint64_t dotId = 0;
    uint32_t targetId = 0;
    uint32_t sourceId = 0;
    std::string dotType;
    int32_t tickValue = 0;
    int tickIntervalMs = 1000;
    int ticksRemaining = 0;
    try {
      dotId = std::stoull(row[0]);
      targetId = static_cast<uint32_t>(std::stoul(row[1]));
      if (!row[2].empty()) sourceId = static_cast<uint32_t>(std::stoul(row[2]));
      dotType = row[3];
      tickValue = std::stoi(row[4]);
      tickIntervalMs = std::stoi(row[5]);
      ticksRemaining = std::stoi(row[6]);
    } catch (...) {
      continue;
    }

    int32_t delta = tickValue;
    int32_t deltaMana = 0;
    if (dotType == "HEAL") {
      delta = tickValue;
    } else if (dotType == "MANA") {
      delta = 0;
      deltaMana = tickValue;
    } else {
      delta = -tickValue;
    }

    int32_t newHp = 0, maxHp = 1, newMp = 0, maxMp = 1;
    bool isDead = false;
    if (!applyVitalsInDb(sourceId, targetId, delta, deltaMana, newHp, maxHp, newMp, maxMp, isDead)) {
      continue;
    }

    PlayerVitalsPayload vitals;
    vitals.playerId = targetId;
    vitals.currentHealth = newHp;
    vitals.maxHealth = maxHp;
    vitals.currentMana = newMp;
    vitals.maxMana = maxMp;
    vitals.sourcePlayerId = sourceId;
    vitals.reason = static_cast<uint8_t>(CombatReason::Dot);
    movementServer->broadcastVitalsAndCombat(targetId, vitals, sourceId, delta, isDead);

    DotTickPayload dotPayload;
    dotPayload.targetId = targetId;
    dotPayload.dotId = dotId;
    dotPayload.delta = delta != 0 ? delta : deltaMana;
    dotPayload.dotType = dotTypeToByte(dotType);
    movementServer->broadcastDotTick(targetId, dotPayload);

    --ticksRemaining;
    const std::string dotIdStr = std::to_string(dotId);
    if (ticksRemaining <= 0) {
      db_->executePreparedInsert("DELETE FROM active_dots WHERE dot_id = ?", {dotIdStr});
    } else {
      db_->executePreparedInsert(
          "UPDATE active_dots SET ticks_remaining = ?, next_tick_at = DATE_ADD(NOW(3), INTERVAL ? MILLISECOND) WHERE dot_id = ?",
          {std::to_string(ticksRemaining), std::to_string(tickIntervalMs), dotIdStr});
    }
  }
}

}  // namespace Zone
}  // namespace Umbra
