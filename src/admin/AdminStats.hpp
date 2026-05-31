#pragma once

#include <cstdint>

namespace Umbra {
namespace Admin {

struct ProcessStats {
  double cpuPct = 0.0;
  uint64_t memMb = 0;
  uint32_t threads = 0;
};

/** Coleta CPU/RAM/threads do processo atual. */
ProcessStats collectProcessStats();

}  // namespace Admin
}  // namespace Umbra
