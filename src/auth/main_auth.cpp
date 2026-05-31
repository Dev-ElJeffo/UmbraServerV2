#include "AuthServer.hpp"
#include "admin/AdminBootstrap.hpp"
#include "admin/ServiceAdminRegister.hpp"
#include "core/Logger.hpp"
#include "core/ConfigManager.hpp"
#include "database/MySQLConnector.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>

std::atomic<bool> running(true);

void signalHandler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    running = false;
  }
}

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  Umbra::Core::Logger::getInstance().initialize("logs/auth_server.log");
  Umbra::Core::Logger::getInstance().info("Starting Auth Server...");
  
  auto& configManager = Umbra::Core::ConfigManager::getInstance();
  if (!configManager.loadConfig("config/server.json")) {
    Umbra::Core::Logger::getInstance().error("Failed to load server config");
    return 1;
  }
  
  Umbra::Database::MySQLConnector::Config dbConfig;
  dbConfig.host = configManager.get<std::string>("database.host", "localhost");
  dbConfig.port = configManager.get<uint16_t>("database.port", 3306);
  dbConfig.database = configManager.get<std::string>("database.name", "umbra_eternum");
  dbConfig.username = configManager.get<std::string>("database.user", "root");
  dbConfig.password = configManager.get<std::string>("database.password", "");
  
  auto dbConnector = std::make_shared<Umbra::Database::MySQLConnector>(dbConfig);
  
  Umbra::Auth::AuthServer::Config authConfig;
  authConfig.port = configManager.get<uint16_t>("auth.port", 8080);
  authConfig.jwtSecret = configManager.get<std::string>("auth.jwt_secret", "default_secret");
  authConfig.sessionDurationMinutes = configManager.get<uint32_t>("auth.session_duration", 60);
  
  Umbra::Auth::AuthServer authServer(authConfig, dbConnector);
  
  const uint16_t adminPort = configManager.get<uint16_t>("admin.auth_port", 9100);
  auto adminServer = Umbra::Admin::createFromConfig(
      "auth_server", adminPort,
      [](int) { running = false; });
  if (adminServer) {
    Umbra::Admin::registerAuthCommands(adminServer->getRegistry(), authServer);
    adminServer->start();
  }
  
  if (!authServer.start()) {
    Umbra::Core::Logger::getInstance().error("Failed to start Auth Server");
    return 1;
  }
  
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);
  
  Umbra::Core::Logger::getInstance().info("Auth Server running. Press Ctrl+C to stop.");
  
  while (running) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  Umbra::Core::Logger::getInstance().info("Shutting down Auth Server...");
  if (adminServer) adminServer->stop();
  authServer.stop();
  
  return 0;
}
