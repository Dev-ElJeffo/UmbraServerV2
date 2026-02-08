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
  // Dados do personagem
  std::string characterName;
  std::string characterTitle;
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
      // Verificar tipo de mensagem primeiro
      if (data.empty()) return;
      MovementMsgType msgType = static_cast<MovementMsgType>(data[0]);
      
      // ========== PROCESSAR MENSAGENS SOCIAIS ==========
      
      // Party Invite
      if (msgType == MovementMsgType::PartyInvite) {
        uint32_t fromId, toId, partyId;
        if (decodePartyInvite(data, fromId, toId, partyId)) {
          Umbra::Core::Logger::getInstance().info("Received PartyInvite from client {} (player {}): to={}, party={}", 
                                                  cid, fromId, toId, partyId);
          std::lock_guard<std::mutex> lock(mu_);
          sendToPlayerUnlocked(toId, encodePartyInviteReceived(fromId, toId, partyId));
        }
        return;
      }
      
      // Trade Request
      if (msgType == MovementMsgType::TradeRequest) {
        uint32_t fromId, toId;
        if (decodeTradeRequest(data, fromId, toId)) {
          Umbra::Core::Logger::getInstance().info("Received TradeRequest from client {} (player {}): to={}", 
                                                  cid, fromId, toId);
          std::lock_guard<std::mutex> lock(mu_);
          sendToPlayerUnlocked(toId, encodeTradeRequestReceived(fromId, toId));
        }
        return;
      }
      
      // Trade Started Notify (cliente aceitou via HTTP, notifica para ambos verem WBP_Trade)
      if (msgType == MovementMsgType::TradeStartedNotify) {
        uint32_t tradeSessionId, player1Id, player2Id;
        if (decodeTradeStartedNotify(data, tradeSessionId, player1Id, player2Id)) {
          Umbra::Core::Logger::getInstance().info("Received TradeStartedNotify from client {}: session={}, p1={}, p2={}", 
                                                  cid, tradeSessionId, player1Id, player2Id);
          std::lock_guard<std::mutex> lock(mu_);
          auto msg = encodeTradeStarted(tradeSessionId, player1Id, player2Id);
          sendToPlayerUnlocked(player1Id, msg);
          sendToPlayerUnlocked(player2Id, msg);
        }
        return;
      }
      
      // Friend Request
      if (msgType == MovementMsgType::FriendRequest) {
        uint32_t fromId, toId;
        if (decodeFriendRequest(data, fromId, toId)) {
          Umbra::Core::Logger::getInstance().info("Received FriendRequest from client {} (player {}): to={}", 
                                                  cid, fromId, toId);
          std::lock_guard<std::mutex> lock(mu_);
          sendToPlayerUnlocked(toId, encodeFriendRequestReceived(fromId, toId));
        }
        return;
      }
      
      // Whisper Message
      if (msgType == MovementMsgType::WhisperMessage) {
        uint32_t fromId, toId;
        std::string message;
        if (decodeWhisper(data, fromId, toId, message)) {
          Umbra::Core::Logger::getInstance().info("Received Whisper from client {} (player {}): to={}, msg={}", 
                                                  cid, fromId, toId, message);
          std::lock_guard<std::mutex> lock(mu_);
          sendToPlayerUnlocked(toId, encodeWhisperReceived(fromId, toId, message));
        }
        return;
      }
      
      // Party/Trade/Friend Response
      if (msgType == MovementMsgType::PartyInviteResponse || 
          msgType == MovementMsgType::TradeRequestResponse ||
          msgType == MovementMsgType::FriendRequestResponse) {
        uint32_t requestId;
        bool accepted;
        if (decodeSocialResponse(data, requestId, accepted)) {
          Umbra::Core::Logger::getInstance().info("Received SocialResponse from client {}: requestId={}, accepted={}", 
                                                  cid, requestId, accepted);
          // A resposta será processada pela API PHP, aqui apenas logamos
        }
        return;
      }
      
      // ========== PROCESSAR MENSAGENS DE MOVIMENTO ==========
      
      // Processar PlayerInfoUpdate
      if (msgType == MovementMsgType::PlayerInfoUpdate) {
        uint32_t playerId;
        std::string name, title;
        if (decodePlayerInfoUpdate(data, playerId, name, title)) {
          std::lock_guard<std::mutex> lock(mu_);
          
          Umbra::Core::Logger::getInstance().info("Received PlayerInfoUpdate from client {}: playerId={}, name={}, title={}", 
                                                  cid, playerId, name, title);
          
          // Verificar se é um novo player
          bool isNewPlayer = (players_.find(playerId) == players_.end());
          
          Umbra::Core::Logger::getInstance().info("📥 Received PlayerInfoUpdate from client {}: playerId={}, name={}, title={}, isNewPlayer={}, total_players={}", 
                                                  cid, playerId, name, title, isNewPlayer, players_.size());
          
          // Atualizar PlayerStateNet
          if (players_.find(playerId) != players_.end()) {
            // Player já existe: apenas atualizar nome/título
            players_[playerId].characterName = name;
            players_[playerId].characterTitle = title;
            Umbra::Core::Logger::getInstance().info("✅ Updated existing PlayerStateNet for player {} (name={}, title={})", 
                                                    playerId, name, title);
          } else {
            // ✅ CRÍTICO: NÃO criar PlayerStateNet com posição padrão aqui!
            // Isso causa problemas porque quando o primeiro MoveUpdate chega com a posição real,
            // a distância da posição padrão para a real é enorme e é rejeitado como teleporte.
            // 
            // Solução: Criar PlayerStateNet apenas com nome/título, SEM posição.
            // A posição será definida quando o primeiro MoveUpdate chegar.
            // 
            // NOTA: Isso significa que o snapshot inicial não enviará StateUpdate para este player
            // até que ele envie seu primeiro MoveUpdate. Mas isso é OK porque o PlayerInfoUpdate
            // já foi enviado, então o client pode spawnar o actor e aguardar o StateUpdate.
            PlayerStateNet newPlayer;
            newPlayer.playerId = playerId;
            newPlayer.characterName = name;
            newPlayer.characterTitle = title;
            // ✅ NÃO definir posição aqui - será definida no primeiro MoveUpdate
            // Usar valores que indicam "posição ainda não definida"
            newPlayer.x = 0.0f;
            newPlayer.y = 0.0f;
            newPlayer.z = 0.0f;
            newPlayer.yaw = 0.0f;
            newPlayer.tsMs = 0;
            players_[playerId] = newPlayer;
            
            Umbra::Core::Logger::getInstance().info("✅ Created new PlayerStateNet for player {} (name={}, title={}) - posição será definida no primeiro MoveUpdate", 
                                                    playerId, name, title);
          }
          
          // Fazer broadcast do PlayerInfoUpdate para todos os clientes (EXCETO o próprio que enviou)
          auto broadcastMsg = encodePlayerInfoUpdate(playerId, name, title);
          ws_.broadcastBinary(broadcastMsg);
          Umbra::Core::Logger::getInstance().info("📤 Broadcasted PlayerInfoUpdate for player {} (name={}, title={}) to all clients", 
                                                  playerId, name, title);
          
        }
        return;
      }
      
      // Processar MoveUpdate (código existente)
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
    size_t sentStateCount = 0;
    size_t sentInfoCount = 0;
    
    // ✅ CRÍTICO: Primeiro enviar PlayerInfoUpdate para todos os players existentes
    // Isso garante que o novo client receba os nomes/títulos ANTES dos StateUpdate
    // que spawnam os actors
    for (const auto& [pid, st] : players_) {
      // Só enviar PlayerInfoUpdate se houver nome/título (players que já enviaram seu PlayerInfoUpdate)
      if (!st.characterName.empty() || !st.characterTitle.empty()) {
        auto infoMsg = encodePlayerInfoUpdate(st.playerId, st.characterName, st.characterTitle);
        if (ws_.sendBinary(clientId, infoMsg)) {
          sentInfoCount++;
          Umbra::Core::Logger::getInstance().info("📤 Sending initial PlayerInfoUpdate to client {}: PlayerID={}, name={}, title={}", 
                                                   clientId, st.playerId, st.characterName, st.characterTitle);
        }
      }
    }
    
    // Depois enviar StateUpdate para spawnar os actors
    // ✅ CRÍTICO: Só enviar StateUpdate se o player tiver uma posição válida (não 0,0,0)
    // Players que acabaram de enviar PlayerInfoUpdate ainda não têm posição definida
    for (const auto& [pid, st] : players_) {
      // Verificar se a posição é válida (não é 0,0,0 que indica "posição ainda não definida")
      bool hasValidPosition = (st.x != 0.0f || st.y != 0.0f || st.z != 0.0f);
      if (!hasValidPosition) {
        Umbra::Core::Logger::getInstance().info("Skipping StateUpdate for player {} in initial snapshot - position not yet defined (will be sent after first MoveUpdate)", 
                                                 st.playerId);
        continue;
      }
      
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
        sentStateCount++;
      }
    }
    Umbra::Core::Logger::getInstance().info("Sent initial snapshot to client {} ({} PlayerInfoUpdate, {} StateUpdate)", 
                                             clientId, sentInfoCount, sentStateCount);
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
    
    // ✅ CRÍTICO: Se for um novo player OU se o player existir mas tiver posição inválida (0,0,0),
    // não validar teleporte no primeiro movimento
    bool isFirstMovement = isNewPlayer;
    if (!isNewPlayer) {
      auto it = players_.find(f.playerId);
      if (it != players_.end()) {
        // Verificar se a posição é válida (não é 0,0,0 que indica "posição ainda não definida")
        bool hasValidPosition = (it->second.x != 0.0f || it->second.y != 0.0f || it->second.z != 0.0f);
        if (!hasValidPosition) {
          isFirstMovement = true;
          Umbra::Core::Logger::getInstance().info("First movement for player {} (position was not yet defined)", f.playerId);
        }
      }
    }
    
    // SEMPRE usar timestamp relativo do cliente (f.tsMs) para manter consistência
    // O cliente usa "Game Time" (relativo), então todos os timestamps devem ser relativos
    if (!isFirstMovement) {
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
      // Primeiro movimento: aceitar sem validação de teleporte
      Umbra::Core::Logger::getInstance().info("First MoveUpdate from player {} (client {}): accepting with client timestamp {} (no teleport validation)", 
                                             f.playerId, cid, f.tsMs);
    }
    
    // SEMPRE usar timestamp relativo do cliente para manter consistência
    // Isso permite cálculo correto de velocidade entre movimentos
    uint32_t finalTimestamp = f.tsMs;
    
    // ✅ CRÍTICO: Verificar se esta é a primeira vez que este player define sua posição
    // (posição anterior era 0,0,0 indicando "posição ainda não definida")
    // IMPORTANTE: Fazer isso ANTES de atualizar o map, para podermos verificar a posição anterior
    bool isFirstPositionUpdate = false;
    if (!isNewPlayer) {
      auto it = players_.find(f.playerId);
      if (it != players_.end()) {
        bool hadInvalidPosition = (it->second.x == 0.0f && it->second.y == 0.0f && it->second.z == 0.0f);
        if (hadInvalidPosition) {
          isFirstPositionUpdate = true;
          Umbra::Core::Logger::getInstance().info("First position update for player {} (from client {}) - position was (0,0,0), now setting to ({}, {}, {})", 
                                                  f.playerId, cid, f.x, f.y, f.z);
        }
      }
    } else {
      // Se é um novo player, também é a primeira atualização de posição
      isFirstPositionUpdate = true;
      Umbra::Core::Logger::getInstance().info("New player {} (from client {}) - first position update: ({}, {}, {})", 
                                              f.playerId, cid, f.x, f.y, f.z);
    }
    
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
    
    if (isFirstPositionUpdate) {
      // ✅ CRÍTICO: Se esta é a primeira vez que a posição é definida (era 0,0,0 ou é novo player),
      // enviar StateUpdate para TODOS os clients, incluindo os que já estão conectados
      // mas não receberam o StateUpdate inicial porque a posição ainda não estava definida
      Umbra::Core::Logger::getInstance().info("🚨🚨🚨 First position update for player {} (from client {}) - broadcasting StateUpdate to all clients (including those that connected before position was set) 🚨🚨🚨", 
                                              f.playerId, cid);
      // Criar StateUpdate e fazer broadcast para TODOS os clients
      MovementFrame out{MovementMsgType::StateUpdate, f.playerId, f.x, f.y, f.z, f.yaw, finalTimestamp};
      std::vector<uint8_t> broadcastBytes;
      if (hasAnimation) {
        broadcastBytes = encodeWithAnimation(out, speed, velocityZ, isInAir);
      } else {
        broadcastBytes = encodeWithAnimation(out, 0.0f, 0.0f, false); // Usar encodeWithAnimation mesmo sem animação para consistência
      }
      
      // ✅ CRÍTICO: Fazer broadcast ANTES de atualizar o map para garantir que todos os clients recebam
      // Isso é especialmente importante para clients que já estão conectados mas não receberam o StateUpdate inicial
      size_t clientCount = ws_.getClientCount();
      Umbra::Core::Logger::getInstance().info("📡 Broadcasting first StateUpdate for player {} to {} connected clients: pos=({}, {}, {}), yaw={}, frame_size={} bytes", 
                                              f.playerId, clientCount, f.x, f.y, f.z, f.yaw, broadcastBytes.size());
      ws_.broadcastBinary(broadcastBytes);
      Umbra::Core::Logger::getInstance().info("✅✅✅ Broadcasted first StateUpdate for player {} to ALL {} clients: pos=({}, {}, {}), yaw={} ✅✅✅", 
                                              f.playerId, clientCount, f.x, f.y, f.z, f.yaw);
      
      // Se for um novo player, também enviar snapshot completo para garantir
      if (isNewPlayer) {
        Umbra::Core::Logger::getInstance().info("New player {} - also sending full snapshot to all clients", f.playerId);
        sendFullSnapshotToAllUnlocked();
      }
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
      broadcastBytes = encodeWithAnimation(out, 0.0f, 0.0f, false); // Usar encodeWithAnimation mesmo sem animação para consistência
      Umbra::Core::Logger::getInstance().debug("Broadcasting StateUpdate (no animation): PlayerID={}, pos=({}, {}, {}), yaw={}, ts={}, frame_size={} bytes", 
                                               out.playerId, out.x, out.y, out.z, out.yaw, finalTimestamp, broadcastBytes.size());
    }
    
    if (broadcastBytes.size() >= 5) {
      Umbra::Core::Logger::getInstance().debug("  Frame bytes [0-4]: {:02X} {:02X} {:02X} {:02X} {:02X}", 
                                               broadcastBytes[0], broadcastBytes[1], broadcastBytes[2], broadcastBytes[3], broadcastBytes[4]);
    }
    
    // ✅ IMPORTANTE: Se já fizemos broadcast acima (isFirstPositionUpdate), não fazer novamente aqui
    // para evitar duplicação
    if (!isFirstPositionUpdate) {
      ws_.broadcastBinary(broadcastBytes);
      Umbra::Core::Logger::getInstance().debug("Broadcasted StateUpdate for player {} (from client {}, ts={}, hasAnimation={})", f.playerId, cid, finalTimestamp, hasAnimation);
    }
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
        
        // Notificar todos os OUTROS clientes que este player desconectou
        // IMPORTANTE: Não fazer broadcast se não houver outros clients conectados
        // O WebSocketServer continua rodando mesmo sem clients
        try {
          auto disconnectMsg = encodePlayerDisconnected(playerId);
          // broadcastBinary é seguro mesmo sem clients (apenas não envia nada)
          ws_.broadcastBinary(disconnectMsg);
          Umbra::Core::Logger::getInstance().info("Broadcasted PlayerDisconnected message for player {} (if other clients exist)", playerId);
        } catch (const std::exception& e) {
          Umbra::Core::Logger::getInstance().error("Exception while broadcasting PlayerDisconnected for player {}: {}", playerId, e.what());
          // NÃO parar o servidor por causa de erro no broadcast
        } catch (...) {
          Umbra::Core::Logger::getInstance().error("Unknown exception while broadcasting PlayerDisconnected for player {}", playerId);
          // NÃO parar o servidor por causa de erro no broadcast
        }
      }
      clientIdToPlayerId_.erase(it);
    } else {
      Umbra::Core::Logger::getInstance().debug("Client {} disconnected but had no associated player", cid);
    }
    
    // IMPORTANTE: O servidor WebSocket DEVE continuar rodando mesmo sem players
    // Apenas removemos o player do map, mas o servidor continua aceitando novas conexões
    Umbra::Core::Logger::getInstance().info("Client {} disconnected. Server continues running. Remaining players: {}", cid, players_.size());
  }

  // Enviar mensagem para um jogador específico (por PlayerID) - versão sem lock (chamada com lock já adquirido)
  void sendToPlayerUnlocked(uint32_t playerId, const std::vector<uint8_t>& message) {
    // Encontrar clientId associado ao playerId
    for (const auto& [cid, pid] : clientIdToPlayerId_) {
      if (pid == playerId) {
        ws_.sendBinary(cid, message);
        Umbra::Core::Logger::getInstance().debug("Sent message to player {} (client {})", playerId, cid);
        return;
      }
    }
    Umbra::Core::Logger::getInstance().warn("Player {} not found online, cannot send message", playerId);
  }
  
  // Versão pública com lock (para uso externo)
  void sendToPlayer(uint32_t playerId, const std::vector<uint8_t>& message) {
    std::lock_guard<std::mutex> lock(mu_);
    sendToPlayerUnlocked(playerId, message);
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


