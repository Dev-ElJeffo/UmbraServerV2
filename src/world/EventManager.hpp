#pragma once

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <mutex>

namespace Umbra {
namespace World {

struct WorldEvent {
  std::string type;
  std::string data;
  uint64_t timestamp;
};

class EventManager {
 public:
  using EventCallback = std::function<void(const WorldEvent&)>;
  
  void subscribe(const std::string& eventType, EventCallback callback);
  void publish(const WorldEvent& event);
  void processEvents();

 private:
  std::map<std::string, std::vector<EventCallback>> subscribers_;
  std::vector<WorldEvent> eventQueue_;
  std::mutex mutex_;
};

}  // namespace World
}  // namespace Umbra

