#pragma once

#include "services/SkillTypes.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace Umbra {
namespace Zone {

/** Representação mínima de um efeito temporário ativo, independente do alvo. */
struct RuntimeStatEffect {
  Combat::EffectType type = Combat::EffectType::BUFF_STAT;
  std::string targetStat;
  int32_t valueFlat = 0;
  int32_t valuePercent = 0;
  uint8_t stacks = 1;
};

/**
 * Recomposição determinística do estado usado pelo combate.
 *
 * A ordem é sempre: base + flats, percentuais acumulados e derivados de atributos.
 * O método parte de baseStats, portanto um snapshot reidratado nunca é aplicado duas vezes.
 */
class CombatEffectRuntime {
public:
  static void recompose(Combat::CharacterState& state,
                        const std::vector<RuntimeStatEffect>& effects);

  static int32_t crowdControlResistance(const Combat::CharacterStats& stats,
                                        Combat::EffectType type);
  static int32_t crowdControlChance(const Combat::CharacterStats& stats,
                                    Combat::EffectType type);
  static int32_t finalCrowdControlChance(int32_t baseChance, int32_t casterChance,
                                        int32_t targetResistance, int32_t penetration);
};

}  // namespace Zone
}  // namespace Umbra
