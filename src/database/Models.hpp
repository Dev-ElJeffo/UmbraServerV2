#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <chrono>

namespace Umbra {
namespace Database {

/**
 * @brief Modelo de conta de usuário
 */
struct Account {
  uint64_t id = 0;
  std::string username;
  std::string email;
  std::string passwordHash;
  std::string salt;
  bool banned = false;
  std::string banReason;
  std::chrono::system_clock::time_point createdAt;
  std::chrono::system_clock::time_point lastLoginAt;
  
  bool isValid() const {
    return !username.empty() && !email.empty() && !passwordHash.empty();
  }
};

/**
 * @brief Modelo de personagem do jogador
 */
struct Player {
  uint64_t id = 0;
  uint64_t accountId = 0;
  std::string characterName;
  uint32_t level = 1;
  uint64_t experience = 0;
  
  // Position
  float posX = 0.0f;
  float posY = 0.0f;
  float posZ = 0.0f;
  std::string currentZone;
  
  // Stats
  uint32_t health = 100;
  uint32_t maxHealth = 100;
  uint32_t mana = 100;
  uint32_t maxMana = 100;
  uint32_t stamina = 100;
  uint32_t maxStamina = 100;
  
  // Attributes
  uint32_t strength = 10;
  uint32_t dexterity = 10;
  uint32_t intelligence = 10;
  uint32_t vitality = 10;
  
  std::chrono::system_clock::time_point createdAt;
  std::chrono::system_clock::time_point lastPlayedAt;
  
  bool isValid() const {
    return !characterName.empty() && accountId > 0;
  }
};

/**
 * @brief Modelo de item
 */
struct Item {
  uint64_t id = 0;
  std::string templateId;
  std::string name;
  std::string description;
  uint32_t stackSize = 1;
  uint32_t maxStackSize = 1;
  bool tradeable = true;
  
  enum class Rarity {
    COMMON = 0,
    UNCOMMON = 1,
    RARE = 2,
    EPIC = 3,
    LEGENDARY = 4
  };
  
  Rarity rarity = Rarity::COMMON;
  
  bool isValid() const {
    return !templateId.empty() && !name.empty();
  }
};

/**
 * @brief Modelo de inventário do jogador
 */
struct InventorySlot {
  uint64_t id = 0;
  uint64_t playerId = 0;
  uint32_t slotIndex = 0;
  Item item;
  
  bool isEmpty() const {
    return item.id == 0;
  }
};

/**
 * @brief Modelo de sessão de jogador
 */
struct Session {
  std::string token;
  uint64_t accountId = 0;
  uint64_t playerId = 0;
  std::string ipAddress;
  std::chrono::system_clock::time_point createdAt;
  std::chrono::system_clock::time_point expiresAt;
  bool valid = true;
  
  bool isExpired() const {
    return std::chrono::system_clock::now() >= expiresAt;
  }
};

/**
 * @brief Modelo de guilda
 */
struct Guild {
  uint64_t id = 0;
  std::string name;
  std::string tag;
  uint64_t leaderId = 0;
  std::string description;
  uint32_t level = 1;
  uint32_t memberCount = 0;
  uint32_t maxMembers = 50;
  std::chrono::system_clock::time_point createdAt;
  
  bool isValid() const {
    return !name.empty() && leaderId > 0;
  }
};

}  // namespace Database
}  // namespace Umbra

