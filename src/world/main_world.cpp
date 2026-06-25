#include "WorldServer.hpp"
#include "admin/AdminBootstrap.hpp"
#include "admin/ServiceAdminRegister.hpp"
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
  Umbra::Core::Logger::getInstance().initialize("logs/world_server.log");
  Umbra::Core::Logger::getInstance().info("Starting World Server...");
  
  auto& configManager = Umbra::Core::ConfigManager::getInstance();
  configManager.loadConfig("config/server.json");
  
  Umbra::World::WorldServer::Config config;
  config.port = configManager.get<uint16_t>("world.port", 8081);
  config.tickRate = configManager.get<uint32_t>("world.tick_rate", 20);
  Umbra::World::WorldServer worldServer(config);
  
  const uint16_t adminPort = configManager.get<uint16_t>("admin.world_port", 9101);
  auto adminServer = Umbra::Admin::createFromConfig(
      "world_server", adminPort,
      [](int) { running = false; });
  if (adminServer) {
    Umbra::Admin::registerWorldCommands(adminServer->getRegistry(), worldServer);
    adminServer->start();
  }
  
  if (!worldServer.start()) {
    return 1;
  }
  
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);
  
  while (running) {
    worldServer.update();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  
  if (adminServer) adminServer->stop();
  worldServer.stop();
  return 0;
}
