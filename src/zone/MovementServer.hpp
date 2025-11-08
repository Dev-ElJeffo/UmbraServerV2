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
  // Dados de animação
  float speed = 0.0f;
  float velocityZ = 0.0f;
  bool isInAir = false;
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
        // Remover player associado a este client quando desconectar
        std::lock_guard<std::mutex> lock(mu_);
        handleClientDisconnect(cid);
      }
    });

    ws_.setBinaryCallback([this](uint32_t cid, const std::vector<uint8_t>& data){
      MovementFrame f{};
      float speed = 0.0f;
      float velocityZ = 0.0f;
      bool isInAir = false;
      
      // Tentar decodificar como frame com animação primeiro (34 bytes)
      bool hasAnimation = decodeWithAnimation(data, f, speed, velocityZ, isInAir);
      
      if (f.type != MovementMsgType::MoveUpdate) {
        Umbra::Core::Logger::getInstance().debug("Received non-MoveUpdate frame from client {} (type: {})", cid, static_cast<int>(f.type));
        return;
      }
      
      if (hasAnimation) {
        Umbra::Core::Logger::getInstance().debug("Received MoveUpdate with animation from client {}: player_id={}, pos=({}, {}, {}), yaw={}, speed={}, velocityZ={}, isInAir={}", 
                                                  cid, f.playerId, f.x, f.y, f.z, f.yaw, speed, velocityZ, isInAir);
      } else {
        Umbra::Core::Logger::getInstance().debug("Received MoveUpdate (no animation) from client {}: player_id={}, pos=({}, {}, {}), yaw={}", 
                                                  cid, f.playerId, f.x, f.y, f.z, f.yaw);
      }
      
      handleMoveUpdate(cid, f, hasAnimation, speed, velocityZ, isInAir);
    });

    return ws_.start();
  }

  void stop() { ws_.stop(); }

  // snapshot a 10–20 Hz deve ser chamado externamente por um timer
  void broadcastSnapshot() {
    std::lock_guard<std::mutex> lock(mu_);
    for (const auto& [pid, st] : players_) {
      MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
      // Usar encodeWithAnimation se houver dados de animação (sempre usar agora)
      auto bytes = encodeWithAnimation(f, st.speed, st.velocityZ, st.isInAir);
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
      // Usar encodeWithAnimation para enviar sempre frames de 34 bytes
      auto bytes = encodeWithAnimation(f, st.speed, st.velocityZ, st.isInAir);
      Umbra::Core::Logger::getInstance().info("Sending initial snapshot to client {}: PlayerID={}, pos=({}, {}, {}), yaw={}, speed={}, velocityZ={}, isInAir={}, frame_size={} bytes", 
                                               clientId, f.playerId, f.x, f.y, f.z, f.yaw, st.speed, st.velocityZ, st.isInAir, bytes.size());
      if (bytes.size() >= 5) {
        Umbra::Core::Logger::getInstance().info("  Frame bytes [0-4]: {:02X} {:02X} {:02X} {:02X} {:02X}", 
                                                 bytes[0], bytes[1], bytes[2], bytes[3], bytes[4]);
      }
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
      // Usar encodeWithAnimation para enviar sempre frames de 34 bytes
      auto bytes = encodeWithAnimation(f, st.speed, st.velocityZ, st.isInAir);
      Umbra::Core::Logger::getInstance().debug("Broadcasting snapshot: PlayerID={}, pos=({}, {}, {}), yaw={}, speed={}, velocityZ={}, isInAir={}, frame_size={} bytes", 
                                               f.playerId, f.x, f.y, f.z, f.yaw, st.speed, st.velocityZ, st.isInAir, bytes.size());
      if (bytes.size() >= 5) {
        Umbra::Core::Logger::getInstance().debug("  Frame bytes [0-4]: {:02X} {:02X} {:02X} {:02X} {:02X}", 
                                                  bytes[0], bytes[1], bytes[2], bytes[3], bytes[4]);
      }
      ws_.broadcastBinary(bytes);
    }
    Umbra::Core::Logger::getInstance().debug("Broadcasted full snapshot to all clients ({} players)", players_.size());
  }

  void handleMoveUpdate(uint32_t cid, const MovementFrame& f, bool hasAnimation, float speed, float velocityZ, bool isInAir) {
    std::lock_guard<std::mutex> lock(mu_);
    
    // Atualizar mapeamento ClientID -> PlayerID
    clientIdToPlayerId_[cid] = f.playerId;
    
    bool isNewPlayer = (players_.find(f.playerId) == players_.end());
    
    // SEMPRE usar timestamp relativo do cliente (f.tsMs) para manter consistência
    // O cliente usa "Game Time" (relativo), então todos os timestamps devem ser relativos
    if (!isNewPlayer) {
      auto it = players_.find(f.playerId);
      if (it != players_.end()) {
        float dx = f.x - it->second.x;
        float dy = f.y - it->second.y;
        float dz = f.z - it->second.z;
        float dist2 = dx*dx + dy*dy + dz*dz;
        
        // Verificar teleporte primeiro
        if (dist2 > maxTeleportDist_ * maxTeleportDist_) {
          Umbra::Core::Logger::getInstance().warn("MoveUpdate from client {} rejected: teleport distance too high (dist={})", 
                                                  cid, std::sqrt(dist2));
          return;
        }
        
        // Calcular velocidade apenas se houver movimento significativo
        if (dist2 > 0.01f) {  // Ignorar movimentos muito pequenos (< 0.1 unidades)
          uint32_t prevTs = it->second.tsMs;
          float dt;
          
          // Se é o primeiro movimento após spawn ou timestamp anterior é 0, usar intervalo padrão
          if (prevTs == 0) {
            dt = 0.033f;  // ~30 FPS como padrão seguro
            Umbra::Core::Logger::getInstance().debug("First movement after spawn for player {}, using default dt=0.033s", f.playerId);
          } else if (f.tsMs > prevTs) {
            // Calcular dt baseado na diferença entre timestamps relativos do cliente
            uint32_t timeDiff = f.tsMs - prevTs;
            // Se a diferença for muito grande (>10s), provavelmente houve um reset ou problema
            if (timeDiff > 10000) {
              dt = 0.033f;  // Usar padrão seguro
              Umbra::Core::Logger::getInstance().debug("Large time difference for player {} ({}ms), using default dt", f.playerId, timeDiff);
            } else {
              dt = timeDiff / 1000.0f;
              // Garantir dt mínimo razoável (pelo menos 1 frame = ~16ms)
              if (dt < 0.001f) {
                dt = 0.033f;  // Se dt for muito pequeno, usar padrão
                Umbra::Core::Logger::getInstance().debug("Very small dt for player {} ({}s), using default dt=0.033s", f.playerId, dt);
              }
            }
          } else {
            // Timestamp regrediu ou igual: pode ser reset de Game Time no cliente
            // Se a distância for razoável (não é teleporte), aceitar sem validação de velocidade
            // Se for grande, já foi validado como teleporte acima
            if (dist2 > (maxTeleportDist_ * maxTeleportDist_)) {
              // Distância grande: já passou validação de teleporte, aceitar
              dt = 0.033f;  // Usar padrão, mas não validar velocidade (teleporte legítimo)
            } else {
              // Distância pequena: usar dt conservador maior para evitar rejeição incorreta
              dt = 0.1f;  // 100ms como padrão mais conservador
            }
            Umbra::Core::Logger::getInstance().debug("Timestamp regressed or equal for player {} (prev={}, curr={}), using dt={}s (dist={})", 
                                                      f.playerId, prevTs, f.tsMs, dt, std::sqrt(dist2));
            // Quando timestamp regrediu, não validar velocidade (pode ser reset legítimo)
            // Apenas validar teleporte (já feito acima)
            goto skip_speed_check;
          }
          
          float calculatedSpeed = std::sqrt(dist2) / dt;
          if (calculatedSpeed > maxSpeed_) {
            Umbra::Core::Logger::getInstance().warn("MoveUpdate from client {} rejected: speed too high (speed={}, dist={}, dt={}, prevTs={}, currTs={})", 
                                                    cid, calculatedSpeed, std::sqrt(dist2), dt, prevTs, f.tsMs);
            return;
          }
          skip_speed_check:;
        }
      }
    } else {
      Umbra::Core::Logger::getInstance().info("First MoveUpdate from player {} (client {}): accepting with client timestamp {}", 
                                             f.playerId, cid, f.tsMs);
    }
    
    // SEMPRE usar timestamp relativo do cliente para manter consistência
    // Isso permite cálculo correto de velocidade entre movimentos
    uint32_t finalTimestamp = f.tsMs;
    
    // Atualizar estado do player com dados de animação se disponíveis
    players_[f.playerId] = PlayerStateNet{
      f.playerId, 
      f.x, f.y, f.z, 
      f.yaw, 
      finalTimestamp,
      speed,        // Dados de animação
      velocityZ,
      isInAir
    };
    
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
    std::vector<uint8_t> broadcastBytes;
    
    // Se o frame original tinha animação, reenviar com animação também
    if (hasAnimation) {
      broadcastBytes = encodeWithAnimation(out, speed, velocityZ, isInAir);
      Umbra::Core::Logger::getInstance().debug("Broadcasting StateUpdate with animation: PlayerID={}, pos=({}, {}, {}), yaw={}, speed={}, velocityZ={}, isInAir={}, ts={}, frame_size={} bytes", 
                                               out.playerId, out.x, out.y, out.z, out.yaw, speed, velocityZ, isInAir, finalTimestamp, broadcastBytes.size());
    } else {
      broadcastBytes = encode(out);
      Umbra::Core::Logger::getInstance().debug("Broadcasting StateUpdate (no animation): PlayerID={}, pos=({}, {}, {}), yaw={}, ts={}, frame_size={} bytes", 
                                               out.playerId, out.x, out.y, out.z, out.yaw, finalTimestamp, broadcastBytes.size());
    }
    
    if (broadcastBytes.size() >= 5) {
      Umbra::Core::Logger::getInstance().debug("  Frame bytes [0-4]: {:02X} {:02X} {:02X} {:02X} {:02X}", 
                                               broadcastBytes[0], broadcastBytes[1], broadcastBytes[2], broadcastBytes[3], broadcastBytes[4]);
    }
    ws_.broadcastBinary(broadcastBytes);
    Umbra::Core::Logger::getInstance().debug("Broadcasted StateUpdate for player {} (from client {}, ts={}, hasAnimation={})", f.playerId, cid, finalTimestamp, hasAnimation);
  }

  // Remove player quando client desconecta
  void handleClientDisconnect(uint32_t cid) {
    auto it = clientIdToPlayerId_.find(cid);
    if (it != clientIdToPlayerId_.end()) {
      uint32_t playerId = it->second;
      auto playerIt = players_.find(playerId);
      if (playerIt != players_.end()) {
        Umbra::Core::Logger::getInstance().info("Removing player {} (client {}) from players map", playerId, cid);
        players_.erase(playerIt);
        
        // Notificar todos os clientes que este player desconectou
        auto disconnectMsg = encodePlayerDisconnected(playerId);
        ws_.broadcastBinary(disconnectMsg);
        Umbra::Core::Logger::getInstance().info("Broadcasted PlayerDisconnected message for player {}", playerId);
      }
      clientIdToPlayerId_.erase(it);
    } else {
      Umbra::Core::Logger::getInstance().debug("Client {} disconnected but had no associated player", cid);
    }
  }

  Umbra::Network::WebSocketServer ws_;
  std::mutex mu_;
  std::unordered_map<uint32_t, PlayerStateNet> players_;
  std::unordered_map<uint32_t, uint32_t> clientIdToPlayerId_; // Mapeamento ClientID -> PlayerID
  float maxSpeed_ = 1200.0f;
  float maxTeleportDist_ = 3000.0f;
  uint32_t maxDelayMs_ = 300;
};

} // namespace Zone
} // namespace Umbra


