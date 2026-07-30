#pragma once

#include "zone/MovementProtocol.hpp"
#include "zone/ExperienceService.hpp"
#include "database/MySQLConnector.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace Umbra {
namespace Zone {

class MovementServer;

struct NpcLootEntryDef {
  uint8_t entryKind = 0;  // 0 item, 1 gold
  uint32_t itemTemplateId = 0;
  float dropChance = 0.f;
  uint32_t minQty = 1;
  uint32_t maxQty = 1;
  int sortOrder = 0;
};

struct NpcLootTableDef {
  int64_t killExp = 0;
  std::vector<NpcLootEntryDef> entries;
};

struct LootCorpseSlot {
  uint8_t slotIndex = 0;
  uint8_t entryKind = 0;
  uint32_t itemTemplateId = 0;
  uint32_t quantity = 0;
  bool taken = false;
};

struct LootCorpseRuntime {
  uint64_t corpseId = 0;
  uint32_t zoneId = 0;
  uint32_t npcInstanceId = 0;
  uint32_t npcTemplateId = 0;
  uint32_t killerPlayerId = 0;
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
  int64_t expiresAtMs = 0;
  bool closed = false;
  std::vector<LootCorpseSlot> slots;
};

class LootService {
public:
  static constexpr int kMaxLootSlots = 10;
  static constexpr int64_t kCorpseTtlMs = 60000;

  LootService(uint32_t zoneId, std::shared_ptr<Database::MySQLConnector> db,
              MovementServer* movementServer, ExperienceService* experienceService);

  void setResolvePartyMembers(std::function<std::vector<uint32_t>(uint32_t)> cb) {
    resolvePartyMembers_ = std::move(cb);
  }
  void setShareRadiusUu(float radiusUu) { shareRadiusUu_ = radiusUu; }

  void loadFromDatabase();
  void tick(float deltaSeconds);

  /** Chamado no kill do NPC (killer autoritativo). Concede EXP (party share) e abre loot se houver drops. */
  void onNpcKilled(uint32_t killerPlayerId, uint32_t npcInstanceId, uint32_t npcTemplateId,
                    uint32_t zoneId, float x, float y, float z);

  void handleLootTakeItem(uint32_t playerId, const LootTakeItemPayload& payload);
  void handleLootTakeAll(uint32_t playerId, const LootTakeAllPayload& payload);

private:
  void broadcastExpForGrant(uint32_t playerId, const ExperienceGrantResult& grant);
  std::vector<LootCorpseSlot> rollSlots(uint32_t npcTemplateId);
  uint32_t rollQty(uint32_t minQty, uint32_t maxQty);
  bool grantItemToPlayer(uint32_t playerId, uint32_t itemTemplateId, uint32_t quantity);
  int findFreeInventorySlot(uint32_t playerId);
  int64_t grantGoldToPlayer(uint32_t playerId, uint32_t amount);
  bool takeSlot(LootCorpseRuntime& corpse, uint8_t slotIndex, uint32_t playerId,
                bool& outInventoryChanged, int64_t& outPlayerGold);
  void sendWindowOpen(uint32_t playerId, const LootCorpseRuntime& corpse);
  void sendWindowUpdate(uint32_t playerId, const LootCorpseRuntime& corpse, int64_t playerGold,
                        bool inventoryChanged);
  void sendWindowClose(uint32_t playerId, uint64_t corpseId, LootCloseReason reason);
  void markCorpseClosed(uint64_t corpseId);
  void markSlotTakenDb(uint64_t corpseId, uint8_t slotIndex);
  std::vector<LootSlotPayload> remainingPayloadSlots(const LootCorpseRuntime& corpse) const;
  int64_t nowMs() const;

  uint32_t zoneId_ = 0;
  std::shared_ptr<Database::MySQLConnector> db_;
  MovementServer* movementServer_ = nullptr;
  ExperienceService* experienceService_ = nullptr;
  std::function<std::vector<uint32_t>(uint32_t)> resolvePartyMembers_;
  float shareRadiusUu_ = 5000.f;

  std::mutex mu_;
  std::unordered_map<uint32_t, NpcLootTableDef> tablesByTemplate_;
  std::unordered_map<uint64_t, LootCorpseRuntime> corpses_;
  std::mt19937 rng_;
  float expireAccum_ = 0.f;
};

}  // namespace Zone
}  // namespace Umbra
