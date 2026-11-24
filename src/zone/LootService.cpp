#include "zone/LootService.hpp"
#include "zone/EnchantRoll.hpp"
#include "zone/MovementServer.hpp"
#include "zone/PartyShare.hpp"
#include "zone/QuestProgressService.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace Umbra {
namespace Zone {

LootService::LootService(uint32_t zoneId, std::shared_ptr<Database::MySQLConnector> db,
                         MovementServer* movementServer, ExperienceService* experienceService)
    : zoneId_(zoneId),
      db_(std::move(db)),
      movementServer_(movementServer),
      experienceService_(experienceService),
      rng_(static_cast<uint32_t>(
          std::chrono::steady_clock::now().time_since_epoch().count())) {}

LootService::~LootService() {
  {
    std::lock_guard<std::mutex> lock(petMu_);
    petStop_ = true;
  }
  petCv_.notify_all();
  if (petWorker_.joinable()) petWorker_.join();
}

void LootService::setCompanionService(CompanionService* companions) {
  companionService_ = companions;
  if (companions && !petWorker_.joinable()) {
    petWorker_ = std::thread([this] { petWorkerLoop(); });
  }
}

int64_t LootService::nowMs() const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

void LootService::loadFromDatabase() {
  if (!db_) return;
  std::unordered_map<uint32_t, NpcLootTableDef> next;

  try {
    auto expRows = db_->executePreparedQuery(
        "SELECT npc_template_id, kill_exp FROM npc_templates", {});
    for (const auto& row : expRows) {
      if (row.size() < 2) continue;
      const uint32_t tid = static_cast<uint32_t>(std::stoul(row[0]));
      next[tid].killExp = std::stoll(row[1]);
    }
  } catch (const std::exception& e) {
    Core::Logger::getInstance().warn(
        "[LootService] kill_exp load failed (coluna pode nao existir ainda): {}", e.what());
  }

  try {
    auto rows = db_->executePreparedQuery(
        "SELECT npc_template_id, entry_kind, COALESCE(item_template_id,0), drop_chance, "
        "min_qty, max_qty, sort_order FROM npc_loot_entries WHERE enabled = 1 "
        "ORDER BY npc_template_id, sort_order, loot_entry_id",
        {});
    for (const auto& row : rows) {
      if (row.size() < 7) continue;
      NpcLootEntryDef e;
      const uint32_t tid = static_cast<uint32_t>(std::stoul(row[0]));
      e.entryKind = static_cast<uint8_t>(std::stoi(row[1]));
      e.itemTemplateId = static_cast<uint32_t>(std::stoul(row[2]));
      e.dropChance = std::stof(row[3]);
      e.minQty = static_cast<uint32_t>(std::stoul(row[4]));
      e.maxQty = static_cast<uint32_t>(std::max(e.minQty, static_cast<uint32_t>(std::stoul(row[5]))));
      e.sortOrder = std::stoi(row[6]);
      if (e.entryKind == static_cast<uint8_t>(LootEntryKind::Item) && e.itemTemplateId == 0) continue;
      next[tid].entries.push_back(e);
    }
  } catch (const std::exception& ex) {
    Core::Logger::getInstance().warn("[LootService] loot entries load failed: {}", ex.what());
  }

  // Raridade dos itens para o filtro do pet de coleta.
  std::unordered_map<uint32_t, uint8_t> rarities;
  try {
    auto rows = db_->executePreparedQuery(
        "SELECT item_id, COALESCE(rarity,'common') FROM item_templates", {});
    for (const auto& row : rows) {
      if (row.size() < 2) continue;
      rarities[static_cast<uint32_t>(std::stoul(row[0]))] = companionRarityRank(row[1]);
    }
  } catch (const std::exception& ex) {
    Core::Logger::getInstance().warn("[LootService] item rarity load failed: {}", ex.what());
  }

  {
    std::lock_guard<std::mutex> lock(mu_);
    tablesByTemplate_ = std::move(next);
    itemRarity_ = std::move(rarities);
  }
  Core::Logger::getInstance().info("[LootService] loaded loot tables for {} templates (zone {})",
                                   tablesByTemplate_.size(), zoneId_);
}

void LootService::loadGameRates() {
  dropMultiplier_ = 1.0;
  if (!db_ || !db_->isConnected()) return;
  try {
    auto rows = db_->executePreparedQuery(
        "SELECT rate_value FROM game_rates WHERE rate_key = 'drop_multiplier' LIMIT 1", {});
    if (!rows.empty() && !rows[0].empty()) {
      dropMultiplier_ = std::stod(rows[0][0]);
      if (dropMultiplier_ < 0.0) dropMultiplier_ = 0.0;
    }
  } catch (const std::exception& e) {
    Core::Logger::getInstance().warn("[LootService] loadGameRates: {}", e.what());
  }
  Core::Logger::getInstance().info("[LootService] drop_multiplier={}", dropMultiplier_);
}

uint32_t LootService::rollQty(uint32_t minQty, uint32_t maxQty) {
  if (maxQty < minQty) maxQty = minQty;
  if (minQty == maxQty) return minQty;
  std::uniform_int_distribution<uint32_t> dist(minQty, maxQty);
  return dist(rng_);
}

std::vector<LootCorpseSlot> LootService::rollSlots(uint32_t npcTemplateId) {
  NpcLootTableDef table;
  {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = tablesByTemplate_.find(npcTemplateId);
    if (it == tablesByTemplate_.end()) return {};
    table = it->second;
  }

  std::vector<LootCorpseSlot> rolled;
  rolled.reserve(kMaxLootSlots);
  std::uniform_real_distribution<float> chanceDist(0.f, 1.f);

  for (const auto& e : table.entries) {
    if (static_cast<int>(rolled.size()) >= kMaxLootSlots) break;
    if (e.dropChance <= 0.f) continue;
    const float effectiveChance = static_cast<float>(
        std::min(1.0, static_cast<double>(e.dropChance) * dropMultiplier_));
    if (effectiveChance <= 0.f) continue;
    if (chanceDist(rng_) > effectiveChance) continue;
    LootCorpseSlot s;
    s.slotIndex = static_cast<uint8_t>(rolled.size());
    s.entryKind = e.entryKind;
    s.itemTemplateId =
        (e.entryKind == static_cast<uint8_t>(LootEntryKind::Gold)) ? 0 : e.itemTemplateId;
    s.quantity = rollQty(e.minQty, e.maxQty);
    s.taken = false;
    if (s.quantity == 0) continue;
    rolled.push_back(s);
  }
  return rolled;
}

void LootService::broadcastExpForGrant(uint32_t playerId, const ExperienceGrantResult& grant) {
  if (!movementServer_ || !grant.success) return;
  ExpGainNotifyPayload expPayload;
  expPayload.playerId = playerId;
  expPayload.expGained = static_cast<int32_t>(grant.expGranted);
  expPayload.totalExp = grant.totalExperience;
  expPayload.expForNext = grant.expForNextLevel;
  expPayload.expInCurrentLevel = grant.expInCurrentLevel;
  expPayload.progressPercent = static_cast<uint8_t>(
      std::clamp(static_cast<int>(std::lround(grant.expProgressPercent)), 0, 100));
  movementServer_->broadcastExpGain(playerId, expPayload);

  if (grant.levelsGained > 0 && db_) {
    LevelUpNotifyPayload levelPayload;
    levelPayload.playerId = playerId;
    levelPayload.newLevel = grant.newLevel;
    levelPayload.levelsGained =
        static_cast<uint8_t>(std::min<uint32_t>(grant.levelsGained, 255));
    levelPayload.statPointsGained =
        static_cast<uint16_t>(std::min<uint32_t>(grant.statPointsGained, 65535));
    levelPayload.skillPointsAvail = 0;
    auto skillOpt = db_->executePreparedScalar(
        "SELECT points_available FROM player_skill_points WHERE player_id = ?",
        {std::to_string(playerId)});
    if (skillOpt) {
      levelPayload.skillPointsAvail =
          static_cast<uint16_t>(std::min<int>(std::stoi(*skillOpt), 65535));
    }
    movementServer_->broadcastLevelUp(playerId, levelPayload);
  }
}

int64_t LootService::grantExperienceAndNotify(uint32_t playerId, int64_t amount, const std::string& source) {
  if (!experienceService_ || playerId == 0 || amount <= 0) return 0;
  auto grant = experienceService_->grantExperience(playerId, amount, source);
  broadcastExpForGrant(playerId, grant);
  return grant.success ? grant.expGranted : 0;
}

void LootService::onNpcKilled(uint32_t killerPlayerId, uint32_t npcInstanceId, uint32_t npcTemplateId,
                               uint32_t zoneId, float x, float y, float z) {
  if (killerPlayerId == 0 || !db_) return;

  int64_t killExp = 0;
  {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = tablesByTemplate_.find(npcTemplateId);
    if (it != tablesByTemplate_.end()) killExp = it->second.killExp;
  }

  if (killExp > 0 && experienceService_) {
    auto recipients = collectPartyShareRecipients(killerPlayerId, shareRadiusUu_, movementServer_,
                                                  resolvePartyMembers_);
    if (recipients.empty()) {
      recipients.push_back(killerPlayerId);
    }
    const size_t n = recipients.size();
    const int64_t per = killExp / static_cast<int64_t>(n);
    int64_t remainder = killExp - per * static_cast<int64_t>(n);
    const std::string source = "npc_kill:" + std::to_string(npcTemplateId);
    for (uint32_t pid : recipients) {
      int64_t amount = per;
      if (pid == killerPlayerId && remainder != 0) {
        amount += remainder;
        remainder = 0;
      }
      if (amount <= 0) continue;
      auto grant = experienceService_->grantExperience(pid, amount, source);
      broadcastExpForGrant(pid, grant);
    }
    Core::Logger::getInstance().info(
        "[LootService] exp share killer={} npcTpl={} total={} recipients={} per={} radius={:.0f}",
        killerPlayerId, npcTemplateId, killExp, n, per, shareRadiusUu_);
  }

  auto slots = rollSlots(npcTemplateId);
  if (slots.empty()) return;

  // Persist corpse — fix NULL item_template_id for gold
  uint64_t corpseId = 0;
  {
    const int64_t created = nowMs();
    const int64_t expires = created + kCorpseTtlMs;
    if (!db_->executePreparedInsert(
            "INSERT INTO loot_corpses (zone_id, npc_instance_id, npc_template_id, killer_player_id, "
            "pos_x, pos_y, pos_z, created_at, expires_at, closed) VALUES "
            "(?,?,?,?,?,?,?, FROM_UNIXTIME(? DIV 1000), FROM_UNIXTIME(? DIV 1000), 0)",
            {std::to_string(zoneId), std::to_string(npcInstanceId), std::to_string(npcTemplateId),
             std::to_string(killerPlayerId), std::to_string(x), std::to_string(y), std::to_string(z),
             std::to_string(created), std::to_string(expires)})) {
      Core::Logger::getInstance().warn("[LootService] insert corpse failed");
      return;
    }
    corpseId = db_->getLastInsertId();
    if (corpseId == 0) return;

    for (const auto& s : slots) {
      if (s.entryKind == static_cast<uint8_t>(LootEntryKind::Gold)) {
        db_->executePreparedInsert(
            "INSERT INTO loot_corpse_items (corpse_id, slot_index, entry_kind, item_template_id, "
            "quantity, taken) VALUES (?,?,?,NULL,?,0)",
            {std::to_string(corpseId), std::to_string(s.slotIndex), std::to_string(s.entryKind),
             std::to_string(s.quantity)});
      } else {
        db_->executePreparedInsert(
            "INSERT INTO loot_corpse_items (corpse_id, slot_index, entry_kind, item_template_id, "
            "quantity, taken) VALUES (?,?,?,?,?,0)",
            {std::to_string(corpseId), std::to_string(s.slotIndex), std::to_string(s.entryKind),
             std::to_string(s.itemTemplateId), std::to_string(s.quantity)});
      }
    }
  }

  LootCorpseRuntime corpse;
  corpse.corpseId = corpseId;
  corpse.zoneId = zoneId;
  corpse.npcInstanceId = npcInstanceId;
  corpse.npcTemplateId = npcTemplateId;
  corpse.killerPlayerId = killerPlayerId;
  corpse.x = x;
  corpse.y = y;
  corpse.z = z;
  corpse.expiresAtMs = nowMs() + kCorpseTtlMs;
  corpse.closed = false;
  corpse.slots = std::move(slots);

  {
    std::lock_guard<std::mutex> lock(mu_);
    corpses_[corpseId] = corpse;
  }

  // Pet de coleta ativo: a janela só abre depois, com o que o pet deixar (filtro/bolsa cheia/fora do raio).
  PetLootRule petRule;
  if (companionService_ && companionService_->getActivePetRule(killerPlayerId, petRule)) {
    PendingPetPickup pickup;
    pickup.corpseId = corpseId;
    pickup.ownerId = killerPlayerId;
    pickup.dueAtMs = nowMs() + static_cast<int64_t>(petRule.lootDelayMs);
    enqueuePetPickup(pickup);
    return;
  }

  sendWindowOpen(killerPlayerId, corpse);
  Core::Logger::getInstance().info(
      "[LootService] corpse {} opened for killer {} npcTpl={} slots={}", corpseId, killerPlayerId,
      npcTemplateId, corpse.slots.size());
}

std::vector<LootSlotPayload> LootService::remainingPayloadSlots(
    const LootCorpseRuntime& corpse) const {
  std::vector<LootSlotPayload> out;
  for (const auto& s : corpse.slots) {
    if (s.taken) continue;
    LootSlotPayload p;
    p.slotIndex = s.slotIndex;
    p.kind = s.entryKind;
    p.itemTemplateId = s.itemTemplateId;
    p.quantity = s.quantity;
    out.push_back(p);
  }
  return out;
}

void LootService::sendWindowOpen(uint32_t playerId, const LootCorpseRuntime& corpse) {
  if (!movementServer_) return;
  LootWindowOpenPayload p;
  p.corpseId = corpse.corpseId;
  p.npcTemplateId = corpse.npcTemplateId;
  p.npcInstanceId = corpse.npcInstanceId;
  p.slots = remainingPayloadSlots(corpse);
  const uint32_t cid = movementServer_->getClientIdForPlayer(playerId);
  if (cid > 0) {
    movementServer_->sendBinaryToClient(cid, encodeLootWindowOpen(p));
  }
}

void LootService::sendWindowUpdate(uint32_t playerId, const LootCorpseRuntime& corpse,
                                   int64_t playerGold, bool inventoryChanged) {
  if (!movementServer_) return;
  LootWindowUpdatePayload p;
  p.corpseId = corpse.corpseId;
  p.playerGold = playerGold;
  p.inventoryChanged = inventoryChanged ? 1 : 0;
  p.slots = remainingPayloadSlots(corpse);
  const uint32_t cid = movementServer_->getClientIdForPlayer(playerId);
  if (cid > 0) {
    movementServer_->sendBinaryToClient(cid, encodeLootWindowUpdate(p));
  }
}

void LootService::sendWindowClose(uint32_t playerId, uint64_t corpseId, LootCloseReason reason) {
  if (!movementServer_) return;
  LootWindowClosePayload p;
  p.corpseId = corpseId;
  p.reason = static_cast<uint8_t>(reason);
  const uint32_t cid = movementServer_->getClientIdForPlayer(playerId);
  if (cid > 0) {
    movementServer_->sendBinaryToClient(cid, encodeLootWindowClose(p));
  }
}

void LootService::markCorpseClosed(uint64_t corpseId) {
  if (!db_) return;
  db_->executePreparedInsert("UPDATE loot_corpses SET closed = 1 WHERE corpse_id = ?",
                             {std::to_string(corpseId)});
}

void LootService::markSlotTakenDb(uint64_t corpseId, uint8_t slotIndex) {
  if (!db_) return;
  db_->executePreparedInsert(
      "UPDATE loot_corpse_items SET taken = 1 WHERE corpse_id = ? AND slot_index = ?",
      {std::to_string(corpseId), std::to_string(slotIndex)});
}

int LootService::findFreeInventorySlot(uint32_t playerId) {
  auto rows = db_->executePreparedQuery(
      "SELECT slot_index FROM player_inventory WHERE player_id = ? ORDER BY slot_index",
      {std::to_string(playerId)});
  std::vector<char> used(50, 0);
  for (const auto& r : rows) {
    if (r.empty()) continue;
    const int idx = std::stoi(r[0]);
    if (idx >= 0 && idx < 50) used[idx] = 1;
  }
  for (int i = 0; i < 50; ++i) {
    if (!used[i]) return i;
  }
  return -1;
}

bool LootService::grantItemToPlayer(uint32_t playerId, uint32_t itemTemplateId, uint32_t quantity) {
  if (!db_ || itemTemplateId == 0 || quantity == 0) return false;
  auto tpl = db_->executePreparedQuery(
      "SELECT item_id, max_stack_size, COALESCE(item_category,''), COALESCE(equipment_slot,''), "
      "COALESCE(item_type,'') FROM item_templates WHERE item_id = ? LIMIT 1",
      {std::to_string(itemTemplateId)});
  if (tpl.empty() || tpl[0].size() < 2) return false;
  const uint32_t maxStack = std::max(1u, static_cast<uint32_t>(std::stoul(tpl[0][1])));
  const bool isEquipment = tpl[0].size() >= 5 &&
      EnchantRoll::isEquipmentTemplate(tpl[0][2], tpl[0][3], tpl[0][4]);

  // Prefer stack existing. PK é inventory_id (antes "id" → Prepare falhava, nunca
  // empilhava e cada loot ocupava um slot novo). Mesma regra do add_item.php: não
  // empilhar em item equipado nem anunciado no leilão.
  auto stacks = db_->executePreparedQuery(
      "SELECT inventory_id, quantity, slot_index FROM player_inventory WHERE player_id = ? AND "
      "item_template_id = ? AND COALESCE(is_equipped,0) = 0 AND auction_listing_id IS NULL "
      "ORDER BY slot_index",
      {std::to_string(playerId), std::to_string(itemTemplateId)});

  uint32_t remaining = quantity;
  for (const auto& row : stacks) {
    if (remaining == 0) break;
    if (row.size() < 3) continue;
    const uint32_t invId = static_cast<uint32_t>(std::stoul(row[0]));
    const uint32_t curQty = static_cast<uint32_t>(std::stoul(row[1]));
    if (curQty >= maxStack) continue;
    const uint32_t space = maxStack - curQty;
    const uint32_t add = std::min(space, remaining);
    db_->executePreparedInsert("UPDATE player_inventory SET quantity = quantity + ? WHERE inventory_id = ?",
                               {std::to_string(add), std::to_string(invId)});
    remaining -= add;
  }

  while (remaining > 0) {
    const int slot = findFreeInventorySlot(playerId);
    if (slot < 0) {
      Core::Logger::getInstance().warn("[LootService] inventory full for player {}", playerId);
      return false;
    }
    const uint32_t chunk = std::min(remaining, maxStack);
    if (!db_->executePreparedInsert(
            "INSERT INTO player_inventory (player_id, item_template_id, quantity, slot_index) "
            "VALUES (?,?,?,?)",
            {std::to_string(playerId), std::to_string(itemTemplateId), std::to_string(chunk),
             std::to_string(slot)})) {
      return false;
    }
    if (isEquipment) {
      const uint64_t invId = db_->getLastInsertId();
      const std::string ench = EnchantRoll::rollSpawnJson(*db_, tpl[0][3]);
      if (invId > 0) {
        db_->executePreparedInsert(
            "UPDATE player_inventory SET enchantments_json = ? WHERE inventory_id = ?",
            {ench.empty() ? "null" : ench, std::to_string(invId)});
      }
    }
    remaining -= chunk;
  }
  return true;
}

int64_t LootService::grantGoldToPlayer(uint32_t playerId, uint32_t amount) {
  if (!db_ || amount == 0) return -1;
  db_->executePreparedInsert("UPDATE players SET gold = gold + ? WHERE id = ?",
                             {std::to_string(amount), std::to_string(playerId)});
  auto g = db_->executePreparedScalar("SELECT gold FROM players WHERE id = ?",
                                      {std::to_string(playerId)});
  if (!g) return -1;
  return std::stoll(*g);
}

bool LootService::takeSlot(LootCorpseRuntime& corpse, uint8_t slotIndex, uint32_t playerId,
                           bool& outInventoryChanged, int64_t& outPlayerGold) {
  outInventoryChanged = false;
  for (auto& s : corpse.slots) {
    if (s.slotIndex != slotIndex || s.taken) continue;
    if (s.entryKind == static_cast<uint8_t>(LootEntryKind::Gold)) {
      const int64_t newGold = grantGoldToPlayer(playerId, s.quantity);
      if (newGold < 0) return false;
      outPlayerGold = newGold;
      if (questService_) {
        QuestEvent ev;
        ev.kind = QuestObjectiveKind::EarnGold;
        ev.amount = static_cast<int32_t>(std::min<uint32_t>(s.quantity, 0x7fffffff));
        questService_->onEvent(playerId, ev);
      }
    } else {
      if (!grantItemToPlayer(playerId, s.itemTemplateId, s.quantity)) return false;
      outInventoryChanged = true;
    }
    s.taken = true;
    markSlotTakenDb(corpse.corpseId, s.slotIndex);
    return true;
  }
  return false;
}

void LootService::handleLootTakeItem(uint32_t playerId, const LootTakeItemPayload& payload) {
  if (playerId == 0 || payload.corpseId == 0) return;
  LootCorpseRuntime corpseCopy;
  {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = corpses_.find(payload.corpseId);
    if (it == corpses_.end() || it->second.closed) return;
    if (it->second.killerPlayerId != playerId) return;
    corpseCopy = it->second;
  }

  bool invChanged = false;
  int64_t playerGold = -1;
  if (!takeSlot(corpseCopy, payload.slotIndex, playerId, invChanged, playerGold)) {
    return;
  }

  const bool empty = remainingPayloadSlots(corpseCopy).empty();
  {
    std::lock_guard<std::mutex> lock(mu_);
    corpses_[payload.corpseId] = corpseCopy;
    if (empty) {
      corpses_[payload.corpseId].closed = true;
    }
  }

  if (empty) {
    markCorpseClosed(payload.corpseId);
    sendWindowUpdate(playerId, corpseCopy, playerGold, invChanged);
    sendWindowClose(playerId, payload.corpseId, LootCloseReason::Empty);
  } else {
    sendWindowUpdate(playerId, corpseCopy, playerGold, invChanged);
  }
}

void LootService::handleLootTakeAll(uint32_t playerId, const LootTakeAllPayload& payload) {
  if (playerId == 0 || payload.corpseId == 0) return;
  LootCorpseRuntime corpseCopy;
  {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = corpses_.find(payload.corpseId);
    if (it == corpses_.end() || it->second.closed) return;
    if (it->second.killerPlayerId != playerId) return;
    corpseCopy = it->second;
  }

  bool invChanged = false;
  int64_t playerGold = -1;
  for (auto& s : corpseCopy.slots) {
    if (s.taken) continue;
    bool slotInv = false;
    int64_t slotGold = -1;
    if (!takeSlot(corpseCopy, s.slotIndex, playerId, slotInv, slotGold)) {
      // inventário cheio — para e envia estado parcial
      break;
    }
    if (slotInv) invChanged = true;
    if (slotGold >= 0) playerGold = slotGold;
  }

  const bool empty = remainingPayloadSlots(corpseCopy).empty();
  {
    std::lock_guard<std::mutex> lock(mu_);
    corpses_[payload.corpseId] = corpseCopy;
    if (empty) corpses_[payload.corpseId].closed = true;
  }

  sendWindowUpdate(playerId, corpseCopy, playerGold, invChanged);
  if (empty) {
    markCorpseClosed(payload.corpseId);
    sendWindowClose(playerId, payload.corpseId, LootCloseReason::Empty);
  }
}

uint8_t LootService::itemRarityOf(uint32_t itemTemplateId) {
  std::lock_guard<std::mutex> lock(mu_);
  auto it = itemRarity_.find(itemTemplateId);
  return it == itemRarity_.end() ? 0 : it->second;
}

void LootService::enqueuePetPickup(const PendingPetPickup& pickup) {
  {
    std::lock_guard<std::mutex> lock(petMu_);
    petQueue_.push_back(pickup);
  }
  petCv_.notify_one();
}

void LootService::petWorkerLoop() {
  std::unique_lock<std::mutex> lock(petMu_);
  while (!petStop_) {
    if (petQueue_.empty()) {
      petCv_.wait(lock, [this] { return petStop_ || !petQueue_.empty(); });
      continue;
    }
    // Fila quase em ordem (mesmo delay por pet): pega o vencimento mais cedo.
    auto earliest = std::min_element(petQueue_.begin(), petQueue_.end(),
                                     [](const PendingPetPickup& a, const PendingPetPickup& b) {
                                       return a.dueAtMs < b.dueAtMs;
                                     });
    const int64_t waitMs = earliest->dueAtMs - nowMs();
    if (waitMs > 0) {
      petCv_.wait_for(lock, std::chrono::milliseconds(waitMs));
      continue;
    }
    const PendingPetPickup job = *earliest;
    petQueue_.erase(earliest);
    lock.unlock();
    try {
      processPetPickup(job);
    } catch (const std::exception& e) {
      Core::Logger::getInstance().error("[LootService] pet pickup corpse {} falhou: {}", job.corpseId, e.what());
    }
    lock.lock();
  }
}

void LootService::processPetPickup(const PendingPetPickup& pickup) {
  LootCorpseRuntime corpseCopy;
  {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = corpses_.find(pickup.corpseId);
    if (it == corpses_.end() || it->second.closed) return;
    corpseCopy = it->second;
  }

  // O pet pode ter sido recolhido ou o dono saído do raio durante o delay: janela normal.
  PetLootRule rule;
  float ox = 0.f, oy = 0.f, oz = 0.f;
  const bool online = movementServer_ && movementServer_->tryGetPlayerPosition(pickup.ownerId, ox, oy, oz);
  if (!online) return;
  if (!companionService_ || !companionService_->getActivePetRule(pickup.ownerId, rule) ||
      !companionWithinRadius(ox, oy, corpseCopy.x, corpseCopy.y, rule.lootRadiusUu)) {
    sendWindowOpen(pickup.ownerId, corpseCopy);
    return;
  }

  CompanionLootCollectedPayload out;
  out.ownerId = pickup.ownerId;
  out.corpseId = corpseCopy.corpseId;
  out.x = corpseCopy.x;
  out.y = corpseCopy.y;
  out.z = corpseCopy.z;
  bool invChanged = false;
  int64_t playerGold = -1;
  for (auto& s : corpseCopy.slots) {
    if (s.taken) continue;
    const bool isGold = s.entryKind == static_cast<uint8_t>(LootEntryKind::Gold);
    const uint8_t rarity = isGold ? 0 : itemRarityOf(s.itemTemplateId);
    if (!companionShouldTakeSlot(rule, s.entryKind, rarity)) continue;
    const uint32_t qty = s.quantity;
    const uint32_t itemId = s.itemTemplateId;
    bool slotInv = false;
    int64_t slotGold = -1;
    // Bolsa cheia: o item fica no corpo (ouro ainda pode entrar), sem parar o resto.
    if (!takeSlot(corpseCopy, s.slotIndex, pickup.ownerId, slotInv, slotGold)) continue;
    if (isGold) {
      out.goldCollected += qty;
      if (slotGold >= 0) playerGold = slotGold;
    } else {
      out.items.push_back({itemId, qty});
      if (slotInv) invChanged = true;
    }
  }

  const bool empty = remainingPayloadSlots(corpseCopy).empty();
  {
    std::lock_guard<std::mutex> lock(mu_);
    corpses_[corpseCopy.corpseId] = corpseCopy;
    if (empty) corpses_[corpseCopy.corpseId].closed = true;
  }

  const bool collected = out.goldCollected > 0 || !out.items.empty();
  if (collected) {
    out.playerGold = playerGold;
    out.inventoryChanged = invChanged ? 1 : 0;
    // Dono atualiza bolsa/ouro; quem está perto só vê o pet ir até o corpo.
    movementServer_->broadcastNearPlayer(pickup.ownerId, encodeCompanionLootCollected(out));
  }
  if (empty) {
    markCorpseClosed(corpseCopy.corpseId);
  } else {
    sendWindowOpen(pickup.ownerId, corpseCopy);
  }
  Core::Logger::getInstance().info(
      "[LootService] pet {} do player {} coletou corpse {}: itens={} ouro={} sobrou={}", rule.companionId,
      pickup.ownerId, corpseCopy.corpseId, out.items.size(), out.goldCollected, empty ? 0 : 1);
}

void LootService::tick(float deltaSeconds) {
  expireAccum_ += deltaSeconds;
  if (expireAccum_ < 1.0f) return;
  expireAccum_ = 0.f;
  const int64_t now = nowMs();
  std::vector<std::pair<uint64_t, uint32_t>> expired;
  {
    std::lock_guard<std::mutex> lock(mu_);
    for (auto& kv : corpses_) {
      auto& c = kv.second;
      if (c.closed) continue;
      if (c.expiresAtMs > 0 && now >= c.expiresAtMs) {
        c.closed = true;
        expired.emplace_back(c.corpseId, c.killerPlayerId);
      }
    }
  }
  for (const auto& e : expired) {
    markCorpseClosed(e.first);
    sendWindowClose(e.second, e.first, LootCloseReason::Expired);
  }
}

}  // namespace Zone
}  // namespace Umbra
