#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>
#include <queue>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <atomic>
#include <cstdint>
#include <cstddef>

namespace Umbra {
namespace Database {

class IDatabaseConnector {
 public:
  virtual ~IDatabaseConnector() = default;

  virtual bool connect() = 0;
  virtual void disconnect() = 0;
  virtual bool isConnected() const = 0;

  virtual bool execute(const std::string& query) = 0;
  virtual std::optional<std::string> executeScalar(const std::string& query) = 0;
};

class MySQLConnector : public IDatabaseConnector {
 public:
  struct Config {
    std::string host = "localhost";
    uint16_t port = 3306;
    std::string database = "umbra_eternum";
    std::string username = "root";
    std::string password = "";
    uint32_t connectionTimeout = 10;
    /** Reconnect implícito do client MySQL é inseguro com prepared stmts — default off. */
    bool autoReconnect = false;
    uint32_t poolSize = 5;
  };

  explicit MySQLConnector(const Config& config);
  ~MySQLConnector() override;

  bool connect() override;
  void disconnect() override;
  bool isConnected() const override;

  bool execute(const std::string& query) override;
  std::optional<std::string> executeScalar(const std::string& query) override;

  uint32_t prepareStatement(const std::string& query);
  bool executePrepared(uint32_t statementId,
                       const std::vector<std::string>& params);

  bool executePreparedInsert(const std::string& query,
                             const std::vector<std::string>& params);
  std::vector<std::vector<std::string>> executePreparedQuery(
      const std::string& query, const std::vector<std::string>& params);
  std::optional<std::string> executePreparedScalar(
      const std::string& query, const std::vector<std::string>& params);

  std::string escapeString(const std::string& input);
  uint64_t getLastInsertId();
  std::vector<std::vector<std::string>> executeQuery(const std::string& query);

  bool beginTransaction();
  bool commit();
  bool rollback();

 private:
  Config config_;

  struct PooledConnection {
    void* mysql = nullptr;
    bool inUse = false;
    bool needsRecreate = false;
    uint64_t lastInsertId = 0;
    std::chrono::steady_clock::time_point lastUsed{};
  };

  std::vector<PooledConnection> pool_;
  std::queue<size_t> available_;
  mutable std::mutex poolMutex_;
  std::condition_variable poolCond_;
  bool poolInitialized_ = false;
  std::atomic<bool> shuttingDown_{false};
  std::atomic<int> activeLeases_{0};

  void* connection_ = nullptr;
  mutable std::mutex mutex_;
  bool connected_ = false;
  uint64_t lastInsertId_ = 0;

  /** Lease RAII do pool: devolve a conexão exatamente uma vez (sucesso/erro/exception). */
  class ConnectionLease {
   public:
    ConnectionLease() = default;
    ConnectionLease(MySQLConnector* owner, size_t index);
    ConnectionLease(const ConnectionLease&) = delete;
    ConnectionLease& operator=(const ConnectionLease&) = delete;
    ConnectionLease(ConnectionLease&& other) noexcept;
    ConnectionLease& operator=(ConnectionLease&& other) noexcept;
    ~ConnectionLease();

    bool valid() const { return owner_ != nullptr && index_ != kNone; }
    size_t index() const { return index_; }
    void* mysql() const;
    void markNeedsRecreate();
    void release();

   private:
    static constexpr size_t kNone = SIZE_MAX;
    MySQLConnector* owner_ = nullptr;
    size_t index_ = kNone;
  };

  bool createPooledConnection(PooledConnection& conn);
  ConnectionLease acquireLease(uint32_t timeoutMs = 5000);
  void releaseConnection(size_t index);
  void markConnectionNeedsRecreate(size_t index);
  void ensureMysqlThreadLocal();
  static bool isServerLostError(unsigned int err);

  bool executeOnConnection(void* mysql, const std::string& query);
  std::optional<std::string> executeScalarOnConnection(void* mysql, const std::string& query);
  std::vector<std::vector<std::string>> executeQueryOnConnection(void* mysql, const std::string& query);

  bool reconnect();
  void logError(const std::string& message);
};

}  // namespace Database
}  // namespace Umbra
