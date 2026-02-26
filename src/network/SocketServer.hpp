#pragma once

#include <string>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <chrono>

namespace Umbra {
namespace Network {

/**
 * @brief Tipo de protocolo de rede
 */
enum class ProtocolType {
  TCP,
  UDP
};

/**
 * @brief Dados de conexão do cliente
 */
struct ClientConnection {
  uint32_t id;
  std::string address;
  uint16_t port;
  bool connected;
  int socket;
};

/**
 * @brief Servidor de sockets TCP/UDP base
 * 
 * Implementação básica de servidor de rede usando sockets BSD.
 * Suporta TCP para conexões confiáveis e UDP para baixa latência.
 */
class SocketServer {
 public:
  using MessageCallback = std::function<void(uint32_t clientId, 
                                             const std::vector<uint8_t>& data)>;
  using ConnectionCallback = std::function<void(uint32_t clientId, bool connected)>;
  
  SocketServer(ProtocolType type, uint16_t port);
  ~SocketServer();
  
  /**
   * @brief Inicia o servidor
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
   * @brief Define callback para mensagens recebidas
   * @param callback Função a ser chamada
   */
  void setMessageCallback(MessageCallback callback);
  
  /**
   * @brief Define callback para mudanças de conexão
   * @param callback Função a ser chamada
   */
  void setConnectionCallback(ConnectionCallback callback);
  
  /**
   * @brief Envia dados para cliente
   * @param clientId ID do cliente
   * @param data Dados a enviar
   * @return true se enviado com sucesso
   */
  bool sendToClient(uint32_t clientId, const std::vector<uint8_t>& data);
  
  /**
   * @brief Envia dados para todos os clientes
   * @param data Dados a enviar
   */
  void broadcast(const std::vector<uint8_t>& data);
  
  /**
   * @brief Desconecta cliente
   * @param clientId ID do cliente
   */
  void disconnectClient(uint32_t clientId);
  
  /**
   * @brief Obtém número de clientes conectados
   * @return Número de conexões ativas
   */
  size_t getClientCount() const;
  
  /**
   * @brief Define limite de taxa (rate limiting)
   * @param maxMessagesPerSecond Máximo de mensagens por segundo por cliente
   */
  void setRateLimit(uint32_t maxMessagesPerSecond);

  void setMaxConnections(uint32_t maxConnections);

 private:
  ProtocolType type_;
  uint16_t port_;
  int serverSocket_;
  std::atomic<bool> running_;
  std::unique_ptr<std::thread> acceptThread_;
  std::vector<std::unique_ptr<std::thread>> workerThreads_;
  
  std::vector<ClientConnection> clients_;
  mutable std::mutex clientsMutex_;
  std::atomic<uint32_t> nextClientId_;
  
  MessageCallback messageCallback_;
  ConnectionCallback connectionCallback_;
  
  uint32_t rateLimitPerSecond_;
  uint32_t maxConnections_ = 10000;
  
  struct ClientRateInfo {
    uint32_t messageCount = 0;
    std::chrono::steady_clock::time_point windowStart;
  };
  std::unordered_map<uint32_t, ClientRateInfo> clientRates_;
  std::mutex rateMutex_;
  
  bool checkRateLimit(uint32_t clientId);
  
  bool initializeSocket();
  void acceptLoop();
  void handleTcpClient(int clientSocket, const std::string& address, uint16_t port);
  void handleUdpMessages();
  void closeSocket(int socket);
  
  ClientConnection* findClient(uint32_t clientId);
};

}  // namespace Network
}  // namespace Umbra

