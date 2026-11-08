#pragma once

#include "auth/JWTManager.hpp"
#include "database/MySQLConnector.hpp"
#include "zone/MovementProtocol.hpp"
#include "core/Logger.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace Umbra {
namespace Zone {

enum class ClientAuthState : uint8_t { Pending = 0, Authenticated = 1, Rejected = 2 };

/** Autenticação WS por JWT + session_version (sessão única por conta). */
class MovementSessionAuth {
 public:
  void setJwtManager(Umbra::Auth::JWTManager* jwt) { jwt_ = jwt; }
  void setDatabase(std::shared_ptr<Umbra::Database::MySQLConnector> db) { db_ = std::move(db); }

  void onClientConnected(uint32_t clientId) {
    std::lock_guard<std::mutex> lock(mu_);
    clientAuthState_[clientId] = ClientAuthState::Pending;
    authDeadlineByClient_[clientId] =
        std::chrono::steady_clock::now() + std::chrono::seconds(kAuthTimeoutSeconds);
  }

  void onClientDisconnected(uint32_t clientId) {
    std::lock_guard<std::mutex> lock(mu_);
    auto accIt = clientIdToAccountId_.find(clientId);
    if (accIt != clientIdToAccountId_.end()) {
      auto mapIt = accountIdToClientId_.find(accIt->second);
      if (mapIt != accountIdToClientId_.end() && mapIt->second == clientId) {
        accountIdToClientId_.erase(mapIt);
      }
    }
    clientAuthState_.erase(clientId);
    clientIdToAccountId_.erase(clientId);
    authDeadlineByClient_.erase(clientId);
    clientSessionVersion_.erase(clientId);
  }

  ClientAuthState getAuthState(uint32_t clientId) const {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = clientAuthState_.find(clientId);
    return it != clientAuthState_.end() ? it->second : ClientAuthState::Rejected;
  }

  bool isAuthenticated(uint32_t clientId) const {
    return getAuthState(clientId) == ClientAuthState::Authenticated;
  }

  uint32_t getAccountIdForClient(uint32_t clientId) const {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = clientIdToAccountId_.find(clientId);
    return it != clientIdToAccountId_.end() ? it->second : 0;
  }

  bool playerBelongsToAccount(uint32_t playerId, uint32_t accountId) const {
    if (!db_ || !db_->isConnected() || playerId == 0 || accountId == 0) return false;
    auto opt = db_->executePreparedScalar(
        "SELECT id FROM players WHERE id = ? AND account_id = ? LIMIT 1",
        {std::to_string(playerId), std::to_string(accountId)});
    if (opt.has_value() && !opt->empty()) return true;
    const auto rows = db_->executeQuery(
        "SELECT id FROM players WHERE id = " + std::to_string(playerId) +
        " AND account_id = " + std::to_string(accountId) + " LIMIT 1");
    return !rows.empty() && !rows[0].empty();
  }

  /** Retorna clientId anterior da mesma conta (0 se nenhum) para kick com mensagem. */
  bool handleSessionAuth(uint32_t clientId, const std::string& token, uint32_t& outKickClientId,
                         std::string& outKickMessage) {
    outKickClientId = 0;
    outKickMessage.clear();
    if (!jwt_ || !db_ || !db_->isConnected()) {
      rejectClient(clientId, SessionRevokeReason::InvalidToken, "Servidor indisponivel.");
      return false;
    }

    auto payloadOpt = jwt_->validateToken(token);
    if (!payloadOpt) {
      rejectClient(clientId, SessionRevokeReason::InvalidToken, "Token invalido ou expirado.");
      return false;
    }

    const uint32_t accountId = static_cast<uint32_t>(payloadOpt->accountId);
    const uint32_t tokenSv = payloadOpt->sessionVersion;

    auto dbSvOpt = db_->executePreparedScalar(
        "SELECT session_version FROM accounts WHERE id = ? LIMIT 1", {std::to_string(accountId)});
    if (!dbSvOpt || dbSvOpt->empty()) {
      // Fallback: prepared scalar pode falhar em colunas INT remotas (MySQL 8.4).
      const auto rows = db_->executeQuery(
          "SELECT session_version FROM accounts WHERE id = " + std::to_string(accountId) + " LIMIT 1");
      if (!rows.empty() && !rows[0].empty()) {
        dbSvOpt = rows[0][0];
      }
    }
    if (!dbSvOpt || dbSvOpt->empty()) {
      rejectClient(clientId, SessionRevokeReason::InvalidToken, "Conta nao encontrada.");
      return false;
    }

    uint32_t dbSv = 0;
    try {
      dbSv = static_cast<uint32_t>(std::stoul(*dbSvOpt));
    } catch (...) {
      rejectClient(clientId, SessionRevokeReason::InvalidToken, "Sessao invalida.");
      return false;
    }

    if (tokenSv != dbSv) {
      rejectClient(clientId, SessionRevokeReason::InvalidToken,
                   "Sessao expirada. Faca login novamente.");
      return false;
    }

    {
      std::lock_guard<std::mutex> lock(mu_);
      auto oldIt = accountIdToClientId_.find(accountId);
      if (oldIt != accountIdToClientId_.end() && oldIt->second != clientId) {
        outKickClientId = oldIt->second;
        outKickMessage = "Sua conta entrou em outro cliente.";
        accountIdToClientId_.erase(oldIt);
        clientAuthState_.erase(outKickClientId);
        clientIdToAccountId_.erase(outKickClientId);
        authDeadlineByClient_.erase(outKickClientId);
        clientSessionVersion_.erase(outKickClientId);
      }
      clientAuthState_[clientId] = ClientAuthState::Authenticated;
      clientIdToAccountId_[clientId] = accountId;
      accountIdToClientId_[accountId] = clientId;
      clientSessionVersion_[clientId] = tokenSv;
      authDeadlineByClient_.erase(clientId);
    }

    Core::Logger::getInstance().info("WS client {} autenticado account={} session_version={}",
                                    clientId, accountId, tokenSv);
    return true;
  }

  void tickAuthTimeouts(const std::function<void(uint32_t, SessionRevokeReason, const std::string&)>& onExpire) {
    const auto now = std::chrono::steady_clock::now();
    std::vector<uint32_t> expired;
    {
      std::lock_guard<std::mutex> lock(mu_);
      for (const auto& [cid, deadline] : authDeadlineByClient_) {
        auto stIt = clientAuthState_.find(cid);
        if (stIt != clientAuthState_.end() && stIt->second == ClientAuthState::Pending && now >= deadline) {
          expired.push_back(cid);
        }
      }
      for (uint32_t cid : expired) {
        clientAuthState_[cid] = ClientAuthState::Rejected;
        authDeadlineByClient_.erase(cid);
      }
    }
    for (uint32_t cid : expired) {
      onExpire(cid, SessionRevokeReason::AuthTimeout, "Tempo de autenticacao esgotado.");
    }
  }

 private:
  static constexpr int kAuthTimeoutSeconds = 30;

  void rejectClient(uint32_t clientId, SessionRevokeReason reason, const std::string& message) {
    std::lock_guard<std::mutex> lock(mu_);
    clientAuthState_[clientId] = ClientAuthState::Rejected;
    authDeadlineByClient_.erase(clientId);
    (void)reason;
    (void)message;
    Core::Logger::getInstance().warn("WS client {} auth rejeitada: {}", clientId, message);
  }

  Umbra::Auth::JWTManager* jwt_ = nullptr;
  std::shared_ptr<Umbra::Database::MySQLConnector> db_;
  mutable std::mutex mu_;
  std::unordered_map<uint32_t, ClientAuthState> clientAuthState_;
  std::unordered_map<uint32_t, uint32_t> clientIdToAccountId_;
  std::unordered_map<uint32_t, uint32_t> accountIdToClientId_;
  std::unordered_map<uint32_t, uint32_t> clientSessionVersion_;
  std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> authDeadlineByClient_;
};

}  // namespace Zone
}  // namespace Umbra
