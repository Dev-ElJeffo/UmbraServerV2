#include "WorldServer.hpp"
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
  Umbra::Core::Logger::getInstance().initialize("logs/world_server.log");
  Umbra::Core::Logger::getInstance().info("Starting World Server...");
  
  Umbra::World::WorldServer::Config config;
  Umbra::World::WorldServer worldServer(config);
  
  if (!worldServer.start()) {
    return 1;
  }
  
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);
  
  while (running) {
    worldServer.update();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  
  worldServer.stop();
  return 0;
}

