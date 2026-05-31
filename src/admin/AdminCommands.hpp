#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <nlohmann/json.hpp>

namespace Umbra {
namespace Admin {

using AdminCommandHandler = std::function<nlohmann::json(const nlohmann::json& args)>;

/**
 * Registro de comandos admin (ping, stats, kick, etc.).
 */
class CommandRegistry {
 public:
  void registerCommand(const std::string& name, AdminCommandHandler handler);
  bool hasCommand(const std::string& name) const;
  nlohmann::json execute(const std::string& name, const nlohmann::json& args) const;

 private:
  mutable std::mutex mutex_;
  std::map<std::string, AdminCommandHandler> commands_;
};

struct AdminContext {
  std::string serviceName;
  std::string version = "1.3.0";
  std::string configPath = "config/server.json";
  std::function<void(int graceSec)> requestShutdown;
};

/** Comandos comuns: ping, stats, set_log_level, reload_config, shutdown. */
void registerCommonCommands(CommandRegistry& registry, AdminContext& ctx);

}  // namespace Admin
}  // namespace Umbra
