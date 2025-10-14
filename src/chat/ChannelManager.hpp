#pragma once

#include <string>
#include <map>
#include <set>
#include <mutex>
#include <cstdint>

namespace Umbra {
namespace Chat {

struct ChatChannel {
  std::string name;
  std::set<uint64_t> members;
  bool persistent = true;
};

class ChannelManager {
 public:
  bool createChannel(const std::string& name);
  bool deleteChannel(const std::string& name);
  
  bool joinChannel(const std::string& channel, uint64_t playerId);
  bool leaveChannel(const std::string& channel, uint64_t playerId);
  
  bool broadcastToChannel(const std::string& channel, uint64_t fromPlayerId, 
                          const std::string& message);
  
  size_t getChannelCount() const;

 private:
  std::map<std::string, ChatChannel> channels_;
  mutable std::mutex mutex_;
};

}  // namespace Chat
}  // namespace Umbra

