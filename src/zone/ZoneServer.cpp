#include "ZoneServer.hpp"
#include "core/Logger.hpp"
#include "database/MySQLConnector.hpp"

namespace Umbra {
namespace Zone {

namespace {
void removePlayerFromSessions(Umbra::Database::MySQLConnector* db, uint32_t playerId) {
  if (!db || !db->isConnected()) return;
  std::string pidStr = std::to_string(playerId);
  if (db->execute("DELETE FROM player_sessions WHERE player_id = " + pidStr)) {
    Core::Logger::getInstance().info("Player {} removed from player_sessions (disconnect)", playerId);
  }
}

uint32_t removePlayerFromParty(Umbra::Database::MySQLConnector* db, uint32_t playerId) {
  if (!db || !db->isConnected()) return 0;
  std::string pidStr = std::to_string(playerId);
  auto partyIdOpt = db->executeScalar("SELECT party_id FROM party_members WHERE player_id = " + pidStr);
  if (!partyIdOpt || partyIdOpt->empty()) return 0;
  std::string partyIdStr = *partyIdOpt;
  uint32_t partyId = static_cast<uint32_t>(std::stoul(partyIdStr));
  db->beginTransaction();
  bool ok = db->execute("DELETE FROM party_members WHERE party_id = " + partyIdStr + " AND player_id = " + pidStr);
  if (!ok) { db->rollback(); return 0; }
  auto countOpt = db->executeScalar("SELECT COUNT(*) FROM party_members WHERE party_id = " + partyIdStr);
  int count = countOpt ? std::stoi(*countOpt) : 0;
  if (count == 0) {
    db->execute("DELETE FROM parties WHERE party_id = " + partyIdStr);
  } else {
    auto leaderOpt = db->executeScalar("SELECT leader_id FROM parties WHERE party_id = " + partyIdStr);
    if (leaderOpt && *leaderOpt == pidStr) {
      auto newLeaderOpt = db->executeScalar("SELECT player_id FROM party_members WHERE party_id = " + partyIdStr + " ORDER BY joined_at ASC LIMIT 1");
      if (newLeaderOpt && !newLeaderOpt->empty()) {
        db->execute("UPDATE parties SET leader_id = " + *newLeaderOpt + " WHERE party_id = " + partyIdStr + " AND leader_id = " + pidStr);
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
  // Snapshot a ~10 Hz
  snapshotAccumulator_ += deltaTime;
  if (snapshotAccumulator_ >= 0.1f) {
    if (movementServer_) movementServer_->broadcastSnapshot();
    snapshotAccumulator_ = 0.0f;
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

