#include "CombatService.hpp"
#include <cmath>
#include <random>

namespace Umbra {
namespace Services {

DamageResult CombatService::calculateDamage(uint32_t attackerPower, 
                                            uint32_t defenderDefense) {
  DamageResult result;
  
  float baseDamage = static_cast<float>(attackerPower) - (defenderDefense * 0.5f);
  if (baseDamage < 1.0f) baseDamage = 1.0f;
  
  // Random variance
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.9, 1.1);
  
  baseDamage *= dis(gen);
  
  // Critical hit chance (10%)
  std::uniform_int_distribution<> critDis(0, 99);
  result.isCritical = (critDis(gen) < 10);
  
  if (result.isCritical) {
    baseDamage *= 2.0f;
  }
  
  result.damageDealt = static_cast<uint32_t>(baseDamage);
  result.isDead = false;
  
  return result;
}

bool CombatService::isInRange(float x1, float y1, float z1, 
                              float x2, float y2, float z2, 
                              float maxRange) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  float dz = z2 - z1;
  
  float distanceSquared = dx*dx + dy*dy + dz*dz;
  float maxRangeSquared = maxRange * maxRange;
  
  return distanceSquared <= maxRangeSquared;
}

float CombatService::calculateExperience(uint32_t enemyLevel, uint32_t playerLevel) {
  float baseExp = static_cast<float>(enemyLevel) * 10.0f;
  
  int32_t levelDiff = static_cast<int32_t>(enemyLevel) - static_cast<int32_t>(playerLevel);
  
  if (levelDiff > 5) {
    baseExp *= 1.5f;
  } else if (levelDiff < -5) {
    baseExp *= 0.5f;
  }
  
  return baseExp;
}

}  // namespace Services
}  // namespace Umbra

