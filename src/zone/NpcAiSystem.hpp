#pragma once

#include <cstdint>

namespace Umbra {
namespace Zone {

class NpcManager;
class MovementServer;
class CombatCoreEngine;

/**
 * AI autoritativa de mobs: wander circular, aggro por raio/dano, chase, leash e basic attack.
 * Roda no tick da zone; sync de posição via opcode 102 (throttle).
 */
class NpcAiSystem {
public:
  NpcAiSystem(NpcManager* npcManager, MovementServer* movementServer, CombatCoreEngine* combat);

  void tick(float deltaSeconds);

private:
  NpcManager* npcManager_ = nullptr;
  MovementServer* movementServer_ = nullptr;
  CombatCoreEngine* combat_ = nullptr;
};

}  // namespace Zone
}  // namespace Umbra
