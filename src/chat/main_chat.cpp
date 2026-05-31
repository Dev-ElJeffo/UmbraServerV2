#include "ChatServer.hpp"
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
  Umbra::Core::Logger::getInstance().initialize("logs/chat_server.log");
  Umbra::Core::Logger::getInstance().info("Starting Chat Server...");
  
  auto& configManager = Umbra::Core::ConfigManager::getInstance();
  configManager.loadConfig("config/server.json");
  
  Umbra::Chat::ChatServer::Config config;
  config.port = configManager.get<uint16_t>("chat.port", 8083);
  Umbra::Chat::ChatServer chatServer(config);
  
  const uint16_t adminPort = configManager.get<uint16_t>("admin.chat_port", 9103);
  auto adminServer = Umbra::Admin::createFromConfig(
      "chat_server", adminPort,
      [](int) { running = false; });
  if (adminServer) {
    Umbra::Admin::registerChatCommands(adminServer->getRegistry(), chatServer);
    adminServer->start();
  }
  
  if (!chatServer.start()) {
    return 1;
  }
  
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);
  
  while (running) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  if (adminServer) adminServer->stop();
  chatServer.stop();
  return 0;
}
