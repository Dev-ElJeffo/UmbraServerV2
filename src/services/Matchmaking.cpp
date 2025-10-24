#include "Matchmaking.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace Services {

void Matchmaking::addToQueue(const MatchmakingRequest& request) {
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push_back(request);
  Core::Logger::getInstance().debug("Player {} added to matchmaking queue", 
                                    request.playerId);
}

void Matchmaking::removeFromQueue(uint64_t playerId) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = std::remove_if(queue_.begin(), queue_.end(),
    [playerId](const MatchmakingRequest& r) { return r.playerId == playerId; });
  
  if (it != queue_.end()) {
    queue_.erase(it, queue_.end());
    Core::Logger::getInstance().debug("Player {} removed from matchmaking queue", 
                                      playerId);
  }
}

std::vector<Match> Matchmaking::processQueue() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<Match> matches;
  
  // TODO: Implement matchmaking algorithm
  
  return matches;
}

size_t Matchmaking::getQueueSize() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.size();
}

}  // namespace Services
}  // namespace Umbra

