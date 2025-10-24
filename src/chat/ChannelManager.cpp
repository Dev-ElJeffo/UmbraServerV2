#include "ChannelManager.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace Chat {

bool ChannelManager::createChannel(const std::string& name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (channels_.find(name) != channels_.end()) {
    return false;
  }
  
  ChatChannel channel;
  channel.name = name;
  channels_[name] = channel;
  
  Core::Logger::getInstance().info("Created chat channel: {}", name);
  return true;
}

bool ChannelManager::deleteChannel(const std::string& name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = channels_.find(name);
  if (it != channels_.end()) {
    channels_.erase(it);
    Core::Logger::getInstance().info("Deleted chat channel: {}", name);
    return true;
  }
  
  return false;
}

bool ChannelManager::joinChannel(const std::string& channel, uint64_t playerId) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = channels_.find(channel);
  if (it != channels_.end()) {
    it->second.members.insert(playerId);
    Core::Logger::getInstance().debug("Player {} joined channel {}", 
                                      playerId, channel);
    return true;
  }
  
  return false;
}

bool ChannelManager::leaveChannel(const std::string& channel, uint64_t playerId) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = channels_.find(channel);
  if (it != channels_.end()) {
    it->second.members.erase(playerId);
    Core::Logger::getInstance().debug("Player {} left channel {}", 
                                      playerId, channel);
    return true;
  }
  
  return false;
}

bool ChannelManager::broadcastToChannel(const std::string& channel, 
                                        uint64_t fromPlayerId, 
                                        const std::string& message) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = channels_.find(channel);
  if (it == channels_.end()) {
    return false;
  }
  
  Core::Logger::getInstance().debug("[{}] Player {}: {}", 
                                    channel, fromPlayerId, message);
  
  // TODO: Send message to all members
  return true;
}

size_t ChannelManager::getChannelCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return channels_.size();
}

}  // namespace Chat
}  // namespace Umbra

