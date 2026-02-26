#include "LoadBalancer.hpp"
#include "core/Logger.hpp"
#include <algorithm>

namespace Umbra {
namespace Gateway {

void LoadBalancer::registerServer(const ServerInfo& server) {
  std::lock_guard<std::mutex> lock(mutex_);

  for (auto& s : servers_) {
    if (s.serverId == server.serverId) {
      s = server;
      s.lastHeartbeat = std::chrono::steady_clock::now();
      Core::Logger::getInstance().info("Updated zone server {} ({}:{}, zone='{}', instance={})",
                                       server.serverId, server.address, server.port,
                                       server.zoneName, server.instanceId);
      return;
    }
  }

  ServerInfo s = server;
  s.lastHeartbeat = std::chrono::steady_clock::now();
  servers_.push_back(s);
  Core::Logger::getInstance().info("Registered zone server {} ({}:{}, zone='{}', instance={}, maxLoad={})",
                                   server.serverId, server.address, server.port,
                                   server.zoneName, server.instanceId, server.maxLoad);
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
  
  if (servers_.empty()) return nullptr;
  
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

ServerInfo* LoadBalancer::selectServerForZone(const std::string& zoneName) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  ServerInfo* best = nullptr;
  uint32_t bestLoad = UINT32_MAX;
  
  for (auto& server : servers_) {
    if (server.zoneName == zoneName && server.available && server.currentLoad < server.maxLoad) {
      if (server.currentLoad < bestLoad) {
        best = &server;
        bestLoad = server.currentLoad;
      }
    }
  }
  
  return best;
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

void LoadBalancer::heartbeat(uint32_t serverId, uint32_t currentLoad) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& server : servers_) {
    if (server.serverId == serverId) {
      server.currentLoad = currentLoad;
      server.lastHeartbeat = std::chrono::steady_clock::now();
      server.available = true;
      return;
    }
  }
}

void LoadBalancer::pruneStaleServers(uint32_t timeoutSeconds) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto now = std::chrono::steady_clock::now();

  auto it = std::remove_if(servers_.begin(), servers_.end(),
    [&](const ServerInfo& s) {
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - s.lastHeartbeat);
      if (elapsed.count() > timeoutSeconds) {
        Core::Logger::getInstance().warn("Zone server {} (zone='{}') timed out after {}s, removing",
                                         s.serverId, s.zoneName, elapsed.count());
        return true;
      }
      return false;
    });

  servers_.erase(it, servers_.end());
}

std::vector<ServerInfo> LoadBalancer::getZoneList() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return servers_;
}

size_t LoadBalancer::getServerCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return servers_.size();
}

}  // namespace Gateway
}  // namespace Umbra
