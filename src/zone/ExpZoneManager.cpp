#include "zone/ExpZoneManager.hpp"
#include "zone/MovementServer.hpp"
#include "zone/MovementProtocol.hpp"
#include "core/Logger.hpp"
#include <cmath>

namespace Umbra {
namespace Zone {

ExpZoneManager::ExpZoneManager(uint32_t zoneId, std::shared_ptr<Database::MySQLConnector> db,
                               ExperienceService* experienceService, MovementServer* movementServer)
    : zoneId_(zoneId),
      db_(std::move(db)),
      experienceService_(experienceService),
      movementServer_(movementServer) {
  loadZonesFromMySQL();
}

void ExpZoneManager::loadZonesFromMySQL() {
  if (!db_ || !db_->isConnected()) return;

  zones_.clear();
  auto rows = db_->executePreparedQuery(
      "SELECT exp_zone_id, zone_id, name, center_x, center_y, center_z, radius, "
      "exp_per_tick, tick_interval_sec, min_player_level, max_player_level, enabled "
      "FROM exp_zones WHERE zone_id = ? AND enabled = 1",
      {std::to_string(zoneId_)});

  for (const auto& row : rows) {
    if (row.size() < 12) continue;
    ExpZoneDef z;
    z.expZoneId = static_cast<uint32_t>(std::stoul(row[0]));
    z.zoneId = static_cast<uint32_t>(std::stoul(row[1]));
    z.name = row[2];
    z.centerX = std::stof(row[3]);
    z.centerY = std::stof(row[4]);
    z.centerZ = std::stof(row[5]);
    z.radius = std::stof(row[6]);
    z.expPerTick = std::stoi(row[7]);
    z.tickIntervalSec = std::stof(row[8]);
    z.minPlayerLevel = std::stoi(row[9]);
    z.maxPlayerLevel = std::stoi(row[10]);
    z.enabled = row[11] != "0";
    if (z.expPerTick > 0 && z.radius > 0.f) {
      zones_.push_back(std::move(z));
    }
  }

  Core::Logger::getInstance().info("[ExpZoneManager] loaded {} exp zones from MySQL for zone {}",
                                   zones_.size(), zoneId_);
}

bool ExpZoneManager::isPlayerInZone(const ExpZoneDef& zone, float x, float y, float z,
                                    uint32_t playerLevel) const {
  if (zone.minPlayerLevel > 0 && static_cast<int32_t>(playerLevel) < zone.minPlayerLevel) {
    return false;
  }
  if (zone.maxPlayerLevel > 0 && static_cast<int32_t>(playerLevel) > zone.maxPlayerLevel) {
    return false;
  }
  const float dx = x - zone.centerX;
  const float dy = y - zone.centerY;
  const float horiz = std::sqrt(dx * dx + dy * dy);
  if (horiz > zone.radius) return false;
  if (std::fabs(z - zone.centerZ) > kHeightTolerance) return false;
  return true;
}

void ExpZoneManager::broadcastGrant(uint32_t playerId, const ExperienceGrantResult& grant) {
  if (!movementServer_ || !grant.success) return;

  ExpGainNotifyPayload expPayload;
  expPayload.playerId = playerId;
  expPayload.expGained = static_cast<int32_t>(grant.expGranted);
  expPayload.totalExp = grant.totalExperience;
  expPayload.expForNext = grant.expForNextLevel;
  expPayload.expInCurrentLevel = grant.expInCurrentLevel;
  expPayload.progressPercent = static_cast<uint8_t>(std::clamp(
      static_cast<int>(std::lround(grant.expProgressPercent)), 0, 100));
  movementServer_->broadcastExpGain(playerId, expPayload);

  if (grant.levelsGained > 0) {
    LevelUpNotifyPayload levelPayload;
    levelPayload.playerId = playerId;
    levelPayload.newLevel = grant.newLevel;
    levelPayload.levelsGained =
        static_cast<uint8_t>(std::min<uint32_t>(grant.levelsGained, 255));
    levelPayload.statPointsGained =
        static_cast<uint16_t>(std::min<uint32_t>(grant.statPointsGained, 65535));
    levelPayload.skillPointsAvail = 0;
    auto skillOpt = db_->executePreparedScalar(
        "SELECT points_available FROM player_skill_points WHERE player_id = ?",
        {std::to_string(playerId)});
    if (skillOpt) {
      levelPayload.skillPointsAvail =
          static_cast<uint16_t>(std::min<int>(std::stoi(*skillOpt), 65535));
    } else {
      Core::Logger::getInstance().warn(
          "[ExpZoneManager] player {} leveled up but player_skill_points row missing", playerId);
    }
    movementServer_->broadcastLevelUp(playerId, levelPayload);
  }
}

void ExpZoneManager::tick(float deltaSeconds) {
  if (!experienceService_ || !movementServer_) return;
  if (zones_.empty()) return;

  tickAccum_ += deltaSeconds;
  if (tickAccum_ < tickIntervalSec_) return;
  tickAccum_ = 0.f;

  const auto players = movementServer_->getPlayerStates();
  const auto now = std::chrono::steady_clock::now();

  for (const auto& [playerId, state] : players) {
    if (state.isDead) continue;

    uint32_t playerLevel = 1;
    if (auto lvl = db_->executePreparedScalar("SELECT level FROM players WHERE id = ?",
                                              {std::to_string(playerId)})) {
      playerLevel = static_cast<uint32_t>(std::stoul(*lvl));
    }

    for (const ExpZoneDef& zone : zones_) {
      if (!isPlayerInZone(zone, state.x, state.y, state.z, playerLevel)) {
        continue;
      }

      auto& cd = playerCooldowns_[playerId];
      if (cd.expZoneId == zone.expZoneId && cd.nextTickAt > now) {
        continue;
      }

      const std::string source = "exp_zone:" + zone.name;
      auto grant = experienceService_->grantExperience(playerId, zone.expPerTick, source);
      if (!grant.success) continue;

      broadcastGrant(playerId, grant);
      cd.expZoneId = zone.expZoneId;
      cd.nextTickAt = now + std::chrono::milliseconds(
                                static_cast<int>(zone.tickIntervalSec * 1000.f));
      break;
    }
  }
}

}  // namespace Zone
}  // namespace Umbra
