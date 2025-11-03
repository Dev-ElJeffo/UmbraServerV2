#include "ZoneServer.hpp"
#include "core/Logger.hpp"
#include "core/ConfigManager.hpp"
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<bool> running(true);

void signalHandler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    running = false;
  }
}

int main(int argc, char* argv[]) {
  Umbra::Core::Logger::getInstance().initialize("logs/zone_server.log");
  Umbra::Core::Logger::getInstance().info("Starting Zone Server...");
  
  Umbra::Zone::ZoneServer::Config config;
  config.zoneId = (argc > 1) ? std::atoi(argv[1]) : 0;
  config.zoneName = "Zone_" + std::to_string(config.zoneId);
  config.port = 8082 + config.zoneId;
  
  Umbra::Zone::ZoneServer zoneServer(config);
  
  if (!zoneServer.start()) {
    return 1;
  }
  
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);
  
  auto lastUpdate = std::chrono::steady_clock::now();
  
  while (running) {
    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastUpdate).count();
    lastUpdate = now;
    
    zoneServer.update(deltaTime);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 FPS
  }
  
  zoneServer.stop();
  return 0;
}

