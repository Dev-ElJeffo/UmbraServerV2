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

inline bool isInRange3D(float x1, float y1, float z1, float x2, float y2, float z2, float maxRange) {
  if (maxRange <= 0.f) return true;
  const float maxSq = maxRange * maxRange;
  return distanceSquared3D(x1, y1, z1, x2, y2, z2) <= maxSq;
}

/** Margem para latencia / posicao defasada do caster no cliente. */
constexpr float kRangeLatencyTolerance = 1.10f;

inline float effectiveMaxRange(float rangeMax) {
  const float base = (rangeMax > 0.f) ? rangeMax : 100.f;
  return base * kRangeLatencyTolerance;
}

}  // namespace Zone
}  // namespace Umbra
