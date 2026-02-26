#include "AccountDAO.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include <mysql.h>
#include <sstream>
#include <ctime>

namespace Umbra {
namespace Database {

const std::string AccountDAO::ACCOUNT_SELECT_FIELDS =
    "id, username, email, password_hash, salt, banned, ban_reason, created_at, last_login_at";

AccountDAO::AccountDAO(std::shared_ptr<MySQLConnector> connector)
    : connector_(connector) {
}

std::optional<Account> AccountDAO::parseAccountRow(const std::vector<std::string>& row) {
  if (row.size() < 9) return std::nullopt;
  try {
    Account account;
    account.id = std::stoull(row[0]);
    account.username = row[1];
    account.email = row[2];
    account.passwordHash = row[3];
    account.salt = row[4];
    account.banned = (row[5] == "1" || row[5] == "true");
    account.banReason = row[6];
    return account;
  } catch (const std::exception& e) {
    Core::Logger::getInstance().error("Failed to parse account data: {}", e.what());
    return std::nullopt;
  }
}

uint64_t AccountDAO::createAccount(const Account& account) {
  if (!account.isValid()) {
    Core::Logger::getInstance().error("Cannot create account: invalid data");
    return 0;
  }

  bool ok = connector_->executePreparedInsert(
    "INSERT INTO accounts (username, email, password_hash, salt, created_at) VALUES (?, ?, ?, ?, NOW())",
    {account.username, account.email, account.passwordHash, account.salt});

  if (ok) {
    uint64_t id = connector_->getLastInsertId();
    Core::Logger::getInstance().info("Created account: {} (ID: {})", account.username, id);
    return id;
  }

  return 0;
}

std::optional<Account> AccountDAO::getAccountById(uint64_t id) {
  auto results = connector_->executePreparedQuery(
    "SELECT " + ACCOUNT_SELECT_FIELDS + " FROM accounts WHERE id = ?",
    {std::to_string(id)});

  if (results.empty()) return std::nullopt;
  return parseAccountRow(results[0]);
}

std::optional<Account> AccountDAO::getAccountByUsername(const std::string& username) {
  auto results = connector_->executePreparedQuery(
    "SELECT " + ACCOUNT_SELECT_FIELDS + " FROM accounts WHERE username = ?",
    {username});

  if (results.empty()) return std::nullopt;
  return parseAccountRow(results[0]);
}

std::optional<Account> AccountDAO::getAccountByEmail(const std::string& email) {
  auto results = connector_->executePreparedQuery(
    "SELECT " + ACCOUNT_SELECT_FIELDS + " FROM accounts WHERE email = ?",
    {email});

  if (results.empty()) return std::nullopt;
  return parseAccountRow(results[0]);
}

bool AccountDAO::updateAccount(const Account& account) {
  if (!account.isValid() || account.id == 0) return false;

  return connector_->executePreparedInsert(
    "UPDATE accounts SET email = ?, password_hash = ? WHERE id = ?",
    {account.email, account.passwordHash, std::to_string(account.id)});
}

bool AccountDAO::deleteAccount(uint64_t id) {
  return connector_->executePreparedInsert(
    "DELETE FROM accounts WHERE id = ?",
    {std::to_string(id)});
}

bool AccountDAO::usernameExists(const std::string& username) {
  auto result = connector_->executePreparedScalar(
    "SELECT COUNT(*) FROM accounts WHERE username = ?",
    {username});
  return result && *result != "0";
}

bool AccountDAO::emailExists(const std::string& email) {
  auto result = connector_->executePreparedScalar(
    "SELECT COUNT(*) FROM accounts WHERE email = ?",
    {email});
  return result && *result != "0";
}

bool AccountDAO::banAccount(uint64_t id, const std::string& reason) {
  bool ok = connector_->executePreparedInsert(
    "UPDATE accounts SET banned = 1, ban_reason = ? WHERE id = ?",
    {reason, std::to_string(id)});

  if (ok) {
    Core::Logger::getInstance().info("Banned account {}: {}", id, reason);
  }
  return ok;
}

bool AccountDAO::unbanAccount(uint64_t id) {
  bool ok = connector_->executePreparedInsert(
    "UPDATE accounts SET banned = 0, ban_reason = NULL WHERE id = ?",
    {std::to_string(id)});

  if (ok) {
    Core::Logger::getInstance().info("Unbanned account {}", id);
  }
  return ok;
}

bool AccountDAO::updateLastLogin(uint64_t id) {
  return connector_->executePreparedInsert(
    "UPDATE accounts SET last_login_at = NOW() WHERE id = ?",
    {std::to_string(id)});
}

std::optional<Account> AccountDAO::parseAccountFromQuery(const std::string& query) {
  auto results = connector_->executeQuery(query);
  if (results.empty()) return std::nullopt;
  return parseAccountRow(results[0]);
}

}  // namespace Database
}  // namespace Umbra
