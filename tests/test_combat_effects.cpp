#include "zone/CombatEffectRuntime.hpp"

#include <gtest/gtest.h>

#include <iterator>

using Umbra::Combat::CharacterState;
using Umbra::Combat::EffectType;
using Umbra::Zone::CombatEffectRuntime;
using Umbra::Zone::RuntimeStatEffect;

TEST(CombatEffectRuntimeTests, EnumeratesAllTwentyFiveEffects) {
  const EffectType effects[] = {
      EffectType::DAMAGE,           EffectType::HEAL,
      EffectType::SHIELD,           EffectType::BUFF_STAT,
      EffectType::DEBUFF_STAT,      EffectType::DOT,
      EffectType::HOT,              EffectType::CLEANSE,
      EffectType::DISPEL,           EffectType::STUN,
      EffectType::SILENCE,          EffectType::SLOW,
      EffectType::ROOT,             EffectType::KNOCKBACK,
      EffectType::TAUNT,            EffectType::STEALTH,
      EffectType::INVULNERABLE,     EffectType::LIFESTEAL,
      EffectType::MANASTEAL,        EffectType::SUMMON,
      EffectType::TELEPORT,         EffectType::EXECUTE,
      EffectType::REFLECT,          EffectType::COOLDOWN_RESET,
      EffectType::RESOURCE_RESTORE,
  };
  EXPECT_EQ(std::size(effects), 25U);
}

TEST(CombatEffectRuntimeTests, RecomposeAppliesFlatThenPercentAndDerivedStats) {
  CharacterState state;
  state.baseStats.strength = 20;
  state.baseStats.dexterity = 20;
  state.baseStats.physicalAttack = 100;
  state.baseStats.maxHealth = 1000;
  state.baseStats.currentHealth = 700;

  std::vector<RuntimeStatEffect> effects = {
      {EffectType::BUFF_STAT, "strength", 10, 0, 1},
      {EffectType::BUFF_STAT, "physical_attack", 20, 50, 1},
  };
  CombatEffectRuntime::recompose(state, effects);

  // +4 de derivado por STR (20 -> 30), depois (104 + 20) * 1,5.
  EXPECT_EQ(state.buffedStats.physicalAttack, 186);
  EXPECT_EQ(state.buffedStats.currentHealth, 700);

  // Recompor novamente parte da base, sem aplicar o snapshot duas vezes.
  CombatEffectRuntime::recompose(state, effects);
  EXPECT_EQ(state.buffedStats.physicalAttack, 186);
}

TEST(CombatEffectRuntimeTests, CrowdControlFlagsSlowAndShieldAreAuthoritative) {
  CharacterState state;
  state.baseStats.movementSpeed = 100;
  state.baseStats.currentHealth = 100;

  CombatEffectRuntime::recompose(
      state, {{EffectType::STUN, "", 0, 0, 1},
              {EffectType::ROOT, "", 0, 0, 1},
              {EffectType::SLOW, "", 0, 40, 1},
              {EffectType::SHIELD, "", 250, 0, 2},
              {EffectType::INVULNERABLE, "", 0, 0, 1},
              {EffectType::STEALTH, "", 0, 0, 1}});

  EXPECT_FALSE(state.canMove());
  EXPECT_FALSE(state.canUseSkill());
  EXPECT_EQ(state.buffedStats.movementSpeed, 60);
  EXPECT_EQ(state.currentShield, 500);
  EXPECT_TRUE(state.isInvulnerable);
  EXPECT_TRUE(state.isStealthed);
}

TEST(CombatEffectRuntimeTests, CrowdControlFormulaClampsResistanceAndPenetration) {
  EXPECT_EQ(CombatEffectRuntime::finalCrowdControlChance(60, 10, 0, 0), 70);
  EXPECT_EQ(CombatEffectRuntime::finalCrowdControlChance(60, 0, 100, 0), 0);
  EXPECT_EQ(CombatEffectRuntime::finalCrowdControlChance(60, 0, 100, 50), 10);
  EXPECT_EQ(CombatEffectRuntime::finalCrowdControlChance(100, 100, 0, 0), 100);
}
