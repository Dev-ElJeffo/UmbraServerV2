#include "GatewayServer.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace Gateway {

GatewayServer::GatewayServer(const Config& config)
    : config_(config), running_(false) {
  loadBalancer_ = std::make_unique<LoadBalancer>();
  jwtManager_ = std::make_unique<Auth::JWTManager>(config.jwtSecret);
}

GatewayServer::~GatewayServer() {
  stop();
}

bool GatewayServer::start() {
  running_ = true;
  Core::Logger::getInstance().info("GatewayServer started on port {}", config_.port);
  return true;
}

void GatewayServer::stop() {
  running_ = false;
  Core::Logger::getInstance().info("GatewayServer stopped");
}

bool GatewayServer::isRunning() const {
  return running_;
}

LoadBalancer& GatewayServer::getLoadBalancer() {
  return *loadBalancer_;
}

void GatewayServer::handleConnection(uint32_t clientId) {
  // TODO: Validate JWT and route to appropriate zone server
  Core::Logger::getInstance().debug("Handling connection for client {}", clientId);
}

}  // namespace Gateway
}  // namespace Umbra

