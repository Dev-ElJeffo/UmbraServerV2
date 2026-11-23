#include "zone/CompanionService.hpp"
#include "zone/MovementServer.hpp"
#include "core/Logger.hpp"
#include <algorithm>

namespace Umbra {
namespace Zone {

namespace {

float parseFloatOr(const std::string& s, float def) {
  try {
    return s.empty() ? def : std::stof(s);
  } catch (...) {
    return def;
  }
}

long long parseIntOr(const std::string& s, long long def) {
  try {
    return s.empty() ? def : std::stoll(s);
  } catch (...) {
    return def;
  }
}

}  // namespace

CompanionService::CompanionService(std::shared_ptr<Database::MySQLConnector> db,
                                   MovementServer* movementServer)
    : db_(std::move(db)), movementServer_(movementServer) {}

void CompanionService::loadTemplates() {
  if (!db_ || !db_->isConnected()) return;
  std::unordered_map<uint32_t, CompanionTemplateDef> next;
  try {
    // Ordem posicional: conferir os índices abaixo ao mudar o SELECT.
    auto rows = db_->executePreparedQuery(
        "SELECT companion_id, companion_key, name, companion_type, mesh_path, idle_anim_path, "
        "move_anim_path, pickup_vfx_path, scale, follow_distance, hover_height, loot_radius_uu, "
        "loot_delay_ms, required_level, is_active FROM companion_templates",
        {});
    for (const auto& r : rows) {
      if (r.size() < 15) continue;
      CompanionTemplateDef t;
      t.companionId = static_cast<uint32_t>(parseIntOr(r[0], 0));
      if (t.companionId == 0) continue;
      t.companionKey = r[1];
      t.name = r[2];
      t.slot = (r[3] == "elemental") ? CompanionSlot::Elemental : CompanionSlot::Pet;
      t.meshPath = r[4];
      t.idleAnimPath = r[5];
      t.moveAnimPath = r[6];
      t.pickupVfxPath = r[7];
      t.scale = std::clamp(parseFloatOr(r[8], 1.f), 0.1f, 5.f);
      t.followDistance = std::clamp(parseFloatOr(r[9], 150.f), 50.f, 1000.f);
      t.hoverHeight = std::clamp(parseFloatOr(r[10], 0.f), 0.f, 500.f);
      t.lootRadiusUu = std::clamp(parseFloatOr(r[11], 1500.f), 0.f, 5000.f);
      t.lootDelayMs = static_cast<uint32_t>(std::clamp<long long>(parseIntOr(r[12], 800), 0, 5000));
      t.requiredLevel = static_cast<int>(std::max<long long>(1, parseIntOr(r[13], 1)));
      t.enabled = parseIntOr(r[14], 1) != 0;
      next[t.companionId] = std::move(t);
    }
  } catch (const std::exception& e) {
    Core::Logger::getInstance().warn("[CompanionService] load falhou (rodar create_companion_system.sql?): {}",
                                     e.what());
    return;
  }
  size_t n = 0;
  {
    std::lock_guard<std::mutex> lock(mu_);
    templates_ = std::move(next);
    n = templates_.size();
  }
  Core::Logger::getInstance().info("[CompanionService] {} modelos de companheiro carregados", n);
}

size_t CompanionService::templateCount() const {
  std::lock_guard<std::mutex> lock(mu_);
  return templates_.size();
}

bool CompanionService::findTemplate(uint32_t companionId, CompanionTemplateDef& out) const {
  std::lock_guard<std::mutex> lock(mu_);
  auto it = templates_.find(companionId);
  if (it == templates_.end()) return false;
  out = it->second;
  return true;
}

CompanionStatePayload CompanionService::buildState(uint32_t ownerId, const CompanionTemplateDef& tpl,
                                                   bool active) const {
  CompanionStatePayload s;
  s.ownerId = ownerId;
  s.slot = static_cast<uint8_t>(tpl.slot);
  s.companionId = tpl.companionId;
  s.active = active ? 1 : 0;
  if (active) {
    s.scale = tpl.scale;
    s.followDistance = tpl.followDistance;
    s.hoverHeight = tpl.hoverHeight;
    s.name = tpl.name;
    s.meshPath = tpl.meshPath;
    s.idleAnimPath = tpl.idleAnimPath;
    s.moveAnimPath = tpl.moveAnimPath;
    s.pickupVfxPath = tpl.pickupVfxPath;
  }
  return s;
}

void CompanionService::handleRequest(uint32_t playerId, const CompanionRequestPayload& payload) {
  if (playerId == 0 || payload.companionId == 0) return;
  CompanionTemplateDef tpl;
  if (!findTemplate(payload.companionId, tpl)) {
    Core::Logger::getInstance().warn("[CompanionService] player {} pediu companheiro {} inexistente",
                                     playerId, payload.companionId);
    return;
  }
  switch (static_cast<CompanionAction>(payload.action)) {
    case CompanionAction::Summon:
      summon(playerId, tpl);
      break;
    case CompanionAction::Dismiss:
      dismiss(playerId, tpl);
      break;
    case CompanionAction::Settings:
      updateSettings(playerId, tpl, std::min(payload.minRarity, kCompanionMaxRarity),
                     (payload.flags & kCompanionFlagLootGold) != 0);
      break;
  }
}

void CompanionService::summon(uint32_t playerId, const CompanionTemplateDef& tpl) {
  if (!db_ || !movementServer_) return;
  if (!tpl.enabled) {
    Core::Logger::getInstance().warn("[CompanionService] companheiro {} desativado (player {})",
                                     tpl.companionId, playerId);
    return;
  }
  // Posse + validade + nível (o filtro vem do DB; o cliente só muda via action Settings).
  auto rows = db_->executePreparedQuery(
      "SELECT p.level, pc.loot_min_rarity, pc.loot_gold FROM player_companions pc "
      "JOIN players p ON p.id = pc.player_id "
      "WHERE pc.player_id = ? AND pc.companion_id = ? "
      "AND (pc.expires_at IS NULL OR pc.expires_at > NOW()) LIMIT 1",
      {std::to_string(playerId), std::to_string(tpl.companionId)});
  if (rows.empty() || rows[0].size() < 3) {
    Core::Logger::getInstance().warn("[CompanionService] player {} não possui o companheiro {}",
                                     playerId, tpl.companionId);
    return;
  }
  const long long level = parseIntOr(rows[0][0], 1);
  if (level < tpl.requiredLevel) {
    Core::Logger::getInstance().warn("[CompanionService] player {} nível {} < {} para o companheiro {}",
                                     playerId, level, tpl.requiredLevel, tpl.companionId);
    return;
  }

  ActiveEntry entry;
  entry.companionId = tpl.companionId;
  entry.minRarity = static_cast<uint8_t>(std::clamp<long long>(parseIntOr(rows[0][1], 0), 0, kCompanionMaxRarity));
  entry.lootGold = parseIntOr(rows[0][2], 1) != 0;

  // Um ativo por tipo: ativa este e desativa os outros do mesmo tipo.
  db_->executePreparedInsert(
      "UPDATE player_companions pc JOIN companion_templates ct ON ct.companion_id = pc.companion_id "
      "SET pc.is_active = IF(pc.companion_id = ?, 1, 0) "
      "WHERE pc.player_id = ? AND ct.companion_type = ?",
      {std::to_string(tpl.companionId), std::to_string(playerId),
       tpl.slot == CompanionSlot::Elemental ? "elemental" : "pet"});

  {
    std::lock_guard<std::mutex> lock(mu_);
    active_[activeKey(playerId, tpl.slot)] = entry;
  }
  movementServer_->setPlayerCompanionState(buildState(playerId, tpl, true));
  Core::Logger::getInstance().info("[CompanionService] player {} invocou {} ({}) filtro={} ouro={}",
                                   playerId, tpl.companionId, tpl.companionKey, entry.minRarity,
                                   entry.lootGold);
}

void CompanionService::dismiss(uint32_t playerId, const CompanionTemplateDef& tpl) {
  if (!db_ || !movementServer_) return;
  bool wasActive = false;
  {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = active_.find(activeKey(playerId, tpl.slot));
    if (it != active_.end() && it->second.companionId == tpl.companionId) {
      active_.erase(it);
      wasActive = true;
    }
  }
  db_->executePreparedInsert(
      "UPDATE player_companions SET is_active = 0 WHERE player_id = ? AND companion_id = ?",
      {std::to_string(playerId), std::to_string(tpl.companionId)});
  if (wasActive) {
    movementServer_->setPlayerCompanionState(buildState(playerId, tpl, false));
  }
  Core::Logger::getInstance().info("[CompanionService] player {} recolheu {}", playerId, tpl.companionId);
}

void CompanionService::updateSettings(uint32_t playerId, const CompanionTemplateDef& tpl, uint8_t minRarity,
                                      bool lootGold) {
  if (!db_) return;
  db_->executePreparedInsert(
      "UPDATE player_companions SET loot_min_rarity = ?, loot_gold = ? WHERE player_id = ? AND companion_id = ?",
      {std::to_string(minRarity), lootGold ? "1" : "0", std::to_string(playerId),
       std::to_string(tpl.companionId)});
  std::lock_guard<std::mutex> lock(mu_);
  auto it = active_.find(activeKey(playerId, tpl.slot));
  if (it != active_.end() && it->second.companionId == tpl.companionId) {
    it->second.minRarity = minRarity;
    it->second.lootGold = lootGold;
  }
}

void CompanionService::onPlayerGone(uint32_t playerId) {
  std::lock_guard<std::mutex> lock(mu_);
  active_.erase(activeKey(playerId, CompanionSlot::Pet));
  active_.erase(activeKey(playerId, CompanionSlot::Elemental));
}

bool CompanionService::getActivePetRule(uint32_t playerId, PetLootRule& out) const {
  std::lock_guard<std::mutex> lock(mu_);
  auto it = active_.find(activeKey(playerId, CompanionSlot::Pet));
  if (it == active_.end()) return false;
  auto tit = templates_.find(it->second.companionId);
  if (tit == templates_.end() || !tit->second.enabled) return false;
  out.companionId = it->second.companionId;
  out.lootRadiusUu = tit->second.lootRadiusUu;
  out.lootDelayMs = tit->second.lootDelayMs;
  out.minRarity = it->second.minRarity;
  out.lootGold = it->second.lootGold;
  return true;
}

}  // namespace Zone
}  // namespace Umbra
