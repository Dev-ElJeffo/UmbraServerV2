#pragma once

#include "AdminCommands.hpp"
#include "network/SocketServer.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace Umbra {
namespace Admin {

/**
 * Canal TCP administrativo: framing 4 bytes LE + JSON UTF-8.
 * Handshake HMAC-SHA256 antes de aceitar comandos.
 */
class AdminServer {
 public:
  struct Config {
    bool enabled = true;
    uint16_t port = 9100;
    std::string sharedSecret;
    std::string serviceName = "umbra";
    std::string configPath = "config/server.json";
    uint32_t rateLimitPerMinute = 120;
  };

  explicit AdminServer(const Config& config);
  ~AdminServer();

  bool start();
  void stop();
  bool isRunning() const;

  CommandRegistry& getRegistry();
  AdminContext& getContext();

 private:
  struct ClientState {
    std::vector<uint8_t> buffer;
    bool authenticated = false;
    uint32_t commandCount = 0;
    std::chrono::steady_clock::time_point windowStart{};
  };

  Config config_;
  AdminContext context_;
  CommandRegistry registry_;
  std::unique_ptr<Network::SocketServer> networkServer_;
  std::map<uint32_t, ClientState> clients_;
  std::mutex clientsMutex_;
  std::atomic<bool> running_{false};

  void handleMessage(uint32_t clientId, const std::vector<uint8_t>& data);
  void handleFrame(uint32_t clientId, ClientState& state, const std::vector<uint8_t>& frame);
  bool handleHandshake(uint32_t clientId, ClientState& state, const nlohmann::json& req);
  void handleCommand(uint32_t clientId, ClientState& state, const nlohmann::json& req);
  void sendJson(uint32_t clientId, const nlohmann::json& payload);
  void sendError(uint32_t clientId, const std::string& message, bool closeAfter = false);
  bool checkRateLimit(ClientState& state);
  static std::vector<uint8_t> encodeFrame(const std::string& jsonBody);
};

}  // namespace Admin
}  // namespace Umbra
