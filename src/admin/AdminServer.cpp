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
      Network::ProtocolType::TCP, config_.port, config_.bindHost);
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
    Core::Logger::getInstance().error("AdminServer failed to start on {}:{}",
                                      config_.bindHost, config_.port);
    return false;
  }

  running_ = true;
  Core::Logger::getInstance().info("AdminServer '{}' listening on {}:{}",
                                   config_.serviceName, config_.bindHost, config_.port);
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
  std::vector<std::vector<uint8_t>> frames;
  bool disconnect = false;
  {
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
        disconnect = true;
        break;
      }
      if (state.buffer.size() < 4 + frameSize) {
        break;
      }
      frames.emplace_back(state.buffer.begin() + 4,
                          state.buffer.begin() + 4 + frameSize);
      state.buffer.erase(state.buffer.begin(), state.buffer.begin() + 4 + frameSize);
    }
  }

  if (disconnect) {
    sendError(clientId, "invalid frame size", true);
    if (networkServer_) {
      networkServer_->disconnectClient(clientId);
    }
    return;
  }

  for (const auto& frame : frames) {
    handleFrame(clientId, frame);
  }
}

void AdminServer::handleFrame(uint32_t clientId, const std::vector<uint8_t>& frame) {
  ClientState snapshot;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    auto it = clients_.find(clientId);
    if (it == clients_.end()) {
      return;
    }
    snapshot = it->second;
  }

  try {
    const std::string body(frame.begin(), frame.end());
    const auto json = nlohmann::json::parse(body);
    const std::string type = json.value("type", "");

    if (!snapshot.authenticated) {
      if (type != "handshake") {
        sendError(clientId, "handshake required", true);
        return;
      }
      if (!handleHandshake(clientId, json)) {
        sendError(clientId, "handshake failed", true);
        return;
      }
      nlohmann::json ok;
      ok["success"] = true;
      ok["type"] = "handshake_ok";
      ok["service"] = config_.serviceName;
      if (json.contains("request_id")) {
        ok["request_id"] = json["request_id"];
      }
      sendJson(clientId, ok);
      return;
    }

    if (type != "command") {
      sendError(clientId, "expected command frame");
      return;
    }

    bool rateOk = false;
    {
      std::lock_guard<std::mutex> lock(clientsMutex_);
      auto it = clients_.find(clientId);
      if (it == clients_.end()) {
        return;
      }
      rateOk = checkRateLimit(it->second);
    }
    if (!rateOk) {
      sendError(clientId, "rate limit exceeded", true);
      return;
    }
    handleCommand(clientId, json);
  } catch (const std::exception& e) {
    sendError(clientId, std::string("parse error: ") + e.what());
  }
}

bool AdminServer::handleHandshake(uint32_t clientId, const nlohmann::json& req) {
  const std::string nonce = req.value("nonce", "");
  const std::string hmac = req.value("hmac", "");
  if (nonce.empty() || hmac.empty()) {
    return false;
  }
  const std::string expected = hmacSha256Hex(config_.sharedSecret, nonce);
  if (!hmacEquals(expected, hmac)) {
    Core::Logger::getInstance().warn("Admin handshake failed (bad HMAC)");
    return false;
  }
  std::lock_guard<std::mutex> lock(clientsMutex_);
  auto it = clients_.find(clientId);
  if (it == clients_.end()) {
    return false;
  }
  it->second.authenticated = true;
  return true;
}

void AdminServer::handleCommand(uint32_t clientId, const nlohmann::json& req) {
  const std::string cmd = req.value("cmd", "");
  const nlohmann::json args = req.value("args", nlohmann::json::object());

  nlohmann::json response;
  response["type"] = "response";
  response["cmd"] = cmd;
  if (req.contains("request_id")) {
    response["request_id"] = req["request_id"];
  }

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
  if (!networkServer_) return;
  const std::string body = payload.dump();
  const auto frame = encodeFrame(body);
  networkServer_->sendToClient(clientId, frame);
}

void AdminServer::sendError(uint32_t clientId, const std::string& message, bool closeAfter) {
  nlohmann::json err;
  err["success"] = false;
  err["error"] = message;
  sendJson(clientId, err);
  if (closeAfter && networkServer_) {
    networkServer_->disconnectClient(clientId);
  }
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
