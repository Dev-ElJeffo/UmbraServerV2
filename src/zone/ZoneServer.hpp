#pragma once

#include "PlayerManager.hpp"
#include "EntitySystem.hpp"
#include <memory>
#include <string>
#include "zone/MovementServer.hpp"
#include "zone/ZoneCombatService.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "zone/ExperienceService.hpp"
#include "zone/ExpZoneManager.hpp"

namespace Umbra {
namespace Auth { class JWTManager; }
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
  // Trabalho leve e sensível a latência, chamado em alta frequência (~3ms) pelo
  // loop principal: drena o inbound (casts entram na fila de combate quase na
  // hora) e dispara os hits adiados por castTimeMs sem esperar o tick pesado.
  void pumpInbound();
  
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
  std::unique_ptr<ExperienceService> experienceService_;
  std::unique_ptr<ExpZoneManager> expZoneManager_;
  std::unique_ptr<Umbra::Auth::JWTManager> jwtManager_;
  float expZoneTickAccumulator_ = 0.0f;
  float snapshotAccumulator_ = 0.0f;
  float wsKeepaliveAccumulator_ = 0.0f;
  float dotTickAccumulator_ = 0.0f;
  float buffTickAccumulator_ = 0.0f;
  float autoSaveAccumulator_ = 0.0f;
  float autoSaveInterval_ = 30.0f;
  bool running_;

  void autoSavePlayerPositions();
};

}  // namespace Zone
}  // namespace Umbra

