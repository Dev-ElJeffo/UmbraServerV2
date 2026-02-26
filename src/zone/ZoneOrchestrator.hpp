#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <cstdint>

namespace Umbra {
namespace Zone {

/**
 * Informações de uma instância de zone server rodando.
 */
struct ZoneInstance {
  uint32_t instanceId;
  std::string zoneName;
  uint16_t port;
  uint32_t playerCount = 0;
  uint32_t maxPlayers = 100;
  int pid = -1;
  bool managed = false;
  std::chrono::steady_clock::time_point startTime;
};

/**
 * Configuração de uma zone (mapa).
 */
struct ZoneConfig {
  std::string zoneName;
  uint32_t maxPlayersPerInstance = 100;
  uint32_t spawnThreshold = 80;
  uint32_t despawnThreshold = 10;
  uint32_t minInstances = 1;
  uint32_t maxInstances = 5;
};

/**
 * ZoneOrchestrator: gerencia instâncias de zone servers dinamicamente.
 *
 * Monitora a carga (playerCount) de cada instância.
 * Quando uma instância ultrapassa spawnThreshold, cria nova instância.
 * Quando uma instância cai abaixo de despawnThreshold (e há mais que minInstances),
 * marca para shutdown graceful.
 *
 * Pode ser usado standalone (como processo de monitoramento) ou
 * integrado no WorldServer.
 *
 * Não depende de nenhum componente existente — é aditivo.
 */
class ZoneOrchestrator {
 public:
  ZoneOrchestrator();
  ~ZoneOrchestrator();

  /** Define o executável do zone_server. */
  void setZoneServerBinary(const std::string& path) { binaryPath_ = path; }

  /** Define porta base para zones (zona 0 = base, zona 1 = base+1, etc.). */
  void setBasePort(uint16_t port) { basePort_ = port; }

  /** Define o hostname do Gateway para registro. */
  void setGatewayAddress(const std::string& host, uint16_t port) {
    gatewayHost_ = host; gatewayPort_ = port;
  }

  /** Registra configuração de uma zona (mapa). */
  void registerZone(const ZoneConfig& config);

  /** Atualiza contagem de jogadores de uma instância (chamado periodicamente). */
  void updateInstanceLoad(uint32_t instanceId, uint32_t playerCount);

  /** Inicia o loop de monitoramento em background. */
  void start(float checkIntervalSeconds = 10.0f);

  /** Para o monitoramento e todas as instâncias gerenciadas. */
  void stop();

  /** Executa uma verificação manual de scaling. */
  void checkScaling();

  /** Retorna lista de instâncias ativas. */
  std::vector<ZoneInstance> getInstances() const;

  /** Retorna a porta de uma instância com espaço para o jogador na zona dada. */
  uint16_t getAvailablePort(const std::string& zoneName) const;

  /** Callback para quando uma nova instância é criada. */
  void setOnInstanceCreated(std::function<void(const ZoneInstance&)> cb) {
    onInstanceCreated_ = std::move(cb);
  }

  /** Callback para quando uma instância é removida. */
  void setOnInstanceRemoved(std::function<void(uint32_t instanceId)> cb) {
    onInstanceRemoved_ = std::move(cb);
  }

 private:
  std::string binaryPath_ = "./zone_server";
  uint16_t basePort_ = 8082;
  std::string gatewayHost_ = "localhost";
  uint16_t gatewayPort_ = 9000;

  std::unordered_map<std::string, ZoneConfig> zoneConfigs_;
  std::vector<ZoneInstance> instances_;
  mutable std::mutex mutex_;
  std::atomic<uint32_t> nextInstanceId_{1};

  std::unique_ptr<std::thread> monitorThread_;
  std::atomic<bool> running_{false};
  float checkInterval_ = 10.0f;

  std::function<void(const ZoneInstance&)> onInstanceCreated_;
  std::function<void(uint32_t)> onInstanceRemoved_;

  uint16_t allocatePort();
  bool spawnInstance(const std::string& zoneName, const ZoneConfig& config);
  void removeInstance(uint32_t instanceId);
  void monitorLoop();
};

}  // namespace Zone
}  // namespace Umbra
