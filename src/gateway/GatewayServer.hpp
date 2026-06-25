#pragma once

#include "LoadBalancer.hpp"
#include "AuthClient.hpp"
#include "AuthConnectionPool.hpp"
#include "auth/JWTManager.hpp"
#include "network/SocketServer.hpp"
#include <memory>
#include <string>
#include <map>
#include <mutex>

namespace Umbra {
namespace Gateway {

class GatewayServer {
 public:
  struct Config {
    uint16_t port = 9000;
    std::string jwtSecret;
    uint32_t rateLimitPerSecond = 100;
    
    // Auth Server connection
    std::string authHost = "localhost";
    uint16_t authPort = 8080;
    uint32_t authTimeoutMs = 5000;
    
    // Connection pool settings
    bool useConnectionPool = true;
    uint32_t maxConnectionsPerHost = 3;
    uint32_t healthCheckIntervalMs = 30000;
  };
  
  GatewayServer(const Config& config);
  ~GatewayServer();
  
  bool start();
  void stop();
  bool isRunning() const;
  
  LoadBalancer& getLoadBalancer();
  
  /**
   * @brief Valida token JWT via TCP com Auth Server
   * @param token Token a validar
   * @return true se token válido
   */
  bool validateToken(const std::string& token);
  
  /**
   * @brief Obtém informações do cliente autenticado
   * @param token Token do cliente
   * @return Informações do cliente ou nullopt se inválido
   */
  std::optional<AuthResponse> getClientInfo(const std::string& token);

  std::string getAdminClientsJson();
  bool kickClient(uint32_t clientId);
  std::string getAuthStatsJson() const;

 private:
  Config config_;
  std::unique_ptr<LoadBalancer> loadBalancer_;
  std::unique_ptr<AuthClient> authClient_;
  std::unique_ptr<AuthConnectionPool> authPool_;
  std::unique_ptr<Network::SocketServer> networkServer_;
  std::unique_ptr<Auth::JWTManager> jwtManager_;
  
  bool running_;
  
  // Client session management
  std::map<std::string, AuthResponse> clientSessions_;
  std::mutex sessionsMutex_;
  
  void handleConnection(uint32_t clientId);
  void handleClientMessage(uint32_t clientId, const std::vector<uint8_t>& data);
  void sendResponse(uint32_t clientId, const std::string& response);
  void cleanupExpiredSessions();
};

}  // namespace Gateway
}  // namespace Umbra

