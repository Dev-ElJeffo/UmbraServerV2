#include "AuthServer.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include <nlohmann/json.hpp>

namespace Umbra {
namespace Auth {

AuthServer::AuthServer(const Config& config,
                       std::shared_ptr<Database::MySQLConnector> dbConnector)
    : config_(config),
      dbConnector_(dbConnector) {
  
  jwtManager_ = std::make_unique<JWTManager>(config.jwtSecret);
  sessionManager_ = std::make_unique<SessionManager>();
  accountDAO_ = std::make_unique<Database::AccountDAO>(dbConnector);
  playerDAO_ = std::make_unique<Database::PlayerDAO>(dbConnector);
  
  networkServer_ = std::make_unique<Network::SocketServer>(
    Network::ProtocolType::TCP, config.port);
  
  networkServer_->setMessageCallback(
    [this](uint32_t clientId, const std::vector<uint8_t>& data) {
      handleClientMessage(clientId, data);
    });

  networkServer_->setConnectionCallback(
    [this](uint32_t clientId, bool connected) {
      if (!connected) {
        std::lock_guard<std::mutex> lock(recvBuffersMutex_);
        recvBuffers_.erase(clientId);
      }
    });
}

AuthServer::~AuthServer() {
  stop();
}

bool AuthServer::start() {
  if (!dbConnector_->isConnected()) {
    if (!dbConnector_->connect()) {
      Core::Logger::getInstance().error("Failed to connect to database");
      return false;
    }
  }
  
  if (!networkServer_->start()) {
    Core::Logger::getInstance().error("Failed to start auth network server");
    return false;
  }
  
  Core::Logger::getInstance().info("AuthServer started on port {}", config_.port);
  return true;
}

void AuthServer::stop() {
  if (networkServer_) {
    networkServer_->stop();
  }
  
  Core::Logger::getInstance().info("AuthServer stopped");
}

bool AuthServer::isRunning() const {
  return networkServer_ && networkServer_->isRunning();
}

AuthResult AuthServer::registerAccount(const std::string& username,
                                       const std::string& email,
                                       const std::string& password) {
  AuthResult result;
  
  // Validate inputs
  if (username.length() < 3 || username.length() > 20) {
    result.message = "Username must be between 3 and 20 characters";
    return result;
  }
  
  if (!Core::Utils::isValidEmail(email)) {
    result.message = "Invalid email format";
    return result;
  }
  
  if (password.length() < 6) {
    result.message = "Password must be at least 6 characters";
    return result;
  }
  
  // Check if username exists
  if (accountDAO_->usernameExists(username)) {
    result.message = "Username already exists";
    return result;
  }
  
  // Check if email exists
  if (accountDAO_->emailExists(email)) {
    result.message = "Email already registered";
    return result;
  }
  
  // Create account
  // Com novo formato PBKDF2, o salt já está incluído no hash
  Database::Account account;
  account.username = username;
  account.email = email;
  account.passwordHash = Core::Utils::hashPassword(password); // Gera salt automaticamente
  // Manter salt vazio ou usar valor placeholder para compatibilidade com schema atual
  account.salt = ""; // Salt agora está dentro do passwordHash no formato $pbkdf2$...
  account.createdAt = std::chrono::system_clock::now();
  
  uint64_t accountId = accountDAO_->createAccount(account);
  
  if (accountId == 0) {
    result.message = "Failed to create account";
    return result;
  }
  
  result.success = true;
  result.message = "Account created successfully";
  result.accountId = accountId;
  result.username = username;
  
  Core::Logger::getInstance().info("New account registered: {} (ID: {})", 
                                   username, accountId);
  
  return result;
}

AuthResult AuthServer::login(const std::string& usernameOrEmail,
                             const std::string& password,
                             const std::string& ipAddress) {
  AuthResult result;
  
  // Check login attempts
  if (!checkLoginAttempts(usernameOrEmail)) {
    result.message = "Too many login attempts. Please try again later.";
    return result;
  }
  
  // Find account
  auto account = accountDAO_->getAccountByUsername(usernameOrEmail);
  if (!account) {
    account = accountDAO_->getAccountByEmail(usernameOrEmail);
  }
  
  if (!account) {
    recordLoginAttempt(usernameOrEmail, false);
    result.message = "Invalid credentials";
    return result;
  }
  
  // Check if banned
  if (account->banned) {
    result.message = "Account is banned: " + account->banReason;
    return result;
  }
  
  // Verify password (usa verifyPassword para lidar com novo formato PBKDF2)
  if (!Core::Utils::verifyPassword(password, account->passwordHash)) {
    recordLoginAttempt(usernameOrEmail, false);
    result.message = "Invalid credentials";
    return result;
  }
  
  // Get player (assuming first character for now)
  auto players = playerDAO_->getPlayersByAccountId(account->id);
  uint64_t playerId = players.empty() ? 0 : players[0].id;
  
  // Generate token
  std::string token = jwtManager_->generateToken(
    account->id, playerId, account->username, config_.sessionDurationMinutes);
  
  // Create session
  sessionManager_->createSession(
    account->id, playerId, ipAddress, config_.sessionDurationMinutes);
  
  // Update last login
  accountDAO_->updateLastLogin(account->id);
  
  recordLoginAttempt(usernameOrEmail, true);
  
  result.success = true;
  result.message = "Login successful";
  result.token = token;
  result.accountId = account->id;
  result.playerId = playerId;
  result.username = account->username;
  
  Core::Logger::getInstance().info("User {} logged in from {}", 
                                   account->username, ipAddress);
  
  return result;
}

bool AuthServer::logout(const std::string& token) {
  auto payload = jwtManager_->validateToken(token);
  if (!payload) {
    return false;
  }
  
  jwtManager_->revokeToken(token);
  sessionManager_->invalidateSession(token);
  
  Core::Logger::getInstance().info("User {} logged out", payload->username);
  
  return true;
}

std::optional<JWTPayload> AuthServer::validateToken(const std::string& token) {
  return jwtManager_->validateToken(token);
}

std::string AuthServer::refreshToken(const std::string& oldToken) {
  return jwtManager_->refreshToken(oldToken, config_.sessionDurationMinutes);
}

std::string AuthServer::getStats() const {
  nlohmann::json stats;
  stats["active_connections"] = networkServer_->getClientCount();
  stats["active_sessions"] = sessionManager_->getActiveSessionCount();
  stats["port"] = config_.port;
  
  return stats.dump();
}

size_t AuthServer::getActiveSessionCount() const {
  return sessionManager_->getActiveSessionCount();
}

bool AuthServer::checkLoginAttempts(const std::string& identifier) {
  std::lock_guard<std::mutex> lock(attemptsMutex_);
  
  auto it = loginAttempts_.find(identifier);
  if (it == loginAttempts_.end()) {
    return true;
  }
  
  return it->second < config_.maxLoginAttempts;
}

void AuthServer::recordLoginAttempt(const std::string& identifier, bool success) {
  std::lock_guard<std::mutex> lock(attemptsMutex_);
  
  if (success) {
    loginAttempts_.erase(identifier);
  } else {
    loginAttempts_[identifier]++;
  }
}

void AuthServer::handleClientMessage(uint32_t clientId,
                                     const std::vector<uint8_t>& data) {
  constexpr size_t kMaxPendingBytes = 64 * 1024;
  std::vector<std::string> messages;
  bool overflow = false;
  {
    std::lock_guard<std::mutex> lock(recvBuffersMutex_);
    std::string& buffer = recvBuffers_[clientId];
    buffer.append(data.begin(), data.end());

    // Mensagens completas: uma por linha.
    size_t start = 0;
    size_t newline = 0;
    while ((newline = buffer.find('\n', start)) != std::string::npos) {
      std::string line = buffer.substr(start, newline - start);
      if (!line.empty() && line.back() == '\r') line.pop_back();
      if (line.find_first_not_of(" \t") != std::string::npos) {
        messages.push_back(std::move(line));
      }
      start = newline + 1;
    }
    buffer.erase(0, start);

    // Compatibilidade: clientes antigos mandam um JSON sem '\n'.
    if (!buffer.empty() && nlohmann::json::accept(buffer)) {
      messages.push_back(buffer);
      buffer.clear();
    } else if (buffer.size() > kMaxPendingBytes) {
      buffer.clear();
      overflow = true;
    }
  }

  if (overflow) {
    Core::Logger::getInstance().warn("Auth: client {} exceeded pending buffer, disconnecting",
                                     clientId);
    networkServer_->disconnectClient(clientId);
    return;
  }

  for (const auto& message : messages) {
    processRequest(clientId, message);
  }
}

void AuthServer::processRequest(uint32_t clientId, const std::string& message) {
  try {
    auto json = nlohmann::json::parse(message);

    std::string action = json.value("action", "");

    if (action == "validate_token") {
      // Pedido do Gateway (AuthClient::validateToken): {action, data:{token}}.
      std::string token = json.value("token", "");
      if (token.empty() && json.contains("data") && json["data"].is_object()) {
        token = json["data"].value("token", "");
      }

      nlohmann::json response;
      response["action"] = action;
      response["success"] = true;
      response["valid"] = false;

      std::optional<JWTPayload> payload;
      if (!token.empty()) {
        payload = validateToken(token);
      }
      if (payload && !payload->isExpired()) {
        response["valid"] = true;
        response["message"] = "Token valid";
        response["account_id"] = payload->accountId;
        response["player_id"] = payload->playerId;
        response["username"] = payload->username;
      } else {
        response["message"] = token.empty() ? "Token required" : "Invalid token";
      }

      sendResponse(clientId, response.dump());
    } else if (action == "register") {
      auto result = registerAccount(
        json["username"], json["email"], json["password"]);
      
      nlohmann::json response;
      response["success"] = result.success;
      response["message"] = result.message;
      if (result.success) {
        response["account_id"] = result.accountId;
      }
      
      sendResponse(clientId, response.dump());
    } else if (action == "login") {
      auto result = login(
        json["username"], json["password"], json.value("ip", "0.0.0.0"));
      
      nlohmann::json response;
      response["success"] = result.success;
      response["message"] = result.message;
      if (result.success) {
        response["token"] = result.token;
        response["account_id"] = result.accountId;
        response["player_id"] = result.playerId;
      }
      
      sendResponse(clientId, response.dump());
    } else {
      // Sempre responder: o AuthClient casa respostas por ordem (FIFO).
      nlohmann::json response;
      response["action"] = action;
      response["success"] = false;
      response["message"] = "Unknown action: " + action;
      sendResponse(clientId, response.dump());
    }
  } catch (const std::exception& e) {
    Core::Logger::getInstance().error("Error handling auth message: {}", e.what());
    nlohmann::json response;
    response["success"] = false;
    response["message"] = "Invalid request";
    sendResponse(clientId, response.dump());
  }
}

void AuthServer::sendResponse(uint32_t clientId, const std::string& response) {
  // JSON por linha (mesmo contrato do AuthClient::createRequest).
  std::vector<uint8_t> data(response.begin(), response.end());
  data.push_back('\n');
  networkServer_->sendToClient(clientId, data);
}

}  // namespace Auth
}  // namespace Umbra

