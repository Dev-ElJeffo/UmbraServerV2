#pragma once

#include <cstdint>

namespace Umbra {
namespace Services {

struct DamageResult {
  uint32_t damageDealt;
  bool isCritical;
  bool isDead;
};

class CombatService {
 public:
  DamageResult calculateDamage(uint32_t attackerPower, uint32_t defenderDefense);
  bool isInRange(float x1, float y1, float z1, float x2, float y2, float z2, float maxRange);
  float calculateExperience(uint32_t enemyLevel, uint32_t playerLevel);
};

}  // namespace Services
}  // namespace Umbra

