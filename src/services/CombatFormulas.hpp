// CombatFormulas.hpp — conversão de atributos (pontos) em % com retorno decrescente, x / (x + K).
// Constantes em combat_formula_settings (Manager → Fórmulas de combate); trocáveis em runtime.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <mutex>

namespace Umbra {
namespace Combat {

/** Linha única de combat_formula_settings (+ pvp_settings.pvp_damage_mult). Padrões = SQL. */
struct CombatFormulaSettings {
  double critBasePct = 5.0;
  double critBonusMaxPct = 70.0;
  double critK = 400.0;
  double critResistK = 400.0;
  double critCapPct = 75.0;
  double doubleMaxPct = 60.0;
  double doubleK = 400.0;
  double doubleResistK = 400.0;
  int32_t doubleDamagePct = 80;
  double hitBasePct = 80.0;
  double hitAccuracyBonusPct = 15.0;
  double hitAccuracyK = 300.0;
  double hitDodgeMaxPct = 40.0;
  double hitDodgeK = 300.0;
  double hitMinPct = 5.0;
  double hitMaxPct = 95.0;
  double defenseKBase = 500.0;
  double defenseKPerLevel = 50.0;
  double defenseCapPct = 75.0;
  double attributeScalingK = 300.0;
  double damageVariancePct = 7.0;
  /** pvp_settings.pvp_damage_mult. */
  double pvpDamageMult = 0.7;

  /** Garante K > 0 e faixas sãs (valores ruins do banco não derrubam o combate). */
  void sanitize() {
    auto k = [](double& v, double def) { if (!(v > 0.0)) v = def; };
    auto pct = [](double& v, double lo, double hi) { v = std::clamp(v, lo, hi); };
    k(critK, 400.0);
    k(critResistK, 400.0);
    k(doubleK, 400.0);
    k(doubleResistK, 400.0);
    k(hitAccuracyK, 300.0);
    k(hitDodgeK, 300.0);
    k(attributeScalingK, 300.0);
    if (!(defenseKBase > 0.0)) defenseKBase = 500.0;
    defenseKPerLevel = std::max(0.0, defenseKPerLevel);
    pct(critBasePct, 0.0, 100.0);
    pct(critBonusMaxPct, 0.0, 100.0);
    pct(critCapPct, 0.0, 100.0);
    pct(doubleMaxPct, 0.0, 100.0);
    doubleDamagePct = std::clamp(doubleDamagePct, 0, 500);
    pct(hitBasePct, 0.0, 100.0);
    pct(hitAccuracyBonusPct, 0.0, 100.0);
    pct(hitDodgeMaxPct, 0.0, 100.0);
    pct(hitMinPct, 0.0, 100.0);
    pct(hitMaxPct, hitMinPct, 100.0);
    pct(defenseCapPct, 0.0, 95.0);
    pct(damageVariancePct, 0.0, 50.0);
    pvpDamageMult = std::clamp(pvpDamageMult, 0.05, 5.0);
  }
};

namespace CombatFormulas {

/** Constantes em uso (cópia: leitura sem segurar lock durante o cálculo). */
inline std::mutex& settingsMutex() {
  static std::mutex m;
  return m;
}
inline CombatFormulaSettings& settingsStorage() {
  static CombatFormulaSettings s;
  return s;
}
inline CombatFormulaSettings settings() {
  std::lock_guard<std::mutex> lock(settingsMutex());
  return settingsStorage();
}
/** reload_combat_formulas; preserva o multiplicador PvP (vem de pvp_settings). */
inline void setSettings(CombatFormulaSettings s) {
  std::lock_guard<std::mutex> lock(settingsMutex());
  s.pvpDamageMult = settingsStorage().pvpDamageMult;
  s.sanitize();
  settingsStorage() = s;
}
inline void setPvpDamageMult(double mult) {
  std::lock_guard<std::mutex> lock(settingsMutex());
  settingsStorage().pvpDamageMult = std::clamp(mult, 0.05, 5.0);
}

/** x / (x + K), 0 para x <= 0. */
inline double ratio(double points, double k) {
  if (points <= 0.0 || k <= 0.0) return 0.0;
  return points / (points + k);
}

/** Chance de crítico (%) do atacante com C pontos contra R de resistência. */
inline double critChancePct(double critPoints, double resistPoints, const CombatFormulaSettings& s) {
  const double raw = s.critBasePct + s.critBonusMaxPct * ratio(critPoints, s.critK);
  const double afterResist = raw * (1.0 - ratio(resistPoints, s.critResistK));
  return std::clamp(afterResist, 0.0, s.critCapPct);
}

/** Chance de duplo (%). */
inline double doubleChancePct(double ratePoints, double resistPoints, const CombatFormulaSettings& s) {
  const double raw = s.doubleMaxPct * ratio(ratePoints, s.doubleK);
  return std::clamp(raw * (1.0 - ratio(resistPoints, s.doubleResistK)), 0.0, 100.0);
}

/** Chance de acerto (%); ignoreDodgePct (0-100) corta a esquiva do alvo antes da conversão. */
inline double hitChancePct(double accuracyPoints, double dodgePoints, int32_t ignoreDodgePct,
                           const CombatFormulaSettings& s) {
  const double effDodge = dodgePoints * (100.0 - std::clamp(ignoreDodgePct, 0, 100)) / 100.0;
  const double hit = s.hitBasePct + s.hitAccuracyBonusPct * ratio(accuracyPoints, s.hitAccuracyK) -
                     s.hitDodgeMaxPct * ratio(effDodge, s.hitDodgeK);
  return std::clamp(hit, s.hitMinPct, s.hitMaxPct);
}

/** Redução da defesa (0..cap/100). O K cresce com o nível do atacante. */
inline double defenseReduction(double defense, int32_t attackerLevel, const CombatFormulaSettings& s) {
  const double k = s.defenseKBase + s.defenseKPerLevel * std::max(1, attackerLevel);
  return std::min(ratio(defense, k), s.defenseCapPct / 100.0);
}

/** Bônus de atributo da skill (fração): escala% x S/(S+K). */
inline double attributeScaling(double statPoints, double scalingPct, const CombatFormulaSettings& s) {
  if (scalingPct <= 0.0) return 0.0;
  return (scalingPct / 100.0) * ratio(statPoints, s.attributeScalingK);
}

/** Aplica a variação: roll01 em [0,1] vira fator em [1 - v, 1 + v]. */
inline int32_t applyVariance(int32_t value, double roll01, const CombatFormulaSettings& s) {
  if (value <= 0 || s.damageVariancePct <= 0.0) return value;
  const double v = s.damageVariancePct / 100.0;
  const double factor = 1.0 - v + 2.0 * v * std::clamp(roll01, 0.0, 1.0);
  return std::max(1, static_cast<int32_t>(std::lround(static_cast<double>(value) * factor)));
}

}  // namespace CombatFormulas
}  // namespace Combat
}  // namespace Umbra
