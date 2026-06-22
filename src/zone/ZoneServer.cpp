#include "ZoneServer.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "core/Logger.hpp"
#include "core/ConfigManager.hpp"
#include "database/MySQLConnector.hpp"
#include "database/PlayerDAO.hpp"
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

  if (config_.dbConnector && config_.dbConnector->isConnected()) {
    combatService_ = std::make_unique<ZoneCombatService>(config_.dbConnector, config_.zoneId);
    movementServer_->setZoneId(config_.zoneId);
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
      [this](uint32_t playerId, uint32_t zoneId, const std::string& spawnKey,
             PlayerRespawnPayload& outPayload) -> bool {
        if (!combatService_) return false;
        auto result = combatService_->processRespawn(playerId, zoneId, spawnKey);
        if (!result.success) return false;
        outPayload = result.payload;
        return true;
      });

    combatCoreEngine_ = std::make_unique<CombatCoreEngine>();
    if (combatCoreEngine_->initialize(config_.zoneId, config_.dbConnector, movementServer_.get())) {
      combatCoreEngine_->setResolvePartyMembersCallback(
          [db = config_.dbConnector.get()](uint32_t playerId) {
            return resolvePartyMembers(db, playerId);
          });
      movementServer_->setCombatCoreEngine(combatCoreEngine_.get());
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
  if (movementServer_) movementServer_->stop();
  Core::Logger::getInstance().info("ZoneServer '{}' stopped", config_.zoneName);
}

bool ZoneServer::isRunning() const {
  return running_;
}

void ZoneServer::update(float deltaTime) {
  playerManager_->update(deltaTime);
  entitySystem_->update(deltaTime);

  snapshotAccumulator_ += deltaTime;
  if (snapshotAccumulator_ >= 0.1f) {
    if (movementServer_) movementServer_->broadcastSnapshot();
    snapshotAccumulator_ = 0.0f;
  }

  dotTickAccumulator_ += deltaTime;
  if (dotTickAccumulator_ >= 0.25f) {
    if (combatService_ && movementServer_) {
      combatService_->tickActiveDots(movementServer_.get());
    }
    dotTickAccumulator_ = 0.0f;
  }

  buffTickAccumulator_ += deltaTime;
  if (buffTickAccumulator_ >= 0.5f) {
    if (combatCoreEngine_) {
      combatCoreEngine_->tickBuffExpirations();
    }
    buffTickAccumulator_ = 0.0f;
  }

  if (combatCoreEngine_) {
    combatCoreEngine_->tick(deltaTime);
    combatCoreEngine_->tickRegen(deltaTime);
  }

  // Auto-save desabilitado: as posicoes sao salvas pelo PHP (update_position.php)
  // O auto-save C++ competia por locks na tabela players com o PHP, causando
  // "Lock wait timeout exceeded" em todas as APIs (select_character, party, trade, etc.)
  // TODO: reabilitar quando o update_position.php for removido do fluxo do UE5
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

