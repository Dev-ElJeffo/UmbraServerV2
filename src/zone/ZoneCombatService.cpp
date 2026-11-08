#include "zone/ZoneCombatService.hpp"
#include "zone/MovementServer.hpp"
#include "database/MySQLConnector.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace Umbra {
namespace Zone {

ZoneCombatService::ZoneCombatService(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId)
    : db_(std::move(db)), zoneId_(zoneId) {}

RespawnResult ZoneCombatService::processRespawn(uint32_t playerId, uint32_t zoneId,
                                                const std::string& spawnKey,
                                                bool memorySaysDead, int32_t preferredMaxHealth,
                                                int32_t preferredMaxMana) {
  RespawnResult result;
  if (!db_ || !db_->isConnected()) {
    result.error = "database_unavailable";
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
      return result;
    }
  }

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
  return result;
}

}  // namespace Zone
}  // namespace Umbra
