#include "ZoneServer.hpp"
#include "auth/JWTManager.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "zone/ExperienceService.hpp"
#include "zone/ExpZoneManager.hpp"
#include "core/Timer.hpp"
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
  // Login duplicado: o PHP (report_online) do client NOVO grava a sessão no mesmo instante
  // em que o kick do antigo chega aqui. Sem a condição de last_seen, o DELETE (que ficava
  // esperando o lock da linha) apagava a sessão nova → personagem online aparecia offline.
  if (db->executePreparedInsert(
          "DELETE FROM player_sessions WHERE player_id = ? AND last_seen < NOW() - INTERVAL 2 SECOND",
          {pid})) {
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
    // Grupo com 1 membro (ou nenhum) deixa de existir, igual ao leave_party.php.
    // O membro restante recebe o PartyMemberLeft, recarrega e o painel some.
    if (count <= 1) {
      db->executePreparedInsert("DELETE FROM party_members WHERE party_id = ?", {partyIdStr});
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

// Chat de guilda (fase 7, K5): todos os membros da guilda do jogador (incluindo ele). Vazio = sem guilda.
std::vector<uint32_t> resolveGuildMembers(Umbra::Database::MySQLConnector* db, uint32_t playerId) {
  std::vector<uint32_t> members;
  if (!db || !db->isConnected() || playerId == 0) {
    return members;
  }
  auto rows = db->executePreparedQuery(
    "SELECT gm2.player_id FROM guild_members gm1 "
    "JOIN guild_members gm2 ON gm2.guild_id = gm1.guild_id "
    "WHERE gm1.player_id = ?",
    {std::to_string(playerId)});
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
    movementServer_->setResolveGuildMembersCallback(
      [db = config_.dbConnector.get()](uint32_t playerId) {
        return resolveGuildMembers(db, playerId);
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
      experienceService_->loadGameRates();
      lootService_->loadGameRates();
      combatCoreEngine_->setLootService(lootService_.get());
      if (auto* questProgress = combatCoreEngine_->getQuestProgressService()) {
        questProgress->setMovementServer(movementServer_.get());
        questProgress->setResolvePartyMembers(partyCb);
        questProgress->setShareRadiusUu(partyShareRadius);
        // Missões de guilda (3b): mesmos eventos, na thread do QuestProgressService.
        guildMissionService_ = std::make_unique<GuildMissionService>(config_.dbConnector, movementServer_.get());
        questProgress->setGuildMissionService(guildMissionService_.get());
        // Quests v2: ouro, nível e opcodes 131/132.
        lootService_->setQuestProgressService(questProgress);
        experienceService_->setLevelUpCallback([questProgress](uint32_t playerId, uint32_t newLevel) {
          QuestEvent ev;
          ev.kind = QuestObjectiveKind::ReachLevel;
          ev.level = newLevel;
          questProgress->onEvent(playerId, ev);
        });
        movementServer_->setQuestStateChangedHandler([questProgress](uint32_t playerId, uint32_t) {
          questProgress->onQuestStateChanged(playerId);
        });
        movementServer_->setQuestInteractHandler(
            [questProgress](uint32_t playerId, uint32_t questId, uint32_t itemTemplateId) {
              questProgress->onQuestInteract(playerId, questId, itemTemplateId);
            });
      }
      movementServer_->setLootService(lootService_.get());

      // Companheiros (opcodes 124-126): pet de coleta usa o LootService.
      companionService_ = std::make_unique<CompanionService>(config_.dbConnector, movementServer_.get());
      companionService_->loadTemplates();
      lootService_->setCompanionService(companionService_.get());
      movementServer_->setCompanionRequestHandler(
          [svc = companionService_.get()](uint32_t playerId, const CompanionRequestPayload& p) {
            svc->handleRequest(playerId, p);
          });
      // Consumíveis pela zone (127/128) + elemental (129/130).
      consumableService_ = std::make_unique<ConsumableService>(
          config_.dbConnector, movementServer_.get(), combatCoreEngine_.get(), companionService_.get());
      consumableService_->loadTemplates();
      companionService_->setElementalHooks(
          [cs = consumableService_.get()](uint32_t playerId, const CompanionTemplateDef& tpl,
                                          const std::string& rulesJson) {
            cs->onElementalActivated(playerId, tpl, rulesJson);
          },
          [cs = consumableService_.get()](uint32_t playerId, uint32_t companionId) {
            cs->onElementalDeactivated(playerId, companionId);
          });
      movementServer_->setConsumableUseHandler(
          [cs = consumableService_.get()](uint32_t playerId, const ConsumableUseRequestPayload& p) {
            cs->enqueueUse(playerId, p.inventoryId, p.itemTemplateId);
          });
      movementServer_->setElementalRulesHandler(
          [cs = consumableService_.get()](uint32_t playerId, const ElementalRulesSetPayload& p) {
            cs->enqueueSetRules(playerId, p);
          });
      consumableService_->start();

      // Instâncias (etapa 4, opcodes 134-136).
      instanceManager_ = std::make_unique<InstanceManager>(config_.dbConnector, config_.zoneId,
                                                           movementServer_.get(), combatCoreEngine_.get());
      instanceManager_->setResolvePartyMembers(partyCb);
      instanceManager_->setLobbyTimeoutSeconds(
          static_cast<uint32_t>(configManager.get<int>("instance.lobby_timeout_seconds", 120)));
      // Dungeons (etapa 5, opcodes 138-139): recompensas e lockout numa thread própria.
      dungeonService_ = std::make_unique<DungeonService>(config_.dbConnector, movementServer_.get(),
                                                         lootService_.get(),
                                                         combatCoreEngine_->getQuestProgressService());
      dungeonService_->start();
      instanceManager_->setDungeonService(dungeonService_.get());
      // Chefes com fases (etapa 5b, opcode 140).
      bossController_ = std::make_unique<BossController>(config_.dbConnector,
                                                         combatCoreEngine_->getNpcManagerMutable(),
                                                         movementServer_.get(), combatCoreEngine_.get());
      bossController_->reload();
      combatCoreEngine_->setBossController(bossController_.get());
      // Duelo e regras de PvP (etapa 6, opcodes 50-54).
      duelService_ = std::make_unique<DuelService>(config_.dbConnector, config_.zoneId, movementServer_.get(),
                                                   combatCoreEngine_.get());
      duelService_->reload();
      combatCoreEngine_->setDuelService(duelService_.get());
      movementServer_->setDuelHandlers(
          [ds = duelService_.get()](uint32_t from, uint32_t target) { ds->handleRequest(from, target); },
          [ds = duelService_.get()](uint32_t pid, uint8_t action, uint32_t other) {
            ds->handleResponse(pid, action, other);
          });
      combatCoreEngine_->setInstanceNpcKilledHook(
          [im = instanceManager_.get()](uint32_t instanceId, uint32_t npcId, uint8_t role, uint32_t killer) {
            im->onInstanceNpcKilled(instanceId, npcId, role, killer);
          });
      instanceManager_->reload();
      movementServer_->setInstanceEnterHandler(
          [im = instanceManager_.get()](uint32_t playerId, uint32_t portalNpcId, uint8_t action, uint8_t value) {
            im->handleLobbyAction(playerId, portalNpcId, action, value);
          });
      movementServer_->setInstanceLeaveHandler(
          [im = instanceManager_.get()](uint32_t playerId) { im->handleLeave(playerId); });

      movementServer_->setPlayerGoneHook(
          [svc = companionService_.get(), cs = consumableService_.get(),
           qs = combatCoreEngine_->getQuestProgressService(), im = instanceManager_.get(),
           ds = duelService_.get()](uint32_t playerId) {
            svc->onPlayerGone(playerId);
            cs->onPlayerGone(playerId);
            if (qs) qs->onPlayerGone(playerId);
            if (im) im->onPlayerGone(playerId);
            if (ds) ds->onPlayerGone(playerId);
          });
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
  // Antes do combate: o worker de consumíveis enfileira efeitos no combat worker.
  if (consumableService_) consumableService_->stop();
  if (dungeonService_) dungeonService_->stop();
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
  // Telemetria leve por fase (steady_clock, sem I/O). Só loga update lento (>= 50ms).
  const int64_t updCpu0 = Core::threadCpuMs();
  const int64_t updT0 = Core::steadyNowMs();
  int64_t tDrain = 0, tSnap = 0, tKa = 0, tCombat = 0, tRegen = 0, tExp = 0;

  playerManager_->update(deltaTime);
  entitySystem_->update(deltaTime);

  if (movementServer_) {
    const int64_t a = Core::steadyNowMs();
    movementServer_->drainInboundQueue();
    movementServer_->tickSessionAuth();
    movementServer_->refreshPartyCache();
    tDrain = Core::steadyNowMs() - a;
  }

  snapshotAccumulator_ += deltaTime;
  if (snapshotAccumulator_ >= 0.1f) {
    const int64_t a = Core::steadyNowMs();
    if (movementServer_) movementServer_->broadcastSnapshot();
    tSnap = Core::steadyNowMs() - a;
    snapshotAccumulator_ = 0.0f;
  }

  wsKeepaliveAccumulator_ += deltaTime;
  if (wsKeepaliveAccumulator_ >= 2.0f) {
    const int64_t a = Core::steadyNowMs();
    if (movementServer_) movementServer_->sendKeepalivePings();
    tKa = Core::steadyNowMs() - a;
    wsKeepaliveAccumulator_ = 0.0f;
  }

  dotTickAccumulator_ += deltaTime;
  if (dotTickAccumulator_ >= 0.25f) {
    dotTickAccumulator_ = 0.0f;
  }

  if (combatCoreEngine_) {
    const int64_t a = Core::steadyNowMs();
    combatCoreEngine_->tick(deltaTime);
    tCombat = Core::steadyNowMs() - a;
    const int64_t b = Core::steadyNowMs();
    combatCoreEngine_->tickRegen(deltaTime);
    tRegen = Core::steadyNowMs() - b;
  }

  duelTickAccumulator_ += deltaTime;
  if (duelTickAccumulator_ >= 0.25f) {
    duelTickAccumulator_ = 0.0f;
    if (duelService_) duelService_->tick();  // contagem, área, tempo, desconexão (etapa 6)
  }

  expZoneTickAccumulator_ += deltaTime;
  if (expZoneTickAccumulator_ >= 1.0f) {
    const int64_t a = Core::steadyNowMs();
    if (expZoneManager_) {
      expZoneManager_->tick(expZoneTickAccumulator_);
    }
    if (lootService_) {
      lootService_->tick(expZoneTickAccumulator_);
    }
    if (instanceManager_) {
      instanceManager_->tick();
    }
    if (bossController_) {
      bossController_->tick();  // enrage, entrada em combate e reset (no combat worker)
    }
    tExp = Core::steadyNowMs() - a;
    expZoneTickAccumulator_ = 0.0f;
  }

  const int64_t total = Core::steadyNowMs() - updT0;
  if (total >= 50) {
    static Core::LogThrottle slowUpdateThrottle(2000);
    if (slowUpdateThrottle.allow()) {
      Core::Logger::getInstance().warn(
          "[Zone] update lento {}ms cpu={}ms (drain={} snap={} keepalive={} combat={} regen={} exp={}) "
          "+{} omitidos",
          total, Core::threadCpuMs() - updCpu0, tDrain, tSnap, tKa, tCombat, tRegen, tExp,
          slowUpdateThrottle.takeSuppressed());
    }
  }
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
    // Etapa 4: dentro de instância grava o ponto de saída (relog volta para a frente do portal).
    const bool inInstance = state.instanceId != 0;
    const float sx = inInstance ? state.instExitX : state.x;
    const float sy = inInstance ? state.instExitY : state.y;
    const float sz = inInstance ? state.instExitZ : state.z;
    if (config_.dbConnector->executePreparedInsert(
          "UPDATE players SET pos_x=?, pos_y=?, pos_z=?, current_zone=? WHERE id=?",
          {std::to_string(sx), std::to_string(sy), std::to_string(sz),
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

void ZoneServer::reloadGameRates() {
  if (experienceService_) experienceService_->loadGameRates();
  if (lootService_) lootService_->loadGameRates();
}

}  // namespace Zone
}  // namespace Umbra

