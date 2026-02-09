#include "ZoneServer.hpp"
#include "core/Logger.hpp"
#include "core/ConfigManager.hpp"
#include "database/MySQLConnector.hpp"
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
  
  std::shared_ptr<Umbra::Database::MySQLConnector> dbConnector;
  auto& configManager = Umbra::Core::ConfigManager::getInstance();
  if (configManager.loadConfig("config/server.json")) {
    Umbra::Database::MySQLConnector::Config dbConfig;
    dbConfig.host = configManager.get<std::string>("database.host", "localhost");
    dbConfig.port = static_cast<uint16_t>(configManager.get<uint32_t>("database.port", 3306));
    dbConfig.database = configManager.get<std::string>("database.name", "umbra_eternum");
    dbConfig.username = configManager.get<std::string>("database.user", "root");
    dbConfig.password = configManager.get<std::string>("database.password", "");
    dbConnector = std::make_shared<Umbra::Database::MySQLConnector>(dbConfig);
    if (!dbConnector->connect()) {
      Umbra::Core::Logger::getInstance().warn("Zone: Database connection failed - party leave on disconnect disabled");
      dbConnector.reset();
    }
  } else {
    Umbra::Core::Logger::getInstance().warn("Zone: Config not loaded - party leave on disconnect disabled");
  }
  
  Umbra::Zone::ZoneServer::Config config;
  config.zoneId = (argc > 1) ? std::atoi(argv[1]) : 0;
  config.zoneName = "Zone_" + std::to_string(config.zoneId);
  config.port = 8082 + config.zoneId;
  config.dbConnector = dbConnector;
  
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

