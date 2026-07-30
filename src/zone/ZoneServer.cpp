#include "ZoneServer.hpp"
#include "auth/JWTManager.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "zone/ExperienceService.hpp"
#include "zone/ExpZoneManager.hpp"
#include "zone/AgentDebugLog.hpp"
#include "core/Logger.hpp"
#include "core/ConfigManager.hpp"
#include "database/MySQLConnector.hpp"
#include "database/PlayerDAO.hpp"
#include <algorithm>
#include <vector>

namespace Umbra {
namespace Zone {

namespace {
void removePlayerFromSessions(Umbra::Database::MySQLConnector* db, uint32_t playerId) {
  if (!db || !db->isConnected()) return;
  std::string pid = std::to_string(playerId);
  if (db->executePreparedInsert("DELETE FROM player_sessions WHERE player_id = ?", {pid})) {
    Core::Logger::getInstance().info("Player {} removed from player_sessions (disconnect)", playerId);
  }
}

uint32_t removePlayerFromParty(Umbra::Database::MySQLConnector* db, uint32_t playerId) {
  if (!db || !db->isConnected()) return 0;
  std::string pid = std::to_string(playerId);
  auto partyIdOpt = db->executePreparedScalar(
    "SELECT party_id FROM party_members WHERE player_id = ?", {pid});
  if (!partyIdOpt || partyIdOpt->empty()) return 0;
  std::string partyIdStr = *partyIdOpt;
  uint32_t partyId = static_cast<uint32_t>(std::stoul(partyIdStr));
  try {
    db->executePreparedInsert(
      "DELETE FROM party_members WHERE party_id = ? AND player_id = ?", {partyIdStr, pid});
    auto countOpt = db->executePreparedScalar(
      "SELECT COUNT(*) FROM party_members WHERE party_id = ?", {partyIdStr});
    int count = countOpt ? std::stoi(*countOpt) : 0;
    if (count == 0) {
      db->executePreparedInsert("DELETE FROM parties WHERE party_id = ?", {partyIdStr});
    } else {
      auto leaderOpt = db->executePreparedScalar(
        "SELECT leader_id FROM parties WHERE party_id = ?", {partyIdStr});
      if (leaderOpt && *leaderOpt == pid) {
        auto newLeaderOpt = db->executePreparedScalar(
          "SELECT player_id FROM party_members WHERE party_id = ? ORDER BY joined_at ASC LIMIT 1", {partyIdStr});
        if (newLeaderOpt && !newLeaderOpt->empty()) {
          db->executePreparedInsert(
            "UPDATE parties SET leader_id = ? WHERE party_id = ? AND leader_id = ?",
            {*newLeaderOpt, partyIdStr, pid});
        }
      }
    }
    Core::Logger::getInstance().info("Player {} removed from party {} (disconnect)", playerId, partyIdStr);
  } catch (...) {
    Core::Logger::getInstance().error("Error removing player {} from party", playerId);
  }
  return partyId;
}

std::vector<uint32_t> resolvePartyMembers(Umbra::Database::MySQLConnector* db, uint32_t playerId) {
  std::vector<uint32_t> members;
  if (!db || !db->isConnected() || playerId == 0) {
    return members;
  }
  const std::string pid = std::to_string(playerId);
  auto partyIdOpt = db->executePreparedScalar(
    "SELECT party_id FROM party_members WHERE player_id = ? LIMIT 1", {pid});
  if (!partyIdOpt || partyIdOpt->empty()) {
    return members;
  }

  auto rows = db->executePreparedQuery(
    "SELECT player_id FROM party_members WHERE party_id = ?",
    {*partyIdOpt});
  for (const auto& row : rows) {
    if (!row.empty()) {
      try {
        members.push_back(static_cast<uint32_t>(std::stoul(row[0])));
      } catch (...) {
      }
    }
  }
  return members;
}
}  // namespace

ZoneServer::ZoneServer(const Config& config)
    : config_(config), running_(false) {
  playerManager_ = std::make_unique<PlayerManager>(config.maxPlayers);
  entitySystem_ = std::make_unique<EntitySystem>();
  movementServer_ = std::make_unique<MovementServer>(config.port);
}

ZoneServer::~ZoneServer() {
  stop();
}

bool ZoneServer::start() {
  running_ = true;
  auto& configManager = Umbra::Core::ConfigManager::getInstance();
  const size_t maxMessageLength = static_cast<size_t>(configManager.get<uint32_t>("chat.max_message_length", 500));
  const uint32_t rateLimitPerMinute = configManager.get<uint32_t>("chat.rate_limit_per_minute", 30);
  movementServer_->setChatLimits(maxMessageLength, rateLimitPerMinute);
  Core::Logger::getInstance().info("Zone chat limits: max_message_length={}, rate_limit_per_minute={}",
                                   maxMessageLength, rateLimitPerMinute);

  {
    std::string jwtSecret = configManager.get<std::string>("auth.jwt_secret", "");
    if (jwtSecret.empty() || jwtSecret.find("CHANGE_ME") != std::string::npos) {
      jwtSecret = "umbra_eternum_secret_key_2024_very_secure";
      Core::Logger::getInstance().warn(
          "auth.jwt_secret nao configurado; usando chave padrao de desenvolvimento (alinhar com PHP)");
    }
    jwtManager_ = std::make_unique<Umbra::Auth::JWTManager>(jwtSecret);
  }

  if (config_.dbConnector && config_.dbConnector->isConnected()) {
    combatService_ = std::make_unique<ZoneCombatService>(config_.dbConnector, config_.zoneId);
    movementServer_->setZoneId(config_.zoneId);
    movementServer_->setSessionAuth(jwtManager_.get(), config_.dbConnector);
    movementServer_->setOnPlayerDisconnectCallback(
      [db = config_.dbConnector.get()](uint32_t playerId) {
        removePlayerFromSessions(db, playerId);  // lista de amigos: marcar offline
        return removePlayerFromParty(db, playerId);
      });
    movementServer_->setResolvePartyMembersCallback(
      [db = config_.dbConnector.get()](uint32_t playerId) {
        return resolvePartyMembers(db, playerId);
      });
    movementServer_->setRespawnHandler(
      [this](uint32_t playerId, uint32_t zoneId, const std::string& spawnKey, bool memorySaysDead,
             PlayerRespawnPayload& outPayload) -> bool {
        if (!combatService_) return false;
        int32_t preferMaxH = 0;
        int32_t preferMaxM = 0;
        if (combatCoreEngine_) {
          if (auto* loader = combatCoreEngine_->getCharacterStateLoader()) {
            Combat::CharacterState st;
            if (loader->tryGetCachedState(playerId, st) || loader->loadPlayerState(playerId, st)) {
              preferMaxH = std::max(1, st.buffedStats.maxHealth);
              preferMaxM = std::max(1, st.buffedStats.maxMana);
            }
          }
        }
        auto result = combatService_->processRespawn(playerId, zoneId, spawnKey, memorySaysDead,
                                                     preferMaxH, preferMaxM);
        if (!result.success) {
          Core::Logger::getInstance().warn("processRespawn player {} error={}", playerId,
                                           result.error.empty() ? "?" : result.error);
          return false;
        }
        outPayload = result.payload;
        // Cache: HP cheio + isAlive=true (patchCachedHealth atualiza isAlive).
        if (combatCoreEngine_) {
          if (auto* loader = combatCoreEngine_->getCharacterStateLoader()) {
            loader->patchCachedHealth(playerId, result.payload.currentHealth);
            loader->patchCachedMana(playerId, result.payload.currentMana);
          }
        }
        return true;
      });

    combatCoreEngine_ = std::make_unique<CombatCoreEngine>();
    if (combatCoreEngine_->initialize(config_.zoneId, config_.dbConnector, movementServer_.get())) {
      combatCoreEngine_->setResolvePartyMembersCallback(
          [db = config_.dbConnector.get()](uint32_t playerId) {
            return resolvePartyMembers(db, playerId);
          });
      movementServer_->setCombatCoreEngine(combatCoreEngine_.get());

      const float partyShareRadius = static_cast<float>(
          configManager.get<int>("party.share_radius_uu", 5000));
      auto partyCb = [db = config_.dbConnector.get()](uint32_t playerId) {
        return resolvePartyMembers(db, playerId);
      };

      experienceService_ = std::make_unique<ExperienceService>(config_.dbConnector);
      experienceService_->setStateLoader(combatCoreEngine_->getCharacterStateLoader());
      expZoneManager_ = std::make_unique<ExpZoneManager>(
          config_.zoneId, config_.dbConnector, experienceService_.get(), movementServer_.get());
      lootService_ = std::make_unique<LootService>(
          config_.zoneId, config_.dbConnector, movementServer_.get(), experienceService_.get());
      lootService_->setShareRadiusUu(partyShareRadius);
      lootService_->setResolvePartyMembers(partyCb);
      lootService_->loadFromDatabase();
      combatCoreEngine_->setLootService(lootService_.get());
      if (auto* questProgress = combatCoreEngine_->getQuestProgressService()) {
        questProgress->setMovementServer(movementServer_.get());
        questProgress->setResolvePartyMembers(partyCb);
        questProgress->setShareRadiusUu(partyShareRadius);
      }
      movementServer_->setLootService(lootService_.get());
      Core::Logger::getInstance().info(
          "ExperienceService, ExpZoneManager and LootService initialized for zone {} "
          "(party.share_radius_uu={})",
          config_.zoneId, partyShareRadius);
    } else {
      combatCoreEngine_.reset();
      Core::Logger::getInstance().warn("CombatCoreEngine failed to initialize");
    }
  }
  Core::Logger::getInstance().info("ZoneServer '{}' (ID: {}) started on port {}", 
                                   config_.zoneName, config_.zoneId, config_.port);
  if (!movementServer_->start()) {
    Core::Logger::getInstance().error("MovementServer failed to start on port {}", config_.port);
    return false;
  }
  return true;
}

void ZoneServer::stop() {
  running_ = false;
  if (combatCoreEngine_) {
    combatCoreEngine_->shutdown();
  }
  if (movementServer_) movementServer_->stop();
  Core::Logger::getInstance().info("ZoneServer '{}' stopped", config_.zoneName);
}

bool ZoneServer::isRunning() const {
  return running_;
}

void ZoneServer::update(float deltaTime) {
  // #region agent log
  const int64_t updT0 = agentNowMs();
  int64_t tDrain = 0, tSnap = 0, tKa = 0, tCombat = 0, tRegen = 0, tExp = 0;
  // #endregion

  playerManager_->update(deltaTime);
  entitySystem_->update(deltaTime);

  if (movementServer_) {
    // #region agent log
    const int64_t a = agentNowMs();
    // #endregion
    // Drain inbound ANTES de auth/snapshot: libera fila sem bloquear threads de recv.
    movementServer_->drainInboundQueue();
    movementServer_->tickSessionAuth();
    // Reabastece o cache de party (throttle interno ~1s) fora do mu_, para o
    // broadcast de vitals não fazer SELECT segurando o lock global da zona.
    movementServer_->refreshPartyCache();
    // #region agent log
    tDrain = agentNowMs() - a;
    // #endregion
  }

  snapshotAccumulator_ += deltaTime;
  if (snapshotAccumulator_ >= 0.1f) {
    // #region agent log
    const int64_t a = agentNowMs();
    // #endregion
    if (movementServer_) movementServer_->broadcastSnapshot();
    // #region agent log
    tSnap = agentNowMs() - a;
    // #endregion
    snapshotAccumulator_ = 0.0f;
  }

  // Keepalive app-level (opcode 250). LWS também tem PingPongInterval no cliente.
  // Intervalo curto: dual-PIE pode deixar de bombear OnRawMessage por alguns segundos.
  wsKeepaliveAccumulator_ += deltaTime;
  if (wsKeepaliveAccumulator_ >= 2.0f) {
    // #region agent log
    const int64_t a = agentNowMs();
    // #endregion
    if (movementServer_) movementServer_->sendKeepalivePings();
    // #region agent log
    tKa = agentNowMs() - a;
    // #endregion
    wsKeepaliveAccumulator_ = 0.0f;
  }

  dotTickAccumulator_ += deltaTime;
  if (dotTickAccumulator_ >= 0.25f) {
    // V1 ZoneCombatService::tickActiveDots fazia SELECT/UPDATE active_dots no tick da zone
    // (medido 122–265ms no Proxmox). DOTs de player agora são in-memory em CombatCoreEngine.
    // Mantém respawn/outros usos do combatService_; só o poll de dots sai do hot path.
    dotTickAccumulator_ = 0.0f;
  }

  // Expiração de buffs (active_buffs SELECT/DELETE) roda no dbWriter — NÃO no tick da zone.
  // Medido até 4587ms no update → freeze + skills em cascata ao retomar.

  if (combatCoreEngine_) {
    // #region agent log
    const int64_t a = agentNowMs();
    // #endregion
    combatCoreEngine_->tick(deltaTime);
    // #region agent log
    tCombat = agentNowMs() - a;
    const int64_t b = agentNowMs();
    // #endregion
    combatCoreEngine_->tickRegen(deltaTime);
    // #region agent log
    tRegen = agentNowMs() - b;
    // #endregion
  }

  expZoneTickAccumulator_ += deltaTime;
  if (expZoneTickAccumulator_ >= 1.0f) {
    // #region agent log
    const int64_t a = agentNowMs();
    // #endregion
    if (expZoneManager_) {
      expZoneManager_->tick(expZoneTickAccumulator_);
    }
    if (lootService_) {
      lootService_->tick(expZoneTickAccumulator_);
    }
    // #region agent log
    tExp = agentNowMs() - a;
    // #endregion
    expZoneTickAccumulator_ = 0.0f;
  }

  // #region agent log
  {
    const int64_t total = agentNowMs() - updT0;
    if (total >= 50) {
      agentDebugLog("H-D", "ZoneServer.cpp:update", "zone_update_ms",
                    std::string("{\"ms\":") + std::to_string(total) +
                        ",\"drain\":" + std::to_string(tDrain) +
                        ",\"snap\":" + std::to_string(tSnap) +
                        ",\"ka\":" + std::to_string(tKa) +
                        ",\"combat\":" + std::to_string(tCombat) +
                        ",\"regen\":" + std::to_string(tRegen) +
                        ",\"exp\":" + std::to_string(tExp) + "}",
                    "post-fix");
    }
  }
  // #endregion

  // Auto-save desabilitado: as posicoes sao salvas pelo PHP (update_position.php)
  // O auto-save C++ competia por locks na tabela players com o PHP, causando
  // "Lock wait timeout exceeded" em todas as APIs (select_character, party, trade, etc.)
  // TODO: reabilitar quando o update_position.php for removido do fluxo do UE5
}

void ZoneServer::pumpInbound() {
  // Chamado ~3ms pelo loop principal. Só trabalho barato e sensível a latência:
  // - drainInboundQueue: tira o cast (opcode 96) do inbound e joga na fila de
  //   combate quase na hora, em vez de esperar até 16ms pelo update pesado.
  // - tickPendingSkillHits: dispara o dano dos casts com castTimeMs no momento
  //   certo (idempotente: só processa hits já vencidos, então chamar junto ao
  //   update não duplica dano).
  if (movementServer_) {
    movementServer_->drainInboundQueue();
  }
  if (combatCoreEngine_) {
    combatCoreEngine_->tickPendingSkillHits();
  }
}

void ZoneServer::autoSavePlayerPositions() {
  if (!config_.dbConnector || !config_.dbConnector->isConnected()) return;
  if (!movementServer_) return;

  auto players = movementServer_->getPlayerStates();
  if (players.empty()) return;

  uint32_t saved = 0;
  for (const auto& [pid, state] : players) {
    if (state.x == 0.0f && state.y == 0.0f && state.z == 0.0f) continue;
    if (config_.dbConnector->executePreparedInsert(
          "UPDATE players SET pos_x=?, pos_y=?, pos_z=?, current_zone=? WHERE id=?",
          {std::to_string(state.x), std::to_string(state.y), std::to_string(state.z),
           config_.zoneName, std::to_string(pid)})) {
      ++saved;
    }
  }

  if (saved > 0) {
    Core::Logger::getInstance().info("[AutoSave] Saved {}/{} positions in '{}'",
                                     saved, players.size(), config_.zoneName);
  }
}

PlayerManager& ZoneServer::getPlayerManager() {
  return *playerManager_;
}

EntitySystem& ZoneServer::getEntitySystem() {
  return *entitySystem_;
}

MovementServer* ZoneServer::getMovementServer() {
  return movementServer_.get();
}

CombatCoreEngine* ZoneServer::getCombatCoreEngine() {
  return combatCoreEngine_.get();
}

const ZoneServer::Config& ZoneServer::getConfig() const {
  return config_;
}

void ZoneServer::forceSavePositions() {
  autoSavePlayerPositions();
}

}  // namespace Zone
}  // namespace Umbra

