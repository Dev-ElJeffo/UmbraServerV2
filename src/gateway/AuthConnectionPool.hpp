#pragma once

#include "AuthClient.hpp"
#include "core/Logger.hpp"
#include <vector>
#include <memory>
#include <mutex>
#include <queue>
#include <atomic>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

namespace Umbra {
namespace Gateway {

/**
 * @brief Pool de conexões TCP para alta disponibilidade
 * 
 * Gerencia múltiplas conexões TCP com servidores de autenticação
 * para distribuir carga e garantir alta disponibilidade.
 */
class AuthConnectionPool {
 public:
  struct Config {
    std::vector<std::string> authHosts = {"localhost"};
    std::vector<uint16_t> authPorts = {8080};
    uint32_t maxConnectionsPerHost = 3;
    uint32_t connectionTimeoutMs = 5000;
    uint32_t requestTimeoutMs = 3000;
    uint32_t maxRetries = 3;
    uint32_t reconnectIntervalMs = 1000;
    uint32_t healthCheckIntervalMs = 30000;
    uint32_t maxIdleTimeMs = 300000; // 5 minutes
  };
  
  using ResponseCallback = std::function<void(const AuthResponse&)>;
  
  AuthConnectionPool(const Config& config);
  ~AuthConnectionPool();
  
  /**
   * @brief Inicia o pool de conexões
   * @return true se iniciado com sucesso
   */
  bool start();
  
  /**
   * @brief Para o pool
   */
  void stop();
  
  /**
   * @brief Verifica se há conexões ativas
   * @return true se há pelo menos uma conexão ativa
   */
  bool hasActiveConnections() const;
  
  /**
   * @brief Valida token JWT usando uma conexão do pool
   * @param token Token a validar
   * @param callback Callback para resposta assíncrona
   * @return true se requisição enviada
   */
  bool validateToken(const std::string& token, ResponseCallback callback);
  
  /**
   * @brief Valida token JWT de forma síncrona
   * @param token Token a validar
   * @return Resposta da validação
   */
  std::optional<AuthResponse> validateTokenSync(const std::string& token);
  
  /**
   * @brief Verifica se sessão está ativa
   * @param accountId ID da conta
   * @param callback Callback para resposta assíncrona
   * @return true se requisição enviada
   */
  bool checkSession(uint64_t accountId, ResponseCallback callback);
  
  /**
   * @brief Revoga token/sessão
   * @param token Token a revogar
   * @param callback Callback para resposta assíncrona
   * @return true se requisição enviada
   */
  bool revokeToken(const std::string& token, ResponseCallback callback);
  
  /**
   * @brief Obtém estatísticas do pool
   * @return JSON com estatísticas
   */
  std::string getStats() const;

 private:
  Config config_;
  std::vector<std::unique_ptr<AuthClient>> connections_;
  std::queue<AuthClient*> availableConnections_;
  mutable std::mutex poolMutex_;
  std::atomic<bool> running_;
  
  std::unique_ptr<std::thread> healthCheckThread_;
  std::atomic<uint32_t> nextConnectionIndex_;
  
  std::atomic<uint32_t> totalRequests_;
  std::atomic<uint32_t> successfulRequests_;
  std::atomic<uint32_t> failedRequests_;
  std::atomic<uint32_t> poolHits_;
  std::atomic<uint32_t> poolMisses_;
  
  void initializeConnections();
  void healthCheckLoop();
  AuthClient* getAvailableConnection();
  void returnConnection(AuthClient* connection);
  bool isConnectionHealthy(AuthClient* connection);
  void cleanupUnhealthyConnections();
};

}  // namespace Gateway
}  // namespace Umbra
