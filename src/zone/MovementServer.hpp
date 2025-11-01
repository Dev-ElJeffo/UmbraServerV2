#pragma once

#include <unordered_map>
#include <mutex>
#include <chrono>
#include "network/WebSocketServer.hpp"
#include "zone/MovementProtocol.hpp"
#include "core/Logger.hpp"

namespace Umbra {
namespace Zone {

struct PlayerStateNet {
  uint32_t playerId;
  float x, y, z;
  float yaw;
  uint32_t tsMs;
};

class MovementServer {
public:
  explicit MovementServer(uint16_t port)
    : ws_(port) {}

  bool start() {
    ws_.setConnectionCallback([this](uint32_t cid, bool connected){
      if (connected) {
        Umbra::Core::Logger::getInstance().info("WS client {} connected", cid);
        // Enviar snapshot inicial para o novo cliente
        // NOTA: sendInitialSnapshotLocked precisa de lock porque é chamado do callback sem lock
        std::lock_guard<std::mutex> lock(mu_);
        sendInitialSnapshotLocked(cid);
      } else {
        Umbra::Core::Logger::getInstance().info("WS client {} disconnected", cid);
      }
    });

    ws_.setBinaryCallback([this](uint32_t cid, const std::vector<uint8_t>& data){
      MovementFrame f{};
      if (!decode(data, f)) {
        Umbra::Core::Logger::getInstance().warn("Failed to decode binary message from client {}", cid);
        return;
      }
      if (f.type != MovementMsgType::MoveUpdate) {
        Umbra::Core::Logger::getInstance().debug("Received non-MoveUpdate frame from client {} (type: {})", cid, static_cast<int>(f.type));
        return;
      }
      Umbra::Core::Logger::getInstance().debug("Received MoveUpdate from client {}: player_id={}, pos=({}, {}, {}), yaw={}", 
                                                cid, f.playerId, f.x, f.y, f.z, f.yaw);
      handleMoveUpdate(cid, f);
    });

    return ws_.start();
  }

  void stop() { ws_.stop(); }

  // snapshot a 10–20 Hz deve ser chamado externamente por um timer
  void broadcastSnapshot() {
    std::lock_guard<std::mutex> lock(mu_);
    for (const auto& [pid, st] : players_) {
      MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
      auto bytes = encode(f);
      ws_.broadcastBinary(bytes);
    }
  }

  void setLimits(float maxSpeed, float maxTeleportDist, uint32_t maxDelayMs) {
    maxSpeed_ = maxSpeed; maxTeleportDist_ = maxTeleportDist; maxDelayMs_ = maxDelayMs;
  }

private:
  // Versão com lock - chamada do callback de conexão (sem lock prévio)
  void sendInitialSnapshotLocked(uint32_t clientId) {
    size_t sentCount = 0;
    for (const auto& [pid, st] : players_) {
      MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
      auto bytes = encode(f);
      if (ws_.sendBinary(clientId, bytes)) {
        sentCount++;
      }
    }
    Umbra::Core::Logger::getInstance().info("Sent initial snapshot to client {} ({} players)", 
                                             clientId, sentCount);
  }

  // Versão sem lock - chamada de handleMoveUpdate que já tem lock
  void sendFullSnapshotToAllUnlocked() {
    for (const auto& [pid, st] : players_) {
      MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
      auto bytes = encode(f);
      ws_.broadcastBinary(bytes);
    }
    Umbra::Core::Logger::getInstance().debug("Broadcasted full snapshot to all clients ({} players)", players_.size());
  }

  void handleMoveUpdate(uint32_t cid, const MovementFrame& f) {
    auto now = std::chrono::steady_clock::now();
    uint32_t nowMs = static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count());

    std::lock_guard<std::mutex> lock(mu_);
    bool isNewPlayer = (players_.find(f.playerId) == players_.end());
    
    // Para novos players, sempre aceitar o primeiro movimento e usar timestamp do servidor
    // Isso resolve problemas de sincronização de clock (cliente usa "Game Time", servidor usa "epoch")
    if (!isNewPlayer) {
      // Validação de delay apenas para players existentes
      // Como o cliente usa "Game Time" (tempo relativo), não podemos validar delay absoluto
      // Apenas validamos se o timestamp está muito no futuro (mais de 5 segundos do tempo atual)
      if (f.tsMs > nowMs && (f.tsMs - nowMs) > 5000) {
        Umbra::Core::Logger::getInstance().warn("MoveUpdate from client {} rejected: timestamp too far in future ({}ms)", 
                                                cid, f.tsMs - nowMs);
        return;
      }
      
      auto it = players_.find(f.playerId);
      if (it != players_.end()) {
        float dx = f.x - it->second.x;
        float dy = f.y - it->second.y;
        float dz = f.z - it->second.z;
        float dist2 = dx*dx + dy*dy + dz*dz;
        if (dist2 > maxTeleportDist_ * maxTeleportDist_) {
          Umbra::Core::Logger::getInstance().warn("MoveUpdate from client {} rejected: teleport distance too high", cid);
          return;
        }
        // velocidade aprox em uu/s usando timestamp relativo do próprio cliente
        // Se o timestamp do frame é maior que o anterior, usar a diferença
        // Senão, assumir intervalo mínimo (1 frame = ~16ms)
        uint32_t prevTs = it->second.tsMs;
        float dt = (f.tsMs > prevTs) ? (f.tsMs - prevTs) / 1000.0f : 0.016f;
        float speed = (dt > 0.0f) ? std::sqrt(dist2) / dt : 0.0f;
        if (speed > maxSpeed_) {
          Umbra::Core::Logger::getInstance().warn("MoveUpdate from client {} rejected: speed too high ({})", cid, speed);
          return;
        }
      }
    } else {
      Umbra::Core::Logger::getInstance().info("First MoveUpdate from player {} (client {}): accepting with server timestamp", 
                                             f.playerId, cid);
    }
    
    // Para novos players, usar timestamp do servidor (sincronizado)
    // Para players existentes, manter timestamp relativo do cliente (para cálculo de velocidade)
    uint32_t finalTimestamp = isNewPlayer ? nowMs : f.tsMs;
    
    players_[f.playerId] = PlayerStateNet{f.playerId, f.x, f.y, f.z, f.yaw, finalTimestamp};
    
    if (isNewPlayer) {
      Umbra::Core::Logger::getInstance().info("New player {} (from client {}) - broadcasting initial state to all clients", 
                                              f.playerId, cid);
      // Novo player: enviar snapshot completo para todos os clientes existentes
      // NOTA: Usar versão sem lock porque já temos o lock aqui
      sendFullSnapshotToAllUnlocked();
    }

    // broadcast imediato de state_update (além do snapshot periódico)
    // Usar finalTimestamp (timestamp do servidor para novos, relativo para existentes)
    MovementFrame out{MovementMsgType::StateUpdate, f.playerId, f.x, f.y, f.z, f.yaw, finalTimestamp};
    ws_.broadcastBinary(encode(out));
    Umbra::Core::Logger::getInstance().debug("Broadcasted StateUpdate for player {} (from client {}, ts={})", f.playerId, cid, finalTimestamp);
  }

  Umbra::Network::WebSocketServer ws_;
  std::mutex mu_;
  std::unordered_map<uint32_t, PlayerStateNet> players_;
  float maxSpeed_ = 1200.0f;
  float maxTeleportDist_ = 3000.0f;
  uint32_t maxDelayMs_ = 300;
};

} // namespace Zone
} // namespace Umbra


