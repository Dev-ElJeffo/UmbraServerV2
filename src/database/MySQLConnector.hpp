#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>

namespace Umbra {
namespace Database {

/**
 * @brief Interface para conexão com banco de dados
 */
class IDatabaseConnector {
 public:
  virtual ~IDatabaseConnector() = default;
  
  virtual bool connect() = 0;
  virtual void disconnect() = 0;
  virtual bool isConnected() const = 0;
  
  virtual bool execute(const std::string& query) = 0;
  virtual std::optional<std::string> executeScalar(const std::string& query) = 0;
};

/**
 * @brief Wrapper para MySQL connector com prepared statements
 * 
 * Gerencia pool de conexões e execução segura de queries.
 */
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
  };
  
  explicit MySQLConnector(const Config& config);
  ~MySQLConnector() override;
  
  bool connect() override;
  void disconnect() override;
  bool isConnected() const override;
  
  /**
   * @brief Executa query sem retorno
   * @param query Query SQL
   * @return true se executado com sucesso
   */
  bool execute(const std::string& query) override;
  
  /**
   * @brief Executa query e retorna um valor escalar
   * @param query Query SQL
   * @return Valor escalar ou nullopt
   */
  std::optional<std::string> executeScalar(const std::string& query) override;
  
  /**
   * @brief Prepara statement para execução
   * @param query Query SQL com placeholders (?)
   * @return ID do statement preparado
   */
  uint32_t prepareStatement(const std::string& query);
  
  /**
   * @brief Executa prepared statement
   * @param statementId ID do statement
   * @param params Parâmetros para bind
   * @return true se executado com sucesso
   */
  bool executePrepared(uint32_t statementId, 
                       const std::vector<std::string>& params);
  
  /**
   * @brief Escapa string para prevenir SQL injection
   * @param input String a escapar
   * @return String escapada
   */
  std::string escapeString(const std::string& input);
  
  /**
   * @brief Obtém ID do último insert
   * @return ID do último registro inserido
   */
  uint64_t getLastInsertId();
  
  /**
   * @brief Inicia transação
   * @return true se iniciado com sucesso
   */
  bool beginTransaction();
  
  /**
   * @brief Commit de transação
   * @return true se commit realizado
   */
  bool commit();
  
  /**
   * @brief Rollback de transação
   * @return true se rollback realizado
   */
  bool rollback();

 private:
  Config config_;
  void* connection_;  // Pointer to MySQL connection (opaque)
  mutable std::mutex mutex_;
  bool connected_;
  
  bool reconnect();
  void logError(const std::string& message);
};

}  // namespace Database
}  // namespace Umbra

