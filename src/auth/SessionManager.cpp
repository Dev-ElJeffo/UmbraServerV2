#include "SessionManager.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"

namespace Umbra {
namespace Auth {

SessionManager::SessionManager() {
}

std::string SessionManager::createSession(uint64_t accountId,
                                         uint64_t playerId,
                                         const std::string& ipAddress,
                                         uint32_t durationMinutes) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::string token = generateSessionToken();
  
  Database::Session session;
  session.token = token;
  session.accountId = accountId;
  session.playerId = playerId;
  session.ipAddress = ipAddress;
  session.createdAt = std::chrono::system_clock::now();
  session.expiresAt = session.createdAt + std::chrono::minutes(durationMinutes);
  session.valid = true;
  
  sessions_[token] = session;
  
  Core::Logger::getInstance().info("Created session for account {} (IP: {})", 
                                   accountId, ipAddress);
  
  return token;
}

std::optional<Database::Session> SessionManager::validateSession(const std::string& token) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = sessions_.find(token);
  if (it == sessions_.end()) {
    return std::nullopt;
  }
  
  auto& session = it->second;
  
  if (!session.valid) {
    Core::Logger::getInstance().warn("Session is invalid: {}", token);
    return std::nullopt;
  }
  
  if (session.isExpired()) {
    Core::Logger::getInstance().warn("Session is expired: {}", token);
    sessions_.erase(it);
    return std::nullopt;
  }
  
  return session;
}

bool SessionManager::invalidateSession(const std::string& token) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = sessions_.find(token);
  if (it == sessions_.end()) {
    return false;
  }
  
  Core::Logger::getInstance().info("Invalidated session for account {}", 
                                   it->second.accountId);
  
  sessions_.erase(it);
  return true;
}

uint32_t SessionManager::invalidateAccountSessions(uint64_t accountId) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  uint32_t count = 0;
  
  for (auto it = sessions_.begin(); it != sessions_.end();) {
    if (it->second.accountId == accountId) {
      it = sessions_.erase(it);
      ++count;
    } else {
      ++it;
    }
  }
  
  if (count > 0) {
    Core::Logger::getInstance().info("Invalidated {} sessions for account {}", 
                                     count, accountId);
  }
  
  return count;
}

bool SessionManager::extendSession(const std::string& token, uint32_t additionalMinutes) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = sessions_.find(token);
  if (it == sessions_.end()) {
    return false;
  }
  
  it->second.expiresAt += std::chrono::minutes(additionalMinutes);
  
  Core::Logger::getInstance().debug("Extended session {} by {} minutes", 
                                    token, additionalMinutes);
  
  return true;
}

size_t SessionManager::getActiveSessionCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return sessions_.size();
}

std::vector<Database::Session> SessionManager::getAccountSessions(uint64_t accountId) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::vector<Database::Session> result;
  
  for (const auto& [token, session] : sessions_) {
    if (session.accountId == accountId) {
      result.push_back(session);
    }
  }
  
  return result;
}

uint32_t SessionManager::cleanupExpiredSessions() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  uint32_t count = 0;
  auto now = std::chrono::system_clock::now();
  
  for (auto it = sessions_.begin(); it != sessions_.end();) {
    if (now >= it->second.expiresAt) {
      it = sessions_.erase(it);
      ++count;
    } else {
      ++it;
    }
  }
  
  if (count > 0) {
    Core::Logger::getInstance().debug("Cleaned up {} expired sessions", count);
  }
  
  return count;
}

std::string SessionManager::generateSessionToken() {
  return Core::Utils::generateUUID();
}

}  // namespace Auth
}  // namespace Umbra

