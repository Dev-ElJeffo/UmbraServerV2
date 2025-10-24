#include "TimeManager.hpp"

namespace Umbra {
namespace World {

void TimeManager::start() {
  startTime_ = std::chrono::steady_clock::now();
  lastUpdate_ = startTime_;
  running_ = true;
}

void TimeManager::stop() {
  running_ = false;
}

void TimeManager::update() {
  auto now = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - lastUpdate_);
  deltaTime_ = duration.count() / 1000000.0f;
  lastUpdate_ = now;
}

uint64_t TimeManager::getGameTime() const {
  auto now = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_);
  return duration.count();
}

float TimeManager::getDeltaTime() const {
  return deltaTime_;
}

}  // namespace World
}  // namespace Umbra

