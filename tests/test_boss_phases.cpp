// test_boss_phases.cpp — etapa 5b: limiares de fase (inclui pular fases), enrage e opcode 140.
#include <gtest/gtest.h>

#include "zone/BossController.hpp"
#include "zone/MovementProtocol.hpp"

using namespace Umbra::Zone;

namespace {
std::vector<BossPhaseDef> threePhases() {
  std::vector<BossPhaseDef> p(3);
  p[0].order = 1;
  p[0].hpThresholdPct = 100;
  p[1].order = 2;
  p[1].hpThresholdPct = 70;
  p[2].order = 3;
  p[2].hpThresholdPct = 40;
  return p;
}
}  // namespace

TEST(BossPhases, StaysInPhaseOneAboveFirstThreshold) {
  const auto phases = threePhases();
  EXPECT_EQ(bossPhaseIndexForHealth(phases, 1000, 1000), 0u);
  EXPECT_EQ(bossPhaseIndexForHealth(phases, 701, 1000), 0u);
}

TEST(BossPhases, EntersPhaseAtThreshold) {
  const auto phases = threePhases();
  EXPECT_EQ(bossPhaseIndexForHealth(phases, 700, 1000), 1u);  // <= 70%
  EXPECT_EQ(bossPhaseIndexForHealth(phases, 401, 1000), 1u);
  EXPECT_EQ(bossPhaseIndexForHealth(phases, 400, 1000), 2u);
}

TEST(BossPhases, BigHitSkipsToDeepestPhase) {
  // 80% → 30% num golpe: o controller aplica as fases 2 e 3 (D4); a função devolve a mais profunda.
  const auto phases = threePhases();
  EXPECT_EQ(bossPhaseIndexForHealth(phases, 300, 1000), 2u);
  EXPECT_EQ(bossPhaseIndexForHealth(phases, 0, 1000), 2u);
}

TEST(BossPhases, SinglePhaseOrInvalidHealth) {
  std::vector<BossPhaseDef> one(1);
  EXPECT_EQ(bossPhaseIndexForHealth(one, 10, 1000), 0u);
  EXPECT_EQ(bossPhaseIndexForHealth({}, 10, 1000), 0u);
  EXPECT_EQ(bossPhaseIndexForHealth(threePhases(), 10, 0), 0u);
}

TEST(BossPhases, LargeHealthDoesNotOverflow) {
  const auto phases = threePhases();
  EXPECT_EQ(bossPhaseIndexForHealth(phases, 1500000000, 2000000000), 0u);  // 75%
  EXPECT_EQ(bossPhaseIndexForHealth(phases, 700000000, 2000000000), 2u);   // 35%
}

TEST(BossEnrage, DueAfterSeconds) {
  EXPECT_FALSE(bossEnrageDue(0, 999999));
  EXPECT_FALSE(bossEnrageDue(120, 119999));
  EXPECT_TRUE(bossEnrageDue(120, 120000));
}

TEST(BossProtocol, StateRoundTrip) {
  BossStatePayload p;
  p.npcId = 0x40000010u;
  p.phase = 2;
  p.phaseCount = 3;
  p.flags = kBossFlagInCombat | kBossFlagShowBar | kBossFlagInvulnerable;
  p.enrageInSeconds = 150;
  p.invulnerableMs = 2000;
  p.phaseName = "Fúria";
  p.announce = "Vocês vão pagar por isso!";
  p.animPath = "/Game/Anims/Boss/AM_Roar.AM_Roar";
  p.vfxPath = "";
  p.thresholds = {70, 40};
  const auto data = encodeBossState(p);
  EXPECT_EQ(data[0], 140);
  BossStatePayload out;
  ASSERT_TRUE(decodeBossState(data, out));
  EXPECT_EQ(out.npcId, p.npcId);
  EXPECT_EQ(out.phase, 2);
  EXPECT_EQ(out.phaseCount, 3);
  EXPECT_EQ(out.flags, p.flags);
  EXPECT_EQ(out.enrageInSeconds, 150);
  EXPECT_EQ(out.invulnerableMs, 2000);
  EXPECT_EQ(out.phaseName, p.phaseName);
  EXPECT_EQ(out.announce, p.announce);
  EXPECT_EQ(out.animPath, p.animPath);
  ASSERT_EQ(out.thresholds.size(), 2u);
  EXPECT_EQ(out.thresholds[1], 40);
}
