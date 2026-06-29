#include <gtest/gtest.h>
#include <algorithm>
#include <cstdint>

namespace {

int32_t clampVitalDelta(int32_t current, int32_t max, int32_t delta) {
  return std::max(0, std::min(max, current + delta));
}

}  // namespace

TEST(CombatVitalsMath, HotHealUsesMaxHealthNotCurrent) {
  EXPECT_EQ(clampVitalDelta(50, 200, 10), 60);
  EXPECT_EQ(clampVitalDelta(195, 200, 10), 200);
}

TEST(CombatVitalsMath, WrongMaxEqualsCurrentBlocksHeal) {
  // Regressao: SELECT MAX(health) com id unico devolve HP atual, nao max_health.
  EXPECT_EQ(clampVitalDelta(50, 50, 10), 50);
}

TEST(CombatVitalsMath, DotDamageClampsAtZero) {
  EXPECT_EQ(clampVitalDelta(5, 200, -10), 0);
}
