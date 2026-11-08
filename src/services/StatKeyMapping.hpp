#pragma once

#include "SkillTypes.hpp"

#include <string>
#include <unordered_map>

namespace Umbra {
namespace Combat {
namespace StatKeyMapping {

/** Mapeia target_stat do SQL/effects_json para chave canônica do loader (totals map). */
inline std::string mapTargetStatToCanonical(const std::string& raw) {
  if (raw.empty()) return raw;
  // ATK físico
  if (raw == "physical_attack" || raw == "physicalAttack" || raw == "phys_atk" || raw == "atk" ||
      raw == "attack") {
    return "attack";
  }
  // DEF físico
  if (raw == "physical_defense" || raw == "physicalDefense" || raw == "phys_def" || raw == "def" ||
      raw == "defense") {
    return "defense";
  }
  // ATK mágico
  if (raw == "magic_attack" || raw == "magicAttack" || raw == "mag_atk" || raw == "matk") {
    return "magic_attack";
  }
  // DEF mágico
  if (raw == "magic_defense" || raw == "magicDefense" || raw == "mag_def" || raw == "mdef") {
    return "magic_defense";
  }
  if (raw == "movement_speed") return "movement";
  if (raw == "critical_chance") return "critical";
  if (raw == "critical_resistance") return "resistance";
  if (raw == "double_attack_chance") return "double_attack_rate";
  if (raw == "max_health") return "health_bonus";
  if (raw == "max_mana") return "mana_bonus";
  if (raw == "max_stamina" || raw == "stamina") return "stamina_bonus";
  if (raw == "health") return "health_bonus";
  if (raw == "mana") return "mana_bonus";
  if (raw == "damage_reduction") return "damage_reduction";
  if (raw == "all_resistance" || raw == "all_resist") return "all_resistance";
  if (raw == "physical_resistance" || raw == "physical_resist") return "physical_resistance";
  if (raw == "shadow_resistance" || raw == "shadow_resist") return "shadow_resistance";
  if (raw == "fire_resistance" || raw == "fire_resist") return "fire_resistance";
  if (raw == "holy_resistance" || raw == "holy_resist") return "holy_resistance";
  if (raw == "poison_resistance" || raw == "poison_resist") return "poison_resistance";
  if (raw == "ice_resistance" || raw == "ice_resist") return "ice_resistance";
  if (raw == "lightning_resistance" || raw == "lightning_resist") return "lightning_resistance";
  if (raw == "arcane_resistance" || raw == "arcane_resist") return "arcane_resistance";
  if (raw == "healing_bonus") return "healing_bonus";
  if (raw == "stun_resist" || raw == "stunResist") return "stun_resist";
  if (raw == "silence_resist" || raw == "silenceResist") return "silence_resist";
  if (raw == "root_resist" || raw == "rootResist") return "root_resist";
  if (raw == "slow_resist" || raw == "slowResist") return "slow_resist";
  if (raw == "stun_chance" || raw == "stunChance") return "stun_chance";
  if (raw == "silence_chance" || raw == "silenceChance") return "silence_chance";
  if (raw == "root_chance" || raw == "rootChance") return "root_chance";
  if (raw == "slow_chance" || raw == "slowChance") return "slow_chance";
  return raw;
}

/** Aplica bônus flat na mapa de totais (antes dos derivados de atributo). */
inline void applyFlatToTotals(const std::string& canonicalKey, int64_t flat,
                              std::unordered_map<std::string, int64_t>& totals) {
  if (flat == 0 || canonicalKey.empty()) return;
  auto it = totals.find(canonicalKey);
  if (it != totals.end()) {
    it->second += flat;
    return;
  }
  totals[canonicalKey] = flat;
}

/** Percentual sobre chaves do mapa totals (health_bonus/mana_bonus/atributos) antes do max final. */
inline void applyPercentToTotals(const std::string& canonicalKey, int32_t pct,
                                 std::unordered_map<std::string, int64_t>& totals) {
  if (pct == 0 || canonicalKey.empty()) return;
  auto it = totals.find(canonicalKey);
  if (it == totals.end()) return;
  it->second += static_cast<int64_t>(it->second) * pct / 100;
}

/** Aplica bônus/penalidade flat direto em CharacterStats (overlay runtime). */
inline void applyFlatToCharacterStats(const std::string& canonicalKey, int64_t flat,
                                      CharacterStats& stats) {
  if (flat == 0 || canonicalKey.empty()) return;
  auto add = [&](int32_t& v) { v = static_cast<int32_t>(static_cast<int64_t>(v) + flat); };
  if (canonicalKey == "attack") add(stats.physicalAttack);
  else if (canonicalKey == "magic_attack") add(stats.magicAttack);
  else if (canonicalKey == "defense") add(stats.physicalDefense);
  else if (canonicalKey == "magic_defense") add(stats.magicDefense);
  else if (canonicalKey == "accuracy") add(stats.accuracy);
  else if (canonicalKey == "dodge") add(stats.dodge);
  else if (canonicalKey == "critical") add(stats.criticalChance);
  else if (canonicalKey == "resistance") add(stats.criticalResistance);
  else if (canonicalKey == "double_attack_rate") add(stats.doubleAttackRate);
  else if (canonicalKey == "double_attack_resistance") add(stats.doubleAttackResistance);
  else if (canonicalKey == "movement") add(stats.movementSpeed);
  else if (canonicalKey == "strength") add(stats.strength);
  else if (canonicalKey == "dexterity") add(stats.dexterity);
  else if (canonicalKey == "intelligence") add(stats.intelligence);
  else if (canonicalKey == "vitality") add(stats.vitality);
  else if (canonicalKey == "luck") add(stats.luck);
  else if (canonicalKey == "health_bonus") add(stats.maxHealth);
  else if (canonicalKey == "mana_bonus") add(stats.maxMana);
  else if (canonicalKey == "stamina_bonus") add(stats.maxStamina);
  else if (canonicalKey == "damage_reduction") add(stats.damageReduction);
  else if (canonicalKey == "healing_bonus") add(stats.healingBonus);
  else if (canonicalKey == "physical_resistance") add(stats.physicalRes);
  else if (canonicalKey == "shadow_resistance") add(stats.shadowRes);
  else if (canonicalKey == "fire_resistance") add(stats.fireRes);
  else if (canonicalKey == "holy_resistance") add(stats.holyRes);
  else if (canonicalKey == "poison_resistance") add(stats.poisonRes);
  else if (canonicalKey == "ice_resistance") add(stats.iceRes);
  else if (canonicalKey == "lightning_resistance") add(stats.lightningRes);
  else if (canonicalKey == "arcane_resistance") add(stats.arcaneRes);
  else if (canonicalKey == "all_resistance") {
    add(stats.physicalRes); add(stats.shadowRes); add(stats.fireRes); add(stats.holyRes);
    add(stats.poisonRes); add(stats.iceRes); add(stats.lightningRes); add(stats.arcaneRes);
  }
  else if (canonicalKey == "stun_resist") add(stats.stunResist);
  else if (canonicalKey == "silence_resist") add(stats.silenceResist);
  else if (canonicalKey == "root_resist") add(stats.rootResist);
  else if (canonicalKey == "slow_resist") add(stats.slowResist);
  else if (canonicalKey == "stun_chance") add(stats.stunChance);
  else if (canonicalKey == "silence_chance") add(stats.silenceChance);
  else if (canonicalKey == "root_chance") add(stats.rootChance);
  else if (canonicalKey == "slow_chance") add(stats.slowChance);
}

/** Percentual sobre stats de combate já derivados (CharacterStats). */
inline void applyPercentToCharacterStats(const std::string& canonicalKey, int32_t pct,
                                         CharacterStats& stats) {
  if (pct == 0 || canonicalKey.empty()) return;
  // damage_reduction % = redução percentual de dano recebido (não % sobre flat 0).
  if (canonicalKey == "damage_reduction") {
    stats.damageReductionPercent += pct;
    return;
  }
  auto mod = [&](int32_t& v) { v += static_cast<int32_t>(static_cast<int64_t>(v) * pct / 100); };
  if (canonicalKey == "attack") mod(stats.physicalAttack);
  else if (canonicalKey == "magic_attack") mod(stats.magicAttack);
  else if (canonicalKey == "defense") mod(stats.physicalDefense);
  else if (canonicalKey == "magic_defense") mod(stats.magicDefense);
  else if (canonicalKey == "accuracy") mod(stats.accuracy);
  else if (canonicalKey == "dodge") mod(stats.dodge);
  else if (canonicalKey == "critical") mod(stats.criticalChance);
  else if (canonicalKey == "resistance") mod(stats.criticalResistance);
  else if (canonicalKey == "double_attack_rate") mod(stats.doubleAttackRate);
  else if (canonicalKey == "double_attack_resistance") mod(stats.doubleAttackResistance);
  else if (canonicalKey == "movement") mod(stats.movementSpeed);
  else if (canonicalKey == "strength") mod(stats.strength);
  else if (canonicalKey == "dexterity") mod(stats.dexterity);
  else if (canonicalKey == "intelligence") mod(stats.intelligence);
  else if (canonicalKey == "vitality") mod(stats.vitality);
  else if (canonicalKey == "luck") mod(stats.luck);
  else if (canonicalKey == "health_bonus") mod(stats.maxHealth);
  else if (canonicalKey == "mana_bonus") mod(stats.maxMana);
  else if (canonicalKey == "stamina_bonus") mod(stats.maxStamina);
  else if (canonicalKey == "healing_bonus") mod(stats.healingBonus);
  else if (canonicalKey == "physical_resistance") mod(stats.physicalRes);
  else if (canonicalKey == "shadow_resistance") mod(stats.shadowRes);
  else if (canonicalKey == "fire_resistance") mod(stats.fireRes);
  else if (canonicalKey == "holy_resistance") mod(stats.holyRes);
  else if (canonicalKey == "poison_resistance") mod(stats.poisonRes);
  else if (canonicalKey == "ice_resistance") mod(stats.iceRes);
  else if (canonicalKey == "lightning_resistance") mod(stats.lightningRes);
  else if (canonicalKey == "arcane_resistance") mod(stats.arcaneRes);
  else if (canonicalKey == "all_resistance") {
    mod(stats.physicalRes); mod(stats.shadowRes); mod(stats.fireRes); mod(stats.holyRes);
    mod(stats.poisonRes); mod(stats.iceRes); mod(stats.lightningRes); mod(stats.arcaneRes);
  }
  else if (canonicalKey == "stun_resist") mod(stats.stunResist);
  else if (canonicalKey == "silence_resist") mod(stats.silenceResist);
  else if (canonicalKey == "root_resist") mod(stats.rootResist);
  else if (canonicalKey == "slow_resist") mod(stats.slowResist);
  else if (canonicalKey == "stun_chance") mod(stats.stunChance);
  else if (canonicalKey == "silence_chance") mod(stats.silenceChance);
  else if (canonicalKey == "root_chance") mod(stats.rootChance);
  else if (canonicalKey == "slow_chance") mod(stats.slowChance);
}

/** Chaves percentuais que devem ser aplicadas no mapa totals (antes de max HP/MP). */
inline bool isTotalsPercentKey(const std::string& canonicalKey) {
  return canonicalKey == "health_bonus" || canonicalKey == "mana_bonus" ||
         canonicalKey == "stamina_bonus" ||
         canonicalKey == "strength" || canonicalKey == "dexterity" ||
         canonicalKey == "intelligence" || canonicalKey == "vitality" || canonicalKey == "luck";
}

}  // namespace StatKeyMapping
}  // namespace Combat
}  // namespace Umbra
