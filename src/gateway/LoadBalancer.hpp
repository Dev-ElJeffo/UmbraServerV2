#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <cstdint>

namespace Umbra {
namespace Gateway {

struct ServerInfo {
  uint32_t serverId;
  std::string address;
  uint16_t port;
  uint32_t currentLoad;
  uint32_t maxLoad;
  bool available;
};

class LoadBalancer {
 public:
  void registerServer(const ServerInfo& server);
  void unregisterServer(uint32_t serverId);
  
  ServerInfo* selectServer();
  void updateServerLoad(uint32_t serverId, uint32_t load);
  
  size_t getServerCount() const;

 private:
  std::vector<ServerInfo> servers_;
  mutable std::mutex mutex_;
  size_t nextServerIndex_ = 0;
};

}  // namespace Gateway
}  // namespace Umbra

