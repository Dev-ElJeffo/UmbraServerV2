#include "PlayerDAO.hpp"
#include "core/Logger.hpp"
#include <sstream>
#include <iomanip>

namespace Umbra {
namespace Database {

PlayerDAO::PlayerDAO(std::shared_ptr<MySQLConnector> connector)
    : connector_(connector) {
}

uint64_t PlayerDAO::createPlayer(const Player& player) {
  if (!player.isValid()) {
    Core::Logger::getInstance().error("Cannot create player: invalid data");
    return 0;
  }
  
  std::ostringstream query;
  query << std::fixed << std::setprecision(2);
  query << "INSERT INTO players (account_id, character_name, level, experience, "
        << "pos_x, pos_y, pos_z, current_zone, "
        << "health, max_health, mana, max_mana, stamina, max_stamina, "
        << "strength, dexterity, intelligence, vitality, created_at) "
        << "VALUES ("
        << player.accountId << ", '"
        << connector_->escapeString(player.characterName) << "', "
        << player.level << ", "
        << player.experience << ", "
        << player.posX << ", "
        << player.posY << ", "
        << player.posZ << ", '"
        << connector_->escapeString(player.currentZone) << "', "
        << player.health << ", "
        << player.maxHealth << ", "
        << player.mana << ", "
        << player.maxMana << ", "
        << player.stamina << ", "
        << player.maxStamina << ", "
        << player.strength << ", "
        << player.dexterity << ", "
        << player.intelligence << ", "
        << player.vitality << ", "
        << "NOW())";
  
  if (connector_->execute(query.str())) {
    uint64_t id = connector_->getLastInsertId();
    Core::Logger::getInstance().info("Created player: {} (ID: {})", 
                                     player.characterName, id);
    return id;
  }
  
  return 0;
}

std::optional<Player> PlayerDAO::getPlayerById(uint64_t id) {
  std::ostringstream query;
  query << "SELECT id, account_id, character_name, level, experience, "
        << "pos_x, pos_y, pos_z, current_zone, "
        << "health, max_health, mana, max_mana, stamina, max_stamina, "
        << "strength, dexterity, intelligence, vitality, "
        << "created_at, last_played_at FROM players WHERE id = " << id;
  
  auto result = connector_->executeScalar(query.str());
  if (!result) {
    return std::nullopt;
  }
  
  // TODO: Parse actual result set
  Player player;
  player.id = id;
  
  return player;
}

std::optional<Player> PlayerDAO::getPlayerByName(const std::string& characterName) {
  std::ostringstream query;
  query << "SELECT id, account_id, character_name, level, experience, "
        << "pos_x, pos_y, pos_z, current_zone, "
        << "health, max_health, mana, max_mana, stamina, max_stamina, "
        << "strength, dexterity, intelligence, vitality, "
        << "created_at, last_played_at FROM players WHERE character_name = '"
        << connector_->escapeString(characterName) << "'";
  
  auto result = connector_->executeScalar(query.str());
  if (!result) {
    return std::nullopt;
  }
  
  // TODO: Parse actual result set
  Player player;
  player.characterName = characterName;
  
  return player;
}

std::vector<Player> PlayerDAO::getPlayersByAccountId(uint64_t accountId) {
  std::vector<Player> players;
  
  std::ostringstream query;
  query << "SELECT id, account_id, character_name, level, experience, "
        << "pos_x, pos_y, pos_z, current_zone, "
        << "health, max_health, mana, max_mana, stamina, max_stamina, "
        << "strength, dexterity, intelligence, vitality, "
        << "created_at, last_played_at FROM players WHERE account_id = " << accountId;
  
  // TODO: Execute query and parse results into vector
  
  return players;
}

bool PlayerDAO::updatePlayer(const Player& player) {
  if (!player.isValid() || player.id == 0) {
    return false;
  }
  
  std::ostringstream query;
  query << std::fixed << std::setprecision(2);
  query << "UPDATE players SET "
        << "level = " << player.level << ", "
        << "experience = " << player.experience << ", "
        << "pos_x = " << player.posX << ", "
        << "pos_y = " << player.posY << ", "
        << "pos_z = " << player.posZ << ", "
        << "current_zone = '" << connector_->escapeString(player.currentZone) << "', "
        << "health = " << player.health << ", "
        << "max_health = " << player.maxHealth << ", "
        << "mana = " << player.mana << ", "
        << "max_mana = " << player.maxMana << ", "
        << "stamina = " << player.stamina << ", "
        << "max_stamina = " << player.maxStamina << ", "
        << "strength = " << player.strength << ", "
        << "dexterity = " << player.dexterity << ", "
        << "intelligence = " << player.intelligence << ", "
        << "vitality = " << player.vitality << " "
        << "WHERE id = " << player.id;
  
  return connector_->execute(query.str());
}

bool PlayerDAO::updatePosition(uint64_t id, float x, float y, float z, 
                               const std::string& zone) {
  std::ostringstream query;
  query << std::fixed << std::setprecision(2);
  query << "UPDATE players SET "
        << "pos_x = " << x << ", "
        << "pos_y = " << y << ", "
        << "pos_z = " << z << ", "
        << "current_zone = '" << connector_->escapeString(zone) << "' "
        << "WHERE id = " << id;
  
  return connector_->execute(query.str());
}

bool PlayerDAO::updateStats(uint64_t id, uint32_t health, uint32_t mana, uint32_t stamina) {
  std::ostringstream query;
  query << "UPDATE players SET "
        << "health = " << health << ", "
        << "mana = " << mana << ", "
        << "stamina = " << stamina << " "
        << "WHERE id = " << id;
  
  return connector_->execute(query.str());
}

bool PlayerDAO::addExperience(uint64_t id, uint64_t expAmount) {
  std::ostringstream query;
  query << "UPDATE players SET "
        << "experience = experience + " << expAmount << " "
        << "WHERE id = " << id;
  
  return connector_->execute(query.str());
}

bool PlayerDAO::deletePlayer(uint64_t id) {
  std::ostringstream query;
  query << "DELETE FROM players WHERE id = " << id;
  
  bool result = connector_->execute(query.str());
  if (result) {
    Core::Logger::getInstance().info("Deleted player {}", id);
  }
  
  return result;
}

bool PlayerDAO::characterNameExists(const std::string& characterName) {
  std::ostringstream query;
  query << "SELECT COUNT(*) FROM players WHERE character_name = '"
        << connector_->escapeString(characterName) << "'";
  
  auto result = connector_->executeScalar(query.str());
  return result && *result != "0";
}

bool PlayerDAO::updateLastPlayed(uint64_t id) {
  std::ostringstream query;
  query << "UPDATE players SET last_played_at = NOW() WHERE id = " << id;
  
  return connector_->execute(query.str());
}

Player PlayerDAO::resultToPlayer(const std::string& result) {
  // TODO: Implement proper result parsing
  Player player;
  return player;
}

}  // namespace Database
}  // namespace Umbra

