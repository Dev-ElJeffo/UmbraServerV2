#include <gtest/gtest.h>
#include "zone/CombatVitalsUtil.hpp"

using Umbra::Zone::applyVitalDelta;

TEST(CombatVitalsUtil, HotHealsWhenMaxAboveCurrent) {
  // Cenário que falhava com MAX(health): teto = HP atual impedia cura.
  EXPECT_EQ(applyVitalDelta(500, 1000, 50), 550);
}

TEST(CombatVitalsUtil, DamageClampsAtZero) {
  EXPECT_EQ(applyVitalDelta(80, 1000, -200), 0);
}

TEST(CombatVitalsUtil, HealClampsAtMax) {
  EXPECT_EQ(applyVitalDelta(980, 1000, 50), 1000);
}

TEST(CombatVitalsUtil, WrongMaxEqualToCurrentBlocksHeal) {
  // Documenta o bug anterior: MAX(health) devolvia o HP atual como teto.
  EXPECT_EQ(applyVitalDelta(500, 500, 50), 500);
}
