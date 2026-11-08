#include "zone/CombatEffectRuntime.hpp"

#include "services/StatKeyMapping.hpp"

#include <algorithm>
#include <unordered_map>

namespace Umbra {
namespace Zone {
namespace {

bool isAttribute(const std::string& key) {
  return key == "strength" || key == "dexterity" || key == "intelligence" ||
         key == "vitality" || key == "luck";
}

void addDerivedDelta(const Combat::CharacterStats& base, Combat::CharacterStats& stats) {
  const auto physAttack = [](const Combat::CharacterStats& s) {
    return (s.strength / 5) * 2 + (s.dexterity / 10);
  };
  const auto magicAttack = [](const Combat::CharacterStats& s) {
    return (s.intelligence / 5) * 2;
  };
  const auto accuracy = [](const Combat::CharacterStats& s) { return s.dexterity / 5; };
  const auto dodge = [](const Combat::CharacterStats& s) { return s.dexterity / 10; };
  const auto critical = [](const Combat::CharacterStats& s) {
    return (s.strength / 10) + (s.intelligence / 10);
  };
  const auto criticalResist = [](const Combat::CharacterStats& s) { return s.vitality / 5; };
  const auto doubleRate = [](const Combat::CharacterStats& s) { return s.strength / 10; };
  const auto doubleResist = [](const Combat::CharacterStats& s) { return s.vitality / 10; };
  const auto health = [](const Combat::CharacterStats& s) { return (s.vitality / 10) * 30; };
  const auto mana = [](const Combat::CharacterStats& s) { return (s.intelligence / 10) * 30; };

  stats.physicalAttack += physAttack(stats) - physAttack(base);
  stats.magicAttack += magicAttack(stats) - magicAttack(base);
  stats.accuracy += accuracy(stats) - accuracy(base);
  stats.dodge += dodge(stats) - dodge(base);
  stats.criticalChance += critical(stats) - critical(base);
  stats.criticalResistance += criticalResist(stats) - criticalResist(base);
  stats.doubleAttackRate += doubleRate(stats) - doubleRate(base);
  stats.doubleAttackResistance += doubleResist(stats) - doubleResist(base);
  stats.maxHealth += health(stats) - health(base);
  stats.maxMana += mana(stats) - mana(base);
}

void clampStats(Combat::CharacterStats& stats) {
  stats.maxHealth = std::max(1, stats.maxHealth);
  stats.maxMana = std::max(0, stats.maxMana);
  stats.maxStamina = std::max(0, stats.maxStamina);
  stats.currentHealth = std::clamp(stats.currentHealth, 0, stats.maxHealth);
  stats.currentMana = std::clamp(stats.currentMana, 0, stats.maxMana);
  stats.currentStamina = std::clamp(stats.currentStamina, 0, stats.maxStamina);
  stats.physicalAttack = std::max(0, stats.physicalAttack);
  stats.magicAttack = std::max(0, stats.magicAttack);
  stats.physicalDefense = std::max(0, stats.physicalDefense);
  stats.magicDefense = std::max(0, stats.magicDefense);
  stats.accuracy = std::max(0, stats.accuracy);
  stats.dodge = std::max(0, stats.dodge);
  stats.criticalChance = std::clamp(stats.criticalChance, 0, 100);
  stats.criticalResistance = std::clamp(stats.criticalResistance, 0, 100);
  stats.doubleAttackRate = std::clamp(stats.doubleAttackRate, 0, 100);
  stats.doubleAttackResistance = std::clamp(stats.doubleAttackResistance, 0, 100);
  stats.movementSpeed = std::clamp(stats.movementSpeed, 10, 500);
  stats.damageReductionPercent = std::clamp(stats.damageReductionPercent, 0, 90);
  stats.stunResist = std::clamp(stats.stunResist, 0, 100);
  stats.silenceResist = std::clamp(stats.silenceResist, 0, 100);
  stats.rootResist = std::clamp(stats.rootResist, 0, 100);
  stats.slowResist = std::clamp(stats.slowResist, 0, 100);
  stats.stunChance = std::clamp(stats.stunChance, 0, 100);
  stats.silenceChance = std::clamp(stats.silenceChance, 0, 100);
  stats.rootChance = std::clamp(stats.rootChance, 0, 100);
  stats.slowChance = std::clamp(stats.slowChance, 0, 100);
  stats.physicalRes = std::clamp(stats.physicalRes, 0, 100);
  stats.shadowRes = std::clamp(stats.shadowRes, 0, 100);
  stats.fireRes = std::clamp(stats.fireRes, 0, 100);
  stats.holyRes = std::clamp(stats.holyRes, 0, 100);
  stats.poisonRes = std::clamp(stats.poisonRes, 0, 100);
  stats.iceRes = std::clamp(stats.iceRes, 0, 100);
  stats.lightningRes = std::clamp(stats.lightningRes, 0, 100);
  stats.arcaneRes = std::clamp(stats.arcaneRes, 0, 100);
}

}  // namespace

void CombatEffectRuntime::recompose(Combat::CharacterState& state,
                                    const std::vector<RuntimeStatEffect>& effects) {
  const int32_t currentHealth = state.baseStats.currentHealth;
  const int32_t currentMana = state.baseStats.currentMana;
  const int32_t currentStamina = state.baseStats.currentStamina;
  state.buffedStats = state.baseStats;
  state.isStunned = false;
  state.isSilenced = false;
  state.isRooted = false;
  state.isInvulnerable = false;
  state.isStealthed = false;
  state.currentShield = 0;
  state.maxShield = 0;

  std::unordered_map<std::string, int64_t> flats;
  std::unordered_map<std::string, int32_t> percents;
  for (const RuntimeStatEffect& effect : effects) {
    const int32_t stacks = std::max(1, static_cast<int32_t>(effect.stacks));
    switch (effect.type) {
      case Combat::EffectType::STUN: state.isStunned = true; break;
      case Combat::EffectType::SILENCE: state.isSilenced = true; break;
      case Combat::EffectType::ROOT: state.isRooted = true; break;
      case Combat::EffectType::INVULNERABLE: state.isInvulnerable = true; break;
      case Combat::EffectType::STEALTH: state.isStealthed = true; break;
      case Combat::EffectType::SLOW: {
        const int32_t slow = effect.valuePercent != 0 ? effect.valuePercent : effect.valueFlat;
        percents["movement"] += (slow > 0 ? -slow : slow) * stacks;
        break;
      }
      case Combat::EffectType::SHIELD:
        state.currentShield += std::max(0, effect.valueFlat) * stacks;
        break;
      case Combat::EffectType::BUFF_STAT:
      case Combat::EffectType::DEBUFF_STAT: {
        const std::string key =
            Combat::StatKeyMapping::mapTargetStatToCanonical(effect.targetStat);
        if (!key.empty()) {
          flats[key] += static_cast<int64_t>(effect.valueFlat) * stacks;
          percents[key] += effect.valuePercent * stacks;
        }
        break;
      }
      default: break;
    }
  }

  for (const auto& [key, value] : flats) {
    if (isAttribute(key)) {
      Combat::StatKeyMapping::applyFlatToCharacterStats(key, value, state.buffedStats);
    }
  }
  for (const auto& [key, value] : percents) {
    if (isAttribute(key)) {
      Combat::StatKeyMapping::applyPercentToCharacterStats(key, value, state.buffedStats);
    }
  }
  addDerivedDelta(state.baseStats, state.buffedStats);
  for (const auto& [key, value] : flats) {
    if (!isAttribute(key)) {
      Combat::StatKeyMapping::applyFlatToCharacterStats(key, value, state.buffedStats);
    }
  }
  for (const auto& [key, value] : percents) {
    if (!isAttribute(key)) {
      Combat::StatKeyMapping::applyPercentToCharacterStats(key, value, state.buffedStats);
    }
  }

  state.buffedStats.currentHealth = currentHealth;
  state.buffedStats.currentMana = currentMana;
  state.buffedStats.currentStamina = currentStamina;
  state.maxShield = state.currentShield;
  clampStats(state.buffedStats);
  state.isAlive = state.isAlive && state.buffedStats.currentHealth > 0;
}

int32_t CombatEffectRuntime::crowdControlResistance(const Combat::CharacterStats& stats,
                                                     Combat::EffectType type) {
  switch (type) {
    case Combat::EffectType::STUN: return stats.stunResist;
    case Combat::EffectType::SILENCE: return stats.silenceResist;
    case Combat::EffectType::ROOT: return stats.rootResist;
    case Combat::EffectType::SLOW: return stats.slowResist;
    default: return 0;
  }
}

int32_t CombatEffectRuntime::crowdControlChance(const Combat::CharacterStats& stats,
                                                 Combat::EffectType type) {
  switch (type) {
    case Combat::EffectType::STUN: return stats.stunChance;
    case Combat::EffectType::SILENCE: return stats.silenceChance;
    case Combat::EffectType::ROOT: return stats.rootChance;
    case Combat::EffectType::SLOW: return stats.slowChance;
    default: return 0;
  }
}

int32_t CombatEffectRuntime::finalCrowdControlChance(int32_t baseChance, int32_t casterChance,
                                                     int32_t targetResistance,
                                                     int32_t penetration) {
  const int32_t effectiveResistance = std::max(0, targetResistance - penetration);
  return std::clamp(baseChance + casterChance - effectiveResistance, 0, 100);
}

}  // namespace Zone
}  // namespace Umbra
