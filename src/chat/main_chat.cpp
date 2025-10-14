#include "ChatServer.hpp"
#include "core/Logger.hpp"
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
  
  Umbra::Chat::ChatServer::Config config;
  Umbra::Chat::ChatServer chatServer(config);
  
  if (!chatServer.start()) {
    return 1;
  }
  
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);
  
  while (running) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  chatServer.stop();
  return 0;
}

