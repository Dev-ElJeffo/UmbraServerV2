#include "core/Logger.hpp"
#include "core/ConfigManager.hpp"
#include "gateway/GatewayServer.hpp"
#include "auth/AuthServer.hpp"
#include "world/WorldServer.hpp"
#include "database/MySQLConnector.hpp"
#include "gateway/LoadBalancer.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <memory>

std::atomic<bool> running(true);

void signalHandler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    std::cout << "\nReceived shutdown signal..." << std::endl;
    running = false;
  }
}

int main(int, char*[]) {
  std::cout << "===========================================\n";
  std::cout << "    UmbraEternum Server Stack v1.4.0      \n";
  std::cout << "    Production Scaling Edition             \n";
  std::cout << "===========================================\n\n";

  Umbra::Core::Logger::getInstance().initialize("logs/umbra_server.log");
  Umbra::Core::Logger::getInstance().info("Starting UmbraEternum Server Stack v1.4.0...");

  auto& configManager = Umbra::Core::ConfigManager::getInstance();
  if (!configManager.loadConfig("config/server.json")) {
    Umbra::Core::Logger::getInstance().error("Failed to load configuration");
    std::cerr << "ERROR: Could not load config/server.json" << std::endl;
    return 1;
  }

  Umbra::Core::Logger::getInstance().info("Configuration loaded successfully");

  // ========== Database (with connection pool) ==========
  Umbra::Database::MySQLConnector::Config dbConfig;
  dbConfig.host = configManager.get<std::string>("database.host", "localhost");
  dbConfig.port = configManager.get<uint16_t>("database.port", 3306);
  dbConfig.database = configManager.get<std::string>("database.name", "umbra_eternum");
  dbConfig.username = configManager.get<std::string>("database.user", "root");
  dbConfig.password = configManager.get<std::string>("database.password", "");
  dbConfig.poolSize = configManager.get<uint32_t>("database.pool_size", 5);

  auto dbConnector = std::make_shared<Umbra::Database::MySQLConnector>(dbConfig);

  if (!dbConnector->connect()) {
    Umbra::Core::Logger::getInstance().warn("Database connection failed, continuing anyway");
    std::cout << "[WARN] Database connection failed\n";
  } else {
    std::cout << "[OK] MySQL connected (pool: " << dbConfig.poolSize << " connections)\n";
  }

  // ========== Auth Server ==========
  Umbra::Auth::AuthServer::Config authConfig;
  authConfig.port = configManager.get<uint16_t>("auth.port", 8080);
  authConfig.jwtSecret = configManager.get<std::string>("auth.jwt_secret", "default_secret_change_me");

  auto authServer = std::make_unique<Umbra::Auth::AuthServer>(authConfig, dbConnector);

  if (!authServer->start()) {
    Umbra::Core::Logger::getInstance().error("Failed to start Auth Server");
    return 1;
  }

  std::cout << "[OK] Auth Server started on port " << authConfig.port << std::endl;

  // ========== World Server ==========
  Umbra::World::WorldServer::Config worldConfig;
  worldConfig.port = configManager.get<uint16_t>("world.port", 8081);

  auto worldServer = std::make_unique<Umbra::World::WorldServer>(worldConfig);

  if (!worldServer->start()) {
    Umbra::Core::Logger::getInstance().error("Failed to start World Server");
    return 1;
  }

  std::cout << "[OK] World Server started on port " << worldConfig.port << std::endl;

  // ========== Gateway Server ==========
  Umbra::Gateway::GatewayServer::Config gatewayConfig;
  gatewayConfig.port = configManager.get<uint16_t>("gateway.port", 9000);
  gatewayConfig.jwtSecret = authConfig.jwtSecret;
  gatewayConfig.authHost = configManager.get<std::string>("gateway.auth_host", "localhost");
  gatewayConfig.authPort = configManager.get<uint16_t>("gateway.auth_port", 8080);
  gatewayConfig.authTimeoutMs = configManager.get<uint32_t>("gateway.auth_timeout_ms", 5000);
  gatewayConfig.rateLimitPerSecond = configManager.get<uint32_t>("gateway.rate_limit", 100);
  gatewayConfig.useConnectionPool = configManager.get<bool>("gateway.use_connection_pool", true);
  gatewayConfig.maxConnectionsPerHost = configManager.get<uint32_t>("gateway.max_connections_per_host", 3);
  gatewayConfig.healthCheckIntervalMs = configManager.get<uint32_t>("gateway.health_check_interval_ms", 30000);

  auto gatewayServer = std::make_unique<Umbra::Gateway::GatewayServer>(gatewayConfig);

  if (!gatewayServer->start()) {
    Umbra::Core::Logger::getInstance().error("Failed to start Gateway Server");
    return 1;
  }

  std::cout << "[OK] Gateway Server started on port " << gatewayConfig.port << std::endl;

  // ========== Systems status ==========
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);

  std::cout << "\n===========================================\n";
  std::cout << "  All servers running. Press Ctrl+C to stop.\n";
  std::cout << "===========================================\n";
  std::cout << "\n  Systems active:\n";
  std::cout << "    - MySQL Connection Pool (" << dbConfig.poolSize << " connections)\n";
  std::cout << "    - Per-client Rate Limiting (" << gatewayConfig.rateLimitPerSecond << " msg/s)\n";
  std::cout << "    - Spatial Grid AOI (cell=10000u, range=3x3 = 30000u radius)\n";
  std::cout << "    - Auto-save positions (every 30s)\n";
  std::cout << "    - Zone Orchestrator (available for zone_server)\n";
  std::cout << "\n  Ports:\n";
  std::cout << "    Auth:    " << authConfig.port << "\n";
  std::cout << "    World:   " << worldConfig.port << "\n";
  std::cout << "    Gateway: " << gatewayConfig.port << "\n";
  std::cout << "    Zone:    " << configManager.get<uint16_t>("zone.base_port", 8082) << "+ (run zone_server separately)\n";
  std::cout << std::endl;

  Umbra::Core::Logger::getInstance().info("All servers initialized successfully");
  Umbra::Core::Logger::getInstance().info("Systems: DB Pool={}, RateLimit={}/s, AOI=10000u cells (30000u radius), AutoSave=30s",
                                           dbConfig.poolSize, gatewayConfig.rateLimitPerSecond);

  // Main loop
  while (running) {
    worldServer->update();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  // Shutdown
  std::cout << "\nShutting down servers...\n";
  Umbra::Core::Logger::getInstance().info("Shutting down server stack...");

  gatewayServer->stop();
  std::cout << "[OK] Gateway Server stopped\n";

  worldServer->stop();
  std::cout << "[OK] World Server stopped\n";

  authServer->stop();
  std::cout << "[OK] Auth Server stopped\n";

  dbConnector->disconnect();
  std::cout << "[OK] Database disconnected\n";

  Umbra::Core::Logger::getInstance().info("Server stack shutdown complete");
  std::cout << "\nGoodbye!\n";

  return 0;
}
