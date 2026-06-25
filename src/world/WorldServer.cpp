#include "WorldServer.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace World {

WorldServer::WorldServer(const Config& config)
    : config_(config), running_(false) {
  eventManager_ = std::make_unique<EventManager>();
  timeManager_ = std::make_unique<TimeManager>();
}

WorldServer::~WorldServer() {
  stop();
}

bool WorldServer::start() {
  running_ = true;
  timeManager_->start();
  Core::Logger::getInstance().info("WorldServer started on port {}", config_.port);
  return true;
}

void WorldServer::stop() {
  running_ = false;
  timeManager_->stop();
  Core::Logger::getInstance().info("WorldServer stopped");
}

bool WorldServer::isRunning() const {
  return running_;
}

EventManager& WorldServer::getEventManager() {
  return *eventManager_;
}

TimeManager& WorldServer::getTimeManager() {
  return *timeManager_;
}

const WorldServer::Config& WorldServer::getConfig() const {
  return config_;
}

void WorldServer::update() {
  timeManager_->update();
  eventManager_->processEvents();
}

}  // namespace World
}  // namespace Umbra

