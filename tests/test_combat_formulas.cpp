// test_combat_formulas.cpp — fórmulas de combate com retorno decrescente (CombatFormulas.hpp).
// Exemplos com os atributos reais do QA do duelo (ElJeffo #1 x LoMartial #32, nível 50).
#include <gtest/gtest.h>

#include "services/CombatFormulas.hpp"

using namespace Umbra::Combat;

namespace {
const CombatFormulaSettings kDefaults{};
}

TEST(CombatFormulas, CritNoLongerZeroedByHigherResistance) {
  // LoMartial (crit 509) x ElJeffo (res 204): antes 100 - 100 = 0%.
  const double a = CombatFormulas::critChancePct(509, 204, kDefaults);
  EXPECT_NEAR(a, 29.0, 1.0);
  // ElJeffo (crit 40) x LoMartial (res 500): antes 0%; agora pequeno mas positivo.
  const double b = CombatFormulas::critChancePct(40, 500, kDefaults);
  EXPECT_GT(b, 3.0);
  EXPECT_LT(b, 8.0);
}

TEST(CombatFormulas, CritBaseAndCap) {
  EXPECT_DOUBLE_EQ(CombatFormulas::critChancePct(0, 0, kDefaults), 5.0);
  EXPECT_LE(CombatFormulas::critChancePct(1e9, 0, kDefaults), kDefaults.critCapPct);
  // Mais pontos sempre ajudam (monotônico), mesmo contra resistência enorme.
  EXPECT_GT(CombatFormulas::critChancePct(800, 2000, kDefaults), CombatFormulas::critChancePct(400, 2000, kDefaults));
}

TEST(CombatFormulas, DoubleChance) {
  EXPECT_DOUBLE_EQ(CombatFormulas::doubleChancePct(0, 0, kDefaults), 0.0);
  const double a = CombatFormulas::doubleChancePct(392, 53, kDefaults);
  EXPECT_NEAR(a, 26.2, 1.0);
}

TEST(CombatFormulas, HitChanceRange) {
  EXPECT_DOUBLE_EQ(CombatFormulas::hitChancePct(0, 0, 0, kDefaults), 80.0);
  const double hit = CombatFormulas::hitChancePct(475, 76, 0, kDefaults);
  EXPECT_GT(hit, 80.0);
  EXPECT_LT(hit, 90.0);
  EXPECT_GE(CombatFormulas::hitChancePct(0, 1e9, 0, kDefaults), kDefaults.hitMinPct);
  // ignore_dodge 100% anula a esquiva.
  EXPECT_DOUBLE_EQ(CombatFormulas::hitChancePct(0, 5000, 100, kDefaults), 80.0);
}

TEST(CombatFormulas, DefenseScalesWithAttackerLevel) {
  // DEF ~1900 contra atacante nível 50: K = 500 + 2500 = 3000 → ~39% (antes 90% fixo no teto).
  EXPECT_NEAR(CombatFormulas::defenseReduction(1900, 50, kDefaults), 0.388, 0.01);
  // A mesma DEF protege mais contra nível baixo.
  EXPECT_GT(CombatFormulas::defenseReduction(1900, 10, kDefaults), CombatFormulas::defenseReduction(1900, 50, kDefaults));
  EXPECT_LE(CombatFormulas::defenseReduction(1e9, 1, kDefaults), kDefaults.defenseCapPct / 100.0);
  EXPECT_DOUBLE_EQ(CombatFormulas::defenseReduction(0, 50, kDefaults), 0.0);
}

TEST(CombatFormulas, AttributeScalingIsBounded) {
  // Skill 96 do LoMartial (STR 35%, DEX 25%): antes +309%; agora ~+38%.
  const double total = CombatFormulas::attributeScaling(462, 35, kDefaults) +
                       CombatFormulas::attributeScaling(614, 25, kDefaults);
  EXPECT_NEAR(total, 0.38, 0.02);
  EXPECT_DOUBLE_EQ(CombatFormulas::attributeScaling(500, 0, kDefaults), 0.0);
}

TEST(CombatFormulas, VarianceRange) {
  EXPECT_EQ(CombatFormulas::applyVariance(1000, 0.0, kDefaults), 930);
  EXPECT_EQ(CombatFormulas::applyVariance(1000, 1.0, kDefaults), 1070);
  EXPECT_EQ(CombatFormulas::applyVariance(1000, 0.5, kDefaults), 1000);
  CombatFormulaSettings fixed = kDefaults;
  fixed.damageVariancePct = 0;
  EXPECT_EQ(CombatFormulas::applyVariance(1000, 0.0, fixed), 1000);
  EXPECT_EQ(CombatFormulas::applyVariance(1, 0.0, kDefaults), 1);  // nunca vira 0
}

TEST(CombatFormulas, SanitizeRejectsBadConstants) {
  CombatFormulaSettings s;
  s.critK = 0;
  s.defenseCapPct = 300;
  s.hitMinPct = 50;
  s.hitMaxPct = 10;
  s.sanitize();
  EXPECT_GT(s.critK, 0.0);
  EXPECT_LE(s.defenseCapPct, 95.0);
  EXPECT_GE(s.hitMaxPct, s.hitMinPct);
}
