#include "MySQLConnector.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include <mysql.h>
#include <cctype>
#include <chrono>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#ifndef CR_SERVER_GONE_ERROR
#define CR_SERVER_GONE_ERROR 2006
#endif
#ifndef CR_SERVER_LOST
#define CR_SERVER_LOST 2013
#endif

namespace {

constexpr unsigned long kResultColumnBuf = 4096;
constexpr unsigned long kClientMaxPacket = 64UL * 1024UL * 1024UL;
constexpr size_t POOL_NONE = SIZE_MAX;

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
  // Reconnect implícito + prepared statements = abort clássico em libmysql.
  // MYSQL_OPT_RECONNECT está deprecated no client 8.x — só setar se explicitamente pedido.
  if (config.autoReconnect) {
    const char reconnectFlag = 1;
    mysql_options(mysql, MYSQL_OPT_RECONNECT, &reconnectFlag);
  }
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

std::once_flag gMysqlLibraryOnce;

}  // namespace

namespace Umbra {
namespace Database {

// ---------------------------------------------------------------------------
// ConnectionLease
// ---------------------------------------------------------------------------

MySQLConnector::ConnectionLease::ConnectionLease(MySQLConnector* owner, size_t index)
    : owner_(owner), index_(index) {
  if (owner_ && index_ != kNone) {
    owner_->activeLeases_.fetch_add(1, std::memory_order_relaxed);
  }
}

MySQLConnector::ConnectionLease::ConnectionLease(ConnectionLease&& other) noexcept
    : owner_(other.owner_), index_(other.index_) {
  other.owner_ = nullptr;
  other.index_ = kNone;
}

MySQLConnector::ConnectionLease& MySQLConnector::ConnectionLease::operator=(
    ConnectionLease&& other) noexcept {
  if (this != &other) {
    release();
    owner_ = other.owner_;
    index_ = other.index_;
    other.owner_ = nullptr;
    other.index_ = kNone;
  }
  return *this;
}

MySQLConnector::ConnectionLease::~ConnectionLease() {
  release();
}

void* MySQLConnector::ConnectionLease::mysql() const {
  if (!valid()) return nullptr;
  return owner_->pool_[index_].mysql;
}

void MySQLConnector::ConnectionLease::markNeedsRecreate() {
  if (valid()) {
    owner_->markConnectionNeedsRecreate(index_);
  }
}

void MySQLConnector::ConnectionLease::release() {
  if (!owner_ || index_ == kNone) return;
  MySQLConnector* owner = owner_;
  const size_t idx = index_;
  owner_ = nullptr;
  index_ = kNone;
  owner->releaseConnection(idx);
  owner->activeLeases_.fetch_sub(1, std::memory_order_relaxed);
}

// ---------------------------------------------------------------------------
// ctor / dtor / thread helpers
// ---------------------------------------------------------------------------

MySQLConnector::MySQLConnector(const Config& config)
    : config_(config),
      connection_(nullptr),
      connected_(false) {
  std::call_once(gMysqlLibraryOnce, []() {
    if (mysql_library_init(0, nullptr, nullptr) != 0) {
      Core::Logger::getInstance().error("MySQL library initialization failed");
    } else {
      Core::Logger::getInstance().debug("MySQL library initialized");
    }
  });
}

MySQLConnector::~MySQLConnector() {
  disconnect();
}

void MySQLConnector::ensureMysqlThreadLocal() {
  // mysql_thread_init na 1ª operação C API desta thread.
  // NÃO chamar mysql_thread_end no dtor thread_local: no client 8.0.x (Windows)
  // isso aborta com 0x80000003 (STATUS_BREAKPOINT) quando a thread termina enquanto
  // o pool ainda usa a lib — sintoma típico: zone sobe ~1s e cai em libmysql.dll.
  thread_local bool inited = false;
  if (!inited) {
    mysql_thread_init();
    inited = true;
  }
}

bool MySQLConnector::isServerLostError(unsigned int err) {
  return err == CR_SERVER_GONE_ERROR || err == CR_SERVER_LOST;
}

// ---------------------------------------------------------------------------
// Pool management
// ---------------------------------------------------------------------------

bool MySQLConnector::createPooledConnection(PooledConnection& conn) {
  ensureMysqlThreadLocal();
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
  conn.needsRecreate = false;
  conn.lastUsed = std::chrono::steady_clock::now();
  return true;
}

MySQLConnector::ConnectionLease MySQLConnector::acquireLease(uint32_t timeoutMs) {
  ensureMysqlThreadLocal();

  if (shuttingDown_.load(std::memory_order_acquire)) {
    return ConnectionLease{};
  }

  size_t idx = POOL_NONE;
  std::chrono::steady_clock::time_point lastUsed{};
  void* mysqlRaw = nullptr;
  bool needsRecreate = false;

  {
    std::unique_lock<std::mutex> lock(poolMutex_);

    if (!poolInitialized_ || pool_.empty()) {
      return ConnectionLease{};
    }

    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);

    while (available_.empty()) {
      if (shuttingDown_.load(std::memory_order_acquire)) {
        return ConnectionLease{};
      }
      if (poolCond_.wait_until(lock, deadline) == std::cv_status::timeout) {
        Core::Logger::getInstance().warn(
            "Pool: acquire timed out after {}ms (Pool exhausted — sem fallback à primary)",
            timeoutMs);
        return ConnectionLease{};
      }
    }

    idx = available_.front();
    available_.pop();
    pool_[idx].inUse = true;
    lastUsed = pool_[idx].lastUsed;
    mysqlRaw = pool_[idx].mysql;
    needsRecreate = pool_[idx].needsRecreate || (mysqlRaw == nullptr);
  }

  constexpr auto kIdlePingThreshold = std::chrono::seconds(30);
  const auto now = std::chrono::steady_clock::now();
  const bool idleTooLong =
      (lastUsed.time_since_epoch().count() == 0) || ((now - lastUsed) > kIdlePingThreshold);

  if (needsRecreate || (mysqlRaw && idleTooLong)) {
    MYSQL* mysql = static_cast<MYSQL*>(mysqlRaw);
    bool mustRecreate = needsRecreate;
    if (!mustRecreate && mysql && mysql_ping(mysql) != 0) {
      mustRecreate = true;
    }
    if (mustRecreate) {
      std::lock_guard<std::mutex> lock(poolMutex_);
      Core::Logger::getInstance().warn("Pool: connection {} lost/stale, recreating...", idx);
      if (pool_[idx].mysql) {
        mysql_close(static_cast<MYSQL*>(pool_[idx].mysql));
        pool_[idx].mysql = nullptr;
      }
      pool_[idx].needsRecreate = false;
      if (!createPooledConnection(pool_[idx])) {
        pool_[idx].inUse = false;
        available_.push(idx);
        poolCond_.notify_one();
        return ConnectionLease{};
      }
      pool_[idx].inUse = true;
    }
  }

  return ConnectionLease(this, idx);
}

void MySQLConnector::releaseConnection(size_t index) {
  std::lock_guard<std::mutex> lock(poolMutex_);
  if (index < pool_.size()) {
    pool_[index].inUse = false;
    pool_[index].lastUsed = std::chrono::steady_clock::now();
    available_.push(index);
    poolCond_.notify_one();
  }
}

void MySQLConnector::markConnectionNeedsRecreate(size_t index) {
  std::lock_guard<std::mutex> lock(poolMutex_);
  if (index < pool_.size()) {
    pool_[index].needsRecreate = true;
  }
}

// ---------------------------------------------------------------------------
// connect / disconnect
// ---------------------------------------------------------------------------

bool MySQLConnector::connect() {
  ensureMysqlThreadLocal();
  std::lock_guard<std::mutex> lock(mutex_);

  if (connected_) return true;
  shuttingDown_.store(false, std::memory_order_release);

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
    Core::Logger::getInstance().info("MySQL connection pool: {}/{} connections created", created,
                                     poolSize);
  }

  return true;
}

void MySQLConnector::disconnect() {
  shuttingDown_.store(true, std::memory_order_release);
  poolCond_.notify_all();

  // Aguarda leases ativos (máx ~5s) para não fechar MYSQL* em uso.
  for (int i = 0; i < 50 && activeLeases_.load(std::memory_order_acquire) > 0; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  if (activeLeases_.load(std::memory_order_acquire) > 0) {
    Core::Logger::getInstance().warn(
        "MySQL disconnect: {} lease(s) ainda ativos — fechando mesmo assim",
        activeLeases_.load());
  }

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

std::optional<std::string> MySQLConnector::executeScalarOnConnection(void* conn,
                                                                    const std::string& query) {
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

std::vector<std::vector<std::string>> MySQLConnector::executeQueryOnConnection(
    void* conn, const std::string& query) {
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
// Public API — pool only (sem fallback à primary sem lock)
// ---------------------------------------------------------------------------

bool MySQLConnector::execute(const std::string& query) {
  ConnectionLease lease = acquireLease(5000);
  if (!lease.valid()) {
    logError("Cannot execute query: pool exhausted or not connected");
    return false;
  }
  Core::Logger::getInstance().debug("Executing query (pool[{}]): {}", lease.index(), query);
  const bool ok = executeOnConnection(lease.mysql(), query);
  if (!ok) {
    MYSQL* mysql = static_cast<MYSQL*>(lease.mysql());
    if (mysql && isServerLostError(mysql_errno(mysql))) {
      lease.markNeedsRecreate();
    }
  }
  return ok;
}

std::optional<std::string> MySQLConnector::executeScalar(const std::string& query) {
  ConnectionLease lease = acquireLease(5000);
  if (!lease.valid()) {
    logError("Cannot execute scalar: pool exhausted or not connected");
    return std::nullopt;
  }
  Core::Logger::getInstance().debug("Executing scalar (pool[{}]): {}", lease.index(), query);
  auto result = executeScalarOnConnection(lease.mysql(), query);
  if (!result) {
    MYSQL* mysql = static_cast<MYSQL*>(lease.mysql());
    if (mysql && isServerLostError(mysql_errno(mysql))) {
      lease.markNeedsRecreate();
    }
  }
  return result;
}

std::vector<std::vector<std::string>> MySQLConnector::executeQuery(const std::string& query) {
  ConnectionLease lease = acquireLease(5000);
  if (!lease.valid()) {
    logError("Cannot execute query: pool exhausted or not connected");
    return {};
  }
  Core::Logger::getInstance().debug("Executing result query (pool[{}]): {}", lease.index(), query);
  auto result = executeQueryOnConnection(lease.mysql(), query);
  MYSQL* mysql = static_cast<MYSQL*>(lease.mysql());
  if (mysql && isServerLostError(mysql_errno(mysql))) {
    lease.markNeedsRecreate();
  }
  return result;
}

// ---------------------------------------------------------------------------
// Prepared statements
// ---------------------------------------------------------------------------

uint32_t MySQLConnector::prepareStatement(const std::string& query) {
  (void)query;
  Core::Logger::getInstance().warn(
      "prepareStatement(id) deprecated — use executePreparedQuery/executePreparedInsert directly");
  return 0;
}

bool MySQLConnector::executePrepared(uint32_t statementId, const std::vector<std::string>& params) {
  (void)statementId;
  (void)params;
  Core::Logger::getInstance().warn(
      "executePrepared(id) deprecated — use executePreparedInsert directly");
  return false;
}

bool MySQLConnector::executePreparedInsert(const std::string& query,
                                           const std::vector<std::string>& params) {
  ConnectionLease lease = acquireLease(5000);
  if (!lease.valid()) {
    logError("Cannot execute prepared insert: pool exhausted or not connected");
    return false;
  }

  MYSQL* mysql = static_cast<MYSQL*>(lease.mysql());
  MYSQL_STMT* stmt = mysql_stmt_init(mysql);
  if (!stmt) {
    logError("mysql_stmt_init failed");
    return false;
  }

  bool ok = false;
  if (mysql_stmt_prepare(stmt, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
    logError("Prepare failed: " + std::string(mysql_stmt_error(stmt)));
    if (isServerLostError(mysql_stmt_errno(stmt))) {
      lease.markNeedsRecreate();
    }
  } else {
    unsigned long paramCount = mysql_stmt_param_count(stmt);
    if (paramCount != params.size()) {
      logError("Param count mismatch: expected " + std::to_string(paramCount) + " got " +
               std::to_string(params.size()));
    } else {
      bool bindOk = true;
      PreparedParamBind paramBind;
      if (paramCount > 0) {
        std::string bindError;
        bindOk = bindPreparedParams(stmt, params, paramBind, bindError);
        if (!bindOk) {
          logError("Bind failed: " + bindError);
        }
      }
      if (bindOk) {
        if (mysql_stmt_execute(stmt) != 0) {
          logError("Execute failed: " + std::string(mysql_stmt_error(stmt)));
          if (isServerLostError(mysql_stmt_errno(stmt))) {
            lease.markNeedsRecreate();
          }
        } else {
          ok = true;
          const uint64_t insertId = mysql_stmt_insert_id(stmt);
          if (lease.index() < pool_.size()) {
            pool_[lease.index()].lastInsertId = insertId;
          }
          {
            std::lock_guard<std::mutex> lock(mutex_);
            lastInsertId_ = insertId;
          }
        }
      }
    }
  }

  mysql_stmt_close(stmt);
  return ok;
}

std::vector<std::vector<std::string>> MySQLConnector::executePreparedQuery(
    const std::string& query, const std::vector<std::string>& params) {
  std::vector<std::vector<std::string>> results;

  ConnectionLease lease = acquireLease(5000);
  if (!lease.valid()) {
    logError("Cannot execute prepared query: pool exhausted or not connected");
    return results;
  }

  MYSQL* mysql = static_cast<MYSQL*>(lease.mysql());
  MYSQL_STMT* stmt = mysql_stmt_init(mysql);
  if (!stmt) {
    logError("mysql_stmt_init failed");
    return results;
  }

  if (mysql_stmt_prepare(stmt, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
    logError("Prepare failed: " + std::string(mysql_stmt_error(stmt)));
    if (isServerLostError(mysql_stmt_errno(stmt))) {
      lease.markNeedsRecreate();
    }
    mysql_stmt_close(stmt);
    return results;
  }

  unsigned long paramCount = mysql_stmt_param_count(stmt);
  if (paramCount > 0) {
    PreparedParamBind paramBind;
    std::string bindError;
    if (!bindPreparedParams(stmt, params, paramBind, bindError)) {
      logError("Bind failed: " + bindError);
      mysql_stmt_close(stmt);
      return results;
    }
  }

  if (mysql_stmt_execute(stmt) != 0) {
    logError("Execute failed: " + std::string(mysql_stmt_error(stmt)));
    if (isServerLostError(mysql_stmt_errno(stmt))) {
      lease.markNeedsRecreate();
    }
    mysql_stmt_close(stmt);
    return results;
  }

  MYSQL_RES* meta = mysql_stmt_result_metadata(stmt);
  if (!meta) {
    logError("Prepared query result_metadata nulo (stmt_errno=" +
             std::to_string(mysql_stmt_errno(stmt)) + "): " +
             std::string(mysql_stmt_error(stmt)));
    mysql_stmt_close(stmt);
    return results;
  }

  const unsigned int numFields = mysql_num_fields(meta);
  PreparedResultBind resultBind;
  std::string resultBindError;
  if (!bindPreparedResults(meta, resultBind, resultBindError)) {
    logError("Bind result setup failed: " + resultBindError);
    mysql_free_result(meta);
    mysql_stmt_close(stmt);
    return results;
  }

  if (mysql_stmt_bind_result(stmt, resultBind.binds.data()) != 0) {
    logError("Bind result failed: " + std::string(mysql_stmt_error(stmt)));
    mysql_free_result(meta);
    mysql_stmt_close(stmt);
    return results;
  }

  if (mysql_stmt_store_result(stmt) != 0) {
    logError("Store result failed: " + std::string(mysql_stmt_error(stmt)));
    if (isServerLostError(mysql_stmt_errno(stmt))) {
      lease.markNeedsRecreate();
    }
    mysql_free_result(meta);
    mysql_stmt_close(stmt);
    return results;
  }

  int fetchRc = 0;
  while (true) {
    fetchRc = mysql_stmt_fetch(stmt);
    if (fetchRc != 0 && fetchRc != MYSQL_DATA_TRUNCATED) break;

    std::vector<std::string> row;
    row.reserve(numFields);
    for (unsigned int i = 0; i < numFields; ++i) {
      if (!resultBind.isInteger[i] && resultBind.lengths[i] > kResultColumnBuf) {
        std::vector<char> fullBuf(resultBind.lengths[i]);
        MYSQL_BIND rebind;
        memset(&rebind, 0, sizeof(rebind));
        unsigned long realLen = 0;
        char nullFlag = 0;
        rebind.buffer_type = MYSQL_TYPE_STRING;
        rebind.buffer = fullBuf.data();
        rebind.buffer_length = static_cast<unsigned long>(fullBuf.size());
        rebind.length = &realLen;
        rebind.is_null = reinterpret_cast<bool*>(&nullFlag);
        if (mysql_stmt_fetch_column(stmt, &rebind, i, 0) == 0 && !nullFlag) {
          const unsigned long n = realLen ? realLen : static_cast<unsigned long>(fullBuf.size());
          row.emplace_back(fullBuf.data(), n);
        } else {
          row.emplace_back();
        }
      } else {
        row.emplace_back(readPreparedResultCell(resultBind, i));
      }
    }
    results.push_back(std::move(row));
  }

  if (fetchRc == 1) {
    logError("Prepared fetch failed (stmt_errno=" + std::to_string(mysql_stmt_errno(stmt)) +
             "): " + std::string(mysql_stmt_error(stmt)));
  } else if (results.empty()) {
    Core::Logger::getInstance().debug(
        "Prepared query 0 linhas (num_rows={}, fetch_rc={})",
        static_cast<unsigned long long>(mysql_stmt_num_rows(stmt)), fetchRc);
  }

  mysql_stmt_free_result(stmt);
  mysql_free_result(meta);
  mysql_stmt_close(stmt);
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
