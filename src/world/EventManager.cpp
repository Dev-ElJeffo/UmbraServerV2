#include "EventManager.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace World {

void EventManager::subscribe(const std::string& eventType, EventCallback callback) {
  std::lock_guard<std::mutex> lock(mutex_);
  subscribers_[eventType].push_back(callback);
}

void EventManager::publish(const WorldEvent& event) {
  std::lock_guard<std::mutex> lock(mutex_);
  eventQueue_.push_back(event);
}

void EventManager::processEvents() {
  std::vector<WorldEvent> events;
  
  {
    std::lock_guard<std::mutex> lock(mutex_);
    events = std::move(eventQueue_);
    eventQueue_.clear();
  }
  
  for (const auto& event : events) {
    auto it = subscribers_.find(event.type);
    if (it != subscribers_.end()) {
      for (const auto& callback : it->second) {
        try {
          callback(event);
        } catch (const std::exception& e) {
          Core::Logger::getInstance().error("Event callback error: {}", e.what());
        }
      }
    }
  }
}

}  // namespace World
}  // namespace Umbra

