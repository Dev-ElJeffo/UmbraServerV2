#include "AdminStats.hpp"
#include <chrono>
#include <thread>

#ifdef _WIN32
  #include <windows.h>
  #include <psapi.h>
#else
  #include <fstream>
  #include <sstream>
  #include <unistd.h>
#endif

namespace Umbra {
namespace Admin {

namespace {
#ifdef _WIN32
  ULARGE_INTEGER fileTimeToLargeInteger(const FILETIME& ft) {
    ULARGE_INTEGER li{};
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return li;
  }

  static ULARGE_INTEGER g_lastCpu{};
  static ULARGE_INTEGER g_lastSys{};
  static bool g_hasLast = false;
#endif
}  // namespace

ProcessStats collectProcessStats() {
  ProcessStats stats{};
  stats.threads = static_cast<uint32_t>(std::thread::hardware_concurrency());

#ifdef _WIN32
  HANDLE process = GetCurrentProcess();
  PROCESS_MEMORY_COUNTERS pmc{};
  if (GetProcessMemoryInfo(process, &pmc, sizeof(pmc))) {
    stats.memMb = pmc.WorkingSetSize / (1024 * 1024);
  }

  FILETIME createTime{}, exitTime{}, kernelTime{}, userTime{};
  if (GetProcessTimes(process, &createTime, &exitTime, &kernelTime, &userTime)) {
    ULARGE_INTEGER kernel = fileTimeToLargeInteger(kernelTime);
    ULARGE_INTEGER user = fileTimeToLargeInteger(userTime);
    ULARGE_INTEGER total{};
    total.QuadPart = kernel.QuadPart + user.QuadPart;

    FILETIME sysIdle{}, sysKernel{}, sysUser{};
    if (GetSystemTimes(&sysIdle, &sysKernel, &sysUser)) {
      ULARGE_INTEGER sysKernelLi = fileTimeToLargeInteger(sysKernel);
      ULARGE_INTEGER sysUserLi = fileTimeToLargeInteger(sysUser);
      ULARGE_INTEGER sysTotal{};
      sysTotal.QuadPart = sysKernelLi.QuadPart + sysUserLi.QuadPart;

      if (g_hasLast) {
        const double procDelta = static_cast<double>(total.QuadPart - g_lastCpu.QuadPart);
        const double sysDelta = static_cast<double>(sysTotal.QuadPart - g_lastSys.QuadPart);
        if (sysDelta > 0.0) {
          stats.cpuPct = (procDelta / sysDelta) * 100.0;
          if (stats.cpuPct < 0.0) stats.cpuPct = 0.0;
          if (stats.cpuPct > 100.0 * static_cast<double>(stats.threads)) {
            stats.cpuPct = 100.0;
          }
        }
      }
      g_lastCpu = total;
      g_lastSys = sysTotal;
      g_hasLast = true;
    }
  }
#else
  std::ifstream status("/proc/self/status");
  std::string line;
  while (std::getline(status, line)) {
    if (line.rfind("VmRSS:", 0) == 0) {
      std::istringstream iss(line.substr(6));
      long kb = 0;
      iss >> kb;
      stats.memMb = static_cast<uint64_t>(kb / 1024);
    } else if (line.rfind("Threads:", 0) == 0) {
      std::istringstream iss(line.substr(8));
      iss >> stats.threads;
    }
  }
  stats.cpuPct = 0.0;
#endif

  return stats;
}

}  // namespace Admin
}  // namespace Umbra
