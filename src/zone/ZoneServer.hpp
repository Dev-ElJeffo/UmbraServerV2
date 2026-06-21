#pragma once

#include "PlayerManager.hpp"
#include "EntitySystem.hpp"
#include <memory>
#include <string>
#include "zone/MovementServer.hpp"
#include "zone/ZoneCombatService.hpp"
#include "zone/CombatCoreEngine.hpp"

namespace Umbra {
namespace Database { class MySQLConnector; }
namespace Zone {

class ZoneServer {
 public:
  struct Config {
    uint32_t zoneId;
    std::string zoneName;
    uint16_t port = 8082;
    uint32_t maxPlayers = 1000;
    float tickRate = 60.0f;
    std::shared_ptr<Umbra::Database::MySQLConnector> dbConnector;
  };
  
  explicit ZoneServer(const Config& config);
  ~ZoneServer();
  
  bool start();
  void stop();
  bool isRunning() const;
  void update(float deltaTime);
  
  PlayerManager& getPlayerManager();
  EntitySystem& getEntitySystem();
  MovementServer* getMovementServer();
  CombatCoreEngine* getCombatCoreEngine();
  const Config& getConfig() const;
  void forceSavePositions();

 private:
  Config config_;
  std::unique_ptr<PlayerManager> playerManager_;
  std::unique_ptr<EntitySystem> entitySystem_;
  std::unique_ptr<MovementServer> movementServer_;
  std::unique_ptr<ZoneCombatService> combatService_;
  std::unique_ptr<CombatCoreEngine> combatCoreEngine_;
  float snapshotAccumulator_ = 0.0f;
  float dotTickAccumulator_ = 0.0f;
  float autoSaveAccumulator_ = 0.0f;
  float autoSaveInterval_ = 30.0f;
  bool running_;

  void autoSavePlayerPositions();
};

}  // namespace Zone
}  // namespace Umbra

