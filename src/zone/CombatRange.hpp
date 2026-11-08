#pragma once

#include <cmath>

namespace Umbra {
namespace Zone {

/** Distancia ao quadrado entre dois pontos 3D (mesmas unidades WS/UE). */
inline float distanceSquared3D(float x1, float y1, float z1, float x2, float y2, float z2) {
  const float dx = x2 - x1;
  const float dy = y2 - y1;
  const float dz = z2 - z1;
  return dx * dx + dy * dy + dz * dz;
}

inline float distanceSquared2D(float x1, float y1, float x2, float y2) {
  const float dx = x2 - x1;
  const float dy = y2 - y1;
  return dx * dx + dy * dy;
}

inline bool isInRange3D(float x1, float y1, float z1, float x2, float y2, float z2, float maxRange) {
  if (maxRange <= 0.f) return true;
  const float maxSq = maxRange * maxRange;
  return distanceSquared3D(x1, y1, z1, x2, y2, z2) <= maxSq;
}

/** Range horizontal (XY). Usar vs NPC: Z do MySQL/home nao bate com landscape UE. */
inline bool isInRange2D(float x1, float y1, float x2, float y2, float maxRange) {
  if (maxRange <= 0.f) return true;
  const float maxSq = maxRange * maxRange;
  return distanceSquared2D(x1, y1, x2, y2) <= maxSq;
}

/** Margem para latencia / posicao defasada do caster no cliente. */
constexpr float kRangeLatencyTolerance = 1.10f;

/** Soma ao range vs NPC (centro-centro vs mesh/capsula ~42uu + folga). */
constexpr float kNpcCapsuleRangeMargin = 80.f;

/**
 * Folga pelo throttle do opcode 102 (~8 Hz): moveSpeed tipico 200 → ~25uu/tick.
 * Usa 50uu para cobrir 200–400 uu/s + jitter de rede.
 */
constexpr float kNpcAiBroadcastSlack = 50.f;

/** Hz do broadcast de estado NPC (NpcAiSystem). */
constexpr float kNpcStateBroadcastHz = 8.f;

inline float effectiveMaxRange(float rangeMax) {
  const float base = (rangeMax > 0.f) ? rangeMax : 100.f;
  return base * kRangeLatencyTolerance;
}

inline float effectiveMaxRangeVsNpc(float rangeMax, float moveSpeed = 200.f) {
  const float speed = (moveSpeed > 0.f) ? moveSpeed : 200.f;
  const float aiSlack = std::max(kNpcAiBroadcastSlack, speed / kNpcStateBroadcastHz);
  return effectiveMaxRange(rangeMax) + kNpcCapsuleRangeMargin + aiSlack;
}

}  // namespace Zone
}  // namespace Umbra
