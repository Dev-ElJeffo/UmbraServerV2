#include "zone/ZoneCombatService.hpp"
#include "zone/MovementServer.hpp"
#include "zone/AgentDebugLog.hpp"
#include "database/MySQLConnector.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <chrono>
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
                                                const std::string& spawnKey,
                                                bool memorySaysDead, int32_t preferredMaxHealth,
                                                int32_t preferredMaxMana) {
  RespawnResult result;
  if (!db_ || !db_->isConnected()) {
    result.error = "database_unavailable";
    // #region agent log
    agentDebugLog("H-RESPAWN", "ZoneCombatService.cpp:processRespawn", "respawn_fail",
                  std::string("{\"playerId\":") + std::to_string(playerId) +
                      ",\"error\":\"database_unavailable\",\"memorySaysDead\":" +
                      (memorySaysDead ? "true" : "false") + "}",
                  "post-fix");
    // #endregion
    return result;
  }

  // pid numérico: queries texto (prepared SELECT no MariaDB remoto retorna vazio → player_not_found).
  const std::string pid = std::to_string(playerId);
  bool isDead = memorySaysDead;
  int health = 0;
  if (!memorySaysDead) {
    auto rows = db_->executeQuery(
        "SELECT COALESCE(is_dead,0), COALESCE(health,0) FROM players WHERE id = " + pid +
        " LIMIT 1");
    if (rows.empty() || rows[0].size() < 2) {
      result.error = "player_not_found";
      // #region agent log
      agentDebugLog("H-RESPAWN", "ZoneCombatService.cpp:processRespawn", "respawn_fail",
                    std::string("{\"playerId\":") + std::to_string(playerId) +
                        ",\"error\":\"player_not_found\",\"memorySaysDead\":false}",
                    "post-fix");
      // #endregion
      return result;
    }
    try {
      isDead = (std::stoi(rows[0][0]) != 0);
      health = std::stoi(rows[0][1]);
    } catch (...) {
      result.error = "player_not_found";
      return result;
    }
    if (!isDead && health > 0) {
      result.error = "NOT_DEAD";
      // #region agent log
      agentDebugLog("H-RESPAWN", "ZoneCombatService.cpp:processRespawn", "respawn_fail",
                    std::string("{\"playerId\":") + std::to_string(playerId) +
                        ",\"error\":\"NOT_DEAD\",\"dbIsDead\":false,\"dbHealth\":" +
                        std::to_string(health) + ",\"memorySaysDead\":false}",
                    "post-fix");
      // #endregion
      return result;
    }
  }
  // #region agent log
  agentDebugLog("H-RESPAWN", "ZoneCombatService.cpp:processRespawn", "respawn_ok_gate",
                std::string("{\"playerId\":") + std::to_string(playerId) +
                    ",\"dbIsDead\":" + (isDead ? "true" : "false") +
                    ",\"dbHealth\":" + std::to_string(health) +
                    ",\"memorySaysDead\":" + (memorySaysDead ? "true" : "false") +
                    ",\"preferMaxH\":" + std::to_string(preferredMaxHealth) + "}",
                "post-fix");
  // #endregion

  if (zoneId == 0) zoneId = zoneId_;

  std::string key = spawnKey.empty() ? "cidade_inicial" : spawnKey;
  // spawn_key escapado de forma básica (só alfanumérico/_ do cliente); zoneId/pid numéricos.
  std::string safeKey;
  for (char c : key) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' ||
        c == '-') {
      safeKey.push_back(c);
    }
  }
  if (safeKey.empty()) safeKey = "cidade_inicial";

  auto rows = db_->executeQuery(
      "SELECT spawn_key, pos_x, pos_y, pos_z, yaw FROM spawn_points WHERE zone_id = " +
      std::to_string(zoneId) + " AND (spawn_key = '" + safeKey +
      "' OR is_default = 1) ORDER BY (spawn_key = '" + safeKey +
      "') DESC, is_default DESC LIMIT 1");

  float px = 0.f, py = 0.f, pz = 200.f, yaw = 0.f;
  if (!rows.empty() && rows[0].size() >= 5) {
    try {
      px = std::stof(rows[0][1]);
      py = std::stof(rows[0][2]);
      pz = std::stof(rows[0][3]);
      yaw = std::stof(rows[0][4]);
    } catch (...) {}
  }

  int32_t maxHealth = 100;
  int32_t maxMana = 50;
  if (preferredMaxHealth > 0) {
    maxHealth = preferredMaxHealth;
  } else {
    auto mh = db_->executeQuery("SELECT COALESCE(max_health,100) FROM players WHERE id = " + pid +
                                " LIMIT 1");
    if (!mh.empty() && !mh[0].empty()) {
      try {
        maxHealth = std::max(1, std::stoi(mh[0][0]));
      } catch (...) {}
    }
  }
  if (preferredMaxMana > 0) {
    maxMana = preferredMaxMana;
  } else {
    auto mm = db_->executeQuery("SELECT COALESCE(max_mana,50) FROM players WHERE id = " + pid +
                                " LIMIT 1");
    if (!mm.empty() && !mm[0].empty()) {
      try {
        maxMana = std::max(1, std::stoi(mm[0][0]));
      } catch (...) {}
    }
  }

  const std::string upd =
      "UPDATE players SET health = " + std::to_string(maxHealth) +
      ", mana = " + std::to_string(maxMana) +
      ", is_dead = 0, pos_x = " + std::to_string(px) +
      ", pos_y = " + std::to_string(py) +
      ", pos_z = " + std::to_string(pz) +
      ", current_zone = " + std::to_string(zoneId) +
      " WHERE id = " + pid;
  db_->executeQuery(upd);

  // Confirma escrita (MariaDB remoto).
  auto verify = db_->executeQuery(
      "SELECT COALESCE(health,0), COALESCE(is_dead,1) FROM players WHERE id = " + pid + " LIMIT 1");
  int verifyHp = 0;
  int verifyDead = 1;
  if (!verify.empty() && verify[0].size() >= 2) {
    try {
      verifyHp = std::stoi(verify[0][0]);
      verifyDead = std::stoi(verify[0][1]);
    } catch (...) {}
  }
  if (verifyDead != 0 || verifyHp <= 0) {
    result.error = "update_not_applied";
    // #region agent log
    agentDebugLog("H-RESPAWN", "ZoneCombatService.cpp:processRespawn", "respawn_fail",
                  std::string("{\"playerId\":") + std::to_string(playerId) +
                      ",\"error\":\"update_not_applied\",\"verifyHp\":" + std::to_string(verifyHp) +
                      ",\"verifyDead\":" + std::to_string(verifyDead) + "}",
                  "post-fix");
    // #endregion
    return result;
  }

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
  // #region agent log
  agentDebugLog("H-RESPAWN", "ZoneCombatService.cpp:processRespawn", "respawn_success",
                std::string("{\"playerId\":") + std::to_string(playerId) +
                    ",\"hp\":" + std::to_string(maxHealth) +
                    ",\"mp\":" + std::to_string(maxMana) +
                    ",\"verifyHp\":" + std::to_string(verifyHp) +
                    ",\"x\":" + std::to_string(static_cast<int>(px)) +
                    ",\"y\":" + std::to_string(static_cast<int>(py)) + "}",
                "post-fix");
  // #endregion
  return result;
}

void ZoneCombatService::tickActiveDots(MovementServer* movementServer) {
  if (!db_ || !db_->isConnected() || !movementServer) return;

  // Sem jogadores online: não gastar round-trip MySQL a cada 250ms (medido até 820ms no Proxmox
  // mesmo com active_dots vazio).
  if (movementServer->getOnlinePlayerCount() == 0) {
    return;
  }

  const auto now = std::chrono::steady_clock::now();
  if (now < nextDotsPollAt_) {
    return;
  }

  // #region agent log
  const int64_t pollT0 = agentNowMs();
  // #endregion
  auto rows = db_->executePreparedQuery(
      "SELECT dot_id, target_player_id, source_player_id, dot_type, tick_value, tick_interval_ms, ticks_remaining "
      "FROM active_dots WHERE next_tick_at <= NOW(3) LIMIT 50",
      {});
  // #region agent log
  const int64_t pollMs = agentNowMs() - pollT0;
  if (pollMs > 5) {
    agentDebugLog("H-B", "ZoneCombatService.cpp:tickActiveDots", "dots_poll_ms",
                  std::string("{\"ms\":") + std::to_string(pollMs) +
                      ",\"rows\":" + std::to_string(rows.size()) + "}");
  }
  // #endregion

  // Backoff: poll vazio → só tentar de novo em 2s (comportamento local inalterado quando há DOTs).
  if (rows.empty()) {
    nextDotsPollAt_ = now + std::chrono::seconds(2);
    return;
  }
  nextDotsPollAt_ = now;  // há trabalho: pode polir no próximo tick de 250ms

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
          "UPDATE active_dots SET ticks_remaining = ?, next_tick_at = DATE_ADD(NOW(3), INTERVAL ? MICROSECOND) WHERE dot_id = ?",
          {std::to_string(ticksRemaining),
           std::to_string(static_cast<uint64_t>(tickIntervalMs) * 1000ULL), dotIdStr});
    }
  }
}

}  // namespace Zone
}  // namespace Umbra
