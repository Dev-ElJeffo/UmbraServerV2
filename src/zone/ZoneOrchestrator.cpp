#include "ZoneOrchestrator.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <cstdlib>

#ifndef _WIN32
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#endif

namespace Umbra {
namespace Zone {

ZoneOrchestrator::ZoneOrchestrator() {}

ZoneOrchestrator::~ZoneOrchestrator() {
  stop();
}

void ZoneOrchestrator::registerZone(const ZoneConfig& config) {
  std::lock_guard<std::mutex> lock(mutex_);
  zoneConfigs_[config.zoneName] = config;
  Core::Logger::getInstance().info("[Orchestrator] Registered zone '{}' (max={}/instance, threshold={}/{}, min={}, max={})",
                                   config.zoneName, config.maxPlayersPerInstance,
                                   config.spawnThreshold, config.despawnThreshold,
                                   config.minInstances, config.maxInstances);
}

void ZoneOrchestrator::updateInstanceLoad(uint32_t instanceId, uint32_t playerCount) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& inst : instances_) {
    if (inst.instanceId == instanceId) {
      inst.playerCount = playerCount;
      return;
    }
  }
}

void ZoneOrchestrator::start(float checkIntervalSeconds) {
  if (running_) return;
  checkInterval_ = checkIntervalSeconds;
  running_ = true;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [zoneName, config] : zoneConfigs_) {
      uint32_t existingCount = 0;
      for (const auto& inst : instances_) {
        if (inst.zoneName == zoneName) ++existingCount;
      }
      while (existingCount < config.minInstances) {
        spawnInstance(zoneName, config);
        ++existingCount;
      }
    }
  }

  monitorThread_ = std::make_unique<std::thread>(&ZoneOrchestrator::monitorLoop, this);
  Core::Logger::getInstance().info("[Orchestrator] Started (check interval={}s)", checkInterval_);
}

void ZoneOrchestrator::stop() {
  running_ = false;
  if (monitorThread_ && monitorThread_->joinable()) {
    monitorThread_->join();
  }

  std::lock_guard<std::mutex> lock(mutex_);
#ifndef _WIN32
  for (auto& inst : instances_) {
    if (inst.managed && inst.pid > 0) {
      Core::Logger::getInstance().info("[Orchestrator] Stopping instance {} (pid={}, zone='{}')",
                                       inst.instanceId, inst.pid, inst.zoneName);
      kill(inst.pid, SIGTERM);
    }
  }
#endif
  instances_.clear();
  Core::Logger::getInstance().info("[Orchestrator] Stopped, all managed instances terminated");
}

void ZoneOrchestrator::checkScaling() {
  std::lock_guard<std::mutex> lock(mutex_);

  for (const auto& [zoneName, config] : zoneConfigs_) {
    std::vector<ZoneInstance*> zoneInstances;
    uint32_t totalPlayers = 0;
    bool needsScaleUp = false;

    for (auto& inst : instances_) {
      if (inst.zoneName == zoneName) {
        zoneInstances.push_back(&inst);
        totalPlayers += inst.playerCount;
        if (inst.playerCount >= config.spawnThreshold) {
          needsScaleUp = true;
        }
      }
    }

    if (needsScaleUp && zoneInstances.size() < config.maxInstances) {
      Core::Logger::getInstance().info("[Orchestrator] Scaling UP zone '{}' ({} instances, {} total players)",
                                       zoneName, zoneInstances.size(), totalPlayers);
      spawnInstance(zoneName, config);
    }

    if (zoneInstances.size() > config.minInstances) {
      for (auto* inst : zoneInstances) {
        if (inst->playerCount <= config.despawnThreshold && inst->managed) {
          auto elapsed = std::chrono::steady_clock::now() - inst->startTime;
          if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() > 60) {
            if (zoneInstances.size() > config.minInstances) {
              Core::Logger::getInstance().info("[Orchestrator] Scaling DOWN zone '{}' instance {} ({} players)",
                                               zoneName, inst->instanceId, inst->playerCount);
              removeInstance(inst->instanceId);
              break;
            }
          }
        }
      }
    }
  }
}

uint16_t ZoneOrchestrator::allocatePort() {
  uint16_t maxPort = basePort_;
  for (const auto& inst : instances_) {
    if (inst.port >= maxPort) {
      maxPort = inst.port + 1;
    }
  }
  return maxPort;
}

bool ZoneOrchestrator::spawnInstance(const std::string& zoneName, const ZoneConfig& config) {
  uint32_t instanceId = nextInstanceId_++;
  uint16_t port = allocatePort();

#ifndef _WIN32
  pid_t pid = fork();
  if (pid == 0) {
    std::string portStr = std::to_string(port);
    std::string instanceStr = std::to_string(instanceId);
    execl(binaryPath_.c_str(), binaryPath_.c_str(),
          instanceStr.c_str(), portStr.c_str(), zoneName.c_str(), nullptr);
    _exit(1);
  } else if (pid < 0) {
    Core::Logger::getInstance().error("[Orchestrator] Failed to fork zone instance for '{}'", zoneName);
    return false;
  }
#else
  int pid = -1;
  std::string cmd = binaryPath_ + " " + std::to_string(instanceId) + " " +
                    std::to_string(port) + " " + zoneName;
  pid = static_cast<int>(system(cmd.c_str()));
#endif

  ZoneInstance inst;
  inst.instanceId = instanceId;
  inst.zoneName = zoneName;
  inst.port = port;
  inst.maxPlayers = config.maxPlayersPerInstance;
  inst.pid = pid;
  inst.managed = true;
  inst.startTime = std::chrono::steady_clock::now();
  instances_.push_back(inst);

  Core::Logger::getInstance().info("[Orchestrator] Spawned zone '{}' instance {} on port {} (pid={})",
                                   zoneName, instanceId, port, pid);

  if (onInstanceCreated_) {
    onInstanceCreated_(inst);
  }

  return true;
}

void ZoneOrchestrator::removeInstance(uint32_t instanceId) {
  auto it = std::find_if(instances_.begin(), instances_.end(),
    [instanceId](const ZoneInstance& i) { return i.instanceId == instanceId; });

  if (it == instances_.end()) return;

#ifndef _WIN32
  if (it->managed && it->pid > 0) {
    kill(it->pid, SIGTERM);
    Core::Logger::getInstance().info("[Orchestrator] Sent SIGTERM to instance {} (pid={})", instanceId, it->pid);
  }
#endif

  if (onInstanceRemoved_) {
    onInstanceRemoved_(instanceId);
  }

  instances_.erase(it);
}

void ZoneOrchestrator::monitorLoop() {
  while (running_) {
    auto sleepUntil = std::chrono::steady_clock::now() +
      std::chrono::milliseconds(static_cast<int>(checkInterval_ * 1000));

    checkScaling();

#ifndef _WIN32
    {
      std::lock_guard<std::mutex> lock(mutex_);
      for (auto it = instances_.begin(); it != instances_.end(); ) {
        if (it->managed && it->pid > 0) {
          int status = 0;
          pid_t result = waitpid(it->pid, &status, WNOHANG);
          if (result > 0) {
            Core::Logger::getInstance().warn("[Orchestrator] Instance {} (zone='{}', pid={}) exited with status {}",
                                             it->instanceId, it->zoneName, it->pid, WEXITSTATUS(status));
            if (onInstanceRemoved_) onInstanceRemoved_(it->instanceId);
            it = instances_.erase(it);
            continue;
          }
        }
        ++it;
      }
    }
#endif

    std::this_thread::sleep_until(sleepUntil);
  }
}

std::vector<ZoneInstance> ZoneOrchestrator::getInstances() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return instances_;
}

uint16_t ZoneOrchestrator::getAvailablePort(const std::string& zoneName) const {
  std::lock_guard<std::mutex> lock(mutex_);
  uint32_t bestLoad = UINT32_MAX;
  uint16_t bestPort = 0;

  for (const auto& inst : instances_) {
    if (inst.zoneName == zoneName && inst.playerCount < inst.maxPlayers) {
      if (inst.playerCount < bestLoad) {
        bestLoad = inst.playerCount;
        bestPort = inst.port;
      }
    }
  }

  return bestPort;
}

}  // namespace Zone
}  // namespace Umbra
