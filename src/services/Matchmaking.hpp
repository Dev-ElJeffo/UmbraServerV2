#pragma once

#include <vector>
#include <mutex>
#include <cstdint>

namespace Umbra {
namespace Services {

struct MatchmakingRequest {
  uint64_t playerId;
  uint32_t skillRating;
  std::string gameMode;
};

struct Match {
  uint64_t matchId;
  std::vector<uint64_t> playerIds;
  std::string gameMode;
};

class Matchmaking {
 public:
  void addToQueue(const MatchmakingRequest& request);
  void removeFromQueue(uint64_t playerId);
  
  std::vector<Match> processQueue();
  size_t getQueueSize() const;

 private:
  std::vector<MatchmakingRequest> queue_;
  std::atomic<uint64_t> nextMatchId_;
  mutable std::mutex mutex_;
};

}  // namespace Services
}  // namespace Umbra

