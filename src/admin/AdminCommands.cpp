#include "AdminCommands.hpp"
#include "AdminStats.hpp"
#include "core/ConfigManager.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include <chrono>
#include <stdexcept>

namespace Umbra {
namespace Admin {

void CommandRegistry::registerCommand(const std::string& name, AdminCommandHandler handler) {
  std::lock_guard<std::mutex> lock(mutex_);
  commands_[name] = std::move(handler);
}

bool CommandRegistry::hasCommand(const std::string& name) const {
  std::lock_guard<std::mutex> lock(mutex_);
  return commands_.find(name) != commands_.end();
}

nlohmann::json CommandRegistry::execute(const std::string& name, const nlohmann::json& args) const {
  AdminCommandHandler handler;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = commands_.find(name);
    if (it == commands_.end()) {
      throw std::runtime_error("unknown command: " + name);
    }
    handler = it->second;
  }
  return handler(args);
}

void registerCommonCommands(CommandRegistry& registry, AdminContext& ctx) {
  static auto startTime = std::chrono::steady_clock::now();

  registry.registerCommand("ping", [&ctx](const nlohmann::json&) {
    nlohmann::json data;
    data["pong"] = true;
    data["service"] = ctx.serviceName;
    data["version"] = ctx.version;
    data["ts"] = Core::Utils::getCurrentTimestamp();
    return data;
  });

  registry.registerCommand("stats", [&ctx](const nlohmann::json&) {
    const auto now = std::chrono::steady_clock::now();
    const auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
    const ProcessStats ps = collectProcessStats();
    nlohmann::json data;
    data["service"] = ctx.serviceName;
    data["uptime_s"] = uptime;
    data["cpu_pct"] = ps.cpuPct;
    data["mem_mb"] = ps.memMb;
    data["threads"] = ps.threads;
    data["version"] = ctx.version;
    return data;
  });

  registry.registerCommand("set_log_level", [](const nlohmann::json& args) {
    const std::string level = args.value("level", "INFO");
    Core::Logger::Level lv = Core::Logger::Level::INFO;
    if (level == "DEBUG") lv = Core::Logger::Level::DEBUG;
    else if (level == "WARN") lv = Core::Logger::Level::WARN;
    else if (level == "ERROR") lv = Core::Logger::Level::ERROR;
    else if (level == "CRITICAL") lv = Core::Logger::Level::CRITICAL;
    Core::Logger::getInstance().setLevel(lv);
    nlohmann::json data;
    data["level"] = level;
    return data;
  });

  registry.registerCommand("reload_config", [&ctx](const nlohmann::json&) {
    auto& cm = Core::ConfigManager::getInstance();
    const bool ok = cm.reload();
    nlohmann::json data;
    data["reloaded"] = ok;
    data["path"] = ctx.configPath;
    return data;
  });

  registry.registerCommand("shutdown", [&ctx](const nlohmann::json& args) {
    const int grace = args.value("grace_sec", 3);
    if (ctx.requestShutdown) {
      ctx.requestShutdown(grace);
    }
    nlohmann::json data;
    data["grace_sec"] = grace;
    data["scheduled"] = ctx.requestShutdown != nullptr;
    return data;
  });
}

}  // namespace Admin
}  // namespace Umbra
