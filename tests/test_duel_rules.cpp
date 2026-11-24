// test_duel_rules.cpp — regras puras do duelo (etapa 6): hostilidade, 1 de HP, área e protocolo 50-54.
#include <gtest/gtest.h>

#include "zone/DuelService.hpp"
#include "zone/MovementProtocol.hpp"

using namespace Umbra::Zone;

TEST(DuelRules, HostilityWithoutDuelFollowsOpenWorldFlag) {
  EXPECT_FALSE(DuelRules::canAffect(DuelRelation::None, true, false));
  EXPECT_TRUE(DuelRules::canAffect(DuelRelation::None, true, true));
  EXPECT_TRUE(DuelRules::canAffect(DuelRelation::None, false, false));  // cura fora de duelo: livre
}

TEST(DuelRules, OpponentsOnlyHurtEachOtherAfterCountdown) {
  EXPECT_TRUE(DuelRules::canAffect(DuelRelation::Fighting, true, false));
  EXPECT_FALSE(DuelRules::canAffect(DuelRelation::Fighting, false, false));  // não cura o oponente
  EXPECT_FALSE(DuelRules::canAffect(DuelRelation::Countdown, true, true));
  EXPECT_FALSE(DuelRules::canAffect(DuelRelation::Countdown, false, true));
}

TEST(DuelRules, OutsidersCannotInterfere) {
  EXPECT_FALSE(DuelRules::canAffect(DuelRelation::Interfering, true, true));
  EXPECT_FALSE(DuelRules::canAffect(DuelRelation::Interfering, false, true));
}

TEST(DuelRules, LethalHitLeavesOneHealth) {
  auto r = DuelRules::clampLethal(500, -800);
  EXPECT_EQ(r.first, -499);
  EXPECT_TRUE(r.second);
  r = DuelRules::clampLethal(500, -500);
  EXPECT_EQ(r.first, -499);
  EXPECT_TRUE(r.second);
  r = DuelRules::clampLethal(1, -50);
  EXPECT_EQ(r.first, 0);
  EXPECT_TRUE(r.second);
  r = DuelRules::clampLethal(500, -100);
  EXPECT_EQ(r.first, -100);
  EXPECT_FALSE(r.second);
  r = DuelRules::clampLethal(500, 100);
  EXPECT_EQ(r.first, 100);
  EXPECT_FALSE(r.second);
}

TEST(DuelRules, OutOfBoundsIsTwoDimensional) {
  EXPECT_FALSE(DuelRules::isOutOfBounds(0.f, 0.f, 2999.f, 0.f, 3000.f));
  EXPECT_TRUE(DuelRules::isOutOfBounds(0.f, 0.f, 2200.f, 2200.f, 3000.f));
  EXPECT_FALSE(DuelRules::isOutOfBounds(100.f, 100.f, 100.f, 100.f, 3000.f));
}

TEST(DuelProtocol, StartedRoundTripAndShortFrame) {
  DuelStartedPayload p;
  p.duelId = 7;
  p.player1Id = 11;
  p.player2Id = 22;
  p.flagX = 1.5f;
  p.flagY = -2.f;
  p.flagZ = 3.f;
  p.radius = 3000.f;
  p.countdownMs = 3000;
  p.maxDurationSec = 180;
  p.outOfBoundsSec = 5;
  p.opponentName = "Galáxia";
  p.flagVfxPath = "/Game/VFX/Duel/NS_DuelRing.NS_DuelRing";
  const auto data = encodeDuelStarted(p);
  DuelStartedPayload q;
  ASSERT_TRUE(decodeDuelStarted(data, q));
  EXPECT_EQ(q.duelId, 7u);
  EXPECT_EQ(q.player2Id, 22u);
  EXPECT_FLOAT_EQ(q.flagY, -2.f);
  EXPECT_EQ(q.countdownMs, 3000);
  EXPECT_EQ(q.opponentName, "Galáxia");
  EXPECT_TRUE(q.flagMeshPath.empty());
  EXPECT_EQ(q.flagVfxPath, p.flagVfxPath);

  // Frame sem os campos de texto (versão mínima) continua válido.
  std::vector<uint8_t> shortFrame(data.begin(), data.begin() + 1 + 12 + 16 + 5);
  ASSERT_TRUE(decodeDuelStarted(shortFrame, q));
  EXPECT_TRUE(q.opponentName.empty());
}

TEST(DuelProtocol, ResponseAndEndedRoundTrip) {
  uint8_t action = 9;
  uint32_t other = 0;
  ASSERT_TRUE(decodeDuelRequestResponse(encodeDuelRequestResponse(DuelResponseAction::Accept, 42), action, other));
  EXPECT_EQ(action, 1);
  EXPECT_EQ(other, 42u);
  std::vector<uint8_t> bad = encodeDuelRequestResponse(DuelResponseAction::Accept, 42);
  bad[1] = 9;
  EXPECT_FALSE(decodeDuelRequestResponse(bad, action, other));

  DuelEndedPayload e;
  e.duelId = 3;
  e.winnerId = 5;
  e.loserId = 6;
  e.reason = static_cast<uint8_t>(DuelEndReason::Fled);
  DuelEndedPayload f;
  ASSERT_TRUE(decodeDuelEnded(encodeDuelEnded(e), f));
  EXPECT_EQ(f.winnerId, 5u);
  EXPECT_EQ(f.reason, 2);
}
