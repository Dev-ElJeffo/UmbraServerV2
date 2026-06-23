#pragma once

#include "zone/ExperienceService.hpp"
#include "database/MySQLConnector.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Umbra {
namespace Zone {

class MovementServer;

struct ExpZoneDef {
  uint32_t expZoneId = 0;
  uint32_t zoneId = 0;
  std::string name;
  float centerX = 0.f;
  float centerY = 0.f;
  float centerZ = 0.f;
  float radius = 0.f;
  int32_t expPerTick = 0;
  float tickIntervalSec = 5.f;
  int32_t minPlayerLevel = 0;
  int32_t maxPlayerLevel = 0;
  bool enabled = true;
};

class ExpZoneManager {
public:
  ExpZoneManager(uint32_t zoneId, std::shared_ptr<Database::MySQLConnector> db,
                 ExperienceService* experienceService, MovementServer* movementServer);

  void tick(float deltaSeconds);

private:
  void loadZonesFromMySQL();
  bool isPlayerInZone(const ExpZoneDef& zone, float x, float y, float z, uint32_t playerLevel) const;
  void broadcastGrant(uint32_t playerId, const ExperienceGrantResult& grant);

  uint32_t zoneId_;
  std::shared_ptr<Database::MySQLConnector> db_;
  ExperienceService* experienceService_ = nullptr;
  MovementServer* movementServer_ = nullptr;

  std::vector<ExpZoneDef> zones_;

  float tickAccum_ = 0.f;
  float tickIntervalSec_ = 1.f;
  static constexpr float kHeightTolerance = 500.f;

  struct PlayerZoneCooldown {
    uint32_t expZoneId = 0;
    std::chrono::steady_clock::time_point nextTickAt{};
  };
  std::unordered_map<uint32_t, PlayerZoneCooldown> playerCooldowns_;
};

}  // namespace Zone
}  // namespace Umbra
