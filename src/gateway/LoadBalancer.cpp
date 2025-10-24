#include "LoadBalancer.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace Gateway {

void LoadBalancer::registerServer(const ServerInfo& server) {
  std::lock_guard<std::mutex> lock(mutex_);
  servers_.push_back(server);
  Core::Logger::getInstance().info("Registered server {} at {}:{}", 
                                   server.serverId, server.address, server.port);
}

void LoadBalancer::unregisterServer(uint32_t serverId) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = std::remove_if(servers_.begin(), servers_.end(),
    [serverId](const ServerInfo& s) { return s.serverId == serverId; });
  
  if (it != servers_.end()) {
    servers_.erase(it, servers_.end());
    Core::Logger::getInstance().info("Unregistered server {}", serverId);
  }
}

ServerInfo* LoadBalancer::selectServer() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (servers_.empty()) {
    return nullptr;
  }
  
  // Round-robin selection
  size_t startIndex = nextServerIndex_;
  
  do {
    ServerInfo& server = servers_[nextServerIndex_];
    nextServerIndex_ = (nextServerIndex_ + 1) % servers_.size();
    
    if (server.available && server.currentLoad < server.maxLoad) {
      return &server;
    }
  } while (nextServerIndex_ != startIndex);
  
  return nullptr;
}

void LoadBalancer::updateServerLoad(uint32_t serverId, uint32_t load) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  for (auto& server : servers_) {
    if (server.serverId == serverId) {
      server.currentLoad = load;
      break;
    }
  }
}

size_t LoadBalancer::getServerCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return servers_.size();
}

}  // namespace Gateway
}  // namespace Umbra

