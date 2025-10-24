#pragma once

#include "core/Logger.hpp"
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <optional>
#include <chrono>
#include <future>
#include <nlohmann/json.hpp>

namespace Umbra {
namespace Gateway {

/**
 * @brief Resultado de operação de autenticação
 */
struct AuthResponse {
  bool success = false;
  std::string message;
  std::string token;
  uint64_t accountId = 0;
  uint64_t playerId = 0;
  std::string username;
  bool valid = false;
};

/**
 * @brief Cliente TCP para comunicação com Auth Server
 * 
 * Gerencia conexão TCP com o servidor de autenticação para:
 * - Validação de tokens JWT
 * - Verificação de sessões ativas
 * - Operações de autenticação em tempo real
 */
class AuthClient {
 public:
  struct Config {
    std::string host = "localhost";
    uint16_t port = 8080;
    uint32_t connectionTimeoutMs = 5000;
    uint32_t requestTimeoutMs = 3000;
    uint32_t maxRetries = 3;
    uint32_t reconnectIntervalMs = 1000;
  };
  
  using ResponseCallback = std::function<void(const AuthResponse&)>;
  
  AuthClient(const Config& config);
  ~AuthClient();
  
  /**
   * @brief Inicia o cliente TCP
   * @return true se conectado com sucesso
   */
  bool start();
  
  /**
   * @brief Para o cliente
   */
  void stop();
  
  /**
   * @brief Verifica se está conectado
   * @return true se conectado
   */
  bool isConnected() const;
  
  /**
   * @brief Valida token JWT via TCP
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
   * @brief Obtém estatísticas da conexão
   * @return JSON com estatísticas
   */
  std::string getStats() const;

 private:
  Config config_;
  int socket_;
  std::atomic<bool> running_;
  std::atomic<bool> connected_;
  
  std::unique_ptr<std::thread> connectionThread_;
  std::unique_ptr<std::thread> responseThread_;
  
  // Message queue for responses
  struct PendingRequest {
    uint32_t requestId;
    ResponseCallback callback;
    std::chrono::system_clock::time_point timestamp;
  };
  
  std::queue<PendingRequest> pendingRequests_;
  mutable std::mutex pendingMutex_;
  std::condition_variable pendingCondition_;
  
  std::atomic<uint32_t> nextRequestId_;
  std::atomic<uint32_t> totalRequests_;
  std::atomic<uint32_t> successfulRequests_;
  std::atomic<uint32_t> failedRequests_;
  
  bool initializeSocket();
  void connectionLoop();
  void responseLoop();
  bool sendRequest(const std::string& request);
  void handleResponse(const std::string& response);
  void closeSocket();
  bool reconnect();
  
  std::string createRequest(const std::string& action, const nlohmann::json& data);
  AuthResponse parseResponse(const std::string& response);
};

}  // namespace Gateway
}  // namespace Umbra
