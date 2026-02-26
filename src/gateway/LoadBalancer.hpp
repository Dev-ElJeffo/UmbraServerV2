#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <cstdint>
#include <chrono>

namespace Umbra {
namespace Gateway {

struct ServerInfo {
  uint32_t serverId;
  std::string address;
  uint16_t port;
  uint32_t currentLoad;
  uint32_t maxLoad;
  bool available;
  std::string zoneName;
  uint32_t instanceId = 0;
  std::chrono::steady_clock::time_point lastHeartbeat;
};

class LoadBalancer {
 public:
  void registerServer(const ServerInfo& server);
  void unregisterServer(uint32_t serverId);
  
  ServerInfo* selectServer();
  void updateServerLoad(uint32_t serverId, uint32_t load);
  
  /** Busca o melhor zone server para uma zona específica (menor carga). */
  ServerInfo* selectServerForZone(const std::string& zoneName);

  /** Atualiza heartbeat de um server. */
  void heartbeat(uint32_t serverId, uint32_t currentLoad);

  /** Remove servidores que não enviaram heartbeat no timeout. */
  void pruneStaleServers(uint32_t timeoutSeconds = 60);

  /** Retorna lista de zones registradas com carga. */
  std::vector<ServerInfo> getZoneList() const;

  size_t getServerCount() const;

 private:
  std::vector<ServerInfo> servers_;
  mutable std::mutex mutex_;
  size_t nextServerIndex_ = 0;
};

}  // namespace Gateway
}  // namespace Umbra

