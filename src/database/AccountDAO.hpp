#pragma once

#include "Models.hpp"
#include "MySQLConnector.hpp"
#include <memory>
#include <optional>

namespace Umbra {
namespace Database {

/**
 * @brief Data Access Object para contas de usuário
 */
class AccountDAO {
 public:
  explicit AccountDAO(std::shared_ptr<MySQLConnector> connector);
  
  /**
   * @brief Cria nova conta
   * @param account Dados da conta
   * @return ID da conta criada ou 0 se falhou
   */
  uint64_t createAccount(const Account& account);
  
  /**
   * @brief Busca conta por ID
   * @param id ID da conta
   * @return Account ou nullopt se não encontrado
   */
  std::optional<Account> getAccountById(uint64_t id);
  
  /**
   * @brief Busca conta por username
   * @param username Nome de usuário
   * @return Account ou nullopt se não encontrado
   */
  std::optional<Account> getAccountByUsername(const std::string& username);
  
  /**
   * @brief Busca conta por email
   * @param email Email do usuário
   * @return Account ou nullopt se não encontrado
   */
  std::optional<Account> getAccountByEmail(const std::string& email);
  
  /**
   * @brief Atualiza dados da conta
   * @param account Dados atualizados
   * @return true se atualizado com sucesso
   */
  bool updateAccount(const Account& account);
  
  /**
   * @brief Deleta conta
   * @param id ID da conta
   * @return true se deletado com sucesso
   */
  bool deleteAccount(uint64_t id);
  
  /**
   * @brief Verifica se username já existe
   * @param username Nome de usuário
   * @return true se existe
   */
  bool usernameExists(const std::string& username);
  
  /**
   * @brief Verifica se email já existe
   * @param email Email
   * @return true se existe
   */
  bool emailExists(const std::string& email);
  
  /**
   * @brief Bane conta
   * @param id ID da conta
   * @param reason Razão do banimento
   * @return true se banido com sucesso
   */
  bool banAccount(uint64_t id, const std::string& reason);
  
  /**
   * @brief Remove banimento de conta
   * @param id ID da conta
   * @return true se desbanido com sucesso
   */
  bool unbanAccount(uint64_t id);
  
  /**
   * @brief Atualiza último login
   * @param id ID da conta
   * @return true se atualizado com sucesso
   */
  bool updateLastLogin(uint64_t id);

 private:
  std::shared_ptr<MySQLConnector> connector_;
  
  static const std::string ACCOUNT_SELECT_FIELDS;
  
  std::optional<Account> parseAccountFromQuery(const std::string& query);
  std::optional<Account> parseAccountRow(const std::vector<std::string>& row);
};

}  // namespace Database
}  // namespace Umbra

