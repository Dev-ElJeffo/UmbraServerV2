#include "ZoneServer.hpp"
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

  // Parse arguments
  int zoneId = (argc > 1) ? std::atoi(argv[1]) : 0;
  std::string zoneName = "Zone_" + std::to_string(zoneId);
  uint16_t zonePort = 8082 + static_cast<uint16_t>(zoneId);

  std::cout << "===========================================\n";
  std::cout << "  UmbraEternum Zone Server                \n";
  std::cout << "  Zone: " << zoneName << " | Port: " << zonePort << "\n";
  std::cout << "===========================================\n\n";

  Umbra::Core::Logger::getInstance().info("Starting Zone Server '{}' on port {}...", zoneName, zonePort);

  // Database
  std::shared_ptr<Umbra::Database::MySQLConnector> dbConnector;
  auto& configManager = Umbra::Core::ConfigManager::getInstance();
  if (configManager.loadConfig("config/server.json")) {
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

  if (!zoneServer.start()) {
    std::cerr << "[FAIL] Zone Server failed to start\n";
    return 1;
  }

  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);

  std::cout << "[OK] Zone Server '" << zoneName << "' running on port " << zonePort << "\n";
  std::cout << "\n  Systems active:\n";
  std::cout << "    - Spatial Grid AOI (cell=10000u, 3x3 = 30000u radius)\n";
  std::cout << "    - Auto-save positions (every 30s)\n";
  std::cout << "    - Per-client Rate Limiting (via SocketServer)\n";
  std::cout << "    - Movement validation (speed/teleport checks)\n";
  std::cout << "\n  Press Ctrl+C to stop.\n\n";

  Umbra::Core::Logger::getInstance().info("Zone '{}' ready. AOI=10000u cells (30000u radius), AutoSave=30s, MaxPlayers={}",
                                           zoneName, config.maxPlayers);

  auto lastUpdate = std::chrono::steady_clock::now();
  auto lastStats = std::chrono::steady_clock::now();

  while (running) {
    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastUpdate).count();
    lastUpdate = now;

    zoneServer.update(deltaTime);

    auto statsDelta = std::chrono::duration<float>(now - lastStats).count();
    if (statsDelta >= 60.0f) {
      Umbra::Core::Logger::getInstance().info("[Zone '{}'] Stats: players online (check MovementServer)",
                                               zoneName);
      lastStats = now;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  std::cout << "\nShutting down Zone Server...\n";
  zoneServer.stop();
  std::cout << "[OK] Zone Server '" << zoneName << "' stopped\n";

  return 0;
}
