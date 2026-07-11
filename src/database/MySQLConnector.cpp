#include "MySQLConnector.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include <mysql.h>
#include <cctype>
#include <chrono>
#include <cstring>
#include <memory>
#include <string>

namespace {

constexpr unsigned long kResultColumnBuf = 4096;
constexpr unsigned long kClientMaxPacket = 64UL * 1024UL * 1024UL;

bool isUnsignedIntegerParam(const std::string& value) {
  if (value.empty()) return false;
  for (unsigned char ch : value) {
    if (!std::isdigit(ch)) return false;
  }
  return true;
}

void applyMysqlClientOptions(MYSQL* mysql, const Umbra::Database::MySQLConnector::Config& config) {
  unsigned int timeout = config.connectionTimeout;
  mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
  mysql_options(mysql, MYSQL_OPT_RECONNECT, &config.autoReconnect);
  unsigned long maxPacket = kClientMaxPacket;
  mysql_options(mysql, MYSQL_OPT_MAX_ALLOWED_PACKET, &maxPacket);
}

struct PreparedParamBind {
  std::vector<MYSQL_BIND> binds;
  std::vector<unsigned long> lengths;
  std::vector<long long> longValues;
};

struct PreparedResultBind {
  std::vector<MYSQL_BIND> binds;
  std::vector<std::vector<char>> stringBuffers;
  std::vector<unsigned long> lengths;
  std::vector<long long> longValues;
  std::vector<char> isNull;
  std::vector<bool> isInteger;
  std::vector<bool> isUnsigned;
};

bool isIntegerMysqlType(enum enum_field_types type) {
  switch (type) {
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONGLONG:
      return true;
    default:
      return false;
  }
}

bool bindPreparedResults(MYSQL_RES* meta, PreparedResultBind& storage, std::string& errorOut) {
  const unsigned int numFields = mysql_num_fields(meta);
  if (numFields == 0) return true;

  MYSQL_FIELD* fields = mysql_fetch_fields(meta);
  if (!fields) {
    errorOut = "mysql_fetch_fields failed";
    return false;
  }

  storage.binds.assign(numFields, {});
  storage.stringBuffers.assign(numFields, {});
  storage.lengths.assign(numFields, 0);
  storage.longValues.assign(numFields, 0);
  storage.isNull.assign(numFields, 0);
  storage.isInteger.assign(numFields, false);
  storage.isUnsigned.assign(numFields, false);

  for (unsigned int i = 0; i < numFields; ++i) {
    MYSQL_BIND& bind = storage.binds[i];
    memset(&bind, 0, sizeof(MYSQL_BIND));
    bind.is_null = reinterpret_cast<bool*>(&storage.isNull[i]);
    bind.length = &storage.lengths[i];

    if (isIntegerMysqlType(fields[i].type)) {
      storage.isInteger[i] = true;
      storage.isUnsigned[i] = (fields[i].flags & UNSIGNED_FLAG) != 0;
      bind.buffer_type = MYSQL_TYPE_LONGLONG;
      bind.buffer = &storage.longValues[i];
      bind.is_unsigned = storage.isUnsigned[i] ? 1 : 0;
    } else {
      storage.stringBuffers[i].assign(kResultColumnBuf, 0);
      bind.buffer_type = MYSQL_TYPE_STRING;
      bind.buffer = storage.stringBuffers[i].data();
      bind.buffer_length = kResultColumnBuf;
    }
  }
  return true;
}

std::string readPreparedResultCell(const PreparedResultBind& storage, unsigned int column) {
  if (storage.isNull[column]) return "";
  if (storage.isInteger[column]) {
    if (storage.isUnsigned[column]) {
      return std::to_string(static_cast<unsigned long long>(storage.longValues[column]));
    }
    return std::to_string(storage.longValues[column]);
  }
  return std::string(storage.stringBuffers[column].data(), storage.lengths[column]);
}

bool bindPreparedParams(MYSQL_STMT* stmt, const std::vector<std::string>& params,
                        PreparedParamBind& storage, std::string& errorOut) {
  const unsigned long paramCount = mysql_stmt_param_count(stmt);
  if (paramCount != params.size()) {
    errorOut = "Param count mismatch: expected " + std::to_string(paramCount) +
               " got " + std::to_string(params.size());
    return false;
  }
  if (paramCount == 0) return true;

  storage.binds.assign(paramCount, {});
  storage.lengths.assign(paramCount, 0);
  storage.longValues.assign(paramCount, 0);
  for (unsigned long i = 0; i < paramCount; ++i) {
    if (isUnsignedIntegerParam(params[i])) {
      storage.binds[i].buffer_type = MYSQL_TYPE_LONGLONG;
      storage.longValues[i] = std::stoll(params[i]);
      storage.binds[i].buffer = &storage.longValues[i];
      storage.binds[i].is_unsigned = 1;
      storage.binds[i].is_null = nullptr;
    } else {
      storage.binds[i].buffer_type = MYSQL_TYPE_STRING;
      storage.binds[i].buffer = const_cast<char*>(params[i].c_str());
      storage.lengths[i] = static_cast<unsigned long>(params[i].size());
      storage.binds[i].buffer_length = storage.lengths[i];
      storage.binds[i].length = &storage.lengths[i];
      storage.binds[i].is_null = nullptr;
    }
  }
  if (mysql_stmt_bind_param(stmt, storage.binds.data()) != 0) {
    errorOut = std::string(mysql_stmt_error(stmt));
    return false;
  }
  return true;
}

}  // namespace

namespace Umbra {
namespace Database {

static const size_t POOL_NONE = SIZE_MAX;

MySQLConnector::MySQLConnector(const Config& config)
    : config_(config),
      connection_(nullptr),
      connected_(false) {
  static bool mysqlInitialized = false;
  if (!mysqlInitialized) {
    if (mysql_library_init(0, nullptr, nullptr) != 0) {
      Core::Logger::getInstance().error("MySQL library initialization failed");
    } else {
      mysqlInitialized = true;
      Core::Logger::getInstance().debug("MySQL library initialized");
    }
  }
}

MySQLConnector::~MySQLConnector() {
  disconnect();
}

// ---------------------------------------------------------------------------
// Pool management
// ---------------------------------------------------------------------------

bool MySQLConnector::createPooledConnection(PooledConnection& conn) {
  MYSQL* mysql = mysql_init(nullptr);
  if (!mysql) {
    Core::Logger::getInstance().error("Pool: mysql_init failed");
    return false;
  }

  applyMysqlClientOptions(mysql, config_);

  MYSQL* result = mysql_real_connect(
    mysql, config_.host.c_str(), config_.username.c_str(),
    config_.password.c_str(), config_.database.c_str(),
    config_.port, nullptr, 0);

  if (!result) {
    Core::Logger::getInstance().error("Pool: connection failed: {}", mysql_error(mysql));
    mysql_close(mysql);
    return false;
  }

  mysql_set_character_set(mysql, "utf8mb4");
  mysql_autocommit(mysql, 1);
  conn.mysql = mysql;
  conn.inUse = false;
  return true;
}

size_t MySQLConnector::acquireConnection(uint32_t timeoutMs) {
  std::unique_lock<std::mutex> lock(poolMutex_);

  if (!poolInitialized_ || pool_.empty()) {
    return POOL_NONE;
  }

  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);

  while (available_.empty()) {
    if (poolCond_.wait_until(lock, deadline) == std::cv_status::timeout) {
      Core::Logger::getInstance().warn("Pool: acquire timed out after {}ms", timeoutMs);
      return POOL_NONE;
    }
  }

  size_t idx = available_.front();
  available_.pop();
  pool_[idx].inUse = true;

  MYSQL* mysql = static_cast<MYSQL*>(pool_[idx].mysql);
  if (mysql_ping(mysql) != 0) {
    Core::Logger::getInstance().warn("Pool: connection {} lost, reconnecting...", idx);
    mysql_close(mysql);
    pool_[idx].mysql = nullptr;
    if (!createPooledConnection(pool_[idx])) {
      pool_[idx].inUse = false;
      available_.push(idx);
      poolCond_.notify_one();
      return POOL_NONE;
    }
    pool_[idx].inUse = true;
  }

  return idx;
}

void MySQLConnector::releaseConnection(size_t index) {
  std::lock_guard<std::mutex> lock(poolMutex_);
  if (index < pool_.size()) {
    pool_[index].inUse = false;
    available_.push(index);
    poolCond_.notify_one();
  }
}

// ---------------------------------------------------------------------------
// connect / disconnect
// ---------------------------------------------------------------------------

bool MySQLConnector::connect() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (connected_) return true;

  Core::Logger::getInstance().info("Connecting to MySQL: {}:{}/{}",
                                   config_.host, config_.port, config_.database);

  MYSQL* mysql = mysql_init(nullptr);
  if (!mysql) {
    Core::Logger::getInstance().error("MySQL initialization failed: out of memory");
    return false;
  }

  applyMysqlClientOptions(mysql, config_);

  MYSQL* result = mysql_real_connect(
    mysql, config_.host.c_str(), config_.username.c_str(),
    config_.password.c_str(), config_.database.c_str(),
    config_.port, nullptr, 0);

  if (!result) {
    std::string error = mysql_error(mysql);
    Core::Logger::getInstance().error("MySQL connection failed: {}", error);
    mysql_close(mysql);
    return false;
  }

  if (mysql_set_character_set(mysql, "utf8mb4") != 0) {
    Core::Logger::getInstance().warn("Failed to set UTF-8 charset: {}", mysql_error(mysql));
  }

  mysql_autocommit(mysql, 1);

  connection_ = mysql;
  connected_ = true;
  Core::Logger::getInstance().info("Connected to MySQL successfully");

  {
    std::lock_guard<std::mutex> poolLock(poolMutex_);
    uint32_t poolSize = config_.poolSize;
    if (poolSize < 1) poolSize = 1;

    pool_.resize(poolSize);
    uint32_t created = 0;
    for (uint32_t i = 0; i < poolSize; ++i) {
      if (createPooledConnection(pool_[i])) {
        available_.push(i);
        ++created;
      }
    }
    poolInitialized_ = true;
    Core::Logger::getInstance().info("MySQL connection pool: {}/{} connections created", created, poolSize);
  }

  return true;
}

void MySQLConnector::disconnect() {
  {
    std::lock_guard<std::mutex> poolLock(poolMutex_);
    for (auto& conn : pool_) {
      if (conn.mysql) {
        mysql_close(static_cast<MYSQL*>(conn.mysql));
        conn.mysql = nullptr;
      }
    }
    pool_.clear();
    while (!available_.empty()) available_.pop();
    poolInitialized_ = false;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  if (!connected_ || !connection_) return;

  mysql_close(static_cast<MYSQL*>(connection_));
  connected_ = false;
  connection_ = nullptr;

  Core::Logger::getInstance().info("Disconnected from MySQL");
}

bool MySQLConnector::isConnected() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return connected_;
}

// ---------------------------------------------------------------------------
// Helpers that run a query on a specific MYSQL*
// ---------------------------------------------------------------------------

bool MySQLConnector::executeOnConnection(void* conn, const std::string& query) {
  MYSQL* mysql = static_cast<MYSQL*>(conn);
  if (mysql_real_query(mysql, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
    logError("Query failed: " + std::string(mysql_error(mysql)));
    return false;
  }
  MYSQL_RES* res = mysql_store_result(mysql);
  if (res) mysql_free_result(res);
  return true;
}

std::optional<std::string> MySQLConnector::executeScalarOnConnection(void* conn, const std::string& query) {
  MYSQL* mysql = static_cast<MYSQL*>(conn);
  if (mysql_real_query(mysql, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
    logError("Scalar query failed: " + std::string(mysql_error(mysql)));
    return std::nullopt;
  }
  MYSQL_RES* res = mysql_store_result(mysql);
  if (!res) {
    if (mysql_field_count(mysql) == 0) return std::nullopt;
    logError("Store result failed: " + std::string(mysql_error(mysql)));
    return std::nullopt;
  }
  MYSQL_ROW row = mysql_fetch_row(res);
  if (!row || mysql_num_fields(res) == 0) {
    mysql_free_result(res);
    return std::nullopt;
  }
  std::string value = row[0] ? row[0] : "";
  mysql_free_result(res);
  return value;
}

std::vector<std::vector<std::string>> MySQLConnector::executeQueryOnConnection(void* conn, const std::string& query) {
  std::vector<std::vector<std::string>> results;
  MYSQL* mysql = static_cast<MYSQL*>(conn);
  if (mysql_real_query(mysql, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
    logError("Query failed: " + std::string(mysql_error(mysql)));
    return results;
  }
  MYSQL_RES* res = mysql_store_result(mysql);
  if (!res) {
    if (mysql_field_count(mysql) == 0) return results;
    logError("Store result failed: " + std::string(mysql_error(mysql)));
    return results;
  }
  unsigned int numFields = mysql_num_fields(res);
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)) != nullptr) {
    std::vector<std::string> rowData;
    for (unsigned int i = 0; i < numFields; i++) {
      rowData.push_back(row[i] ? row[i] : "");
    }
    results.push_back(std::move(rowData));
  }
  mysql_free_result(res);
  return results;
}

// ---------------------------------------------------------------------------
// Public API — uses pool when available, falls back to primary connection
// ---------------------------------------------------------------------------

bool MySQLConnector::execute(const std::string& query) {
  size_t idx = acquireConnection(5000);
  if (idx != POOL_NONE) {
    Core::Logger::getInstance().debug("Executing query (pool[{}]): {}", idx, query);
    bool ok = executeOnConnection(pool_[idx].mysql, query);
    releaseConnection(idx);
    return ok;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  if (!connected_ || !connection_) {
    logError("Cannot execute query: not connected");
    return false;
  }
  Core::Logger::getInstance().debug("Executing query (primary): {}", query);
  return executeOnConnection(connection_, query);
}

std::optional<std::string> MySQLConnector::executeScalar(const std::string& query) {
  size_t idx = acquireConnection(5000);
  if (idx != POOL_NONE) {
    Core::Logger::getInstance().debug("Executing scalar (pool[{}]): {}", idx, query);
    auto result = executeScalarOnConnection(pool_[idx].mysql, query);
    releaseConnection(idx);
    return result;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  if (!connected_ || !connection_) {
    logError("Cannot execute scalar: not connected");
    return std::nullopt;
  }
  Core::Logger::getInstance().debug("Executing scalar (primary): {}", query);
  return executeScalarOnConnection(connection_, query);
}

std::vector<std::vector<std::string>> MySQLConnector::executeQuery(const std::string& query) {
  size_t idx = acquireConnection(5000);
  if (idx != POOL_NONE) {
    Core::Logger::getInstance().debug("Executing result query (pool[{}]): {}", idx, query);
    auto result = executeQueryOnConnection(pool_[idx].mysql, query);
    releaseConnection(idx);
    return result;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  if (!connected_ || !connection_) {
    logError("Cannot execute query: not connected");
    return {};
  }
  Core::Logger::getInstance().debug("Executing result query (primary): {}", query);
  return executeQueryOnConnection(connection_, query);
}

// ---------------------------------------------------------------------------
// Prepared statements
// ---------------------------------------------------------------------------

uint32_t MySQLConnector::prepareStatement(const std::string& query) {
  Core::Logger::getInstance().warn("prepareStatement(id) deprecated — use executePreparedQuery/executePreparedInsert directly");
  return 0;
}

bool MySQLConnector::executePrepared(uint32_t statementId, const std::vector<std::string>& params) {
  Core::Logger::getInstance().warn("executePrepared(id) deprecated — use executePreparedInsert directly");
  return false;
}

bool MySQLConnector::executePreparedInsert(const std::string& query, const std::vector<std::string>& params) {
  size_t idx = acquireConnection(5000);
  void* conn = nullptr;
  bool usedPool = false;

  if (idx != POOL_NONE) {
    conn = pool_[idx].mysql;
    usedPool = true;
  } else {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_ || !connection_) {
      logError("Cannot execute prepared insert: not connected");
      return false;
    }
    conn = connection_;
  }

  MYSQL* mysql = static_cast<MYSQL*>(conn);
  MYSQL_STMT* stmt = mysql_stmt_init(mysql);
  if (!stmt) {
    logError("mysql_stmt_init failed");
    if (usedPool) releaseConnection(idx);
    return false;
  }

  bool ok = false;
  if (mysql_stmt_prepare(stmt, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
    logError("Prepare failed: " + std::string(mysql_stmt_error(stmt)));
  } else {
    unsigned long paramCount = mysql_stmt_param_count(stmt);
    if (paramCount != params.size()) {
      logError("Param count mismatch: expected " + std::to_string(paramCount) + " got " + std::to_string(params.size()));
    } else if (paramCount > 0) {
      PreparedParamBind paramBind;
      std::string bindError;
      if (!bindPreparedParams(stmt, params, paramBind, bindError)) {
        logError("Bind failed: " + bindError);
      } else if (mysql_stmt_execute(stmt) != 0) {
        logError("Execute failed: " + std::string(mysql_stmt_error(stmt)));
      } else {
        ok = true;
        const uint64_t insertId = mysql_stmt_insert_id(stmt);
        if (usedPool) pool_[idx].lastInsertId = insertId;
        {
          std::lock_guard<std::mutex> lock(mutex_);
          lastInsertId_ = insertId;
        }
      }
    } else {
      if (mysql_stmt_execute(stmt) != 0) {
        logError("Execute (no params) failed: " + std::string(mysql_stmt_error(stmt)));
      } else {
        ok = true;
        const uint64_t insertId = mysql_stmt_insert_id(stmt);
        if (usedPool) pool_[idx].lastInsertId = insertId;
        {
          std::lock_guard<std::mutex> lock(mutex_);
          lastInsertId_ = insertId;
        }
      }
    }
  }

  mysql_stmt_close(stmt);
  if (usedPool) releaseConnection(idx);
  return ok;
}

std::vector<std::vector<std::string>> MySQLConnector::executePreparedQuery(
    const std::string& query, const std::vector<std::string>& params) {
  std::vector<std::vector<std::string>> results;

  size_t idx = acquireConnection(5000);
  void* conn = nullptr;
  bool usedPool = false;

  if (idx != POOL_NONE) {
    conn = pool_[idx].mysql;
    usedPool = true;
  } else {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_ || !connection_) {
      logError("Cannot execute prepared query: not connected");
      return results;
    }
    conn = connection_;
  }

  MYSQL* mysql = static_cast<MYSQL*>(conn);
  MYSQL_STMT* stmt = mysql_stmt_init(mysql);
  if (!stmt) {
    logError("mysql_stmt_init failed");
    if (usedPool) releaseConnection(idx);
    return results;
  }

  if (mysql_stmt_prepare(stmt, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
    logError("Prepare failed: " + std::string(mysql_stmt_error(stmt)));
    mysql_stmt_close(stmt);
    if (usedPool) releaseConnection(idx);
    return results;
  }

  unsigned long paramCount = mysql_stmt_param_count(stmt);
  if (paramCount > 0) {
    PreparedParamBind paramBind;
    std::string bindError;
    if (!bindPreparedParams(stmt, params, paramBind, bindError)) {
      logError("Bind failed: " + bindError);
      mysql_stmt_close(stmt);
      if (usedPool) releaseConnection(idx);
      return results;
    }
  }

  if (mysql_stmt_execute(stmt) != 0) {
    logError("Execute failed: " + std::string(mysql_stmt_error(stmt)));
    mysql_stmt_close(stmt);
    if (usedPool) releaseConnection(idx);
    return results;
  }

  MYSQL_RES* meta = mysql_stmt_result_metadata(stmt);
  if (!meta) {
    mysql_stmt_close(stmt);
    if (usedPool) releaseConnection(idx);
    return results;
  }

  const unsigned int numFields = mysql_num_fields(meta);
  PreparedResultBind resultBind;
  std::string resultBindError;
  if (!bindPreparedResults(meta, resultBind, resultBindError)) {
    logError("Bind result setup failed: " + resultBindError);
    mysql_free_result(meta);
    mysql_stmt_close(stmt);
    if (usedPool) releaseConnection(idx);
    return results;
  }

  if (mysql_stmt_bind_result(stmt, resultBind.binds.data()) != 0) {
    logError("Bind result failed: " + std::string(mysql_stmt_error(stmt)));
    mysql_free_result(meta);
    mysql_stmt_close(stmt);
    if (usedPool) releaseConnection(idx);
    return results;
  }

  mysql_stmt_store_result(stmt);

  while (mysql_stmt_fetch(stmt) == 0) {
    std::vector<std::string> row;
    row.reserve(numFields);
    for (unsigned int i = 0; i < numFields; ++i) {
      row.emplace_back(readPreparedResultCell(resultBind, i));
    }
    results.push_back(std::move(row));
  }

  mysql_free_result(meta);
  mysql_stmt_close(stmt);
  if (usedPool) releaseConnection(idx);
  return results;
}

std::optional<std::string> MySQLConnector::executePreparedScalar(
    const std::string& query, const std::vector<std::string>& params) {
  auto results = executePreparedQuery(query, params);
  if (results.empty() || results[0].empty()) return std::nullopt;
  return results[0][0];
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------

std::string MySQLConnector::escapeString(const std::string& input) {
  return Core::Utils::sanitizeInput(input);
}

uint64_t MySQLConnector::getLastInsertId() {
  std::lock_guard<std::mutex> lock(mutex_);
  return lastInsertId_;
}

bool MySQLConnector::beginTransaction() { return execute("START TRANSACTION"); }
bool MySQLConnector::commit() { return execute("COMMIT"); }
bool MySQLConnector::rollback() { return execute("ROLLBACK"); }

bool MySQLConnector::reconnect() {
  disconnect();
  return connect();
}

void MySQLConnector::logError(const std::string& message) {
  Core::Logger::getInstance().error("MySQL: {}", message);
}

}  // namespace Database
}  // namespace Umbra
