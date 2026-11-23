#pragma once
// CompanionService.hpp — companheiros (pet/elemental): modelos do Manager, ativo por player,
// opcode 124 (invocar/recolher/filtro) e regra de coleta do pet para o LootService.

#include "zone/MovementProtocol.hpp"
#include "database/MySQLConnector.hpp"
#include <cmath>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace Umbra {
namespace Zone {

class MovementServer;

/** Slot do companheiro (= companion_type): um ativo por slot. */
enum class CompanionSlot : uint8_t { Pet = 0, Elemental = 1 };

struct CompanionTemplateDef {
  uint32_t companionId = 0;
  std::string companionKey;
  std::string name;
  CompanionSlot slot = CompanionSlot::Pet;
  std::string meshPath;
  std::string idleAnimPath;
  std::string moveAnimPath;
  std::string pickupVfxPath;
  float scale = 1.f;
  float followDistance = 150.f;
  float hoverHeight = 0.f;
  float lootRadiusUu = 1500.f;
  uint32_t lootDelayMs = 800;
  int requiredLevel = 1;
  bool enabled = true;
};

/** Regra de coleta do pet ativo de um player (cópia para o LootService). */
struct PetLootRule {
  uint32_t companionId = 0;
  float lootRadiusUu = 1500.f;
  uint32_t lootDelayMs = 800;
  uint8_t minRarity = 0;  // 0=comum ... 4=lendário
  bool lootGold = true;
};

constexpr uint8_t kCompanionMaxRarity = 4;

/** item_templates.rarity → 0..4 (desconhecida = comum). */
inline uint8_t companionRarityRank(const std::string& rarity) {
  if (rarity == "uncommon") return 1;
  if (rarity == "rare") return 2;
  if (rarity == "epic") return 3;
  if (rarity == "legendary") return 4;
  return 0;
}

/** O pet pega este slot? entryKind: 0 = item, 1 = ouro (LootEntryKind). */
inline bool companionShouldTakeSlot(const PetLootRule& rule, uint8_t entryKind, uint8_t itemRarity) {
  if (entryKind == 1) return rule.lootGold;
  return itemRarity >= rule.minRarity;
}

/** Corpo dentro do raio do dono (XY, como o share de party). */
inline bool companionWithinRadius(float ownerX, float ownerY, float corpseX, float corpseY,
                                  float radiusUu) {
  if (radiusUu <= 0.f) return false;
  const float dx = corpseX - ownerX;
  const float dy = corpseY - ownerY;
  return dx * dx + dy * dy <= radiusUu * radiusUu;
}

class CompanionService {
public:
  CompanionService(std::shared_ptr<Database::MySQLConnector> db, MovementServer* movementServer);

  /** (Re)carrega companion_templates. Chamado no boot e no admin reload_companions. */
  void loadTemplates();
  size_t templateCount() const;

  /** Opcode 124 (thread do inbound worker; pode ir ao DB). */
  void handleRequest(uint32_t playerId, const CompanionRequestPayload& payload);

  /** Player saiu da zone: some o cache (o is_active no DB fica para o relog). */
  void onPlayerGone(uint32_t playerId);

  /** Regra do pet ativo; false se o player não tem pet invocado. */
  bool getActivePetRule(uint32_t playerId, PetLootRule& out) const;

private:
  struct ActiveEntry {
    uint32_t companionId = 0;
    uint8_t minRarity = 0;
    bool lootGold = true;
  };

  static uint64_t activeKey(uint32_t playerId, CompanionSlot slot) {
    return (static_cast<uint64_t>(playerId) << 8) | static_cast<uint64_t>(slot);
  }

  bool findTemplate(uint32_t companionId, CompanionTemplateDef& out) const;
  CompanionStatePayload buildState(uint32_t ownerId, const CompanionTemplateDef& tpl, bool active) const;
  void summon(uint32_t playerId, const CompanionTemplateDef& tpl);
  void dismiss(uint32_t playerId, const CompanionTemplateDef& tpl);
  void updateSettings(uint32_t playerId, const CompanionTemplateDef& tpl, uint8_t minRarity, bool lootGold);

  std::shared_ptr<Database::MySQLConnector> db_;
  MovementServer* movementServer_ = nullptr;

  mutable std::mutex mu_;
  std::unordered_map<uint32_t, CompanionTemplateDef> templates_;
  std::unordered_map<uint64_t, ActiveEntry> active_;
};

}  // namespace Zone
}  // namespace Umbra
