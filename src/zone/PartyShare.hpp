#pragma once

#include "zone/CombatRange.hpp"
#include "zone/MovementServer.hpp"
#include <cmath>
#include <cstdint>
#include <functional>
#include <unordered_set>
#include <vector>

namespace Umbra {
namespace Zone {

/** Raio padrão de share de kill/EXP em party (50 m). */
constexpr float kDefaultPartyShareRadiusUu = 5000.f;

/**
 * Killer + membros do party dentro de shareRadiusUu (XY 2D) da posição do killer.
 * Sem pos válida / stub (≈0,0,0) / offline na zone → excluído (exceto o killer).
 */
inline std::vector<uint32_t> collectPartyShareRecipients(
    uint32_t killerPlayerId, float shareRadiusUu, MovementServer* movementServer,
    const std::function<std::vector<uint32_t>(uint32_t)>& resolvePartyMembers) {
  std::vector<uint32_t> out;
  if (killerPlayerId == 0) {
    return out;
  }
  out.push_back(killerPlayerId);
  if (!movementServer || shareRadiusUu <= 0.f) {
    return out;
  }

  const auto players = movementServer->getPlayerStates();
  const auto kit = players.find(killerPlayerId);
  if (kit == players.end()) {
    return out;
  }
  const float kx = kit->second.x;
  const float ky = kit->second.y;
  const float kz = kit->second.z;
  const bool killerStub =
      (std::fabs(kx) < 1.f && std::fabs(ky) < 1.f && std::fabs(kz) < 1.f);
  if (killerStub) {
    return out;
  }

  std::vector<uint32_t> members;
  if (resolvePartyMembers) {
    members = resolvePartyMembers(killerPlayerId);
  }
  if (members.empty()) {
    return out;
  }

  std::unordered_set<uint32_t> seen;
  seen.insert(killerPlayerId);
  for (uint32_t mid : members) {
    if (mid == 0 || !seen.insert(mid).second) {
      continue;
    }
    const auto pit = players.find(mid);
    if (pit == players.end()) {
      continue;
    }
    const float x = pit->second.x;
    const float y = pit->second.y;
    const float z = pit->second.z;
    if (std::fabs(x) < 1.f && std::fabs(y) < 1.f && std::fabs(z) < 1.f) {
      continue;
    }
    if (isInRange2D(kx, ky, x, y, shareRadiusUu)) {
      out.push_back(mid);
    }
  }
  return out;
}

}  // namespace Zone
}  // namespace Umbra
