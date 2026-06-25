#pragma once

#include "EventManager.hpp"
#include "TimeManager.hpp"
#include <memory>
#include <cstdint>

namespace Umbra {
namespace World {

/**
 * @brief Servidor de lógica global do mundo
 */
class WorldServer {
 public:
  struct Config {
    uint16_t port = 8081;
    uint32_t tickRate = 20;  // Ticks per second
  };
  
  explicit WorldServer(const Config& config);
  ~WorldServer();
  
  bool start();
  void stop();
  bool isRunning() const;
  
  EventManager& getEventManager();
  TimeManager& getTimeManager();
  const Config& getConfig() const;
  
  void update();

 private:
  Config config_;
  std::unique_ptr<EventManager> eventManager_;
  std::unique_ptr<TimeManager> timeManager_;
  bool running_;
};

}  // namespace World
}  // namespace Umbra

