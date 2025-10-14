#pragma once

#include <string>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <map>

namespace Umbra {
namespace Network {

/**
 * @brief Estrutura de frame WebSocket
 */
struct WebSocketFrame {
  enum class OpCode {
    CONTINUATION = 0x0,
    TEXT = 0x1,
    BINARY = 0x2,
    CLOSE = 0x8,
    PING = 0x9,
    PONG = 0xA
  };
  
  OpCode opcode;
  bool fin;
  std::vector<uint8_t> payload;
};

/**
 * @brief Servidor WebSocket para integração com UE5
 * 
 * Implementação simplificada de WebSocket RFC 6455 para
 * comunicação em tempo real com clientes Unreal Engine.
 */
class WebSocketServer {
 public:
  using MessageCallback = std::function<void(uint32_t clientId, 
                                             const std::string& message)>;
  using BinaryCallback = std::function<void(uint32_t clientId, 
                                            const std::vector<uint8_t>& data)>;
  using ConnectionCallback = std::function<void(uint32_t clientId, bool connected)>;
  
  explicit WebSocketServer(uint16_t port);
  ~WebSocketServer();
  
  /**
   * @brief Inicia o servidor WebSocket
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
   * @brief Define callback para mensagens de texto
   * @param callback Função a ser chamada
   */
  void setMessageCallback(MessageCallback callback);
  
  /**
   * @brief Define callback para mensagens binárias
   * @param callback Função a ser chamada
   */
  void setBinaryCallback(BinaryCallback callback);
  
  /**
   * @brief Define callback para conexões
   * @param callback Função a ser chamada
   */
  void setConnectionCallback(ConnectionCallback callback);
  
  /**
   * @brief Envia mensagem de texto para cliente
   * @param clientId ID do cliente
   * @param message Mensagem a enviar
   * @return true se enviado com sucesso
   */
  bool sendText(uint32_t clientId, const std::string& message);
  
  /**
   * @brief Envia dados binários para cliente
   * @param clientId ID do cliente
   * @param data Dados a enviar
   * @return true se enviado com sucesso
   */
  bool sendBinary(uint32_t clientId, const std::vector<uint8_t>& data);
  
  /**
   * @brief Broadcast de mensagem para todos os clientes
   * @param message Mensagem a enviar
   */
  void broadcastText(const std::string& message);
  
  /**
   * @brief Desconecta cliente
   * @param clientId ID do cliente
   */
  void disconnect(uint32_t clientId);
  
  /**
   * @brief Obtém número de clientes conectados
   * @return Número de conexões ativas
   */
  size_t getClientCount() const;

 private:
  struct ClientState {
    uint32_t id;
    int socket;
    bool handshakeComplete;
    std::string address;
    uint16_t port;
  };
  
  uint16_t port_;
  int serverSocket_;
  std::atomic<bool> running_;
  std::unique_ptr<std::thread> acceptThread_;
  std::vector<std::unique_ptr<std::thread>> workerThreads_;
  
  std::map<uint32_t, ClientState> clients_;
  mutable std::mutex clientsMutex_;
  std::atomic<uint32_t> nextClientId_;
  
  MessageCallback messageCallback_;
  BinaryCallback binaryCallback_;
  ConnectionCallback connectionCallback_;
  
  bool initializeSocket();
  void acceptLoop();
  void handleClient(int clientSocket, const std::string& address, uint16_t port);
  bool performHandshake(int clientSocket);
  WebSocketFrame receiveFrame(int clientSocket);
  bool sendFrame(int clientSocket, const WebSocketFrame& frame);
  std::string generateAcceptKey(const std::string& clientKey);
  void closeSocket(int socket);
};

}  // namespace Network
}  // namespace Umbra

