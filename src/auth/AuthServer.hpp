#pragma once

#include "JWTManager.hpp"
#include "SessionManager.hpp"
#include "database/MySQLConnector.hpp"
#include "database/AccountDAO.hpp"
#include "database/PlayerDAO.hpp"
#include "network/SocketServer.hpp"
#include <memory>
#include <string>

namespace Umbra {
namespace Auth {

/**
 * @brief Resultado de autenticação
 */
struct AuthResult {
  bool success = false;
  std::string message;
  std::string token;
  uint64_t accountId = 0;
  uint64_t playerId = 0;
  std::string username;
};

/**
 * @brief Servidor de autenticação
 * 
 * Gerencia login, registro, validação de tokens e sessões.
 * Endpoint REST para integração com UE5 via VaRestX.
 */
class AuthServer {
 public:
  struct Config {
    uint16_t port = 8080;
    std::string jwtSecret;
    uint32_t sessionDurationMinutes = 60;
    uint32_t maxLoginAttempts = 5;
    uint32_t loginAttemptWindowSeconds = 300;
  };
  
  AuthServer(const Config& config,
             std::shared_ptr<Database::MySQLConnector> dbConnector);
  ~AuthServer();
  
  /**
   * @brief Inicia o servidor de autenticação
   * @return true se iniciado com sucesso
   */
  bool start();
  
  /**
   * @brief Para o servidor
   */
  void stop();
  
  /**
   * @brief Verifica se está rodando
   * @return true se ativo
   */
  bool isRunning() const;
  
  /**
   * @brief Registra nova conta
   * @param username Nome de usuário
   * @param email Email
   * @param password Senha
   * @return Resultado da operação
   */
  AuthResult registerAccount(const std::string& username,
                             const std::string& email,
                             const std::string& password);
  
  /**
   * @brief Realiza login
   * @param usernameOrEmail Username ou email
   * @param password Senha
   * @param ipAddress IP do cliente
   * @return Resultado da autenticação
   */
  AuthResult login(const std::string& usernameOrEmail,
                   const std::string& password,
                   const std::string& ipAddress);
  
  /**
   * @brief Realiza logout
   * @param token Token da sessão
   * @return true se logout realizado
   */
  bool logout(const std::string& token);
  
  /**
   * @brief Valida token JWT
   * @param token Token a validar
   * @return Payload ou nullopt se inválido
   */
  std::optional<JWTPayload> validateToken(const std::string& token);
  
  /**
   * @brief Atualiza token (refresh)
   * @param oldToken Token existente
   * @return Novo token ou string vazia se falhou
   */
  std::string refreshToken(const std::string& oldToken);
  
  /**
   * @brief Obtém estatísticas do servidor
   * @return JSON com estatísticas
   */
  std::string getStats() const;
  size_t getActiveSessionCount() const;

 private:
  Config config_;
  std::unique_ptr<JWTManager> jwtManager_;
  std::unique_ptr<SessionManager> sessionManager_;
  std::shared_ptr<Database::MySQLConnector> dbConnector_;
  std::unique_ptr<Database::AccountDAO> accountDAO_;
  std::unique_ptr<Database::PlayerDAO> playerDAO_;
  std::unique_ptr<Network::SocketServer> networkServer_;
  
  std::map<std::string, uint32_t> loginAttempts_;
  std::mutex attemptsMutex_;
  
  bool checkLoginAttempts(const std::string& identifier);
  void recordLoginAttempt(const std::string& identifier, bool success);
  void handleClientMessage(uint32_t clientId, const std::vector<uint8_t>& data);
  void sendResponse(uint32_t clientId, const std::string& response);
};

}  // namespace Auth
}  // namespace Umbra

