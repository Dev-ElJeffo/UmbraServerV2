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
  if (raw == "health") return "health_bonus";
  if (raw == "mana") return "mana_bonus";
  if (raw == "damage_reduction") return "damage_reduction";
  if (raw == "all_resistance") return "resistance";
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

/** Percentual sobre stats de combate já derivados (CharacterStats). */
inline void applyPercentToCharacterStats(const std::string& canonicalKey, int32_t pct,
                                         CharacterStats& stats) {
  if (pct == 0 || canonicalKey.empty()) return;
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
  else if (canonicalKey == "damage_reduction") mod(stats.damageReduction);
}

/** Chaves percentuais que devem ser aplicadas no mapa totals (antes de max HP/MP). */
inline bool isTotalsPercentKey(const std::string& canonicalKey) {
  return canonicalKey == "health_bonus" || canonicalKey == "mana_bonus" ||
         canonicalKey == "strength" || canonicalKey == "dexterity" ||
         canonicalKey == "intelligence" || canonicalKey == "vitality" || canonicalKey == "luck";
}

}  // namespace StatKeyMapping
}  // namespace Combat
}  // namespace Umbra
