#pragma once

#include <unordered_map>
#include <mutex>
#include <chrono>
#include <functional>
#include "network/WebSocketServer.hpp"
#include "zone/MovementProtocol.hpp"
#include "zone/SpatialGrid.hpp"
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

  /** Callback chamado quando um jogador desconecta. Usado para remover do grupo no servidor. Retorna party_id se estava em grupo, 0 caso contrário. */
  void setOnPlayerDisconnectCallback(std::function<uint32_t(uint32_t playerId)> cb) {
    onPlayerDisconnect_ = std::move(cb);
  }

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
      
      // Party Accept Notify (cliente aceitou grupo via HTTP, broadcast para todos refrescarem)
      if (msgType == MovementMsgType::PartyAcceptNotify) {
        uint32_t partyId;
        if (decodePartyAcceptNotify(data, partyId)) {
          Umbra::Core::Logger::getInstance().info("Received PartyAcceptNotify from client {}: partyId={}", cid, partyId);
          std::lock_guard<std::mutex> lock(mu_);
          auto msg = encodePartyMemberJoined(partyId);
          ws_.broadcastBinary(msg);
        }
        return;
      }

      // Party Stats Refresh (cliente equipou/desequipou item, broadcast HP/MP atualizado)
      if (msgType == MovementMsgType::PartyStatsRefresh) {
        uint32_t partyId;
        if (decodePartyStatsRefresh(data, partyId)) {
          Umbra::Core::Logger::getInstance().info("Received PartyStatsRefresh from client {}: partyId={}", cid, partyId);
          std::lock_guard<std::mutex> lock(mu_);
          auto msg = encodePartyStatsRefresh(partyId);
          ws_.broadcastBinary(msg);
        }
        return;
      }

      // Party Member Left Notify (cliente saiu do grupo via Leave Party, broadcast para todos refrescarem)
      if (msgType == MovementMsgType::PartyMemberLeftNotify) {
        uint32_t partyId;
        if (decodePartyMemberLeftNotify(data, partyId)) {
          Umbra::Core::Logger::getInstance().info("Received PartyMemberLeftNotify from client {}: partyId={}", cid, partyId);
          std::lock_guard<std::mutex> lock(mu_);
          auto msg = encodePartyMemberLeft(partyId);
          ws_.broadcastBinary(msg);
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
          std::lock_guard<std::mutex> lock(mu_);
          // Usar o player ID associado à conexão (autoritativo), não o fromId do payload
          uint32_t senderPlayerId = fromId;
          auto cidIt = clientIdToPlayerId_.find(cid);
          if (cidIt != clientIdToPlayerId_.end()) {
            senderPlayerId = cidIt->second;
          }
          std::string fromName;
          auto it = players_.find(senderPlayerId);
          if (it != players_.end() && !it->second.characterName.empty()) {
            fromName = it->second.characterName;
          } else {
            fromName = "Player_" + std::to_string(senderPlayerId);
          }
          Umbra::Core::Logger::getInstance().info("Received Whisper from client {} (player {}): to={}, fromName={}, msg={}",
                                                  cid, senderPlayerId, toId, fromName, message);
          sendToPlayerUnlocked(toId, encodeWhisperReceived(senderPlayerId, toId, fromName, message));
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
          
          // Mapear clientId -> playerId para handleClientDisconnect poder remover do grupo
          clientIdToPlayerId_[cid] = playerId;
          
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
      
      ++moveUpdateCount_;
      // Log a cada 300 updates (~10s a 30fps) para não poluir
      if (moveUpdateCount_ % 300 == 1) {
        Umbra::Core::Logger::getInstance().debug("MoveUpdate #{} client={} player={} pos=({:.0f},{:.0f},{:.0f}) spd={:.0f} anim={}",
                                                  moveUpdateCount_, cid, f.playerId, f.x, f.y, f.z, speed, hasAnimation);
      }
      
      handleMoveUpdate(cid, f, hasAnimation, speed, velocityZ, isInAir);
    });

    return ws_.start();
  }

  void stop() { ws_.stop(); }

  /** Snapshot periódico (10-20 Hz). Agora usa AOI: cada jogador recebe apenas updates de jogadores próximos. */
  void broadcastSnapshot() {
    std::lock_guard<std::mutex> lock(mu_);
    for (const auto& [pid, st] : players_) {
      MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
      auto bytes = encodeWithAnimation(f, st.speed, st.velocityZ, st.isInAir);

      uint32_t sourceClientId = 0;
      for (const auto& [cid, mappedPid] : clientIdToPlayerId_) {
        if (mappedPid == pid) { sourceClientId = cid; break; }
      }

      if (sourceClientId > 0) {
        auto nearby = aoiGrid_.getNearbyPlayers(sourceClientId);
        for (uint32_t nearbyClientId : nearby) {
          ws_.sendBinary(nearbyClientId, bytes);
        }
      }
    }
  }

  /** Retorna cópia dos estados dos players (thread-safe, para auto-save). */
  std::unordered_map<uint32_t, PlayerStateNet> getPlayerStates() const {
    std::lock_guard<std::mutex> lock(mu_);
    return players_;
  }

  size_t getOnlinePlayerCount() const {
    std::lock_guard<std::mutex> lock(mu_);
    return players_.size();
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
      if (!st.characterName.empty() || !st.characterTitle.empty()) {
        auto infoMsg = encodePlayerInfoUpdate(st.playerId, st.characterName, st.characterTitle);
        if (ws_.sendBinary(clientId, infoMsg)) {
          sentInfoCount++;
        }
      }
    }
    
    // Depois enviar StateUpdate para spawnar os actors
    // ✅ CRÍTICO: Só enviar StateUpdate se o player tiver uma posição válida (não 0,0,0)
    // Players que acabaram de enviar PlayerInfoUpdate ainda não têm posição definida
    for (const auto& [pid, st] : players_) {
      // Verificar se a posição é válida (não é 0,0,0 que indica "posição ainda não definida")
      bool hasValidPosition = (st.x != 0.0f || st.y != 0.0f || st.z != 0.0f);
      if (!hasValidPosition) continue;
      
      MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
      auto bytes = encodeWithAnimation(f, st.speed, st.velocityZ, st.isInAir);
      if (ws_.sendBinary(clientId, bytes)) {
        sentStateCount++;
      }
    }
    Umbra::Core::Logger::getInstance().info("Initial snapshot to client {}: {} info + {} states", 
                                             clientId, sentInfoCount, sentStateCount);
  }

  // Versão sem lock - chamada de handleMoveUpdate que já tem lock
  void sendFullSnapshotToAllUnlocked() {
    for (const auto& [pid, st] : players_) {
      MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
      auto bytes = encodeWithAnimation(f, st.speed, st.velocityZ, st.isInAir);
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
          bool skipSpeedCheck = false;
          
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
            if (dist2 > (maxTeleportDist_ * maxTeleportDist_)) {
              dt = 0.033f;
            } else {
              dt = 0.1f;
            }
            Umbra::Core::Logger::getInstance().debug("Timestamp regressed or equal for player {} (prev={}, curr={}), using dt={}s (dist={})", 
                                                      f.playerId, prevTs, f.tsMs, dt, std::sqrt(dist2));
            skipSpeedCheck = true;
          }
          
          if (!skipSpeedCheck) {
            float calculatedSpeed = std::sqrt(dist2) / dt;
            if (calculatedSpeed > maxSpeed_) {
              Umbra::Core::Logger::getInstance().warn("MoveUpdate from client {} rejected: speed too high (speed={}, dist={}, dt={}, prevTs={}, currTs={})", 
                                                      cid, calculatedSpeed, std::sqrt(dist2), dt, prevTs, f.tsMs);
              return;
            }
          }
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
          Umbra::Core::Logger::getInstance().debug("Player {} first position from client {} ({:.0f},{:.0f},{:.0f})",
                                                  f.playerId, cid, f.x, f.y, f.z);
        }
      }
    } else {
      // Se é um novo player, também é a primeira atualização de posição
      isFirstPositionUpdate = true;
      Umbra::Core::Logger::getInstance().info("New player {} (from client {}) - first position update: ({}, {}, {})", 
                                              f.playerId, cid, f.x, f.y, f.z);
    }
    
    // Atualizar grid espacial (AOI)
    if (isNewPlayer) {
      aoiGrid_.addPlayer(cid, f.x, f.y);
    } else {
      aoiGrid_.updatePlayer(cid, f.x, f.y);
    }

    // Preservar nome/título se já existir (PlayerInfoUpdate pode ter chegado antes do MoveUpdate)
    std::string prevName, prevTitle;
    auto itPrev = players_.find(f.playerId);
    if (itPrev != players_.end()) {
      prevName = itPrev->second.characterName;
      prevTitle = itPrev->second.characterTitle;
    }
    players_[f.playerId] = PlayerStateNet{
      f.playerId, 
      f.x, f.y, f.z, 
      f.yaw, 
      finalTimestamp,
      speed,        // Dados de animação
      velocityZ,
      isInAir
    };
    if (!prevName.empty() || !prevTitle.empty()) {
      players_[f.playerId].characterName = std::move(prevName);
      players_[f.playerId].characterTitle = std::move(prevTitle);
    }
    
    if (isFirstPositionUpdate) {
      MovementFrame out{MovementMsgType::StateUpdate, f.playerId, f.x, f.y, f.z, f.yaw, finalTimestamp};
      std::vector<uint8_t> broadcastBytes;
      if (hasAnimation) {
        broadcastBytes = encodeWithAnimation(out, speed, velocityZ, isInAir);
      } else {
        broadcastBytes = encodeWithAnimation(out, 0.0f, 0.0f, false);
      }
      
      size_t clientCount = ws_.getClientCount();
      ws_.broadcastBinary(broadcastBytes);
      Umbra::Core::Logger::getInstance().info("Player {} first position ({:.0f},{:.0f},{:.0f}) broadcast to {} clients",
                                              f.playerId, f.x, f.y, f.z, clientCount);
      
      if (isNewPlayer) {
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
    
    if (!isFirstPositionUpdate) {
      broadcastToNearby(cid, f.x, f.y, broadcastBytes);
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
        aoiGrid_.removePlayer(cid);
        players_.erase(playerIt);

        // Remover jogador do grupo no servidor (party_members no DB) e broadcast PartyMemberLeft se estava em grupo
        uint32_t partyId = 0;
        if (onPlayerDisconnect_) {
          try {
            partyId = onPlayerDisconnect_(playerId);
          } catch (const std::exception& e) {
            Umbra::Core::Logger::getInstance().error("Exception in onPlayerDisconnect for player {}: {}", playerId, e.what());
          } catch (...) {
            Umbra::Core::Logger::getInstance().error("Unknown exception in onPlayerDisconnect for player {}", playerId);
          }
        }
        if (partyId > 0) {
          auto memberLeftMsg = encodePartyMemberLeft(partyId);
          ws_.broadcastBinary(memberLeftMsg);
          Umbra::Core::Logger::getInstance().info("Broadcasted PartyMemberLeft for party {} (player {} disconnected)", partyId, playerId);
        }
        
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

  /** Broadcast para jogadores próximos (AOI) em vez de todos. Usa SpatialGrid. */
  void broadcastToNearby(uint32_t sourceClientId, float x, float y,
                         const std::vector<uint8_t>& data) {
    auto nearby = aoiGrid_.getNearbyPlayers(sourceClientId);
    for (uint32_t nearbyClientId : nearby) {
      ws_.sendBinary(nearbyClientId, data);
    }
    ws_.sendBinary(sourceClientId, data);
  }

  /** Broadcast para TODOS (mensagens sociais, disconnect, etc.). */
  void broadcastToAll(const std::vector<uint8_t>& data) {
    ws_.broadcastBinary(data);
  }

  Umbra::Network::WebSocketServer ws_;
  mutable std::mutex mu_;
  std::unordered_map<uint32_t, PlayerStateNet> players_;
  std::unordered_map<uint32_t, uint32_t> clientIdToPlayerId_;
  std::function<uint32_t(uint32_t)> onPlayerDisconnect_;
  SpatialGrid aoiGrid_{200.0f};
  uint64_t moveUpdateCount_ = 0;
  float maxSpeed_ = 1200.0f;
  float maxTeleportDist_ = 3000.0f;
  uint32_t maxDelayMs_ = 300;
};

} // namespace Zone
} // namespace Umbra


