#pragma once

#include "PlayerManager.hpp"
#include "EntitySystem.hpp"
#include <memory>
#include <string>
#include "zone/MovementServer.hpp"

namespace Umbra {
namespace Zone {

class ZoneServer {
 public:
  struct Config {
    uint32_t zoneId;
    std::string zoneName;
    uint16_t port = 8082;
    uint32_t maxPlayers = 1000;
    float tickRate = 60.0f;
  };
  
  explicit ZoneServer(const Config& config);
  ~ZoneServer();
  
  bool start();
  void stop();
  bool isRunning() const;
  void update(float deltaTime);
  
  PlayerManager& getPlayerManager();
  EntitySystem& getEntitySystem();

 private:
  Config config_;
  std::unique_ptr<PlayerManager> playerManager_;
  std::unique_ptr<EntitySystem> entitySystem_;
  std::unique_ptr<MovementServer> movementServer_;
  float snapshotAccumulator_ = 0.0f;
  bool running_;
};

}  // namespace Zone
}  // namespace Umbra

