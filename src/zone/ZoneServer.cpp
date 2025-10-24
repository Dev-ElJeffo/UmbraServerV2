#include "ZoneServer.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace Zone {

ZoneServer::ZoneServer(const Config& config)
    : config_(config), running_(false) {
  playerManager_ = std::make_unique<PlayerManager>(config.maxPlayers);
  entitySystem_ = std::make_unique<EntitySystem>();
}

ZoneServer::~ZoneServer() {
  stop();
}

bool ZoneServer::start() {
  running_ = true;
  Core::Logger::getInstance().info("ZoneServer '{}' (ID: {}) started on port {}", 
                                   config_.zoneName, config_.zoneId, config_.port);
  return true;
}

void ZoneServer::stop() {
  running_ = false;
  Core::Logger::getInstance().info("ZoneServer '{}' stopped", config_.zoneName);
}

bool ZoneServer::isRunning() const {
  return running_;
}

void ZoneServer::update(float deltaTime) {
  playerManager_->update(deltaTime);
  entitySystem_->update(deltaTime);
}

PlayerManager& ZoneServer::getPlayerManager() {
  return *playerManager_;
}

EntitySystem& ZoneServer::getEntitySystem() {
  return *entitySystem_;
}

}  // namespace Zone
}  // namespace Umbra

