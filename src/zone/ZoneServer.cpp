#include "ZoneServer.hpp"
#include "core/Logger.hpp"
#include "database/MySQLConnector.hpp"
#include "database/PlayerDAO.hpp"

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
  db->beginTransaction();
  bool ok = db->executePreparedInsert(
    "DELETE FROM party_members WHERE party_id = ? AND player_id = ?", {partyIdStr, pid});
  if (!ok) { db->rollback(); return 0; }
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
  db->commit();
  Core::Logger::getInstance().info("Player {} removed from party {} (disconnect)", playerId, partyIdStr);
  return partyId;
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
  if (config_.dbConnector && config_.dbConnector->isConnected()) {
    movementServer_->setOnPlayerDisconnectCallback(
      [db = config_.dbConnector.get()](uint32_t playerId) {
        removePlayerFromSessions(db, playerId);  // lista de amigos: marcar offline
        return removePlayerFromParty(db, playerId);
      });
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

  autoSaveAccumulator_ += deltaTime;
  if (autoSaveAccumulator_ >= autoSaveInterval_) {
    autoSavePlayerPositions();
    autoSaveAccumulator_ = 0.0f;
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

}  // namespace Zone
}  // namespace Umbra

