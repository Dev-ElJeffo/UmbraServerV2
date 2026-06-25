#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>
#include <queue>
#include <condition_variable>
#include <functional>

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
    bool autoReconnect = true;
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
    uint64_t lastInsertId = 0;
  };

  std::vector<PooledConnection> pool_;
  std::queue<size_t> available_;
  mutable std::mutex poolMutex_;
  std::condition_variable poolCond_;
  bool poolInitialized_ = false;

  void* connection_;
  mutable std::mutex mutex_;
  bool connected_;
  uint64_t lastInsertId_ = 0;

  bool createPooledConnection(PooledConnection& conn);
  size_t acquireConnection(uint32_t timeoutMs = 5000);
  void releaseConnection(size_t index);

  bool executeOnConnection(void* mysql, const std::string& query);
  std::optional<std::string> executeScalarOnConnection(void* mysql, const std::string& query);
  std::vector<std::vector<std::string>> executeQueryOnConnection(void* mysql, const std::string& query);

  bool reconnect();
  void logError(const std::string& message);
};

}  // namespace Database
}  // namespace Umbra
