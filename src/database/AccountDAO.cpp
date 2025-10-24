#include "AccountDAO.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include <sstream>

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
  
  auto result = connector_->executeScalar(query.str());
  if (!result) {
    return std::nullopt;
  }
  
  // TODO: Parse actual result set
  Account account;
  account.id = id;
  
  return account;
}

std::optional<Account> AccountDAO::getAccountByUsername(const std::string& username) {
  std::ostringstream query;
  query << "SELECT id, username, email, password_hash, salt, banned, ban_reason, "
        << "created_at, last_login_at FROM accounts WHERE username = '"
        << connector_->escapeString(username) << "'";
  
  auto result = connector_->executeScalar(query.str());
  if (!result) {
    return std::nullopt;
  }
  
  // TODO: Parse actual result set
  Account account;
  account.username = username;
  
  return account;
}

std::optional<Account> AccountDAO::getAccountByEmail(const std::string& email) {
  std::ostringstream query;
  query << "SELECT id, username, email, password_hash, salt, banned, ban_reason, "
        << "created_at, last_login_at FROM accounts WHERE email = '"
        << connector_->escapeString(email) << "'";
  
  auto result = connector_->executeScalar(query.str());
  if (!result) {
    return std::nullopt;
  }
  
  // TODO: Parse actual result set
  Account account;
  account.email = email;
  
  return account;
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

Account AccountDAO::resultToAccount(const std::string& result) {
  // TODO: Implement proper result parsing
  Account account;
  return account;
}

}  // namespace Database
}  // namespace Umbra

