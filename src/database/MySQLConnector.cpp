#include "MySQLConnector.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include <mysql.h>
#include <cstring>
#include <memory>

namespace Umbra {
namespace Database {

MySQLConnector::MySQLConnector(const Config& config)
    : config_(config),
      connection_(nullptr),
      connected_(false) {
  // Inicializar MySQL client library se necessário
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

bool MySQLConnector::connect() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (connected_ && connection_ != nullptr) {
    Core::Logger::getInstance().warn("Already connected to MySQL");
    return true;
  }
  
  Core::Logger::getInstance().info("Connecting to MySQL: {}:{}/{}", 
                                   config_.host, config_.port, config_.database);
  
  // Criar conexão MySQL
  MYSQL* mysql = mysql_init(nullptr);
  if (mysql == nullptr) {
    Core::Logger::getInstance().error("MySQL initialization failed: out of memory");
    return false;
  }
  
  // Configurar opções de conexão
  unsigned int timeout = config_.connectionTimeout;
  mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
  mysql_options(mysql, MYSQL_OPT_RECONNECT, &config_.autoReconnect);
  
  // Conectar ao servidor
  MYSQL* result = mysql_real_connect(
    mysql,
    config_.host.c_str(),
    config_.username.c_str(),
    config_.password.c_str(),
    config_.database.c_str(),
    config_.port,
    nullptr, // unix_socket
    0        // client_flag
  );
  
  if (result == nullptr) {
    std::string error = mysql_error(mysql);
    Core::Logger::getInstance().error("MySQL connection failed: {}", error);
    mysql_close(mysql);
    return false;
  }
  
  // Definir charset para UTF-8
  if (mysql_set_character_set(mysql, "utf8mb4") != 0) {
    Core::Logger::getInstance().warn("Failed to set UTF-8 charset: {}", mysql_error(mysql));
  }
  
  connection_ = mysql;
  connected_ = true;
  
  Core::Logger::getInstance().info("Connected to MySQL successfully");
  return true;
}

void MySQLConnector::disconnect() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!connected_ || connection_ == nullptr) {
    return;
  }
  
  MYSQL* mysql = static_cast<MYSQL*>(connection_);
  mysql_close(mysql);
  
  connected_ = false;
  connection_ = nullptr;
  
  Core::Logger::getInstance().info("Disconnected from MySQL");
}

bool MySQLConnector::isConnected() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return connected_;
}

bool MySQLConnector::execute(const std::string& query) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!connected_ || connection_ == nullptr) {
    logError("Cannot execute query: not connected");
    return false;
  }
  
  MYSQL* mysql = static_cast<MYSQL*>(connection_);
  
  Core::Logger::getInstance().debug("Executing query: {}", query);
  
  if (mysql_real_query(mysql, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
    std::string error = mysql_error(mysql);
    logError("Query execution failed: " + error);
    return false;
  }
  
  // Limpar resultados se houver
  MYSQL_RES* result = mysql_store_result(mysql);
  if (result != nullptr) {
    mysql_free_result(result);
  }
  
  return true;
}

std::optional<std::string> MySQLConnector::executeScalar(const std::string& query) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!connected_ || connection_ == nullptr) {
    logError("Cannot execute scalar query: not connected");
    return std::nullopt;
  }
  
  MYSQL* mysql = static_cast<MYSQL*>(connection_);
  
  Core::Logger::getInstance().debug("Executing scalar query: {}", query);
  
  if (mysql_real_query(mysql, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
    std::string error = mysql_error(mysql);
    logError("Scalar query execution failed: " + error);
    return std::nullopt;
  }
  
  MYSQL_RES* result = mysql_store_result(mysql);
  if (result == nullptr) {
    // Pode ser que a query não retorne resultados ou houve erro
    if (mysql_field_count(mysql) == 0) {
      // Query sem resultado (INSERT, UPDATE, DELETE, etc.)
      return std::nullopt;
    }
    logError("Failed to store result: " + std::string(mysql_error(mysql)));
    return std::nullopt;
  }
  
  MYSQL_ROW row = mysql_fetch_row(result);
  if (row == nullptr || mysql_num_fields(result) == 0) {
    mysql_free_result(result);
    return std::nullopt;
  }
  
  std::string value = row[0] ? row[0] : "";
  mysql_free_result(result);
  
  return value;
}

uint32_t MySQLConnector::prepareStatement(const std::string& query) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!connected_ || connection_ == nullptr) {
    logError("Cannot prepare statement: not connected");
    return 0;
  }
  
  // TODO: Implement prepared statement usando mysql_stmt_init
  // Por enquanto, retornar placeholder
  // Prepared statements requerem gerenciamento de estado adicional
  Core::Logger::getInstance().warn("Prepared statements not yet fully implemented. Use execute() instead.");
  Core::Logger::getInstance().debug("Preparing statement: {}", query);
  
  return 0;  // Placeholder - não implementado ainda
}

bool MySQLConnector::executePrepared(uint32_t statementId, 
                                     const std::vector<std::string>& params) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!connected_ || connection_ == nullptr) {
    logError("Cannot execute prepared statement: not connected");
    return false;
  }
  
  if (statementId == 0) {
    logError("Invalid prepared statement ID");
    return false;
  }
  
  // TODO: Implement prepared statement execution
  Core::Logger::getInstance().warn("Prepared statements not yet fully implemented. Use execute() instead.");
  Core::Logger::getInstance().debug("Executing prepared statement {} with {} params", 
                                    statementId, params.size());
  
  return false;  // Não implementado ainda
}

std::string MySQLConnector::escapeString(const std::string& input) {
  // Use Utils sanitization as fallback
  return Core::Utils::sanitizeInput(input);
}

uint64_t MySQLConnector::getLastInsertId() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!connected_ || connection_ == nullptr) {
    return 0;
  }
  
  MYSQL* mysql = static_cast<MYSQL*>(connection_);
  return static_cast<uint64_t>(mysql_insert_id(mysql));
}

std::vector<std::vector<std::string>> MySQLConnector::executeQuery(const std::string& query) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::vector<std::vector<std::string>> results;
  
  if (!connected_ || connection_ == nullptr) {
    logError("Cannot execute query: not connected");
    return results;
  }
  
  MYSQL* mysql = static_cast<MYSQL*>(connection_);
  
  Core::Logger::getInstance().debug("Executing query for result set: {}", query);
  
  if (mysql_real_query(mysql, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
    std::string error = mysql_error(mysql);
    logError("Query execution failed: " + error);
    return results;
  }
  
  MYSQL_RES* result = mysql_store_result(mysql);
  if (result == nullptr) {
    if (mysql_field_count(mysql) == 0) {
      // Query sem resultado
      return results;
    }
    logError("Failed to store result: " + std::string(mysql_error(mysql)));
    return results;
  }
  
  unsigned int numFields = mysql_num_fields(result);
  MYSQL_ROW row;
  
  while ((row = mysql_fetch_row(result)) != nullptr) {
    std::vector<std::string> rowData;
    for (unsigned int i = 0; i < numFields; i++) {
      rowData.push_back(row[i] ? row[i] : "");
    }
    results.push_back(rowData);
  }
  
  mysql_free_result(result);
  
  return results;
}

bool MySQLConnector::beginTransaction() {
  return execute("START TRANSACTION");
}

bool MySQLConnector::commit() {
  return execute("COMMIT");
}

bool MySQLConnector::rollback() {
  return execute("ROLLBACK");
}

bool MySQLConnector::reconnect() {
  disconnect();
  return connect();
}

void MySQLConnector::logError(const std::string& message) {
  Core::Logger::getInstance().error("MySQL: {}", message);
}

}  // namespace Database
}  // namespace Umbra

