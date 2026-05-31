#pragma once

#include "ChannelManager.hpp"
#include <memory>
#include <string>

namespace Umbra {
namespace Chat {

class ChatServer {
 public:
  struct Config {
    uint16_t port = 8083;
  };
  
  explicit ChatServer(const Config& config);
  ~ChatServer();
  
  bool start();
  void stop();
  bool isRunning() const;
  
  bool sendMessage(uint64_t playerId, const std::string& channel, 
                   const std::string& message);
  bool sendWhisper(uint64_t fromPlayerId, uint64_t toPlayerId, 
                   const std::string& message);
  
  ChannelManager& getChannelManager();
  const Config& getConfig() const;

 private:
  Config config_;
  std::unique_ptr<ChannelManager> channelManager_;
  bool running_;
};

}  // namespace Chat
}  // namespace Umbra

