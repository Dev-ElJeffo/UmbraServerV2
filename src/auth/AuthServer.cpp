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
  Database::Account account;
  account.username = username;
  account.email = email;
  account.salt = Core::Utils::generateRandomString(16);
  account.passwordHash = Core::Utils::hashPassword(password, account.salt);
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
  
  // Verify password
  std::string passwordHash = Core::Utils::hashPassword(password, account->salt);
  if (passwordHash != account->passwordHash) {
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
  try {
    std::string message(data.begin(), data.end());
    auto json = nlohmann::json::parse(message);
    
    std::string action = json["action"];
    
    if (action == "register") {
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
    }
  } catch (const std::exception& e) {
    Core::Logger::getInstance().error("Error handling auth message: {}", e.what());
  }
}

void AuthServer::sendResponse(uint32_t clientId, const std::string& response) {
  std::vector<uint8_t> data(response.begin(), response.end());
  networkServer_->sendToClient(clientId, data);
}

}  // namespace Auth
}  // namespace Umbra

