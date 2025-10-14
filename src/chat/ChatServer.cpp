#include "ChatServer.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace Chat {

ChatServer::ChatServer(const Config& config)
    : config_(config), running_(false) {
  channelManager_ = std::make_unique<ChannelManager>();
}

ChatServer::~ChatServer() {
  stop();
}

bool ChatServer::start() {
  running_ = true;
  
  // Create default channels
  channelManager_->createChannel("global");
  channelManager_->createChannel("trade");
  channelManager_->createChannel("lfg");
  
  Core::Logger::getInstance().info("ChatServer started on port {}", config_.port);
  return true;
}

void ChatServer::stop() {
  running_ = false;
  Core::Logger::getInstance().info("ChatServer stopped");
}

bool ChatServer::isRunning() const {
  return running_;
}

bool ChatServer::sendMessage(uint64_t playerId, const std::string& channel, 
                             const std::string& message) {
  return channelManager_->broadcastToChannel(channel, playerId, message);
}

bool ChatServer::sendWhisper(uint64_t fromPlayerId, uint64_t toPlayerId, 
                             const std::string& message) {
  Core::Logger::getInstance().debug("Whisper from {} to {}: {}", 
                                    fromPlayerId, toPlayerId, message);
  // TODO: Implement whisper logic
  return true;
}

ChannelManager& ChatServer::getChannelManager() {
  return *channelManager_;
}

}  // namespace Chat
}  // namespace Umbra

