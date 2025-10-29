#include "AccountDAO.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include <mysql.h>
#include <sstream>
#include <ctime>

namespace Umbra {
namespace Database {

AccountDAO::AccountDAO(std::shared_ptr<MySQLConnector> connector)
    : connector_(connector) {
}

uint64_t AccountDAO::createAccount(const Account& account) {
  if (!account.isValid()) {
    Core::Logger::getInstance().error("Cannot create account: invalid data");
    return 0;
  }
  
  std::ostringstream query;
  query << "INSERT INTO accounts (username, email, password_hash, salt, created_at) "
        << "VALUES ('"
        << connector_->escapeString(account.username) << "', '"
        << connector_->escapeString(account.email) << "', '"
        << connector_->escapeString(account.passwordHash) << "', '"
        << connector_->escapeString(account.salt) << "', "
        << "NOW())";
  
  if (connector_->execute(query.str())) {
    uint64_t id = connector_->getLastInsertId();
    Core::Logger::getInstance().info("Created account: {} (ID: {})", 
                                     account.username, id);
    return id;
  }
  
  return 0;
}

std::optional<Account> AccountDAO::getAccountById(uint64_t id) {
  std::ostringstream query;
  query << "SELECT id, username, email, password_hash, salt, banned, ban_reason, "
        << "created_at, last_login_at FROM accounts WHERE id = " << id;
  
  return parseAccountFromQuery(query.str());
}

std::optional<Account> AccountDAO::getAccountByUsername(const std::string& username) {
  std::ostringstream query;
  query << "SELECT id, username, email, password_hash, salt, banned, ban_reason, "
        << "created_at, last_login_at FROM accounts WHERE username = '"
        << connector_->escapeString(username) << "'";
  
  return parseAccountFromQuery(query.str());
}

std::optional<Account> AccountDAO::getAccountByEmail(const std::string& email) {
  std::ostringstream query;
  query << "SELECT id, username, email, password_hash, salt, banned, ban_reason, "
        << "created_at, last_login_at FROM accounts WHERE email = '"
        << connector_->escapeString(email) << "'";
  
  return parseAccountFromQuery(query.str());
}

bool AccountDAO::updateAccount(const Account& account) {
  if (!account.isValid() || account.id == 0) {
    return false;
  }
  
  std::ostringstream query;
  query << "UPDATE accounts SET "
        << "email = '" << connector_->escapeString(account.email) << "', "
        << "password_hash = '" << connector_->escapeString(account.passwordHash) << "' "
        << "WHERE id = " << account.id;
  
  return connector_->execute(query.str());
}

bool AccountDAO::deleteAccount(uint64_t id) {
  std::ostringstream query;
  query << "DELETE FROM accounts WHERE id = " << id;
  
  return connector_->execute(query.str());
}

bool AccountDAO::usernameExists(const std::string& username) {
  std::ostringstream query;
  query << "SELECT COUNT(*) FROM accounts WHERE username = '"
        << connector_->escapeString(username) << "'";
  
  auto result = connector_->executeScalar(query.str());
  return result && *result != "0";
}

bool AccountDAO::emailExists(const std::string& email) {
  std::ostringstream query;
  query << "SELECT COUNT(*) FROM accounts WHERE email = '"
        << connector_->escapeString(email) << "'";
  
  auto result = connector_->executeScalar(query.str());
  return result && *result != "0";
}

bool AccountDAO::banAccount(uint64_t id, const std::string& reason) {
  std::ostringstream query;
  query << "UPDATE accounts SET "
        << "banned = 1, "
        << "ban_reason = '" << connector_->escapeString(reason) << "' "
        << "WHERE id = " << id;
  
  bool result = connector_->execute(query.str());
  if (result) {
    Core::Logger::getInstance().info("Banned account {}: {}", id, reason);
  }
  
  return result;
}

bool AccountDAO::unbanAccount(uint64_t id) {
  std::ostringstream query;
  query << "UPDATE accounts SET "
        << "banned = 0, "
        << "ban_reason = NULL "
        << "WHERE id = " << id;
  
  bool result = connector_->execute(query.str());
  if (result) {
    Core::Logger::getInstance().info("Unbanned account {}", id);
  }
  
  return result;
}

bool AccountDAO::updateLastLogin(uint64_t id) {
  std::ostringstream query;
  query << "UPDATE accounts SET last_login_at = NOW() WHERE id = " << id;
  
  return connector_->execute(query.str());
}

std::optional<Account> AccountDAO::parseAccountFromQuery(const std::string& query) {
  auto results = connector_->executeQuery(query);
  
  if (results.empty() || results[0].size() < 9) {
    return std::nullopt;
  }
  
  Account account;
  const auto& row = results[0];
  
  // Parse dos campos: id, username, email, password_hash, salt, banned, ban_reason, created_at, last_login_at
  try {
    account.id = std::stoull(row[0]);
    account.username = row[1];
    account.email = row[2];
    account.passwordHash = row[3];
    account.salt = row[4];
    account.banned = (row[5] == "1" || row[5] == "true" || !row[5].empty());
    account.banReason = row[6];
    
    // Parse de timestamps (formato MySQL: YYYY-MM-DD HH:MM:SS)
    if (!row[7].empty()) {
      // Simplificado: não fazemos parsing completo de timestamp por enquanto
      // Apenas marcamos que foi criado
    }
    
    if (!row[8].empty()) {
      // Timestamp de último login
    }
    
    return account;
  } catch (const std::exception& e) {
    Core::Logger::getInstance().error("Failed to parse account data: {}", e.what());
    return std::nullopt;
  }
}

}  // namespace Database
}  // namespace Umbra

