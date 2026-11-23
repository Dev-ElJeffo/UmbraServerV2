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

// R1: progressão por rank — valor(r) = base × (1 + crescimento × (r − 1)).
TEST(SkillRankProgressionTests, DamageGrowsFiftyPercentAtRankFive) {
  Umbra::Combat::SkillData skill;
  skill.powerCoef = 180;
  Umbra::Combat::SkillEffect dmg;
  dmg.effectType = EffectType::DAMAGE;
  dmg.valuePercent = 180;
  skill.effects = {dmg};
  EXPECT_EQ(skill.buildEffectsForRank(1)[0].valuePercent, 180);
  EXPECT_EQ(skill.buildEffectsForRank(3)[0].valuePercent, 225);
  EXPECT_EQ(skill.buildEffectsForRank(5)[0].valuePercent, 270);
  EXPECT_EQ(skill.getEffectivePowerCoef(5), 270);
  EXPECT_FLOAT_EQ(skill.getRankBonusFactor(5), 1.0f);
}

TEST(SkillRankProgressionTests, BuffDebuffAndControlScale) {
  Umbra::Combat::SkillData skill;
  Umbra::Combat::SkillEffect buff;
  buff.effectType = EffectType::BUFF_STAT;
  buff.valuePercent = 20;
  buff.valueFlat = 30;
  buff.durationMs = 10000;
  Umbra::Combat::SkillEffect debuff;
  debuff.effectType = EffectType::DEBUFF_STAT;
  debuff.valuePercent = -25;
  Umbra::Combat::SkillEffect stun;
  stun.effectType = EffectType::STUN;
  stun.durationMs = 1500;
  stun.chancePercent = 50;
  Umbra::Combat::SkillEffect teleport;
  teleport.effectType = EffectType::TELEPORT;
  teleport.valueFlat = 500;
  skill.effects = {buff, debuff, stun, teleport};
  const auto r5 = skill.buildEffectsForRank(5);
  EXPECT_EQ(r5[0].valuePercent, 28);   // +40%
  EXPECT_EQ(r5[0].valueFlat, 42);
  EXPECT_EQ(r5[0].durationMs, 10000U); // duração de buff não escala sem duration_bonus_ms
  EXPECT_EQ(r5[1].valuePercent, -35);  // debuff cresce em módulo
  EXPECT_EQ(r5[2].durationMs, 2100U);  // controle: +10%/rank na duração
  EXPECT_EQ(r5[2].chancePercent, 70);  // +5 pontos/rank
  EXPECT_EQ(r5[3].valueFlat, 500);     // TELEPORT não escala
}

TEST(SkillRankProgressionTests, OverridesAndDurationBonus) {
  Umbra::Combat::SkillData skill;
  skill.rankGrowthTenths = 0;  // skill que não escala
  Umbra::Combat::SkillEffect buff;
  buff.effectType = EffectType::BUFF_STAT;
  buff.valuePercent = 20;
  buff.durationMs = 8000;
  Umbra::Combat::SkillEffect heal;
  heal.effectType = EffectType::HEAL;
  heal.valuePercent = 20;
  heal.rankGrowthTenths = 200;  // efeito sobrescreve: 20%/rank
  skill.effects = {buff, heal};
  Umbra::Combat::SkillRankScaling row;
  row.rank = 4;
  row.durationBonusMs = 2000;
  skill.rankScalings = {row};
  const auto r4 = skill.buildEffectsForRank(4);
  EXPECT_EQ(r4[0].valuePercent, 20);
  EXPECT_EQ(r4[0].durationMs, 10000U);  // duration_bonus_ms agora vale no efeito
  EXPECT_EQ(r4[1].valuePercent, 32);    // 20 × 1,6
}

// R3: buffs temporários somam no máximo +50 de esquiva.
TEST(CombatEffectRuntimeTests, DodgeFromEffectsIsCapped) {
  CharacterState state;
  state.baseStats.dodge = 10;
  state.baseStats.maxHealth = 100;
  state.baseStats.currentHealth = 100;
  std::vector<RuntimeStatEffect> effects;
  RuntimeStatEffect smoke;
  smoke.type = EffectType::BUFF_STAT;
  smoke.targetStat = "dodge";
  smoke.valueFlat = 80;
  effects.push_back(smoke);
  CombatEffectRuntime::recompose(state, effects);
  EXPECT_EQ(state.buffedStats.dodge, 10 + CombatEffectRuntime::kMaxDodgeFromEffects);
}
