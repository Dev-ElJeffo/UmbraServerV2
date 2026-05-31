#include "AdminServer.hpp"
#include "AdminCrypto.hpp"
#include "core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <cstring>

namespace Umbra {
namespace Admin {

AdminServer::AdminServer(const Config& config)
    : config_(config) {
  context_.serviceName = config.serviceName;
  context_.configPath = config.configPath;
  registerCommonCommands(registry_, context_);
}

AdminServer::~AdminServer() {
  stop();
}

CommandRegistry& AdminServer::getRegistry() {
  return registry_;
}

AdminContext& AdminServer::getContext() {
  return context_;
}

bool AdminServer::start() {
  if (!config_.enabled) {
    Core::Logger::getInstance().info("AdminServer disabled for {}", config_.serviceName);
    return true;
  }
  if (config_.sharedSecret.empty()) {
    Core::Logger::getInstance().warn("AdminServer: shared_secret empty, admin channel disabled");
    return true;
  }

  networkServer_ = std::make_unique<Network::SocketServer>(
      Network::ProtocolType::TCP, config_.port);
  networkServer_->setMaxConnections(16);
  networkServer_->setRateLimit(1000);
  networkServer_->setMessageCallback(
      [this](uint32_t clientId, const std::vector<uint8_t>& data) {
        handleMessage(clientId, data);
      });
  networkServer_->setConnectionCallback(
      [this](uint32_t clientId, bool connected) {
        if (connected) {
          std::lock_guard<std::mutex> lock(clientsMutex_);
          clients_[clientId] = ClientState{};
          Core::Logger::getInstance().debug("Admin client {} connected", clientId);
        } else {
          std::lock_guard<std::mutex> lock(clientsMutex_);
          clients_.erase(clientId);
        }
      });

  if (!networkServer_->start()) {
    Core::Logger::getInstance().error("AdminServer failed to start on port {}", config_.port);
    return false;
  }

  running_ = true;
  Core::Logger::getInstance().info("AdminServer '{}' listening on port {}",
                                   config_.serviceName, config_.port);
  return true;
}

void AdminServer::stop() {
  running_ = false;
  if (networkServer_) {
    networkServer_->stop();
    networkServer_.reset();
  }
  std::lock_guard<std::mutex> lock(clientsMutex_);
  clients_.clear();
}

bool AdminServer::isRunning() const {
  return running_ && networkServer_ && networkServer_->isRunning();
}

void AdminServer::handleMessage(uint32_t clientId, const std::vector<uint8_t>& data) {
  std::lock_guard<std::mutex> lock(clientsMutex_);
  auto it = clients_.find(clientId);
  if (it == clients_.end()) {
    return;
  }
  auto& state = it->second;
  state.buffer.insert(state.buffer.end(), data.begin(), data.end());

  while (state.buffer.size() >= 4) {
    uint32_t frameSize = 0;
    std::memcpy(&frameSize, state.buffer.data(), 4);
    if (frameSize == 0 || frameSize > 1024 * 1024) {
      sendError(clientId, "invalid frame size", true);
      networkServer_->disconnectClient(clientId);
      return;
    }
    if (state.buffer.size() < 4 + frameSize) {
      break;
    }
    std::vector<uint8_t> frame(state.buffer.begin() + 4,
                               state.buffer.begin() + 4 + frameSize);
    state.buffer.erase(state.buffer.begin(), state.buffer.begin() + 4 + frameSize);
    handleFrame(clientId, state, frame);
  }
}

void AdminServer::handleFrame(uint32_t clientId, ClientState& state,
                              const std::vector<uint8_t>& frame) {
  try {
    const std::string body(frame.begin(), frame.end());
    const auto json = nlohmann::json::parse(body);
    const std::string type = json.value("type", "");

    if (!state.authenticated) {
      if (type != "handshake") {
        sendError(clientId, "handshake required", true);
        networkServer_->disconnectClient(clientId);
        return;
      }
      if (!handleHandshake(clientId, state, json)) {
        sendError(clientId, "handshake failed", true);
        networkServer_->disconnectClient(clientId);
        return;
      }
      nlohmann::json ok;
      ok["success"] = true;
      ok["type"] = "handshake_ok";
      ok["service"] = config_.serviceName;
      sendJson(clientId, ok);
      return;
    }

    if (type != "command") {
      sendError(clientId, "expected command frame");
      return;
    }
    if (!checkRateLimit(state)) {
      sendError(clientId, "rate limit exceeded", true);
      networkServer_->disconnectClient(clientId);
      return;
    }
    handleCommand(clientId, state, json);
  } catch (const std::exception& e) {
    sendError(clientId, std::string("parse error: ") + e.what());
  }
}

bool AdminServer::handleHandshake(uint32_t clientId, ClientState& state,
                                  const nlohmann::json& req) {
  (void)clientId;
  const std::string nonce = req.value("nonce", "");
  const std::string hmac = req.value("hmac", "");
  if (nonce.empty() || hmac.empty()) {
    return false;
  }
  const std::string expected = hmacSha256Hex(config_.sharedSecret, nonce);
  if (expected != hmac) {
    Core::Logger::getInstance().warn("Admin handshake failed (bad HMAC)");
    return false;
  }
  state.authenticated = true;
  return true;
}

void AdminServer::handleCommand(uint32_t clientId, ClientState& state,
                                const nlohmann::json& req) {
  (void)state;
  const std::string cmd = req.value("cmd", "");
  const nlohmann::json args = req.value("args", nlohmann::json::object());

  nlohmann::json response;
  response["type"] = "response";
  response["cmd"] = cmd;

  try {
    if (!registry_.hasCommand(cmd)) {
      response["success"] = false;
      response["error"] = "unknown command: " + cmd;
    } else {
      response["success"] = true;
      response["data"] = registry_.execute(cmd, args);
    }
  } catch (const std::exception& e) {
    response["success"] = false;
    response["error"] = e.what();
  }

  sendJson(clientId, response);
}

void AdminServer::sendJson(uint32_t clientId, const nlohmann::json& payload) {
  const std::string body = payload.dump();
  const auto frame = encodeFrame(body);
  networkServer_->sendToClient(clientId, frame);
}

void AdminServer::sendError(uint32_t clientId, const std::string& message, bool closeAfter) {
  nlohmann::json err;
  err["success"] = false;
  err["error"] = message;
  sendJson(clientId, err);
  (void)closeAfter;
}

bool AdminServer::checkRateLimit(ClientState& state) {
  const auto now = std::chrono::steady_clock::now();
  if (state.windowStart.time_since_epoch().count() == 0) {
    state.windowStart = now;
    state.commandCount = 1;
    return true;
  }
  const auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - state.windowStart);
  if (elapsed.count() >= 1) {
    state.windowStart = now;
    state.commandCount = 1;
    return true;
  }
  ++state.commandCount;
  return state.commandCount <= config_.rateLimitPerMinute;
}

std::vector<uint8_t> AdminServer::encodeFrame(const std::string& jsonBody) {
  const uint32_t size = static_cast<uint32_t>(jsonBody.size());
  std::vector<uint8_t> frame(4 + jsonBody.size());
  std::memcpy(frame.data(), &size, 4);
  std::memcpy(frame.data() + 4, jsonBody.data(), jsonBody.size());
  return frame;
}

}  // namespace Admin
}  // namespace Umbra
