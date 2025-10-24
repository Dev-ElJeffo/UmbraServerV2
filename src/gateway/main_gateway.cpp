#include "GatewayServer.hpp"
#include "core/Logger.hpp"
#include "core/ConfigManager.hpp"
#include <csignal>
#include <atomic>
#include <thread>

std::atomic<bool> running(true);

void signalHandler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    running = false;
  }
}

int main() {
  Umbra::Core::Logger::getInstance().initialize("logs/gateway_server.log");
  Umbra::Core::Logger::getInstance().info("Starting Gateway Server...");
  
  auto& configManager = Umbra::Core::ConfigManager::getInstance();
  configManager.loadConfig("config/server.json");
  
  Umbra::Gateway::GatewayServer::Config config;
  config.port = configManager.get<uint16_t>("gateway.port", 9000);
  config.jwtSecret = configManager.get<std::string>("auth.jwt_secret", "default_secret");
  
  Umbra::Gateway::GatewayServer gatewayServer(config);
  
  if (!gatewayServer.start()) {
    return 1;
  }
  
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);
  
  while (running) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  gatewayServer.stop();
  return 0;
}

