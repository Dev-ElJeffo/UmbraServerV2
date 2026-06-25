#pragma once

#include "AdminServer.hpp"
#include "core/ConfigManager.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace Umbra {
namespace Admin {

/** Helper para iniciar AdminServer a partir de config/server.json. */
inline std::unique_ptr<AdminServer> createFromConfig(
    const std::string& serviceName,
    uint16_t port,
    std::function<void(int)> onShutdown = nullptr) {
  auto& cm = Core::ConfigManager::getInstance();
  if (!cm.get<bool>("admin.enabled", true)) {
    return nullptr;
  }

  AdminServer::Config cfg;
  cfg.enabled = true;
  cfg.port = port;
  cfg.sharedSecret = cm.get<std::string>("admin.shared_secret", "");
  cfg.serviceName = serviceName;
  cfg.configPath = "config/server.json";
  cfg.rateLimitPerMinute = cm.get<uint32_t>("admin.rate_limit_per_minute", 120);

  auto server = std::make_unique<AdminServer>(cfg);
  if (onShutdown) {
    server->getContext().requestShutdown = std::move(onShutdown);
  }
  return server;
}

}  // namespace Admin
}  // namespace Umbra
