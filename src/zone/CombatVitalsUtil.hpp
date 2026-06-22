#pragma once

#include <algorithm>
#include <cstdint>

namespace Umbra {
namespace Zone {

/** Aplica delta de vital clampado em [0, max]. Usado por DOT/HOT e regen. */
inline int32_t applyVitalDelta(int32_t current, int32_t max, int32_t delta) {
  const int32_t cap = std::max(1, max);
  return std::max(0, std::min(cap, current + delta));
}

}  // namespace Zone
}  // namespace Umbra
