#pragma once

#include <chrono>
#include <atomic>

namespace Umbra {
namespace World {

class TimeManager {
 public:
  void start();
  void stop();
  void update();
  
  uint64_t getGameTime() const;
  float getDeltaTime() const;

 private:
  std::chrono::steady_clock::time_point startTime_;
  std::chrono::steady_clock::time_point lastUpdate_;
  float deltaTime_;
  std::atomic<bool> running_;
};

}  // namespace World
}  // namespace Umbra

