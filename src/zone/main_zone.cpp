#include "ZoneServer.hpp"
#include "admin/AdminBootstrap.hpp"
#include "admin/ServiceAdminRegister.hpp"
#include "core/Logger.hpp"
#include "core/ConfigManager.hpp"
#include "database/MySQLConnector.hpp"
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

std::atomic<bool> running(true);

void signalHandler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    running = false;
  }
}

int main(int argc, char* argv[]) {
  Umbra::Core::Logger::getInstance().initialize("logs/zone_server.log");

  int zoneId = (argc > 1) ? std::atoi(argv[1]) : 0;
  std::string zoneName = "Zone_" + std::to_string(zoneId);

  auto& configManager = Umbra::Core::ConfigManager::getInstance();
  configManager.loadConfig("config/server.json");
  const uint16_t zoneBase = configManager.get<uint16_t>("zone.base_port", 8082);
  uint16_t zonePort = static_cast<uint16_t>(zoneBase + zoneId);

  std::cout << "===========================================\n";
  std::cout << "  UmbraEternum Zone Server                \n";
  std::cout << "  Zone: " << zoneName << " | Port: " << zonePort << "\n";
  std::cout << "===========================================\n\n";

  Umbra::Core::Logger::getInstance().info("Starting Zone Server '{}' on port {}...", zoneName, zonePort);

  std::shared_ptr<Umbra::Database::MySQLConnector> dbConnector;
  {
    Umbra::Database::MySQLConnector::Config dbConfig;
    dbConfig.host = configManager.get<std::string>("database.host", "localhost");
    dbConfig.port = static_cast<uint16_t>(configManager.get<uint32_t>("database.port", 3306));
    dbConfig.database = configManager.get<std::string>("database.name", "umbra_eternum");
    dbConfig.username = configManager.get<std::string>("database.user", "root");
    dbConfig.password = configManager.get<std::string>("database.password", "");
    dbConfig.poolSize = configManager.get<uint32_t>("database.pool_size", 5);

    dbConnector = std::make_shared<Umbra::Database::MySQLConnector>(dbConfig);
    if (!dbConnector->connect()) {
      Umbra::Core::Logger::getInstance().warn("Zone: Database connection failed");
      std::cout << "[WARN] Database connection failed\n";
      dbConnector.reset();
    } else {
      std::cout << "[OK] MySQL connected (pool: " << dbConfig.poolSize << " connections)\n";
    }
  }

  Umbra::Zone::ZoneServer::Config config;
  config.zoneId = static_cast<uint32_t>(zoneId);
  config.zoneName = zoneName;
  config.port = zonePort;
  config.maxPlayers = configManager.get<uint32_t>("zone.max_players_per_zone", 1000);
  config.dbConnector = dbConnector;

  Umbra::Zone::ZoneServer zoneServer(config);

  const uint16_t adminBase = configManager.get<uint16_t>("admin.zone_base_port", 9102);
  const uint16_t adminPort = static_cast<uint16_t>(adminBase + zoneId);
  auto adminServer = Umbra::Admin::createFromConfig(
      zoneName, adminPort,
      [](int) { running = false; });
  if (adminServer) {
    Umbra::Admin::registerZoneCommands(adminServer->getRegistry(), zoneServer);
    adminServer->start();
  }

  if (!zoneServer.start()) {
    std::cerr << "[FAIL] Zone Server failed to start\n";
    return 1;
  }

  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);

  std::cout << "[OK] Zone Server '" << zoneName << "' running on port " << zonePort << "\n";
  std::cout << "  Admin port: " << adminPort << "\n";
  std::cout << "\n  Press Ctrl+C to stop.\n\n";

  auto lastUpdate = std::chrono::steady_clock::now();

  while (running) {
    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastUpdate).count();
    lastUpdate = now;
    zoneServer.update(deltaTime);
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  std::cout << "\nShutting down Zone Server...\n";
  if (adminServer) adminServer->stop();
  zoneServer.stop();
  std::cout << "[OK] Zone Server '" << zoneName << "' stopped\n";

  return 0;
}
