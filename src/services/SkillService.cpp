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
      "cooldown_ms, cast_time_ms, range_max, can_crit "
      "FROM skills WHERE is_enabled = 1",
      {});

  for (const auto& row : rows) {
    if (row.size() < 19) continue;
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
