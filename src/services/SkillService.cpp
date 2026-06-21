#include "SkillService.hpp"
#include <algorithm>

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

}  // namespace

bool SkillService::loadSkillsFromDatabase() {
  if (!db_ || !db_->isConnected()) return false;

  std::unique_lock<std::shared_mutex> lock(skillDataMutex_);
  skillDataById_.clear();
  skillIdByKey_.clear();
  skillIdsByClass_.clear();

  auto rows = db_->executePreparedQuery(
      "SELECT skill_id, skill_key, skill_name, class_id, skill_order, required_level, skill_cost, max_rank, "
      "type_id, target_id, element_id, scaling_stat_id, power_coef, resource_type, resource_cost, "
      "cooldown_ms, cast_time_ms, range_max, can_crit, COALESCE(effects_json,'') "
      "FROM skills WHERE is_enabled = 1",
      {});

  for (const auto& row : rows) {
    if (row.size() < 20) continue;
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
      skill.powerCoef = static_cast<uint16_t>(std::stoul(row[12]));
      skill.resourceType = parseResourceType(row[13]);
      skill.resourceCost = static_cast<uint16_t>(std::stoul(row[14]));
      skill.cooldownMs = static_cast<uint32_t>(std::stoul(row[15]));
      skill.castTimeMs = static_cast<uint32_t>(std::stoul(row[16]));
      skill.rangeMax = static_cast<uint16_t>(std::stoul(row[17]));
      skill.canCrit = (std::stoi(row[18]) != 0);
      skill.effects = parseEffectsFromJson(row[19]);
    } catch (...) {
      continue;
    }

    skillDataById_[skill.skillId] = skill;
    if (!skill.skillKey.empty()) {
      skillIdByKey_[skill.skillKey] = skill.skillId;
    }
    skillIdsByClass_[skill.classId].push_back(skill.skillId);
  }

  Core::Logger::getInstance().info("[SkillService] {} skills carregadas do DB", skillDataById_.size());
  return true;
}

bool SkillService::reloadSkills() {
  return loadSkillsFromDatabase();
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

  if (skill->resourceType == ResourceType::MANA &&
      source.baseStats.currentMana < static_cast<int32_t>(skill->resourceCost)) {
    result.errorCode = "NO_MANA";
    result.errorMessage = "Mana insuficiente";
    return result;
  }

  result.isValid = true;
  return result;
}

}  // namespace Combat
}  // namespace Umbra
