#include "SkillService.hpp"
#include <algorithm>
#include <chrono>
#include <nlohmann/json.hpp>

namespace Umbra {
namespace Combat {

namespace {

ResourceType parseResourceType(const std::string& value) {
  if (value == "HEALTH") return ResourceType::HEALTH;
  if (value == "STAMINA") return ResourceType::STAMINA;
  if (value == "NONE") return ResourceType::NONE;
  return ResourceType::MANA;
}

EffectType parseEffectType(const std::string& value) {
  if (value == "DOT") return EffectType::DOT;
  if (value == "HOT") return EffectType::HOT;
  if (value == "HEAL") return EffectType::HEAL;
  if (value == "SHIELD") return EffectType::SHIELD;
  if (value == "BUFF_STAT") return EffectType::BUFF_STAT;
  if (value == "DEBUFF_STAT") return EffectType::DEBUFF_STAT;
  if (value == "STUN") return EffectType::STUN;
  if (value == "SILENCE") return EffectType::SILENCE;
  if (value == "SLOW") return EffectType::SLOW;
  if (value == "ROOT") return EffectType::ROOT;
  return EffectType::DAMAGE;
}

int32_t jsonIntField(const nlohmann::json& j, const char* key, int32_t fallback = 0) {
  auto it = j.find(key);
  if (it == j.end() || it->is_null()) return fallback;
  try {
    if (it->is_number()) return static_cast<int32_t>(it->get<double>());
    if (it->is_string()) return std::stoi(it->get<std::string>());
  } catch (...) {
  }
  return fallback;
}

std::string buffTypeToDbString(BuffType type) {
  switch (type) {
    case BuffType::DEBUFF: return "DEBUFF";
    case BuffType::AURA: return "AURA";
    case BuffType::DOT: return "DOT";
    case BuffType::HOT: return "HOT";
    case BuffType::SHIELD: return "SHIELD";
    default: return "BUFF";
  }
}

BuffType parseBuffTypeFromDb(const std::string& value) {
  if (value == "DEBUFF") return BuffType::DEBUFF;
  if (value == "AURA") return BuffType::AURA;
  if (value == "DOT") return BuffType::DOT;
  if (value == "HOT") return BuffType::HOT;
  if (value == "SHIELD") return BuffType::SHIELD;
  return BuffType::BUFF;
}

BuffType effectToBuffType(EffectType effectType) {
  switch (effectType) {
    case EffectType::DEBUFF_STAT:
    case EffectType::STUN:
    case EffectType::SILENCE:
    case EffectType::ROOT:
    case EffectType::SLOW:
      return BuffType::DEBUFF;
    case EffectType::SHIELD:
      return BuffType::SHIELD;
    default:
      return BuffType::BUFF;
  }
}

std::string effectTypeToString(EffectType t) {
  switch (t) {
    case EffectType::DOT: return "DOT";
    case EffectType::HOT: return "HOT";
    case EffectType::HEAL: return "HEAL";
    case EffectType::SHIELD: return "SHIELD";
    case EffectType::BUFF_STAT: return "BUFF_STAT";
    case EffectType::DEBUFF_STAT: return "DEBUFF_STAT";
    case EffectType::STUN: return "STUN";
    case EffectType::SILENCE: return "SILENCE";
    case EffectType::SLOW: return "SLOW";
    case EffectType::ROOT: return "ROOT";
    default: return "DAMAGE";
  }
}

}  // namespace

bool SkillService::loadSkillsFromDatabase() {
  if (!db_ || !db_->isConnected()) return false;

  std::unique_lock<std::shared_mutex> lock(skillDataMutex_);
  skillDataById_.clear();
  skillIdByKey_.clear();
  skillIdsByClass_.clear();

  auto rows = db_->executePreparedQuery(
      "SELECT skill_id, skill_key, skill_name, class_id, skill_order, required_level, skill_cost, max_rank, "
      "type_id, target_id, element_id, scaling_stat_id, "
      "COALESCE(str_scaling,0), COALESCE(dex_scaling,0), COALESCE(vit_scaling,0), "
      "COALESCE(int_scaling,0), COALESCE(lck_scaling,0), "
      "power_coef, COALESCE(secondary_coef,0), resource_type, resource_cost, "
      "COALESCE(resource_cost_percent,0), cooldown_ms, cast_time_ms, COALESCE(duration_ms,0), "
      "COALESCE(range_min,0), range_max, COALESCE(area_radius,0), "
      "COALESCE(is_stackable,0), COALESCE(max_stacks,1), can_crit, COALESCE(ignores_defense,0), "
      "COALESCE(is_interrupt,0), COALESCE(requires_target,1), COALESCE(can_move_while_casting,0), "
      "COALESCE(threat_modifier,100), COALESCE(pvp_modifier,100), "
      "COALESCE(effects_json,''), COALESCE(icon_path,''), COALESCE(vfx_key,''), COALESCE(sfx_key,''), "
      "COALESCE(description,''), COALESCE(tooltip_template,''), COALESCE(server_tags,'') "
      "FROM skills WHERE is_enabled = 1",
      {});

  for (const auto& row : rows) {
    if (row.size() < 44) continue;
    SkillData skill;
    try {
      skill.skillId = static_cast<uint32_t>(std::stoul(row[0]));
      skill.skillKey = row[1];
      skill.skillName = row[2];
      skill.classId = std::stoull(row[3]);
      skill.skillOrder = static_cast<uint8_t>(std::stoul(row[4]));
      skill.requiredLevel = static_cast<uint8_t>(std::stoul(row[5]));
      skill.skillCost = static_cast<uint8_t>(std::stoul(row[6]));
      skill.maxRank = static_cast<uint8_t>(std::stoul(row[7]));
      skill.type = static_cast<SkillType>(std::stoul(row[8]));
      skill.target = static_cast<TargetType>(std::stoul(row[9]));
      skill.element = static_cast<Element>(std::stoul(row[10]));
      skill.scalingStat = static_cast<ScalingStat>(std::stoul(row[11]));
      skill.strScaling = static_cast<uint8_t>(std::stoul(row[12]));
      skill.dexScaling = static_cast<uint8_t>(std::stoul(row[13]));
      skill.vitScaling = static_cast<uint8_t>(std::stoul(row[14]));
      skill.intScaling = static_cast<uint8_t>(std::stoul(row[15]));
      skill.lckScaling = static_cast<uint8_t>(std::stoul(row[16]));
      skill.powerCoef = static_cast<uint16_t>(std::stoul(row[17]));
      skill.secondaryCoef = static_cast<uint16_t>(std::stoul(row[18]));
      skill.resourceType = parseResourceType(row[19]);
      skill.resourceCost = static_cast<uint16_t>(std::stoul(row[20]));
      skill.resourceCostPercent = static_cast<uint8_t>(std::stoul(row[21]));
      skill.cooldownMs = static_cast<uint32_t>(std::stoul(row[22]));
      skill.castTimeMs = static_cast<uint32_t>(std::stoul(row[23]));
      skill.durationMs = static_cast<uint32_t>(std::stoul(row[24]));
      skill.rangeMin = static_cast<uint16_t>(std::stoul(row[25]));
      skill.rangeMax = static_cast<uint16_t>(std::stoul(row[26]));
      skill.areaRadius = static_cast<uint16_t>(std::stoul(row[27]));
      skill.isStackable = (std::stoi(row[28]) != 0);
      skill.maxStacks = static_cast<uint8_t>(std::stoul(row[29]));
      skill.canCrit = (std::stoi(row[30]) != 0);
      skill.ignoresDefense = (std::stoi(row[31]) != 0);
      skill.isInterrupt = (std::stoi(row[32]) != 0);
      skill.requiresTarget = (std::stoi(row[33]) != 0);
      skill.canMoveWhileCasting = (std::stoi(row[34]) != 0);
      skill.threatModifier = static_cast<int16_t>(std::stoi(row[35]));
      skill.pvpModifier = static_cast<uint8_t>(std::stoul(row[36]));
      skill.effects = parseEffectsFromJson(row[37]);
      skill.iconPath = row[38];
      skill.vfxKey = row[39];
      skill.sfxKey = row[40];
      skill.description = row[41];
      skill.tooltipTemplate = row[42];
      if (!row[43].empty() && row[43] != "null") {
        nlohmann::json tags = nlohmann::json::parse(row[43], nullptr, false);
        if (tags.is_array()) {
          for (const auto& t : tags) {
            if (t.is_string()) skill.serverTags.push_back(t.get<std::string>());
          }
        }
      }
    } catch (...) {
      continue;
    }

    skillDataById_[skill.skillId] = skill;
    if (!skill.skillKey.empty()) {
      skillIdByKey_[skill.skillKey] = skill.skillId;
    }
    skillIdsByClass_[skill.classId].push_back(skill.skillId);
  }

  auto scalingRows = db_->executePreparedQuery(
      "SELECT skill_id, `rank`, power_coef_bonus, resource_cost_bonus, cooldown_reduction_ms, "
      "duration_bonus_ms, COALESCE(extra_effects_json,'') "
      "FROM skill_rank_scaling ORDER BY skill_id, `rank`",
      {});
  size_t scalingAttached = 0;
  for (const auto& row : scalingRows) {
    if (row.size() < 7) continue;
    try {
      const uint32_t skillId = static_cast<uint32_t>(std::stoul(row[0]));
      auto it = skillDataById_.find(skillId);
      if (it == skillDataById_.end()) continue;
      SkillRankScaling scaling;
      scaling.rank = static_cast<uint8_t>(std::stoul(row[1]));
      scaling.powerCoefBonus = static_cast<int16_t>(std::stoi(row[2]));
      scaling.resourceCostBonus = static_cast<int16_t>(std::stoi(row[3]));
      scaling.cooldownReductionMs = std::stoi(row[4]);
      scaling.durationBonusMs = std::stoi(row[5]);
      scaling.extraEffects = parseEffectsFromJson(row[6]);
      it->second.rankScalings.push_back(std::move(scaling));
      ++scalingAttached;
    } catch (...) {
      continue;
    }
  }

  Core::Logger::getInstance().info(
      "[SkillService] {} skills carregadas do DB ({} rank scalings)", skillDataById_.size(),
      scalingAttached);
  return true;
}

bool SkillService::reloadSkills() {
  const bool playerOk = loadSkillsFromDatabase();
  const bool npcOk = loadNpcSkillsFromDatabase();
  return playerOk && npcOk;
}

bool SkillService::loadNpcSkillsFromDatabase() {
  if (!db_ || !db_->isConnected()) return false;
  std::unique_lock<std::shared_mutex> lock(skillDataMutex_);
  npcSkillById_.clear();
  auto rows = db_->executePreparedQuery(
      "SELECT npc_skill_id, skill_key, skill_name, type_id, target_id, element_id, scaling_stat_id, "
      "COALESCE(str_scaling,0), COALESCE(dex_scaling,0), COALESCE(vit_scaling,0), "
      "COALESCE(int_scaling,0), COALESCE(lck_scaling,0), "
      "power_coef, COALESCE(secondary_coef,0), resource_type, COALESCE(resource_cost,0), "
      "COALESCE(resource_cost_percent,0), cooldown_ms, cast_time_ms, COALESCE(duration_ms,0), "
      "COALESCE(range_min,0), range_max, COALESCE(area_radius,0), "
      "can_crit, COALESCE(ignores_defense,0), COALESCE(requires_target,1), "
      "COALESCE(effects_json,''), COALESCE(icon_path,''), COALESCE(vfx_key,''), COALESCE(sfx_key,''), "
      "COALESCE(vfx_path,''), COALESCE(hit_vfx_path,'') "
      "FROM npc_skills WHERE is_enabled = 1",
      {});
  for (const auto& row : rows) {
    if (row.size() < 32) continue;
    SkillData skill;
    try {
      skill.skillId = static_cast<uint32_t>(std::stoul(row[0]));
      skill.skillKey = row[1];
      skill.skillName = row[2];
      skill.type = static_cast<SkillType>(std::stoul(row[3]));
      skill.target = static_cast<TargetType>(std::stoul(row[4]));
      skill.element = static_cast<Element>(std::stoul(row[5]));
      skill.scalingStat = static_cast<ScalingStat>(std::stoul(row[6]));
      skill.strScaling = static_cast<uint8_t>(std::stoul(row[7]));
      skill.dexScaling = static_cast<uint8_t>(std::stoul(row[8]));
      skill.vitScaling = static_cast<uint8_t>(std::stoul(row[9]));
      skill.intScaling = static_cast<uint8_t>(std::stoul(row[10]));
      skill.lckScaling = static_cast<uint8_t>(std::stoul(row[11]));
      skill.powerCoef = static_cast<uint16_t>(std::stoul(row[12]));
      skill.secondaryCoef = static_cast<uint16_t>(std::stoul(row[13]));
      skill.resourceType = parseResourceType(row[14]);
      skill.resourceCost = static_cast<uint16_t>(std::stoul(row[15]));
      skill.resourceCostPercent = static_cast<uint8_t>(std::stoul(row[16]));
      skill.cooldownMs = static_cast<uint32_t>(std::stoul(row[17]));
      skill.castTimeMs = static_cast<uint32_t>(std::stoul(row[18]));
      skill.durationMs = static_cast<uint32_t>(std::stoul(row[19]));
      skill.rangeMin = static_cast<uint16_t>(std::stoul(row[20]));
      skill.rangeMax = static_cast<uint16_t>(std::stoul(row[21]));
      skill.areaRadius = static_cast<uint16_t>(std::stoul(row[22]));
      skill.canCrit = (std::stoi(row[23]) != 0);
      skill.ignoresDefense = (std::stoi(row[24]) != 0);
      skill.requiresTarget = (std::stoi(row[25]) != 0);
      skill.effects = parseEffectsFromJson(row[26]);
      skill.iconPath = row[27];
      skill.vfxKey = row[28];
      skill.sfxKey = row[29];
      skill.vfxPath = row[30];
      skill.hitVfxPath = row[31];
    } catch (...) {
      continue;
    }
    npcSkillById_[skill.skillId] = std::move(skill);
  }
  Core::Logger::getInstance().info("[SkillService] {} npc_skills carregadas", npcSkillById_.size());
  return true;
}

SkillEffect SkillService::parseEffectFromJson(const nlohmann::json& json) {
  SkillEffect effect;
  if (!json.is_object()) return effect;

  std::string typeStr;
  if (json.contains("type") && json["type"].is_string()) {
    typeStr = json["type"].get<std::string>();
  } else if (json.contains("effect_type") && json["effect_type"].is_string()) {
    typeStr = json["effect_type"].get<std::string>();
  }
  effect.effectType = parseEffectType(typeStr);

  if (json.contains("target_stat") && json["target_stat"].is_string()) {
    effect.targetStat = json["target_stat"].get<std::string>();
  }
  effect.valueFlat = jsonIntField(json, "value_flat", jsonIntField(json, "value", 0));
  effect.valuePercent = static_cast<int16_t>(jsonIntField(json, "value_percent", 0));
  effect.durationMs = static_cast<uint32_t>(std::max(0, jsonIntField(json, "duration_ms", 0)));
  effect.tickIntervalMs = static_cast<uint32_t>(std::max(0, jsonIntField(json, "tick_interval_ms", 1000)));
  effect.chancePercent = static_cast<uint8_t>(std::clamp(jsonIntField(json, "chance_percent", 100), 0, 100));
  effect.resistPenetration =
      static_cast<uint8_t>(std::clamp(jsonIntField(json, "resist_penetration", 0), 0, 100));
  if (json.contains("conditions_json") && json["conditions_json"].is_object()) {
    effect.conditions = json["conditions_json"];
  }
  return effect;
}

std::vector<SkillEffect> SkillService::parseEffectsFromJson(const std::string& jsonStr) {
  std::vector<SkillEffect> effects;
  if (jsonStr.empty() || jsonStr == "null") return effects;

  nlohmann::json parsed = nlohmann::json::parse(jsonStr, nullptr, false);
  if (parsed.is_discarded() || !parsed.is_array()) return effects;

  uint8_t order = 1;
  for (const auto& item : parsed) {
    SkillEffect effect = parseEffectFromJson(item);
    effect.effectOrder = order++;
    effects.push_back(std::move(effect));
  }
  return effects;
}

SkillService::ValidationResult SkillService::validateSkillUse(
    const CharacterState& source, const SkillUseRequest& request) {
  ValidationResult result;
  const SkillData* skill = getSkillData(request.skillId);
  if (!skill) {
    result.errorCode = "SKILL_NOT_FOUND";
    result.errorMessage = "Skill não encontrada";
    return result;
  }

  if (!source.canUseSkill()) {
    result.errorCode = "CANNOT_CAST";
    result.errorMessage = "Personagem não pode usar skills";
    return result;
  }

  if (isSkillOnCooldown(source.playerId, request.skillId)) {
    result.errorCode = "ON_COOLDOWN";
    result.errorMessage = "Skill em cooldown";
    return result;
  }

  const uint8_t rank = request.skillRank < 1 ? 1 : request.skillRank;
  const uint16_t effectiveCost = skill->getEffectiveResourceCost(rank);
  if (skill->resourceType == ResourceType::MANA &&
      source.baseStats.currentMana < static_cast<int32_t>(effectiveCost)) {
    result.errorCode = "NO_MANA";
    result.errorMessage = "Mana insuficiente";
    return result;
  }

  result.isValid = true;
  return result;
}

uint64_t SkillService::applyBuff(uint64_t targetPlayerId, uint64_t sourcePlayerId, uint32_t skillId,
                                 const SkillEffect& effect, const CharacterState& /*sourceState*/) {
  if (!db_ || !db_->isConnected() || targetPlayerId == 0) return 0;

  const SkillData* skill = getSkillData(skillId);
  if (!skill) return 0;

  const BuffType buffType = effectToBuffType(effect.effectType);
  // Sem rank no applyBuff legado: usa duration base da skill (cast path usa helpers efetivos).
  uint32_t durationMs = effect.durationMs > 0 ? effect.durationMs : skill->durationMs;
  if (durationMs == 0) durationMs = 5000;

  nlohmann::json snapshot;
  snapshot["target_stat"] = effect.targetStat;
  snapshot["value_flat"] = effect.valueFlat;
  snapshot["value_percent"] = effect.valuePercent;
  snapshot["effect_type"] = effectTypeToString(effect.effectType);
  const std::string snapshotStr = snapshot.dump();

  const int32_t valueSnapshot = effect.valueFlat != 0 ? effect.valueFlat : effect.valuePercent;
  const std::string buffTypeStr = buffTypeToDbString(buffType);
  const std::string tid = std::to_string(targetPlayerId);
  const std::string sid = std::to_string(sourcePlayerId);
  const std::string skillIdStr = std::to_string(skillId);
  const std::string durUsStr = std::to_string(static_cast<uint64_t>(durationMs) * 1000ULL);
  const std::string targetStatKey = effect.targetStat.empty() ? std::string("_default") : effect.targetStat;

  auto existing = db_->executePreparedQuery(
      "SELECT buff_id, current_stacks FROM active_buffs "
      "WHERE target_player_id = ? AND skill_id = ? AND buff_type = ? AND expires_at > NOW(3) "
      "AND COALESCE(JSON_UNQUOTE(JSON_EXTRACT(snapshot_json, '$.target_stat')), '_default') = ? "
      "ORDER BY buff_id DESC LIMIT 1",
      {tid, skillIdStr, buffTypeStr, targetStatKey});

  if (!existing.empty() && existing[0].size() >= 2) {
    const uint64_t existingId = std::stoull(existing[0][0]);
    uint8_t stacks = static_cast<uint8_t>(std::stoul(existing[0][1]));
    if (skill->isStackable && stacks < skill->maxStacks) {
      stacks = static_cast<uint8_t>(std::min<int>(stacks + 1, skill->maxStacks));
    }
    db_->executePreparedInsert(
        "UPDATE active_buffs SET current_stacks = ?, value_snapshot = ?, snapshot_json = ?, "
        "expires_at = DATE_ADD(NOW(3), INTERVAL ? MICROSECOND), started_at = NOW(3) "
        "WHERE buff_id = ?",
        {std::to_string(stacks), std::to_string(valueSnapshot), snapshotStr, durUsStr,
         std::to_string(existingId)});
    Core::Logger::getInstance().info(
        "[SkillService] buff refresh skill={} target={} stat={} buff_id={} stacks={}", skillId,
        targetPlayerId, effect.targetStat, existingId, static_cast<int>(stacks));
    return existingId;
  }

  if (!db_->executePreparedInsert(
          "INSERT INTO active_buffs (target_player_id, source_player_id, skill_id, buff_type, "
          "current_stacks, value_snapshot, expires_at, snapshot_json) VALUES "
          "(?, ?, ?, ?, 1, ?, DATE_ADD(NOW(3), INTERVAL ? MICROSECOND), ?)",
          {tid, sid, skillIdStr, buffTypeStr, std::to_string(valueSnapshot), durUsStr,
           snapshotStr})) {
    return 0;
  }

  const uint64_t newId = db_->getLastInsertId();
  Core::Logger::getInstance().info(
      "[SkillService] buff apply skill={} target={} stat={} buff_id={} type={} dur={}ms", skillId,
      targetPlayerId, effect.targetStat, newId, buffTypeStr, durationMs);
  return newId;
}

void SkillService::removeBuff(uint64_t playerId, uint64_t buffId) {
  if (!db_ || !db_->isConnected() || buffId == 0) return;
  db_->executePreparedInsert("DELETE FROM active_buffs WHERE buff_id = ? AND target_player_id = ?",
                             {std::to_string(buffId), std::to_string(playerId)});
}

void SkillService::removeBuffsBySkill(uint64_t playerId, uint32_t skillId) {
  if (!db_ || !db_->isConnected()) return;
  db_->executePreparedInsert("DELETE FROM active_buffs WHERE target_player_id = ? AND skill_id = ?",
                             {std::to_string(playerId), std::to_string(skillId)});
}

std::vector<SkillService::BuffExpirationEntry> SkillService::processBuffExpirations() {
  std::vector<BuffExpirationEntry> expired;
  if (!db_ || !db_->isConnected()) return expired;

  auto rows = db_->executePreparedQuery(
      "SELECT buff_id, target_player_id, skill_id, buff_type FROM active_buffs "
      "WHERE expires_at <= NOW(3) AND is_permanent = 0",
      {});

  for (const auto& row : rows) {
    if (row.size() < 4) continue;
    BuffExpirationEntry entry;
    try {
      entry.buffId = std::stoull(row[0]);
      entry.targetPlayerId = std::stoull(row[1]);
      entry.skillId = static_cast<uint32_t>(std::stoul(row[2]));
      entry.buffType = static_cast<uint8_t>(parseBuffTypeFromDb(row[3]));
    } catch (...) {
      continue;
    }
    expired.push_back(entry);
  }

  if (!expired.empty()) {
    db_->executePreparedInsert(
        "DELETE FROM active_buffs WHERE expires_at <= NOW(3) AND is_permanent = 0", {});
    Core::Logger::getInstance().info("[SkillService] {} buff(s) expirados removidos",
                                     expired.size());
  }
  return expired;
}

}  // namespace Combat
}  // namespace Umbra
