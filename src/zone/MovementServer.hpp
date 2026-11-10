#pragma once

#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <chrono>
#include <functional>
#include <deque>
#include <condition_variable>
#include <thread>
#include <cctype>
#include <atomic>
#include <algorithm>
#include <memory>
#include <cstdint>
#include <climits>
#include "network/WebSocketServer.hpp"
#include "zone/MovementProtocol.hpp"
#include "zone/SpatialGrid.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "core/Logger.hpp"
#include "zone/MovementSessionAuth.hpp"
#include "auth/JWTManager.hpp"
#include "database/MySQLConnector.hpp"
#include "core/Timer.hpp"

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
  bool isDead = false;
  // Dados do personagem
  std::string characterName;
  std::string characterTitle;
  std::string guildName;
  uint32_t classId = 0;
  uint32_t hair = 0;
  uint32_t head = 0;
  std::vector<EquippedVisualWireEntry> equippedVisual;
  /** Opcode 119: 0 = relaxado (arma nas costas), 1 = combate. Comeca relaxado ao logar. */
  uint8_t combatStance = 0;
};

class MovementServer {
public:
  using Outbox = std::vector<std::pair<uint32_t, std::vector<uint8_t>>>;

  explicit MovementServer(uint16_t port)
    : ws_(port) {}

  void setChatLimits(size_t maxMessageLength, uint32_t rateLimitPerMinute) {
    std::lock_guard<std::mutex> lock(mu_);
    chatMaxMessageLength_ = maxMessageLength;
    chatRateLimitPerMinute_ = rateLimitPerMinute;
  }

  /** Callback chamado quando um jogador desconecta. Usado para remover do grupo no servidor. Retorna party_id se estava em grupo, 0 caso contrário. */
  void setOnPlayerDisconnectCallback(std::function<uint32_t(uint32_t playerId)> cb) {
    onPlayerDisconnect_ = std::move(cb);
  }

  /** Resolve membros do grupo de um jogador (incluindo o próprio). */
  void setResolvePartyMembersCallback(std::function<std::vector<uint32_t>(uint32_t playerId)> cb) {
    resolvePartyMembers_ = std::move(cb);
  }

  /** (playerId, zoneId, spawnKey, memorySaysDead, outPayload) */
  void setRespawnHandler(
      std::function<bool(uint32_t, uint32_t, const std::string&, bool, PlayerRespawnPayload&)> cb) {
    respawnHandler_ = std::move(cb);
  }

  void setZoneId(uint32_t zoneId) { zoneId_ = zoneId; }

  void setCombatCoreEngine(CombatCoreEngine* engine) { combatCoreEngine_ = engine; }
  void setLootService(LootService* loot) { lootService_ = loot; }
  void setSessionAuth(Umbra::Auth::JWTManager* jwt, std::shared_ptr<Umbra::Database::MySQLConnector> db) {
    sessionAuth_.setJwtManager(jwt);
    sessionAuth_.setDatabase(std::move(db));
    sessionAuthEnabled_ = (jwt != nullptr);
  }

  void tickSessionAuth() {
    if (!sessionAuthEnabled_) return;
    sessionAuth_.tickAuthTimeouts([this](uint32_t cid, SessionRevokeReason reason, const std::string& msg) {
      revokeAndDisconnectClient(cid, reason, msg);
    });
  }


  /** Envia pacote binário a um client WS (snapshot NPC, etc.). */
  void sendBinaryToClient(uint32_t clientId, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mu_);
    ws_.sendBinary(clientId, data);
  }

  /** Resolve clientId WS a partir do playerId (0 se offline). */
  uint32_t getClientIdForPlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mu_);
    return findClientIdForPlayerUnlocked(playerId);
  }

  void broadcastVitalsAndCombat(uint32_t targetPlayerId, const PlayerVitalsPayload& vitals,
                              uint32_t sourcePlayerId, int32_t delta, bool triggerDeath,
                              bool isCrit = false, bool isDouble = false) {
    Outbox outbox;
    {
      std::lock_guard<std::mutex> lock(mu_);
      handleVitalsBroadcastUnlocked(targetPlayerId, vitals, sourcePlayerId, delta, triggerDeath, isCrit,
                                     isDouble, outbox);
    }
    flushOutbox(outbox);
  }

  /** Opcode 88 validado pelo CombatCoreEngine (vitals vindos do DB). Calcula o delta pelo
   *  último HP conhecido; o hint do cliente só vale quando não há HP anterior em memória. */
  void broadcastForeignVitals(uint32_t targetPlayerId, const PlayerVitalsPayload& vitals,
                              uint32_t sourcePlayerId) {
    Outbox outbox;
    {
      std::lock_guard<std::mutex> lock(mu_);
      int32_t delta = 0;
      auto prevIt = lastKnownHealth_.find(targetPlayerId);
      if (prevIt != lastKnownHealth_.end()) {
        delta = vitals.currentHealth - prevIt->second;
      } else {
        delta = vitals.deltaAppliedHealth;
      }
      lastKnownHealth_[targetPlayerId] = vitals.currentHealth;
      const bool triggerDeath = (vitals.currentHealth <= 0);
      handleVitalsBroadcastUnlocked(targetPlayerId, vitals, sourcePlayerId, delta, triggerDeath,
                                    false, false, outbox);
    }
    flushOutbox(outbox);
  }

  /** Entrada na zone já morto no DB: respawna automaticamente (evita 0 HP / sem controle). */
  bool autoRespawnOnJoin(uint32_t playerId) {
    if (!respawnHandler_ || playerId == 0) return false;
    PlayerRespawnPayload respawnPayload;
    if (!respawnHandler_(playerId, zoneId_, std::string(), /*memorySaysDead*/ true, respawnPayload)) {
      return false;
    }
    Outbox outbox;
    {
      std::lock_guard<std::mutex> lock(mu_);
      applyRespawnSuccessUnlocked(playerId, respawnPayload, outbox);
    }
    flushOutbox(outbox);
    Umbra::Core::Logger::getInstance().info(
        "Player {} auto-respawned on join at ({:.0f},{:.0f},{:.0f}) hp={}", playerId,
        respawnPayload.x, respawnPayload.y, respawnPayload.z, respawnPayload.currentHealth);
    return true;
  }

  /** Fallback: marca morto e notifica (só se auto-respawn falhar). */
  void forcePlayerDeadState(uint32_t playerId, int32_t currentHealth, int32_t maxHealth,
                            int32_t currentMana, int32_t maxMana) {
    Outbox outbox;
    {
      std::lock_guard<std::mutex> lock(mu_);
      auto it = players_.find(playerId);
      if (it != players_.end() && it->second.isDead) {
        return;
      }
      if (it == players_.end()) {
        PlayerStateNet stub;
        stub.playerId = playerId;
        stub.isDead = false;
        players_[playerId] = stub;
      }
      lastKnownHealth_[playerId] = currentHealth;

      PlayerVitalsPayload vitals;
      vitals.playerId = playerId;
      vitals.currentHealth = std::max(0, currentHealth);
      vitals.maxHealth = std::max(1, maxHealth);
      vitals.currentMana = std::max(0, currentMana);
      vitals.maxMana = std::max(1, maxMana);
      vitals.sourcePlayerId = playerId;
      vitals.reason = 0;
      handleVitalsBroadcastUnlocked(playerId, vitals, playerId, 0, /*triggerDeath*/ true, false, false,
                                    outbox);
    }
    flushOutbox(outbox);
  }

  void broadcastDotTick(uint32_t targetPlayerId, const DotTickPayload& dot) {
    Outbox outbox;
    {
      std::lock_guard<std::mutex> lock(mu_);
      auto dotPkt = encodeDotTickNotify(dot);
      std::unordered_set<uint32_t> recipients;
      collectVitalsRecipientsUnlocked(targetPlayerId, recipients);
      for (uint32_t rid : recipients) {
        enqueueToPlayerUnlocked(rid, dotPkt, outbox);
      }
    }
    flushOutbox(outbox);
  }

  void broadcastExpGain(uint32_t playerId, const ExpGainNotifyPayload& payload) {
    Outbox outbox;
    {
      std::lock_guard<std::mutex> lock(mu_);
      auto pkt = encodeExpGainNotify(payload);
      std::unordered_set<uint32_t> recipients;
      collectVitalsRecipientsUnlocked(playerId, recipients);
      for (uint32_t rid : recipients) {
        enqueueToPlayerUnlocked(rid, pkt, outbox);
      }
    }
    flushOutbox(outbox);
  }

  void broadcastLevelUp(uint32_t playerId, const LevelUpNotifyPayload& payload) {
    Outbox outbox;
    {
      std::lock_guard<std::mutex> lock(mu_);
      auto pkt = encodeLevelUpNotify(payload);
      std::unordered_set<uint32_t> recipients;
      collectVitalsRecipientsUnlocked(playerId, recipients);
      for (uint32_t rid : recipients) {
        enqueueToPlayerUnlocked(rid, pkt, outbox);
      }
    }
    flushOutbox(outbox);
  }

  /** Broadcast binário para todos os clientes conectados (Combat V2, etc.). */
  void broadcastToAll(const std::vector<uint8_t>& data) {
    // Coleta clientIds sob mu_ (rápido); envio DEPOIS de liberar mu_ para não travar
    // as threads de movimento nem o combat worker durante send() bloqueante.
    std::vector<uint32_t> recipients;
    {
      std::lock_guard<std::mutex> lock(mu_);
      recipients.reserve(clientIdToPlayerId_.size());
      for (const auto& [cid, pid] : clientIdToPlayerId_) {
        (void)pid;
        recipients.push_back(cid);
      }
    }
    for (uint32_t cid : recipients) {
      ws_.sendBinary(cid, data);
    }
  }

  /**
   * Broadcast de combate/VFX para jogadores a até `radius` do âncora (playerId).
   * Inclui o próprio âncora. Se o âncora não estiver online/com posição, cai para broadcastToAll.
   * Não é GCD: só reduz fan-out (escala com N players); combos no mesmo raio continuam.
   */
  void broadcastNearPlayer(uint32_t anchorPlayerId, const std::vector<uint8_t>& data,
                           float radius = 8000.f) {
    if (anchorPlayerId == 0) {
      broadcastToAll(data);
      return;
    }
    std::vector<uint32_t> recipients;
    {
      std::lock_guard<std::mutex> lock(mu_);
      auto pit = players_.find(anchorPlayerId);
      if (pit == players_.end()) {
        for (const auto& [cid, pid] : clientIdToPlayerId_) {
          (void)pid;
          recipients.push_back(cid);
        }
      } else {
        const float ax = pit->second.x;
        const float ay = pit->second.y;
        const float r2 = radius * radius;
        for (const auto& [cid, pid] : clientIdToPlayerId_) {
          auto oit = players_.find(pid);
          if (oit == players_.end()) continue;
          const float dx = oit->second.x - ax;
          const float dy = oit->second.y - ay;
          if (dx * dx + dy * dy <= r2) {
            recipients.push_back(cid);
          }
        }
      }
    }
    for (uint32_t cid : recipients) {
      ws_.sendBinary(cid, data);
    }
  }

  /** Broadcast para clientes cujo player está a até `radius` de (x,y) no mundo (NPC/eventos). */
  void broadcastNearWorldXY(float x, float y, const std::vector<uint8_t>& data,
                            float radius = 8000.f) {
    std::vector<uint32_t> recipients;
    {
      std::lock_guard<std::mutex> lock(mu_);
      const float r2 = radius * radius;
      for (const auto& [cid, pid] : clientIdToPlayerId_) {
        auto oit = players_.find(pid);
        if (oit == players_.end()) continue;
        const float dx = oit->second.x - x;
        const float dy = oit->second.y - y;
        if (dx * dx + dy * dy <= r2) {
          recipients.push_back(cid);
        }
      }
    }
    if (recipients.empty()) {
      broadcastToAll(data);
      return;
    }
    for (uint32_t cid : recipients) {
      ws_.sendBinary(cid, data);
    }
  }

  /** Envia mensagem binária a um jogador online (ex.: opcode 105 SkillCastRejected). */
  void sendToPlayer(uint32_t playerId, const std::vector<uint8_t>& message) {
    Outbox outbox;
    {
      std::lock_guard<std::mutex> lock(mu_);
      enqueueToPlayerUnlocked(playerId, message, outbox);
    }
    flushOutbox(outbox);
  }

  bool start() {
    startCleanupWorker();
    ws_.setConnectionCallback([this](uint32_t cid, bool connected){
      if (connected) {
        Umbra::Core::Logger::getInstance().info("WS client {} connected", cid);
        if (sessionAuthEnabled_) {
          sessionAuth_.onClientConnected(cid);
        } else {
          sendPostAuthSnapshots(cid);
        }
      } else {
        Umbra::Core::Logger::getInstance().info("WS client {} disconnected", cid);
        // Remover player associado a este client quando desconectar.
        // Sob o mu_ fazemos SOMENTE as mutacoes de mapa (rapido); o trabalho pesado
        // (DB de party/sessions + broadcasts) roda DEPOIS de liberar o mu_ para nao
        // travar as threads dos outros clientes por segundos (efeito cascata de queda).
        // Fase 2 (DB + broadcasts) vai para a thread de limpeza: o kick de login duplicado
        // é disparado no tick e travava o loop ~250ms esperando o DELETE no MySQL.
        DisconnectCleanup cleanup;
        {
          std::lock_guard<std::mutex> lock(mu_);
          cleanup = handleClientDisconnect(cid);
        }
        enqueueDisconnectCleanup(cid, cleanup);
      }
    });

    ws_.setBinaryCallback([this](uint32_t cid, const std::vector<uint8_t>& data) {
      // CRÍTICO: não processar aqui — este callback roda no thread de recv.
      // MySQL/auth/PlayerInfo no recv trava o TCP e derruba o cliente (Proxmox).
      enqueueInboundBinary(cid, data);
    });

    return ws_.start();
  }

  void enqueueInboundBinary(uint32_t cid, const std::vector<uint8_t>& data) {
    if (data.empty()) return;
    if (static_cast<MovementMsgType>(data[0]) == MovementMsgType::MoveUpdate) {
      std::lock_guard<std::mutex> lock(inboundMu_);
      pendingMoveByCid_[cid] = data;  // coalesce: só a posição mais recente
      return;
    }
    std::lock_guard<std::mutex> lock(inboundMu_);
    inboundQueue_.push_back(InboundMsg{cid, data});
    constexpr size_t kMaxInbound = 4096;
    if (inboundQueue_.size() > kMaxInbound) {
      inboundQueue_.pop_front();
      // Anomalia real (tick não drena a tempo): alerta com limite de frequência.
      static Umbra::Core::LogThrottle dropThrottle(5000);
      if (dropThrottle.allow()) {
        Umbra::Core::Logger::getInstance().warn(
            "Zone inbound queue full ({}), dropping oldest message (+{} suppressed)",
            kMaxInbound, dropThrottle.takeSuppressed());
      }
    }
  }

  void drainInboundQueue() {
    std::deque<InboundMsg> local;
    std::unordered_map<uint32_t, std::vector<uint8_t>> moves;
    {
      std::lock_guard<std::mutex> lock(inboundMu_);
      local.swap(inboundQueue_);
      moves.swap(pendingMoveByCid_);
    }
    for (auto& msg : local) {
      handleInboundBinary(msg.cid, msg.data);
    }
    for (auto& [cid, data] : moves) {
      handleInboundBinary(cid, data);
    }
  }

  void handleInboundBinary(uint32_t cid, const std::vector<uint8_t>& data) {
      // Verificar tipo de mensagem primeiro
      if (data.empty()) return;
      MovementMsgType msgType = static_cast<MovementMsgType>(data[0]);

      if (sessionAuthEnabled_ && msgType == MovementMsgType::SessionAuthNotify) {
        std::string token;
        if (!decodeSessionAuthNotify(data, token)) {
          revokeAndDisconnectClient(cid, SessionRevokeReason::InvalidToken, "Payload de auth invalido.");
          return;
        }
        uint32_t kickCid = 0;
        std::string kickMsg;
        if (!sessionAuth_.handleSessionAuth(cid, token, kickCid, kickMsg)) {
          revokeAndDisconnectClient(cid, SessionRevokeReason::InvalidToken, "Autenticacao falhou.");
          return;
        }
        if (kickCid > 0) {
          revokeAndDisconnectClient(kickCid, SessionRevokeReason::DuplicateLogin, kickMsg);
        }
        sendPostAuthSnapshots(cid);
        return;
      }
      if (msgType == MovementMsgType::WsKeepalive) {
        return;  // server->client only; ignore if echoed
      }
      if (sessionAuthEnabled_ && !sessionAuth_.isAuthenticated(cid)) {
        // Nao kickar: o cliente UE pode enviar PlayerInfoUpdate/MoveUpdate enquanto o 109
        // ainda esta em voo (latencia MySQL remoto). O timeout de auth (10s) ainda desconecta.
        Umbra::Core::Logger::getInstance().warn(
            "WS client {} msg tipo {} ignorada (aguardando SessionAuthNotify)",
            cid, static_cast<int>(msgType));
        return;
      }
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

      // Loja pessoal: cliente abriu loja via HTTP
      if (msgType == MovementMsgType::PersonalShopOpenNotify) {
        uint32_t sellerId, shopId;
        std::string shopName;
        if (decodePersonalShopOpenPayload(data, sellerId, shopId, shopName)) {
          std::lock_guard<std::mutex> lock(mu_);
          uint32_t mappedPid = 0;
          auto cidIt = clientIdToPlayerId_.find(cid);
          if (cidIt != clientIdToPlayerId_.end()) mappedPid = cidIt->second;
          if (mappedPid != 0 && mappedPid != sellerId) {
            Umbra::Core::Logger::getInstance().warn("PersonalShopOpenNotify: client {} player {} != payload seller {}",
                                                    cid, mappedPid, sellerId);
            return;
          }
          if (mappedPid == 0) {
            setClientPlayerMapUnlocked(cid, sellerId);
          }
          personalShopOpenByPlayerId_[sellerId] = std::make_pair(shopId, shopName);
          auto opened = encodePersonalShopOpened(sellerId, shopId, shopName);
          ws_.broadcastBinary(opened);
          Umbra::Core::Logger::getInstance().info("PersonalShopOpened broadcast: seller={}, shop={}, name_len={}",
                                                  sellerId, shopId, shopName.size());
        }
        return;
      }

      if (msgType == MovementMsgType::PersonalShopCloseNotify) {
        uint32_t sellerId, shopId;
        if (decodePersonalShopClosePayload(data, sellerId, shopId)) {
          std::lock_guard<std::mutex> lock(mu_);
          uint32_t mappedPid = 0;
          auto cidIt = clientIdToPlayerId_.find(cid);
          if (cidIt != clientIdToPlayerId_.end()) mappedPid = cidIt->second;
          if (mappedPid != 0 && mappedPid != sellerId) {
            Umbra::Core::Logger::getInstance().warn("PersonalShopCloseNotify: client {} player {} != payload seller {}",
                                                    cid, mappedPid, sellerId);
            return;
          }
          personalShopOpenByPlayerId_.erase(sellerId);
          auto closed = encodePersonalShopClosed(sellerId, shopId);
          ws_.broadcastBinary(closed);
          Umbra::Core::Logger::getInstance().info("PersonalShopClosed broadcast: seller={}, shop={}", sellerId, shopId);
        }
        return;
      }

      if (msgType == MovementMsgType::PersonalShopListingSoldNotify) {
        uint32_t buyerId, sellerId, shopId, listingId;
        if (decodePersonalShopListingSoldNotify(data, buyerId, sellerId, shopId, listingId)) {
          std::lock_guard<std::mutex> lock(mu_);
          uint32_t mappedPid = 0;
          auto cidIt = clientIdToPlayerId_.find(cid);
          if (cidIt != clientIdToPlayerId_.end()) mappedPid = cidIt->second;
          if (mappedPid == 0 || mappedPid != buyerId) {
            Umbra::Core::Logger::getInstance().warn(
                "PersonalShopListingSoldNotify: client {} mapped player {} != buyer {}",
                cid, mappedPid, buyerId);
            return;
          }
          auto changed = encodePersonalShopListingsChanged(sellerId, shopId, listingId);
          ws_.broadcastBinary(changed);
          Umbra::Core::Logger::getInstance().info(
              "PersonalShopListingsChanged broadcast: seller={}, shop={}, listing={}", sellerId, shopId, listingId);
        }
        return;
      }
      
      // Party Accept Notify (cliente aceitou grupo via HTTP, broadcast para todos refrescarem)
      if (msgType == MovementMsgType::PartyAcceptNotify) {
        uint32_t partyId;
        if (decodePartyAcceptNotify(data, partyId)) {
          Umbra::Core::Logger::getInstance().info("Received PartyAcceptNotify from client {}: partyId={}", cid, partyId);
          std::lock_guard<std::mutex> lock(mu_);
          const uint32_t senderPlayerId = playerIdForClientUnlocked(cid);
          auto msg = encodePartyMemberJoined(partyId);
          // Quem acabou de entrar ainda não está no cache: na prática cai no broadcast.
          sendPartyNoticeUnlocked(getPartyMembersCached(senderPlayerId), senderPlayerId, msg);
        }
        return;
      }

      // Buff applied (poção) — rebroadcast para party + jogadores próximos (AOI)
      if (msgType == MovementMsgType::BuffAppliedNotify) {
        RemoteBuffPayload payload;
        if (decodeBuffAppliedNotify(data, payload)) {
          uint32_t senderPlayerId = payload.playerId;
          auto cidIt = clientIdToPlayerId_.find(cid);
          if (cidIt != clientIdToPlayerId_.end() && cidIt->second > 0) {
            senderPlayerId = cidIt->second;
            payload.playerId = senderPlayerId;
          }
          std::lock_guard<std::mutex> lock(mu_);
          auto outMsg = encodeRemoteBuffUpdate(MovementMsgType::RemoteBuffUpdate, payload);

          std::unordered_set<uint32_t> targetPlayerIds;
          if (resolvePartyMembers_) {
            for (uint32_t memberId : resolvePartyMembers_(senderPlayerId)) {
              if (memberId != senderPlayerId) {
                targetPlayerIds.insert(memberId);
              }
            }
          }

          float sx = 0.0f, sy = 0.0f;
          auto playerIt = players_.find(senderPlayerId);
          if (playerIt != players_.end()) {
            sx = playerIt->second.x;
            sy = playerIt->second.y;
          }
          auto nearbyClientIds = aoiGrid_.getNearbyPlayers(cid);
          for (uint32_t nearbyCid : nearbyClientIds) {
            auto pidIt = clientIdToPlayerId_.find(nearbyCid);
            if (pidIt != clientIdToPlayerId_.end() && pidIt->second != senderPlayerId) {
              targetPlayerIds.insert(pidIt->second);
            }
          }

          for (uint32_t targetId : targetPlayerIds) {
            sendToPlayerUnlocked(targetId, outMsg);
          }
          Umbra::Core::Logger::getInstance().info(
              "RemoteBuffUpdate from player {} -> {} recipients (party+AOI)",
              senderPlayerId, targetPlayerIds.size());
        }
        return;
      }

      // Efeito de consumível (poção HP/MP/buff) — rebroadcast floating text para party + AOI
      if (msgType == MovementMsgType::ConsumableEffectNotify) {
        ConsumableEffectPayload payload;
        if (decodeConsumableEffectNotify(data, payload)) {
          uint32_t senderPlayerId = payload.sourcePlayerId;
          auto cidIt = clientIdToPlayerId_.find(cid);
          if (cidIt != clientIdToPlayerId_.end() && cidIt->second > 0) {
            senderPlayerId = cidIt->second;
            payload.sourcePlayerId = senderPlayerId;
          }
          if (payload.targetPlayerId == 0) {
            payload.targetPlayerId = senderPlayerId;
          }
          std::lock_guard<std::mutex> lock(mu_);
          auto outMsg = encodeConsumableEffectUpdate(MovementMsgType::ConsumableEffectUpdate, payload);

          std::unordered_set<uint32_t> targetPlayerIds;
          if (resolvePartyMembers_) {
            for (uint32_t memberId : resolvePartyMembers_(senderPlayerId)) {
              if (memberId != senderPlayerId) {
                targetPlayerIds.insert(memberId);
              }
            }
          }

          auto nearbyClientIds = aoiGrid_.getNearbyPlayers(cid);
          for (uint32_t nearbyCid : nearbyClientIds) {
            auto pidIt = clientIdToPlayerId_.find(nearbyCid);
            if (pidIt != clientIdToPlayerId_.end() && pidIt->second != senderPlayerId) {
              targetPlayerIds.insert(pidIt->second);
            }
          }

          for (uint32_t targetId : targetPlayerIds) {
            sendToPlayerUnlocked(targetId, outMsg);
          }
          Umbra::Core::Logger::getInstance().info(
              "ConsumableEffectUpdate from player {} -> {} recipients (party+AOI)",
              senderPlayerId, targetPlayerIds.size());
        }
        return;
      }

      // HP/MP atualizados (poção, equip, etc.) — rebroadcast para party + AOI
      if (msgType == MovementMsgType::PlayerVitalsNotify) {
        PlayerVitalsPayload payload;
        if (decodePlayerVitalsNotify(data, payload)) {
          uint32_t senderPlayerId = payload.playerId;
          auto cidIt = clientIdToPlayerId_.find(cid);
          if (cidIt != clientIdToPlayerId_.end() && cidIt->second > 0) {
            senderPlayerId = cidIt->second;
            payload.playerId = senderPlayerId;
          }
          // Equip/load: invalida maxHealth stale no CharacterStateLoader (cura “HP cheio”).
          if (combatCoreEngine_ && senderPlayerId > 0) {
            combatCoreEngine_->onPlayerEquipmentOrStatsChanged(senderPlayerId);
            if (auto* loader = combatCoreEngine_->getCharacterStateLoader()) {
              if (payload.maxHealth > 0 || payload.maxMana > 0) {
                loader->patchCachedMaxVitals(senderPlayerId, payload.maxHealth, payload.maxMana);
              }
              if (payload.currentHealth >= 0) {
                loader->patchCachedHealth(senderPlayerId, payload.currentHealth);
              }
              if (payload.currentMana >= 0) {
                loader->patchCachedMana(senderPlayerId, payload.currentMana);
              }
            }
          }
          std::lock_guard<std::mutex> lock(mu_);
          auto outMsg = encodePlayerVitalsUpdate(MovementMsgType::PlayerVitalsUpdate, payload);

          std::unordered_set<uint32_t> targetPlayerIds;
          if (resolvePartyMembers_) {
            for (uint32_t memberId : resolvePartyMembers_(senderPlayerId)) {
              if (memberId != senderPlayerId) {
                targetPlayerIds.insert(memberId);
              }
            }
          }

          float sx = 0.0f, sy = 0.0f;
          auto playerIt = players_.find(senderPlayerId);
          if (playerIt != players_.end()) {
            sx = playerIt->second.x;
            sy = playerIt->second.y;
          }
          auto nearbyClientIds = aoiGrid_.getNearbyPlayers(cid);
          for (uint32_t nearbyCid : nearbyClientIds) {
            auto pidIt = clientIdToPlayerId_.find(nearbyCid);
            if (pidIt != clientIdToPlayerId_.end() && pidIt->second != senderPlayerId) {
              targetPlayerIds.insert(pidIt->second);
            }
          }

          for (uint32_t targetId : targetPlayerIds) {
            sendToPlayerUnlocked(targetId, outMsg);
          }
          Umbra::Core::Logger::getInstance().info(
              "PlayerVitalsUpdate from player {} -> {} recipients (party+AOI)",
              senderPlayerId, targetPlayerIds.size());
        }
        return;
      }

      // Combat V2: skill cast (96)
      if (msgType == MovementMsgType::SkillCastNotify) {
        SkillCastPayload payload;
        if (decodeSkillCastNotify(data, payload)) {
          // Resolver sourcePlayerId sob lock curto e LIBERAR mu_ antes de chamar
          // o CombatCoreEngine (mesmo motivo do BasicAttackNotify: evita deadlock
          // recursivo de mu_ quando o engine chama broadcastToAll/broadcastVitalsAndCombat).
          {
            std::lock_guard<std::mutex> lock(mu_);
            auto cidIt = clientIdToPlayerId_.find(cid);
            if (cidIt != clientIdToPlayerId_.end() && cidIt->second > 0) {
              payload.sourcePlayerId = cidIt->second;
            }
          }
          if (combatCoreEngine_) {
            combatCoreEngine_->enqueueSkillCast(payload.sourcePlayerId, payload);
          }
        }
        return;
      }

      // NPC buff snapshot request (108) — rate-limit: cliente spamava dezenas/s e afogava o tick.
      if (msgType == MovementMsgType::NpcBuffSnapshotRequest) {
        uint32_t npcInstanceId = 0;
        if (decodeNpcBuffSnapshotRequest(data, npcInstanceId) && combatCoreEngine_) {
          const int64_t nowMs = Umbra::Core::steadyNowMs();
          bool allow = true;
          {
            std::lock_guard<std::mutex> lock(mu_);
            int64_t& last = lastNpcBuffSnapMsByClient_[cid];
            if (last > 0 && (nowMs - last) < 250) {
              allow = false;
            } else {
              last = nowMs;
            }
          }
          if (allow) {
            combatCoreEngine_->sendNpcBuffSnapshotForNpc(cid, npcInstanceId);
          }
        }
        return;
      }

      // Combat V2: basic attack (98)
      if (msgType == MovementMsgType::BasicAttackNotify) {
        BasicAttackPayload payload;
        if (decodeBasicAttackNotify(data, payload)) {
          // Resolver sourcePlayerId sob lock curto e LIBERAR mu_ antes de chamar
          // o CombatCoreEngine: ele chama de volta metodos publicos (broadcastToAll,
          // broadcastVitalsAndCombat) que travam mu_ -> manter o lock aqui causa
          // deadlock recursivo (EDEADLK) e derruba a conexao do cliente.
          {
            std::lock_guard<std::mutex> lock(mu_);
            auto cidIt = clientIdToPlayerId_.find(cid);
            if (cidIt != clientIdToPlayerId_.end() && cidIt->second > 0) {
              payload.sourcePlayerId = cidIt->second;
            }
          }
          if (combatCoreEngine_) {
            combatCoreEngine_->enqueueBasicAttack(payload.sourcePlayerId, payload);
          }
        }
        return;
      }

      // Loot take item (113)
      if (msgType == MovementMsgType::LootTakeItem) {
        LootTakeItemPayload payload;
        if (decodeLootTakeItem(data, payload) && lootService_) {
          uint32_t playerId = 0;
          {
            std::lock_guard<std::mutex> lock(mu_);
            auto cidIt = clientIdToPlayerId_.find(cid);
            if (cidIt != clientIdToPlayerId_.end()) playerId = cidIt->second;
          }
          if (playerId > 0) lootService_->handleLootTakeItem(playerId, payload);
        }
        return;
      }

      // Loot take all (114)
      if (msgType == MovementMsgType::LootTakeAll) {
        LootTakeAllPayload payload;
        if (decodeLootTakeAll(data, payload) && lootService_) {
          uint32_t playerId = 0;
          {
            std::lock_guard<std::mutex> lock(mu_);
            auto cidIt = clientIdToPlayerId_.find(cid);
            if (cidIt != clientIdToPlayerId_.end()) playerId = cidIt->second;
          }
          if (playerId > 0) lootService_->handleLootTakeAll(playerId, payload);
        }
        return;
      }

      // HP/MP de alvo (dano/cura via apply_vitals.php) — rebroadcast party+AOI do TARGET.
      // Segurança: HP/MP do payload NÃO são confiáveis. O sender vira o source e o combat
      // worker relê os vitals no DB antes do broadcast (sem DB no tick nem no recv).
      if (msgType == MovementMsgType::ForeignVitalsNotify) {
        PlayerVitalsPayload payload;
        if (decodeForeignVitalsNotify(data, payload)) {
          uint32_t senderPlayerId = 0;
          {
            std::lock_guard<std::mutex> lock(mu_);
            auto cidIt = clientIdToPlayerId_.find(cid);
            if (cidIt != clientIdToPlayerId_.end()) senderPlayerId = cidIt->second;
          }
          if (senderPlayerId == 0) {
            Umbra::Core::Logger::getInstance().warn(
                "ForeignVitalsNotify ignorado: client {} sem player associado", cid);
          } else if (combatCoreEngine_) {
            combatCoreEngine_->enqueueForeignVitals(senderPlayerId, payload);
          }
        }
        return;
      }

      // Respawn request (cliente morto solicita respawn)
      if (msgType == MovementMsgType::RespawnRequest) {
        uint32_t playerId = 0;
        uint32_t zoneId = zoneId_;
        std::string spawnKey;
        if (decodeRespawnRequest(data, playerId, zoneId, spawnKey)) {
          if (!respawnHandler_) {
            Umbra::Core::Logger::getInstance().warn("RespawnRequest: handler não configurado");
            return;
          }
          bool memorySaysDead = false;
          {
            std::lock_guard<std::mutex> lock(mu_);
            auto cidIt = clientIdToPlayerId_.find(cid);
            if (cidIt != clientIdToPlayerId_.end() && cidIt->second > 0) {
              playerId = cidIt->second;
            }
            const auto now = std::chrono::steady_clock::now();
            auto cdIt = respawnCooldownUntil_.find(playerId);
            if (cdIt != respawnCooldownUntil_.end() && now < cdIt->second) {
              Umbra::Core::Logger::getInstance().debug("RespawnRequest: cooldown player {}", playerId);
              return;
            }
            auto pit = players_.find(playerId);
            if (pit != players_.end() && pit->second.isDead) {
              memorySaysDead = true;
            }
            auto hpIt = lastKnownHealth_.find(playerId);
            if (hpIt != lastKnownHealth_.end() && hpIt->second <= 0) {
              memorySaysDead = true;
            }
          }
          // MySQL fora de mu_ (Proxmox: prepared/remoto não pode segurar o lock da zone).
          PlayerRespawnPayload respawnPayload;
          if (!respawnHandler_(playerId, zoneId, spawnKey, memorySaysDead, respawnPayload)) {
            Umbra::Core::Logger::getInstance().warn(
                "RespawnRequest: falhou player {} (memoryDead={})", playerId, memorySaysDead);
            return;
          }
          Outbox outbox;
          {
            std::lock_guard<std::mutex> lock(mu_);
            respawnCooldownUntil_[playerId] =
                std::chrono::steady_clock::now() + std::chrono::seconds(5);
            applyRespawnSuccessUnlocked(playerId, respawnPayload, outbox);
          }
          flushOutbox(outbox);
          Umbra::Core::Logger::getInstance().info("Player {} respawned at ({:.0f},{:.0f},{:.0f})",
                                                  playerId, respawnPayload.x, respawnPayload.y,
                                                  respawnPayload.z);
        }
        return;
      }

      // Party Stats Refresh (cliente equipou/desequipou item, broadcast HP/MP atualizado)
      if (msgType == MovementMsgType::PartyStatsRefresh) {
        uint32_t partyId;
        if (decodePartyStatsRefresh(data, partyId)) {
          uint32_t senderPlayerId = 0;
          auto cidIt = clientIdToPlayerId_.find(cid);
          if (cidIt != clientIdToPlayerId_.end()) {
            senderPlayerId = cidIt->second;
          }
          if (combatCoreEngine_ && senderPlayerId > 0) {
            combatCoreEngine_->onPlayerEquipmentOrStatsChanged(senderPlayerId);
          }
          Umbra::Core::Logger::getInstance().info(
              "Received PartyStatsRefresh from client {}: partyId={} player={}", cid, partyId,
              senderPlayerId);
          std::lock_guard<std::mutex> lock(mu_);
          auto msg = encodePartyStatsRefresh(partyId);
          sendPartyNoticeUnlocked(getPartyMembersCached(senderPlayerId), senderPlayerId, msg);
        }
        return;
      }

      // Party Member Left Notify (cliente saiu do grupo via Leave Party, broadcast para todos refrescarem)
      if (msgType == MovementMsgType::PartyMemberLeftNotify) {
        uint32_t partyId;
        if (decodePartyMemberLeftNotify(data, partyId)) {
          Umbra::Core::Logger::getInstance().info("Received PartyMemberLeftNotify from client {}: partyId={}", cid, partyId);
          std::lock_guard<std::mutex> lock(mu_);
          const uint32_t senderPlayerId = playerIdForClientUnlocked(cid);
          auto msg = encodePartyMemberLeft(partyId);
          // O cache de quem saiu ainda lista os membros de antes da saída (TTL ~1-2 s).
          sendPartyNoticeUnlocked(getPartyMembersCached(senderPlayerId), 0, msg);
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

      // Chat local (proximidade)
      if (msgType == MovementMsgType::ChatLocalMessage) {
        uint32_t fromId = 0;
        std::string message;
        if (!decodeChatClientMessage(data, fromId, message)) {
          std::lock_guard<std::mutex> lock(mu_);
          sendChatErrorToClientUnlocked(cid, ChatErrorCode::InvalidPayload, "Payload de chat local invalido");
          return;
        }
        std::lock_guard<std::mutex> lock(mu_);
        handleChatMessageUnlocked(cid, msgType, fromId, message);
        return;
      }

      // Chat global
      if (msgType == MovementMsgType::ChatGlobalMessage) {
        uint32_t fromId = 0;
        std::string message;
        if (!decodeChatClientMessage(data, fromId, message)) {
          std::lock_guard<std::mutex> lock(mu_);
          sendChatErrorToClientUnlocked(cid, ChatErrorCode::InvalidPayload, "Payload de chat global invalido");
          return;
        }
        std::lock_guard<std::mutex> lock(mu_);
        handleChatMessageUnlocked(cid, msgType, fromId, message);
        return;
      }

      // Chat de grupo
      if (msgType == MovementMsgType::ChatGroupMessage) {
        uint32_t fromId = 0;
        std::string message;
        if (!decodeChatClientMessage(data, fromId, message)) {
          std::lock_guard<std::mutex> lock(mu_);
          sendChatErrorToClientUnlocked(cid, ChatErrorCode::InvalidPayload, "Payload de chat de grupo invalido");
          return;
        }
        std::lock_guard<std::mutex> lock(mu_);
        handleChatMessageUnlocked(cid, msgType, fromId, message);
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
        std::string name, title, guildName;
        uint32_t classId = 0;
        uint32_t hair = 0;
        uint32_t head = 0;
        if (decodePlayerInfoUpdate(data, playerId, name, title, guildName, &classId, &hair, &head)) {
          // Validação de conta FORA do mu_: playerBelongsToAccount faz MySQL e não pode
          // segurar o mutex global do movimento (medido até 174ms sob mu_ no Proxmox).
          if (sessionAuthEnabled_) {
            const uint32_t accountId = sessionAuth_.getAccountIdForClient(cid);
            bool belongs = false;
            bool fromCache = false;
            {
              std::lock_guard<std::mutex> lock(mu_);
              auto cacheIt = belongsOkByCid_.find(cid);
              if (cacheIt != belongsOkByCid_.end() && cacheIt->second == playerId) {
                belongs = true;
                fromCache = true;
              }
            }
            if (!belongs) {
              belongs =
                  (accountId != 0) && sessionAuth_.playerBelongsToAccount(playerId, accountId);
              if (belongs) {
                std::lock_guard<std::mutex> lock(mu_);
                belongsOkByCid_[cid] = playerId;
              }
            }
            if (!belongs) {
              Umbra::Core::Logger::getInstance().warn(
                  "PlayerInfoUpdate rejeitado: client {} player {} nao pertence a account {}",
                  cid, playerId, accountId);
              return;
            }
          }

          bool shouldReloadReactions = false;
          std::vector<uint8_t> broadcastMsg;
          {
            std::lock_guard<std::mutex> lock(mu_);

            Umbra::Core::Logger::getInstance().info("Received PlayerInfoUpdate from client {}: playerId={}, name={}, title={}, guild={}",
                                                    cid, playerId, name, title, guildName);

            setClientPlayerMapUnlocked(cid, playerId);

            const bool isNewPlayer = (players_.find(playerId) == players_.end());

            Umbra::Core::Logger::getInstance().info("📥 Received PlayerInfoUpdate from client {}: playerId={}, name={}, title={}, isNewPlayer={}, total_players={}",
                                                    cid, playerId, name, title, isNewPlayer, players_.size());

            if (players_.find(playerId) != players_.end()) {
              players_[playerId].characterName = name;
              players_[playerId].characterTitle = title;
              players_[playerId].guildName = guildName;
              if (classId > 0) {
                players_[playerId].classId = classId;
                players_[playerId].hair = hair;
                players_[playerId].head = head;
              }
              Umbra::Core::Logger::getInstance().info("✅ Updated existing PlayerStateNet for player {} (name={}, title={}, guild={})",
                                                      playerId, name, title, guildName);
            } else {
              PlayerStateNet newPlayer;
              newPlayer.playerId = playerId;
              newPlayer.characterName = name;
              newPlayer.characterTitle = title;
              newPlayer.guildName = guildName;
              newPlayer.classId = classId;
              newPlayer.hair = hair;
              newPlayer.head = head;
              newPlayer.x = 0.0f;
              newPlayer.y = 0.0f;
              newPlayer.z = 0.0f;
              newPlayer.yaw = 0.0f;
              newPlayer.tsMs = 0;
              players_[playerId] = newPlayer;

              Umbra::Core::Logger::getInstance().info("✅ Created new PlayerStateNet for player {} (name={}, title={}, guild={}) - posição será definida no primeiro MoveUpdate",
                                                      playerId, name, title, guildName);
              shouldReloadReactions = true;
            }

            broadcastMsg = encodePlayerInfoUpdate(playerId, name, title, guildName,
                                                  players_[playerId].classId,
                                                  players_[playerId].hair,
                                                  players_[playerId].head);
          }

          // MySQL em reloadArmedForPlayer: nunca sob mu_ (mesmo padrão de SkillCast/BasicAttack).
          if (combatCoreEngine_) {
            if (shouldReloadReactions) {
              combatCoreEngine_->onPlayerJoinedZone(playerId);
            } else {
              combatCoreEngine_->syncJoinDeathState(playerId);
            }
          }

          ws_.broadcastBinary(broadcastMsg);
          Umbra::Core::Logger::getInstance().info("📤 Broadcasted PlayerInfoUpdate for player {} (name={}, title={}, guild={}) to all clients",
                                                  playerId, name, title, guildName);
        }
        return;
      }

      if (msgType == MovementMsgType::PlayerEquipmentVisualUpdate) {
        uint32_t playerId = 0;
        std::vector<EquippedVisualWireEntry> entries;
        if (decodePlayerEquipmentVisualUpdate(data, playerId, entries)) {
          if (sessionAuthEnabled_) {
            const uint32_t accountId = sessionAuth_.getAccountIdForClient(cid);
            bool belongs = false;
            {
              std::lock_guard<std::mutex> lock(mu_);
              auto cacheIt = belongsOkByCid_.find(cid);
              if (cacheIt != belongsOkByCid_.end() && cacheIt->second == playerId) {
                belongs = true;
              }
            }
            if (!belongs) {
              belongs = (accountId != 0) && sessionAuth_.playerBelongsToAccount(playerId, accountId);
              if (belongs) {
                std::lock_guard<std::mutex> lock(mu_);
                belongsOkByCid_[cid] = playerId;
              }
            }
            if (!belongs) {
              Umbra::Core::Logger::getInstance().warn(
                  "PlayerEquipmentVisualUpdate rejeitado: client {} player {} nao pertence a account {}",
                  cid, playerId, accountId);
              return;
            }
          }

          std::vector<uint8_t> broadcastMsg;
          {
            std::lock_guard<std::mutex> lock(mu_);
            setClientPlayerMapUnlocked(cid, playerId);
            if (players_.find(playerId) != players_.end()) {
              players_[playerId].equippedVisual = entries;
            } else {
              PlayerStateNet newPlayer;
              newPlayer.playerId = playerId;
              newPlayer.equippedVisual = entries;
              players_[playerId] = newPlayer;
            }
            broadcastMsg = encodePlayerEquipmentVisualUpdate(playerId, players_[playerId].equippedVisual);
          }
          ws_.broadcastBinary(broadcastMsg);
        }
        return;
      }

      // Opcode 119: modo relaxado/combate. So guarda o estado e retransmite (sem DB, sem combate).
      if (msgType == MovementMsgType::CombatStanceState) {
        CombatStancePayload payload;
        if (!decodeCombatStanceState(data, payload)) return;
        // O player vem do cid (mapeado no movimento/visual); o playerId do payload precisa bater.
        uint32_t senderPlayerId = 0;
        {
          std::lock_guard<std::mutex> lock(mu_);
          auto it = clientIdToPlayerId_.find(cid);
          if (it != clientIdToPlayerId_.end()) senderPlayerId = it->second;
        }
        if (senderPlayerId == 0 || senderPlayerId != payload.playerId) {
          Umbra::Core::Logger::getInstance().warn(
              "CombatStanceState ignorado: client {} sender {} payload player {}",
              cid, senderPlayerId, payload.playerId);
          return;
        }
        std::vector<uint8_t> broadcastMsg;
        {
          std::lock_guard<std::mutex> lock(mu_);
          auto pit = players_.find(senderPlayerId);
          if (pit == players_.end()) return;
          if (pit->second.combatStance == payload.stance) return;  // sem mudanca: nao retransmite
          pit->second.combatStance = payload.stance;
          CombatStancePayload out;
          out.playerId = senderPlayerId;
          out.stance = payload.stance;
          out.flags = payload.flags & kCombatStanceFlagAuto;
          broadcastMsg = encodeCombatStanceState(out);
        }
        ws_.broadcastBinary(broadcastMsg);
        return;
      }

      // Guild refresh/member notifications encaminhadas pelo cliente (após sucesso HTTP).
      if (msgType == MovementMsgType::GuildInviteReceived) {
        uint32_t inviteId = 0, guildId = 0, fromPlayerId = 0, toPlayerId = 0;
        if (decodeGuildInviteReceived(data, inviteId, guildId, fromPlayerId, toPlayerId)) {
          std::lock_guard<std::mutex> lock(mu_);
          sendToPlayerUnlocked(toPlayerId, encodeGuildInviteReceived(inviteId, guildId, fromPlayerId, toPlayerId));
        }
        return;
      }
      if (msgType == MovementMsgType::GuildStateRefresh) {
        uint32_t guildId = 0;
        if (decodeGuildNotify(data, guildId)) {
          std::lock_guard<std::mutex> lock(mu_);
          ws_.broadcastBinary(encodeGuildNotify(MovementMsgType::GuildStateRefresh, guildId));
        }
        return;
      }
      if (msgType == MovementMsgType::GuildMemberUpdated || msgType == MovementMsgType::GuildMemberKicked) {
        if (data.size() >= 9) {
          std::lock_guard<std::mutex> lock(mu_);
          ws_.broadcastBinary(data);
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
  }

  void stop() {
    ws_.stop();           // callbacks de disconnect ainda enfileiram a fase 2
    stopCleanupWorker();  // drena a fila (DB/broadcast) e encerra
  }

  ~MovementServer() { stopCleanupWorker(); }

  /** PING RFC6455 a todos os clientes (evita WinHTTP HttpActivityTimeout ~30s sem tráfego). */
  void sendKeepalivePings() { ws_.broadcastPing(); }

  /** Snapshot periódico (10 Hz). Coleta sob mu_; envio FORA do lock (send bloqueante). */
  void broadcastSnapshot() {
    Outbox outbox;
    {
      std::lock_guard<std::mutex> lock(mu_);
      for (const auto& [pid, st] : players_) {
        MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
        auto bytes = encodeWithAnimation(f, st.speed, st.velocityZ, st.isInAir);

        uint32_t sourceClientId = 0;
        for (const auto& [cid, mappedPid] : clientIdToPlayerId_) {
          if (mappedPid == pid) {
            sourceClientId = cid;
            break;
          }
        }

        if (sourceClientId > 0) {
          auto nearby = aoiGrid_.getNearbyPlayers(sourceClientId);
          for (uint32_t nearbyClientId : nearby) {
            outbox.emplace_back(nearbyClientId, bytes);
          }
        }
      }
    }
    flushOutbox(outbox);
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

  /** True se o player tem sessão TCP mapeada (client↔player) nesta zone. */
  bool isPlayerConnected(uint32_t playerId) const {
    if (playerId == 0) return false;
    std::lock_guard<std::mutex> lock(mu_);
    for (const auto& [cid, pid] : clientIdToPlayerId_) {
      (void)cid;
      if (pid == playerId) return true;
    }
    return false;
  }

  bool kickPlayer(uint32_t playerId) {
    uint32_t cid = 0;
    {
      std::lock_guard<std::mutex> lock(mu_);
      for (const auto& [c, p] : clientIdToPlayerId_) {
        if (p == playerId) {
          cid = c;
          break;
        }
      }
    }
    if (cid == 0) return false;
    // disconnect callback adquire mu_ de novo — nunca chamar sob lock.
    ws_.disconnect(cid);
    return true;
  }

  bool teleportPlayer(uint32_t playerId, float x, float y, float z) {
    MovementFrame correction{};
    {
      std::lock_guard<std::mutex> lock(mu_);
      auto it = players_.find(playerId);
      if (it == players_.end()) return false;
      it->second.x = x;
      it->second.y = y;
      it->second.z = z;
      // Antes: tsMs = relógio do SERVIDOR, mas a validação compara com o relógio do CLIENTE
      // (game time) → caía em "timestamp regrediu", pulava a checagem e um move antigo
      // desfazia o teleporte. Agora: trava de posição até o cliente confirmar o ponto.
      armPositionGuardUnlocked(playerId, x, y, z);
      uint32_t cid = 0;
      for (const auto& [c, p] : clientIdToPlayerId_) {
        if (p == playerId) {
          cid = c;
          break;
        }
      }
      if (cid > 0) {
        aoiGrid_.updatePlayer(cid, x, y);
      }
      correction = {MovementMsgType::StateUpdate, playerId, x, y, z, it->second.yaw,
                    it->second.tsMs};
    }
    // Correção autoritativa usando o frame little-endian existente (25B).
    ws_.broadcastBinary(encode(correction));
    return true;
  }

  bool broadcastAdminMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(mu_);
    auto packet = encodeChatReceived(
        MovementMsgType::ChatGlobalReceived, 0, "GM", message);
    ws_.broadcastBinary(packet);
    // Toast dedicado (opcode 73) além do chat global (70)
    auto toast = encodeSystemBroadcast(1, 5000, "GM", message);
    ws_.broadcastBinary(toast);
    return true;
  }

  bool notifyMailToPlayer(uint32_t playerId, uint32_t mailId,
                          const std::string& fromName, const std::string& subject) {
    std::lock_guard<std::mutex> lock(mu_);
    const uint32_t cid = findClientIdForPlayerUnlocked(playerId);
    if (cid == 0) return false;
    auto packet = encodeMailNotify(mailId, fromName, subject);
    ws_.sendBinary(cid, packet);
    return true;
  }

  void setLimits(float maxSpeed, float maxTeleportDist, uint32_t maxDelayMs) {
    maxSpeed_ = maxSpeed; maxTeleportDist_ = maxTeleportDist; maxDelayMs_ = maxDelayMs;
  }

private:
  static bool isLikelyValidUtf8(const std::string& text) {
    int expected = 0;
    for (unsigned char c : text) {
      if (expected == 0) {
        if ((c >> 7) == 0b0) {
          continue;
        }
        if ((c >> 5) == 0b110) {
          expected = 1;
          continue;
        }
        if ((c >> 4) == 0b1110) {
          expected = 2;
          continue;
        }
        if ((c >> 3) == 0b11110) {
          expected = 3;
          continue;
        }
        return false;
      }
      if ((c >> 6) != 0b10) {
        return false;
      }
      --expected;
    }
    return expected == 0;
  }

  static std::string trimMessage(const std::string& value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) ++start;
    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) --end;
    return value.substr(start, end - start);
  }

  void sendChatErrorToClientUnlocked(uint32_t clientId, ChatErrorCode errorCode, const std::string& message) {
    ws_.sendBinary(clientId, encodeChatServerError(errorCode, message));
  }

  bool checkAndConsumeChatRateLimitUnlocked(uint32_t playerId, std::chrono::steady_clock::time_point now) {
    auto& history = chatMessageHistoryByPlayer_[playerId];
    const auto windowStart = now - std::chrono::minutes(1);
    while (!history.empty() && history.front() < windowStart) {
      history.pop_front();
    }
    if (chatRateLimitPerMinute_ > 0 && history.size() >= chatRateLimitPerMinute_) {
      return false;
    }
    history.push_back(now);
    return true;
  }

  void handleChatMessageUnlocked(uint32_t cid, MovementMsgType msgType, uint32_t payloadFromId, const std::string& rawMessage) {
    auto cidIt = clientIdToPlayerId_.find(cid);
    if (cidIt == clientIdToPlayerId_.end() || cidIt->second == 0) {
      sendChatErrorToClientUnlocked(cid, ChatErrorCode::NotAuthenticated, "Jogador nao autenticado no zone");
      return;
    }

    uint32_t senderPlayerId = cidIt->second;
    if (payloadFromId != 0 && payloadFromId != senderPlayerId) {
      Umbra::Core::Logger::getInstance().warn(
          "Chat spoof attempt ignored: client {} mapped player {} payload from {}", cid, senderPlayerId, payloadFromId);
    }

    std::string message = trimMessage(rawMessage);
    if (message.empty()) {
      sendChatErrorToClientUnlocked(cid, ChatErrorCode::EmptyMessage, "Mensagem vazia");
      return;
    }
    if (message.size() > chatMaxMessageLength_) {
      sendChatErrorToClientUnlocked(cid, ChatErrorCode::MessageTooLong, "Mensagem excede limite");
      return;
    }
    if (!isLikelyValidUtf8(message)) {
      sendChatErrorToClientUnlocked(cid, ChatErrorCode::InvalidPayload, "Mensagem UTF-8 invalida");
      return;
    }

    auto now = std::chrono::steady_clock::now();
    if (!checkAndConsumeChatRateLimitUnlocked(senderPlayerId, now)) {
      sendChatErrorToClientUnlocked(cid, ChatErrorCode::RateLimitExceeded, "Limite de mensagens por minuto excedido");
      return;
    }

    std::string fromName;
    auto pit = players_.find(senderPlayerId);
    if (pit != players_.end() && !pit->second.characterName.empty()) {
      fromName = pit->second.characterName;
    } else {
      fromName = "Player_" + std::to_string(senderPlayerId);
    }

    if (msgType == MovementMsgType::ChatLocalMessage) {
      auto packet = encodeChatReceived(MovementMsgType::ChatLocalReceived, senderPlayerId, fromName, message);
      broadcastToNearby(cid, packet);
      Umbra::Core::Logger::getInstance().info("Local chat from {} ({}) delivered", senderPlayerId, fromName);
      return;
    }

    if (msgType == MovementMsgType::ChatGlobalMessage) {
      auto packet = encodeChatReceived(MovementMsgType::ChatGlobalReceived, senderPlayerId, fromName, message);
      // Esta funcao ja roda sob mu_ (chamada de handleChatMessageUnlocked com lock).
      // Usar ws_.broadcastBinary direto: broadcastToAll re-trava mu_ -> deadlock recursivo.
      ws_.broadcastBinary(packet);
      Umbra::Core::Logger::getInstance().info("Global chat from {} ({}) delivered", senderPlayerId, fromName);
      return;
    }

    if (msgType == MovementMsgType::ChatGroupMessage) {
      if (!resolvePartyMembers_) {
        sendChatErrorToClientUnlocked(cid, ChatErrorCode::Unknown, "Resolucao de grupo indisponivel");
        return;
      }
      std::vector<uint32_t> members;
      try {
        members = resolvePartyMembers_(senderPlayerId);
      } catch (...) {
        sendChatErrorToClientUnlocked(cid, ChatErrorCode::Unknown, "Falha ao resolver membros do grupo");
        return;
      }
      if (members.size() <= 1) {
        sendChatErrorToClientUnlocked(cid, ChatErrorCode::InvalidPayload, "Voce nao esta em um grupo");
        return;
      }
      auto packet = encodeChatReceived(MovementMsgType::ChatGroupReceived, senderPlayerId, fromName, message);
      size_t sentCount = 0;
      for (uint32_t memberId : members) {
        sendToPlayerUnlocked(memberId, packet);
        ++sentCount;
      }
      Umbra::Core::Logger::getInstance().info("Group chat from {} ({}) delivered to {} members", senderPlayerId, fromName, sentCount);
      return;
    }

    sendChatErrorToClientUnlocked(cid, ChatErrorCode::Unknown, "Tipo de chat desconhecido");
  }

  // Versão com lock - chamada do callback de conexão (sem lock prévio)

  void revokeAndDisconnectClient(uint32_t cid, SessionRevokeReason reason, const std::string& message) {
    Umbra::Core::Logger::getInstance().info("Revogando sessão do client {} (motivo={}): {}", cid,
                                            static_cast<int>(reason), message);
    auto pkt = encodeSessionRevokedNotify(reason, message);
    ws_.sendBinary(cid, pkt);
    ws_.disconnect(cid);
  }

  void sendPostAuthSnapshots(uint32_t cid) {
    Outbox outbox;
    size_t sentInfoCount = 0;
    size_t sentStateCount = 0;
    {
      std::lock_guard<std::mutex> lock(mu_);
      collectInitialSnapshotUnlocked(cid, outbox, sentInfoCount, sentStateCount);
    }
    flushOutbox(outbox);
    Umbra::Core::Logger::getInstance().info("Initial snapshot to client {}: {} info + {} states",
                                             cid, sentInfoCount, sentStateCount);
    if (combatCoreEngine_) {
      combatCoreEngine_->sendNpcSnapshotToClient(cid);
      combatCoreEngine_->sendNpcBuffSnapshotToClient(cid);
      combatCoreEngine_->sendPlayerBuffSnapshotToClient(cid);
    }
  }

  /** Monta o snapshot inicial no outbox (chamar sob mu_). Envio via flushOutbox fora do lock. */
  void collectInitialSnapshotUnlocked(uint32_t clientId, Outbox& outbox, size_t& sentInfoCount,
                                      size_t& sentStateCount) {
    sentInfoCount = 0;
    sentStateCount = 0;

    for (const auto& [pid, st] : players_) {
      if (!st.characterName.empty() || !st.characterTitle.empty() || !st.guildName.empty()) {
        outbox.emplace_back(clientId,
                            encodePlayerInfoUpdate(st.playerId, st.characterName, st.characterTitle,
                                                   st.guildName, st.classId, st.hair, st.head));
        ++sentInfoCount;
      }
    }

    for (const auto& [pid, st] : players_) {
      if (!st.equippedVisual.empty()) {
        outbox.emplace_back(clientId, encodePlayerEquipmentVisualUpdate(st.playerId, st.equippedVisual));
      }
    }

    // Opcode 119: quem chega ve quem ja esta em combate (flag auto = sem anim de sacar).
    for (const auto& [pid, st] : players_) {
      if (st.combatStance != 0) {
        CombatStancePayload cs;
        cs.playerId = st.playerId;
        cs.stance = st.combatStance;
        cs.flags = kCombatStanceFlagAuto;
        outbox.emplace_back(clientId, encodeCombatStanceState(cs));
      }
    }

    for (const auto& [pid, st] : players_) {
      const bool hasValidPosition = (st.x != 0.0f || st.y != 0.0f || st.z != 0.0f);
      if (!hasValidPosition) continue;

      MovementFrame f{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
      outbox.emplace_back(clientId, encodeWithAnimation(f, st.speed, st.velocityZ, st.isInAir));
      ++sentStateCount;
    }

    for (const auto& [pid, info] : personalShopOpenByPlayerId_) {
      outbox.emplace_back(clientId, encodePersonalShopOpened(pid, info.first, info.second));
    }
  }

  void handleMoveUpdate(uint32_t cid, const MovementFrame& f, bool hasAnimation, float speed, float velocityZ, bool isInAir) {
    // loadPlayerState pode ir ao MySQL: ler fora de mu_ para não bloquear WS/combate.
    float moveSpeedPct = 100.f;
    bool canMove = true;
    if (combatCoreEngine_) {
      moveSpeedPct = combatCoreEngine_->getPlayerMovementSpeedPercent(f.playerId);
      canMove = combatCoreEngine_->canPlayerMove(f.playerId);
    }

    std::unique_lock<std::mutex> lock(mu_);

    // Atualizar mapeamento ClientID -> PlayerID
    setClientPlayerMapUnlocked(cid, f.playerId);

    if (personalShopOpenByPlayerId_.find(f.playerId) != personalShopOpenByPlayerId_.end()) {
      Umbra::Core::Logger::getInstance().debug("MoveUpdate rejected: player {} has personal shop open", f.playerId);
      return;
    }

    auto deadIt = players_.find(f.playerId);
    if (deadIt != players_.end() && deadIt->second.isDead) {
      Umbra::Core::Logger::getInstance().trace("MoveUpdate rejected: player {} is dead", f.playerId);
      return;
    }
    if (!canMove) {
      Umbra::Core::Logger::getInstance().debug(
          "MoveUpdate rejected: player {} under STUN/ROOT", f.playerId);
      return;
    }

    // Trava de posição pós respawn/teleporte: MoveUpdates enviados ANTES do cliente aplicar
    // a posição autoritativa ainda estão em trânsito. Antes, o 1º deles (gap longo) era aceito
    // e sobrescrevia o spawn; em seguida o move real do spawn virava "speed too high" (~0,5s).
    bool acceptAuthoritativeSnap = false;
    auto guardIt = positionGuards_.find(f.playerId);
    if (guardIt != positionGuards_.end()) {
      const PositionGuard& g = guardIt->second;
      if (Umbra::Core::steadyNowMs() > g.untilMs) {
        positionGuards_.erase(guardIt);  // cliente nunca confirmou: volta à validação normal
      } else {
        const float gdx = f.x - g.x;
        const float gdy = f.y - g.y;
        const float gdz = f.z - g.z;
        if (gdx * gdx + gdy * gdy + gdz * gdz > kPositionGuardRadius * kPositionGuardRadius) {
          Umbra::Core::Logger::getInstance().debug(
              "MoveUpdate obsoleto ignorado: player {} ainda longe da posição autoritativa", f.playerId);
          return;
        }
        positionGuards_.erase(guardIt);
        acceptAuthoritativeSnap = true;  // cliente chegou ao ponto: aceita sem checar velocidade
      }
    }

    bool isNewPlayer = (players_.find(f.playerId) == players_.end());

    // ✅ CRÍTICO: Se for um novo player OU se o player existir mas tiver posição inválida (0,0,0),
    // não validar teleporte no primeiro movimento
    bool isFirstMovement = isNewPlayer || acceptAuthoritativeSnap;
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
            // Gap longo (lag/spike/host I/O / tab-out): NÃO usar dt=0.033 — isso infla a
            // velocidade (dist/0.033), rejeita o move e deixa prevTs travado → jogador
            // "não anda" até reconectar. Evidência: speed~4k–6k com prevTs fixo e currTs
            // avançando após spikes de sleepMs~2–3s no Proxmox.
            if (timeDiff > 500) {
              dt = timeDiff / 1000.0f;
              skipSpeedCheck = true;
              Umbra::Core::Logger::getInstance().debug(
                  "Large move gap for player {} ({}ms): reconcile without speed reject",
                  f.playerId, timeDiff);
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
            const float allowedMaxSpeed = maxSpeed_ * moveSpeedPct / 100.f;
            if (calculatedSpeed > allowedMaxSpeed) {
              Umbra::Core::Logger::getInstance().warn("MoveUpdate from client {} rejected: speed too high (speed={}, max={}, dist={}, dt={}, prevTs={}, currTs={})", 
                                                      cid, calculatedSpeed, allowedMaxSpeed, std::sqrt(dist2), dt, prevTs, f.tsMs);
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

    // Atualiza só posição/animação no registro existente. Antes o registro era recriado a cada
    // MoveUpdate e perdia combatStance (119), equippedVisual (117), classId/hair/head: o "guardar"
    // chegava como "sem mudança" e não era retransmitido, e o snapshot de quem entrava vinha errado.
    PlayerStateNet& moveState = players_[f.playerId];
    moveState.playerId = f.playerId;
    moveState.x = f.x;
    moveState.y = f.y;
    moveState.z = f.z;
    moveState.yaw = f.yaw;
    moveState.tsMs = finalTimestamp;
    moveState.speed = speed;
    moveState.velocityZ = velocityZ;
    moveState.isInAir = isInAir;
    
    if (isFirstPositionUpdate) {
      MovementFrame firstOut{MovementMsgType::StateUpdate, f.playerId, f.x, f.y, f.z, f.yaw, finalTimestamp};
      std::vector<uint8_t> firstBroadcastBytes;
      if (hasAnimation) {
        firstBroadcastBytes = encodeWithAnimation(firstOut, speed, velocityZ, isInAir);
      } else {
        firstBroadcastBytes = encodeWithAnimation(firstOut, 0.0f, 0.0f, false);
      }

      std::vector<uint32_t> allClients;
      allClients.reserve(clientIdToPlayerId_.size());
      for (const auto& [c, p] : clientIdToPlayerId_) {
        (void)p;
        allClients.push_back(c);
      }

      Outbox newPlayerSnapshot;
      if (isNewPlayer) {
        for (const auto& [pid, st] : players_) {
          const bool hasValidPosition = (st.x != 0.0f || st.y != 0.0f || st.z != 0.0f);
          if (!hasValidPosition) continue;
          MovementFrame sf{MovementMsgType::StateUpdate, st.playerId, st.x, st.y, st.z, st.yaw, st.tsMs};
          auto snapBytes = encodeWithAnimation(sf, st.speed, st.velocityZ, st.isInAir);
          for (uint32_t rcid : allClients) {
            newPlayerSnapshot.emplace_back(rcid, snapBytes);
          }
        }
      }

      const size_t clientCount = allClients.size();
      lock.unlock();
      for (uint32_t rcid : allClients) {
        ws_.sendBinary(rcid, firstBroadcastBytes);
      }
      Umbra::Core::Logger::getInstance().info(
          "Player {} first position ({:.0f},{:.0f},{:.0f}) broadcast to {} clients", f.playerId, f.x,
          f.y, f.z, clientCount);
      if (!newPlayerSnapshot.empty()) {
        flushOutbox(newPlayerSnapshot);
      }
      return;
    }

    // broadcast imediato de state_update (além do snapshot periódico)
    // Usar finalTimestamp (timestamp do servidor para novos, relativo para existentes)
    MovementFrame out{MovementMsgType::StateUpdate, f.playerId, f.x, f.y, f.z, f.yaw, finalTimestamp};
    std::vector<uint8_t> broadcastBytes;
    
    // Se o frame original tinha animação, reenviar com animação também
    if (hasAnimation) {
      broadcastBytes = encodeWithAnimation(out, speed, velocityZ, isInAir);
      Umbra::Core::Logger::getInstance().trace("Broadcasting StateUpdate with animation: PlayerID={}, pos=({}, {}, {}), yaw={}, speed={}, velocityZ={}, isInAir={}, ts={}, frame_size={} bytes",
                                               out.playerId, out.x, out.y, out.z, out.yaw, speed, velocityZ, isInAir, finalTimestamp, broadcastBytes.size());
    } else {
      broadcastBytes = encodeWithAnimation(out, 0.0f, 0.0f, false); // Usar encodeWithAnimation mesmo sem animação para consistência
      Umbra::Core::Logger::getInstance().trace("Broadcasting StateUpdate (no animation): PlayerID={}, pos=({}, {}, {}), yaw={}, ts={}, frame_size={} bytes",
                                               out.playerId, out.x, out.y, out.z, out.yaw, finalTimestamp, broadcastBytes.size());
    }
    
    if (broadcastBytes.size() >= 5) {
      Umbra::Core::Logger::getInstance().trace("  Frame bytes [0-4]: {:02X} {:02X} {:02X} {:02X} {:02X}",
                                               broadcastBytes[0], broadcastBytes[1], broadcastBytes[2], broadcastBytes[3], broadcastBytes[4]);
    }
    
    if (!isFirstPositionUpdate) {
      // Resolver destinatários AOI sob mu_ (aoiGrid_ é mutável por outras threads),
      // depois LIBERAR mu_ e enviar: send() é bloqueante e não pode segurar mu_
      // (senão um cliente lento congela todas as threads e derruba conexões).
      std::vector<uint32_t> moveRecipients = aoiGrid_.getNearbyPlayers(cid);
      moveRecipients.push_back(cid);
      lock.unlock();
      for (uint32_t rcid : moveRecipients) {
        ws_.sendBinary(rcid, broadcastBytes);
      }
    }
  }

  // Trabalho de desconexao a ser executado FORA do mu_ (DB + broadcasts).
  struct DisconnectCleanup {
    bool hasPlayer = false;
    uint32_t playerId = 0;
  };

  // Fase 1 (sob mu_): apenas mutacoes em memoria. Rapido, nao bloqueia outros clientes.
  DisconnectCleanup handleClientDisconnect(uint32_t cid) {
    DisconnectCleanup cleanup;
    {
      std::lock_guard<std::mutex> lock(inboundMu_);
      pendingMoveByCid_.erase(cid);
    }
    belongsOkByCid_.erase(cid);
    if (sessionAuthEnabled_) {
      sessionAuth_.onClientDisconnected(cid);
    }
    auto it = clientIdToPlayerId_.find(cid);
    if (it != clientIdToPlayerId_.end()) {
      uint32_t playerId = it->second;
      auto playerIt = players_.find(playerId);
      if (playerIt != players_.end()) {
        Umbra::Core::Logger::getInstance().info("Removing player {} (client {}) from players map", playerId, cid);
        aoiGrid_.removePlayer(cid);
        players_.erase(playerIt);
        positionGuards_.erase(playerId);
        personalShopOpenByPlayerId_.erase(playerId);
        chatMessageHistoryByPlayer_.erase(playerId);
        cleanup.hasPlayer = true;
        cleanup.playerId = playerId;
      }
      eraseClientMapUnlocked(cid);
    } else {
      Umbra::Core::Logger::getInstance().debug("Client {} disconnected but had no associated player", cid);
    }
    return cleanup;
  }

  // Fase 2 (SEM mu_): DB (sair de party / player_sessions) + broadcasts. Pode levar
  // centenas de ms / segundos no DB remoto sem travar as threads dos outros clientes.
  void finishClientDisconnect(uint32_t cid, const DisconnectCleanup& cleanup) {
    if (cleanup.hasPlayer) {
      const uint32_t playerId = cleanup.playerId;

      // Roda na thread de limpeza: o mesmo personagem pode já ter voltado em outro client
      // (login duplicado / relog rápido). Nesse caso NÃO sair da party nem avisar
      // PlayerDisconnected — isso sumiria com o personagem recém-entrado para os outros.
      auto onlineAgain = [&]() {
        std::lock_guard<std::mutex> lock(mu_);
        return findClientIdForPlayerUnlocked(playerId) != 0;
      };
      if (onlineAgain()) {
        Umbra::Core::Logger::getInstance().info(
            "Player {} já reconectou em outro client: limpeza do client {} ignorada", playerId, cid);
        return;
      }

      // Membros do grupo lidos do cache ANTES de sair: o próximo refresh tira quem desconectou.
      const std::vector<uint32_t> partyMembersBefore = getPartyMembersCached(playerId);

      // Remover jogador do grupo no servidor (party_members no DB) e avisar os membros se estava em grupo
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
        {
          std::lock_guard<std::mutex> lock(mu_);
          sendPartyNoticeUnlocked(partyMembersBefore, 0, memberLeftMsg);
        }
        Umbra::Core::Logger::getInstance().info("Sent PartyMemberLeft for party {} (player {} disconnected, {} membros no cache)",
                                                partyId, playerId, partyMembersBefore.size());
      }

      // Reconectou enquanto o DB rodava: não anunciar desconexão de quem está online.
      if (onlineAgain()) {
        Umbra::Core::Logger::getInstance().info(
            "Player {} reconectou durante a limpeza do client {}: PlayerDisconnected suprimido",
            playerId, cid);
        return;
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

    // IMPORTANTE: O servidor WebSocket DEVE continuar rodando mesmo sem players
    Umbra::Core::Logger::getInstance().info("Client {} disconnected. Server continues running.", cid);
  }

  // Tipo do "outbox": pares (clientId, bytes) resolvidos sob mu_ para envio DEPOIS de liberar mu_.

  /** Resolve o clientId do playerId e enfileira no outbox (chamar sob mu_). Não envia nada. */
  // Mantém clientIdToPlayerId_ e o reverso playerIdToClientId_ em sincronia (1:1).
  void setClientPlayerMapUnlocked(uint32_t cid, uint32_t pid) {
    auto old = clientIdToPlayerId_.find(cid);
    if (old != clientIdToPlayerId_.end() && old->second != pid) {
      auto rev = playerIdToClientId_.find(old->second);
      if (rev != playerIdToClientId_.end() && rev->second == cid) {
        playerIdToClientId_.erase(rev);
      }
    }
    clientIdToPlayerId_[cid] = pid;
    playerIdToClientId_[pid] = cid;
  }

  void eraseClientMapUnlocked(uint32_t cid) {
    auto it = clientIdToPlayerId_.find(cid);
    if (it == clientIdToPlayerId_.end()) return;
    auto rev = playerIdToClientId_.find(it->second);
    if (rev != playerIdToClientId_.end() && rev->second == cid) {
      playerIdToClientId_.erase(rev);
    }
    clientIdToPlayerId_.erase(it);
  }

  uint32_t findClientIdForPlayerUnlocked(uint32_t playerId) const {
    auto it = playerIdToClientId_.find(playerId);
    return (it != playerIdToClientId_.end()) ? it->second : 0;
  }

  uint32_t playerIdForClientUnlocked(uint32_t clientId) const {
    auto it = clientIdToPlayerId_.find(clientId);
    return (it != clientIdToPlayerId_.end()) ? it->second : 0;
  }

  // Aviso de party (opcodes 13/14/27) só para os membros do grupo (+ extraPlayerId).
  // Sem membros conhecidos (cache frio ou grupo recém-criado) cai no broadcast: o cliente
  // filtra pelo próprio CurrentPartyID. Chamar com mu_ travado (ordem mu_ -> partyCacheMu_).
  void sendPartyNoticeUnlocked(const std::vector<uint32_t>& members, uint32_t extraPlayerId,
                               const std::vector<uint8_t>& message) {
    if (members.empty()) {
      ws_.broadcastBinary(message);
      return;
    }
    std::unordered_set<uint32_t> sent;
    auto sendTo = [&](uint32_t playerId) {
      if (playerId == 0 || !sent.insert(playerId).second) return;
      const uint32_t targetClientId = findClientIdForPlayerUnlocked(playerId);
      if (targetClientId != 0) {
        ws_.sendBinary(targetClientId, message);
      }
    };
    for (uint32_t memberId : members) {
      sendTo(memberId);
    }
    sendTo(extraPlayerId);
  }

  void enqueueToPlayerUnlocked(uint32_t playerId, const std::vector<uint8_t>& message, Outbox& outbox) {
    const uint32_t cid = findClientIdForPlayerUnlocked(playerId);
    if (cid != 0) {
      outbox.emplace_back(cid, message);
      return;
    }
    Umbra::Core::Logger::getInstance().warn("Player {} not found online, cannot send message", playerId);
  }

  /** Enfileira tudo do outbox. Agrupa por cid para uma única aquisição de lock por
   *  cliente (a writer thread do WS coalesce os frames num único send()).
   *  Pode ser chamado fora do mu_ pois o envio é assíncrono. */
  void flushOutbox(const Outbox& outbox) {
    if (outbox.empty()) return;
    // Agrupa preservando a ordem de chegada por cliente.
    std::unordered_map<uint32_t, std::vector<const std::vector<uint8_t>*>> byCid;
    std::vector<uint32_t> order;
    order.reserve(outbox.size());
    for (const auto& entry : outbox) {
      auto it = byCid.find(entry.first);
      if (it == byCid.end()) {
        order.push_back(entry.first);
        byCid[entry.first].push_back(&entry.second);
      } else {
        it->second.push_back(&entry.second);
      }
    }
    for (uint32_t cid : order) {
      ws_.sendBinaryBatch(cid, byCid[cid]);
    }
  }

  // Enviar mensagem para um jogador específico (por PlayerID) - versão sem lock (chamada com lock já adquirido)
  void applyRespawnSuccessUnlocked(uint32_t playerId, const PlayerRespawnPayload& payload,
                                   Outbox& outbox) {
    auto it = players_.find(playerId);
    if (it != players_.end()) {
      it->second.isDead = false;
      it->second.x = payload.x;
      it->second.y = payload.y;
      it->second.z = payload.z;
      it->second.yaw = payload.yaw;
      armPositionGuardUnlocked(playerId, payload.x, payload.y, payload.z);
    } else {
      PlayerStateNet stub;
      stub.playerId = playerId;
      stub.isDead = false;
      stub.x = payload.x;
      stub.y = payload.y;
      stub.z = payload.z;
      stub.yaw = payload.yaw;
      players_[playerId] = stub;
      armPositionGuardUnlocked(playerId, payload.x, payload.y, payload.z);
    }
    lastKnownHealth_[playerId] = payload.currentHealth;

    PlayerVitalsPayload vitals;
    vitals.playerId = playerId;
    vitals.currentHealth = payload.currentHealth;
    vitals.maxHealth = std::max(1, payload.maxHealth);
    vitals.currentMana = payload.currentMana;
    vitals.maxMana = std::max(1, payload.maxMana);
    vitals.sourcePlayerId = playerId;
    vitals.reason = 0;
    handleVitalsBroadcastUnlocked(playerId, vitals, playerId, /*delta*/ 0, /*triggerDeath*/ false,
                                  false, false, outbox);

    auto respawnPkt = encodePlayerRespawnedNotify(payload);
    std::unordered_set<uint32_t> recipients;
    collectVitalsRecipientsUnlocked(playerId, recipients);
    for (uint32_t rid : recipients) {
      enqueueToPlayerUnlocked(rid, respawnPkt, outbox);
    }
  }

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

  /** Broadcast para jogadores próximos (AOI) em vez de todos. Usa SpatialGrid. */
  void broadcastToNearby(uint32_t sourceClientId, const std::vector<uint8_t>& data) {
    auto nearby = aoiGrid_.getNearbyPlayers(sourceClientId);
    for (uint32_t nearbyClientId : nearby) {
      ws_.sendBinary(nearbyClientId, data);
    }
    ws_.sendBinary(sourceClientId, data);
  }

  // Lê os membros de party do cache (sem MySQL). Retorna vazio se ainda não
  // aquecido; refreshPartyCache() popula fora do mu_.
  std::vector<uint32_t> getPartyMembersCached(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(partyCacheMu_);
    auto it = partyCache_.find(playerId);
    if (it == partyCache_.end()) return {};
    return it->second.members;
  }

 public:
  // Reabastece o cache de party dos jogadores online. Chamado pelo tick da zone
  // (throttle interno ~1s). Faz os SELECTs FORA do mu_ e sem segurar partyCacheMu_
  // durante o MySQL, então nunca bloqueia o hot path de vitals nem o combat worker.
  void refreshPartyCache() {
    if (!resolvePartyMembers_) return;
    const auto now = std::chrono::steady_clock::now();
    if (now < nextPartyRefresh_) return;
    nextPartyRefresh_ = now + std::chrono::milliseconds(kPartyCacheTtlMs);

    std::vector<uint32_t> onlinePids;
    {
      std::lock_guard<std::mutex> lock(mu_);
      onlinePids.reserve(clientIdToPlayerId_.size());
      for (const auto& [cid, pid] : clientIdToPlayerId_) {
        if (pid > 0) onlinePids.push_back(pid);
      }
    }

    // SELECTs sem nenhum lock desta classe.
    std::unordered_map<uint32_t, std::vector<uint32_t>> fresh;
    fresh.reserve(onlinePids.size());
    for (uint32_t pid : onlinePids) {
      fresh[pid] = resolvePartyMembers_(pid);
    }

    const auto expiresAt = std::chrono::steady_clock::now() +
                           std::chrono::milliseconds(kPartyCacheTtlMs * 2);
    std::lock_guard<std::mutex> lock(partyCacheMu_);
    partyCache_.clear();  // remove jogadores que saíram
    for (auto& [pid, members] : fresh) {
      partyCache_[pid] = PartyCacheEntry{std::move(members), expiresAt};
    }
  }

 private:
  void collectVitalsRecipientsUnlocked(uint32_t targetPlayerId,
                                         std::unordered_set<uint32_t>& recipients) {
    // Party via cache (mutex próprio), NUNCA MySQL aqui: esta função roda sob mu_
    // e todo dano/DOT/regen passa por ela. O cache é reabastecido por
    // refreshPartyCache() fora do mu_. Miss = party ainda entra pelo AOI se perto.
    for (uint32_t memberId : getPartyMembersCached(targetPlayerId)) {
      recipients.insert(memberId);
    }
    recipients.insert(targetPlayerId);

    const uint32_t targetClientId = findClientIdForPlayerUnlocked(targetPlayerId);
    if (targetClientId > 0) {
      auto nearbyClientIds = aoiGrid_.getNearbyPlayers(targetClientId);
      // Cap de fan-out: em altíssima densidade (100+ na mesma célula), limitar quantos
      // vizinhos recebem vitals evita O(N^2) por evento. Party + próprio já entraram.
      for (uint32_t nearbyCid : nearbyClientIds) {
        if (recipients.size() >= kMaxVitalsRecipients) break;
        auto pidIt = clientIdToPlayerId_.find(nearbyCid);
        if (pidIt != clientIdToPlayerId_.end()) {
          recipients.insert(pidIt->second);
        }
      }
    }
  }

  // Preenche o outbox (sob mu_); os envios de rede acontecem depois via flushOutbox, FORA do mu_.
  // Assim um send() bloqueante para um cliente lento nunca congela as demais threads.
  void handleVitalsBroadcastUnlocked(uint32_t targetPlayerId, const PlayerVitalsPayload& payload,
                                     uint32_t sourcePlayerId, int32_t delta, bool triggerDeath,
                                     bool isCrit, bool isDouble, Outbox& outbox) {
    // Supressão de vitals redundantes: o caminho delta=0 (regen/sync de mana) era
    // 100% do tráfego de vitals e enchia o buffer do cliente (cascata). Só enviamos
    // opcode 87 com delta=0 quando (a) HP/MP mudaram desde o último broadcast E
    // (b) passou a janela de throttle (~15Hz). Dano/heal/morte/miss sempre passam.
    const bool isMiss = (payload.reason == 6);  // CombatReason::Miss
    bool sendVitals = true;
    if (delta == 0 && !triggerDeath && !isMiss) {
      auto& st = vitalsBcast_[targetPlayerId];
      const int64_t now = Umbra::Core::steadyNowMs();
      const bool changed =
          (st.hp != payload.currentHealth) || (st.mp != payload.currentMana);
      if (!changed || (now - st.lastMs < kVitalsThrottleMs)) {
        sendVitals = false;
      } else {
        st.hp = payload.currentHealth;
        st.mp = payload.currentMana;
        st.lastMs = now;
      }
      if (!sendVitals) {
        return;  // nada mudou / dentro do throttle: não gera tráfego nem log
      }
    } else {
      auto& st = vitalsBcast_[targetPlayerId];
      st.hp = payload.currentHealth;
      st.mp = payload.currentMana;
      st.lastMs = Umbra::Core::steadyNowMs();
    }

    auto outMsg = encodePlayerVitalsUpdate(MovementMsgType::PlayerVitalsUpdate, payload);
    std::unordered_set<uint32_t> recipients;
    collectVitalsRecipientsUnlocked(targetPlayerId, recipients);
    for (uint32_t rid : recipients) {
      enqueueToPlayerUnlocked(rid, outMsg, outbox);
    }

    if (delta != 0) {
      CombatEventPayload combat;
      combat.targetId = targetPlayerId;
      combat.sourceId = sourcePlayerId;
      combat.delta = delta;
      combat.reason = payload.reason;
      combat.isCrit = isCrit ? 1 : 0;
      combat.isDouble = isDouble ? 1 : 0;
      auto combatPkt = encodeCombatEventNotify(combat);
      for (uint32_t rid : recipients) {
        enqueueToPlayerUnlocked(rid, combatPkt, outbox);
      }
    }

    if (triggerDeath) {
      auto pit = players_.find(targetPlayerId);
      if (pit != players_.end() && !pit->second.isDead) {
        pit->second.isDead = true;
        PlayerDeathPayload death;
        death.playerId = targetPlayerId;
        death.killerId = sourcePlayerId;
        death.reason = payload.reason;
        auto deathPkt = encodePlayerDeathNotify(death);
        for (uint32_t rid : recipients) {
          enqueueToPlayerUnlocked(rid, deathPkt, outbox);
        }
        Umbra::Core::Logger::getInstance().info("Player {} died (killer={})", targetPlayerId, sourcePlayerId);
      }
    }

    Umbra::Core::Logger::getInstance().debug(
        "VitalsUpdate target={} delta={} -> {} recipients",
        targetPlayerId, delta, recipients.size());
  }

  Umbra::Network::WebSocketServer ws_;
  mutable std::mutex mu_;
  std::unordered_map<uint32_t, PlayerStateNet> players_;
  std::unordered_map<uint32_t, uint32_t> clientIdToPlayerId_;
  /** Mapa reverso playerId -> clientId (1:1). Evita varredura O(clientes) no hot path
   *  (enqueueToPlayerUnlocked / collectVitalsRecipientsUnlocked). Mantido em sincronia
   *  com clientIdToPlayerId_ via setClientPlayerMapUnlocked/eraseClientMapUnlocked. */
  std::unordered_map<uint32_t, uint32_t> playerIdToClientId_;
  /** Rate-limit opcode 108 (NpcBuffSnapshotRequest) por client. */
  std::unordered_map<uint32_t, int64_t> lastNpcBuffSnapMsByClient_;
  std::unordered_map<uint32_t, std::deque<std::chrono::steady_clock::time_point>> chatMessageHistoryByPlayer_;
  std::function<uint32_t(uint32_t)> onPlayerDisconnect_;
  std::function<std::vector<uint32_t>(uint32_t)> resolvePartyMembers_;

  // Cache de membros de party (mutex próprio, fora do mu_). Evita 2 SELECTs sob
  // mu_ a cada broadcast de vitals/dano/DOT. Reabastecido por refreshPartyCache().
  struct PartyCacheEntry {
    std::vector<uint32_t> members;
    std::chrono::steady_clock::time_point expiresAt;
  };
  mutable std::mutex partyCacheMu_;
  std::unordered_map<uint32_t, PartyCacheEntry> partyCache_;
  std::chrono::steady_clock::time_point nextPartyRefresh_{};
  static constexpr int kPartyCacheTtlMs = 1000;
  std::function<bool(uint32_t, uint32_t, const std::string&, bool, PlayerRespawnPayload&)> respawnHandler_;
  std::unordered_map<uint32_t, int32_t> lastKnownHealth_;
  // Baseline do último broadcast de vitals por jogador: suprime opcode 87 redundante
  // (delta=0 sem mudança real) e aplica throttle a mudanças de regen/mana.
  struct VitalsBcastState {
    int32_t hp = INT32_MIN;
    int32_t mp = INT32_MIN;
    int64_t lastMs = 0;
  };
  std::unordered_map<uint32_t, VitalsBcastState> vitalsBcast_;
  static constexpr int64_t kVitalsThrottleMs = 66;  // ~15Hz para vitals sem dano
  // Cap de destinatários de vitals por evento (bound do fan-out em alta densidade).
  static constexpr size_t kMaxVitalsRecipients = 60;
  std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> respawnCooldownUntil_;
  uint32_t zoneId_ = 1;
  SpatialGrid aoiGrid_{10000.0f};
  /** Jogadores com loja pessoal aberta: bloqueia MoveUpdate. Par = (shop_id, nome UTF-8). */
  std::unordered_map<uint32_t, std::pair<uint32_t, std::string>> personalShopOpenByPlayerId_;
  uint64_t moveUpdateCount_ = 0;

  /** Posição autoritativa (respawn/teleporte) aguardando confirmação do cliente (sob mu_). */
  struct PositionGuard {
    float x = 0.f, y = 0.f, z = 0.f;
    int64_t untilMs = 0;
  };
  std::unordered_map<uint32_t, PositionGuard> positionGuards_;
  /** Raio (uu) para considerar que o cliente já aplicou a posição autoritativa. */
  static constexpr float kPositionGuardRadius = 500.0f;
  /** Janela máx. ignorando moves obsoletos (latência + cliente processar o 91/StateUpdate). */
  static constexpr int64_t kPositionGuardMs = 3000;

  /** Arma a trava de posição (chamar sob mu_). Não mexe em tsMs (relógio do cliente). */
  void armPositionGuardUnlocked(uint32_t playerId, float x, float y, float z) {
    positionGuards_[playerId] = PositionGuard{x, y, z, Umbra::Core::steadyNowMs() + kPositionGuardMs};
  }

  // ---- Limpeza de disconnect (fase 2) fora do loop/threads de rede ----
  struct PendingDisconnect {
    uint32_t cid = 0;
    DisconnectCleanup cleanup;
  };
  std::mutex cleanupMu_;
  std::condition_variable cleanupCv_;
  std::deque<PendingDisconnect> cleanupQueue_;
  bool cleanupStop_ = false;
  std::thread cleanupThread_;

  void startCleanupWorker() {
    if (cleanupThread_.joinable()) return;
    {
      std::lock_guard<std::mutex> lk(cleanupMu_);
      cleanupStop_ = false;
    }
    cleanupThread_ = std::thread([this] {
      while (true) {
        PendingDisconnect job;
        {
          std::unique_lock<std::mutex> lk(cleanupMu_);
          cleanupCv_.wait(lk, [this] { return cleanupStop_ || !cleanupQueue_.empty(); });
          if (cleanupQueue_.empty()) break;  // stop pedido e fila drenada
          job = std::move(cleanupQueue_.front());
          cleanupQueue_.pop_front();
        }
        finishClientDisconnect(job.cid, job.cleanup);
      }
    });
  }

  void stopCleanupWorker() {
    {
      std::lock_guard<std::mutex> lk(cleanupMu_);
      cleanupStop_ = true;
    }
    cleanupCv_.notify_all();
    if (cleanupThread_.joinable()) cleanupThread_.join();  // drena o que falta antes de sair
  }

  void enqueueDisconnectCleanup(uint32_t cid, const DisconnectCleanup& cleanup) {
    {
      std::lock_guard<std::mutex> lk(cleanupMu_);
      cleanupQueue_.push_back(PendingDisconnect{cid, cleanup});
    }
    cleanupCv_.notify_one();
  }

  float maxSpeed_ = 1200.0f;
  float maxTeleportDist_ = 3000.0f;
  uint32_t maxDelayMs_ = 300;
  size_t chatMaxMessageLength_ = 500;
  uint32_t chatRateLimitPerMinute_ = 30;
  CombatCoreEngine* combatCoreEngine_ = nullptr;
  LootService* lootService_ = nullptr;
  MovementSessionAuth sessionAuth_;
  bool sessionAuthEnabled_ = false;

  struct InboundMsg {
    uint32_t cid = 0;
    std::vector<uint8_t> data;
  };
  std::mutex inboundMu_;
  std::deque<InboundMsg> inboundQueue_;
  std::unordered_map<uint32_t, std::vector<uint8_t>> pendingMoveByCid_;
  /** cid -> playerId já validado (evita MySQL a cada PlayerInfoUpdate). */
  std::unordered_map<uint32_t, uint32_t> belongsOkByCid_;
};

} // namespace Zone
} // namespace Umbra


