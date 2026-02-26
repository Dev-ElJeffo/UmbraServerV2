#include "PlayerDAO.hpp"
#include "core/Logger.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace Umbra {
namespace Database {

const std::string PlayerDAO::PLAYER_SELECT_FIELDS =
    "id, account_id, character_name, level, experience, "
    "pos_x, pos_y, pos_z, current_zone, "
    "health, max_health, mana, max_mana, stamina, max_stamina, "
    "strength, dexterity, intelligence, vitality, "
    "created_at, last_played_at";

PlayerDAO::PlayerDAO(std::shared_ptr<MySQLConnector> connector)
    : connector_(connector) {
}

static Player parsePlayerRow(const std::vector<std::string>& row) {
  Player player;
  player.id = std::stoull(row[0]);
  player.accountId = std::stoull(row[1]);
  player.characterName = row[2];
  player.level = static_cast<uint32_t>(std::stoul(row[3]));
  player.experience = std::stoull(row[4]);
  player.posX = std::stof(row[5]);
  player.posY = std::stof(row[6]);
  player.posZ = std::stof(row[7]);
  player.currentZone = row[8];
  player.health = static_cast<uint32_t>(std::stoul(row[9]));
  player.maxHealth = static_cast<uint32_t>(std::stoul(row[10]));
  player.mana = static_cast<uint32_t>(std::stoul(row[11]));
  player.maxMana = static_cast<uint32_t>(std::stoul(row[12]));
  player.stamina = static_cast<uint32_t>(std::stoul(row[13]));
  player.maxStamina = static_cast<uint32_t>(std::stoul(row[14]));
  player.strength = static_cast<uint32_t>(std::stoul(row[15]));
  player.dexterity = static_cast<uint32_t>(std::stoul(row[16]));
  player.intelligence = static_cast<uint32_t>(std::stoul(row[17]));
  player.vitality = static_cast<uint32_t>(std::stoul(row[18]));
  return player;
}

uint64_t PlayerDAO::createPlayer(const Player& player) {
  if (!player.isValid()) {
    Core::Logger::getInstance().error("Cannot create player: invalid data");
    return 0;
  }

  bool ok = connector_->executePreparedInsert(
    "INSERT INTO players (account_id, character_name, level, experience, "
    "pos_x, pos_y, pos_z, current_zone, "
    "health, max_health, mana, max_mana, stamina, max_stamina, "
    "strength, dexterity, intelligence, vitality, created_at) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW())",
    {std::to_string(player.accountId), player.characterName,
     std::to_string(player.level), std::to_string(player.experience),
     std::to_string(player.posX), std::to_string(player.posY), std::to_string(player.posZ),
     player.currentZone,
     std::to_string(player.health), std::to_string(player.maxHealth),
     std::to_string(player.mana), std::to_string(player.maxMana),
     std::to_string(player.stamina), std::to_string(player.maxStamina),
     std::to_string(player.strength), std::to_string(player.dexterity),
     std::to_string(player.intelligence), std::to_string(player.vitality)});

  if (ok) {
    uint64_t id = connector_->getLastInsertId();
    Core::Logger::getInstance().info("Created player: {} (ID: {})", player.characterName, id);
    return id;
  }

  return 0;
}

std::optional<Player> PlayerDAO::getPlayerById(uint64_t id) {
  auto results = connector_->executePreparedQuery(
    "SELECT " + PLAYER_SELECT_FIELDS + " FROM players WHERE id = ?",
    {std::to_string(id)});

  if (results.empty() || results[0].size() < 21) return std::nullopt;

  try {
    return parsePlayerRow(results[0]);
  } catch (const std::exception& e) {
    Core::Logger::getInstance().error("Failed to parse player data: {}", e.what());
    return std::nullopt;
  }
}

std::optional<Player> PlayerDAO::getPlayerByName(const std::string& characterName) {
  auto results = connector_->executePreparedQuery(
    "SELECT " + PLAYER_SELECT_FIELDS + " FROM players WHERE character_name = ?",
    {characterName});

  if (results.empty() || results[0].size() < 21) return std::nullopt;

  try {
    return parsePlayerRow(results[0]);
  } catch (const std::exception& e) {
    Core::Logger::getInstance().error("Failed to parse player data: {}", e.what());
    return std::nullopt;
  }
}

std::vector<Player> PlayerDAO::getPlayersByAccountId(uint64_t accountId) {
  std::vector<Player> players;

  auto results = connector_->executePreparedQuery(
    "SELECT " + PLAYER_SELECT_FIELDS + " FROM players WHERE account_id = ? ORDER BY last_played_at DESC, created_at DESC",
    {std::to_string(accountId)});

  for (const auto& row : results) {
    if (row.size() < 21) {
      Core::Logger::getInstance().warn("Invalid player row (expected 21 fields, got {})", row.size());
      continue;
    }

    try {
      players.push_back(parsePlayerRow(row));
    } catch (const std::exception& e) {
      Core::Logger::getInstance().error("Failed to parse player data: {}", e.what());
      continue;
    }
  }

  return players;
}

bool PlayerDAO::updatePlayer(const Player& player) {
  if (!player.isValid() || player.id == 0) return false;

  return connector_->executePreparedInsert(
    "UPDATE players SET level=?, experience=?, pos_x=?, pos_y=?, pos_z=?, "
    "current_zone=?, health=?, max_health=?, mana=?, max_mana=?, "
    "stamina=?, max_stamina=?, strength=?, dexterity=?, intelligence=?, vitality=? "
    "WHERE id=?",
    {std::to_string(player.level), std::to_string(player.experience),
     std::to_string(player.posX), std::to_string(player.posY), std::to_string(player.posZ),
     player.currentZone,
     std::to_string(player.health), std::to_string(player.maxHealth),
     std::to_string(player.mana), std::to_string(player.maxMana),
     std::to_string(player.stamina), std::to_string(player.maxStamina),
     std::to_string(player.strength), std::to_string(player.dexterity),
     std::to_string(player.intelligence), std::to_string(player.vitality),
     std::to_string(player.id)});
}

bool PlayerDAO::updatePosition(uint64_t id, float x, float y, float z,
                               const std::string& zone) {
  return connector_->executePreparedInsert(
    "UPDATE players SET pos_x=?, pos_y=?, pos_z=?, current_zone=? WHERE id=?",
    {std::to_string(x), std::to_string(y), std::to_string(z),
     zone, std::to_string(id)});
}

bool PlayerDAO::updateStats(uint64_t id, uint32_t health, uint32_t mana, uint32_t stamina) {
  return connector_->executePreparedInsert(
    "UPDATE players SET health=?, mana=?, stamina=? WHERE id=?",
    {std::to_string(health), std::to_string(mana), std::to_string(stamina),
     std::to_string(id)});
}

bool PlayerDAO::addExperience(uint64_t id, uint64_t expAmount) {
  return connector_->executePreparedInsert(
    "UPDATE players SET experience = experience + ? WHERE id=?",
    {std::to_string(expAmount), std::to_string(id)});
}

bool PlayerDAO::deletePlayer(uint64_t id) {
  bool ok = connector_->executePreparedInsert(
    "DELETE FROM players WHERE id=?",
    {std::to_string(id)});

  if (ok) {
    Core::Logger::getInstance().info("Deleted player {}", id);
  }
  return ok;
}

bool PlayerDAO::characterNameExists(const std::string& characterName) {
  auto result = connector_->executePreparedScalar(
    "SELECT COUNT(*) FROM players WHERE character_name = ?",
    {characterName});
  return result && *result != "0";
}

bool PlayerDAO::updateLastPlayed(uint64_t id) {
  return connector_->executePreparedInsert(
    "UPDATE players SET last_played_at = NOW() WHERE id=?",
    {std::to_string(id)});
}

std::optional<Player> PlayerDAO::parsePlayerFromQuery(const std::string& query) {
  auto results = connector_->executeQuery(query);

  if (results.empty() || results[0].size() < 21) return std::nullopt;

  try {
    return parsePlayerRow(results[0]);
  } catch (const std::exception& e) {
    Core::Logger::getInstance().error("Failed to parse player data: {}", e.what());
    return std::nullopt;
  }
}

}  // namespace Database
}  // namespace Umbra
