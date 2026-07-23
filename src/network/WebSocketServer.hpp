#pragma once

#include <string>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <deque>
#include <condition_variable>
#include <cstdint>

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
   * @brief Enfileira várias mensagens binárias para um cliente numa única
   * aquisição de lock (menos contenção que N chamadas a sendBinary). Cada
   * mensagem vira um frame WS BINARY separado; a writer thread as coalesce
   * em um único send().
   */
  bool sendBinaryBatch(uint32_t clientId,
                       const std::vector<const std::vector<uint8_t>*>& messages);
  
  /**
   * @brief Broadcast de mensagem para todos os clientes
   * @param message Mensagem a enviar
   */
  void broadcastText(const std::string& message);
  /**
   * @brief Broadcast de dados binários para todos os clientes
   * @param data Bytes a enviar
   */
  void broadcastBinary(const std::vector<uint8_t>& data);

  /**
   * @brief Envia frame PING (RFC 6455) a todos os clientes autenticados no handshake.
   * Mantém o receive timeout do WinHTTP/UE (~30s) vivo quando não há AOI/movimento.
   */
  void broadcastPing();
  
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
  /**
   * @brief Fila de saída assíncrona por-cliente.
   *
   * Todo envio de rede vira enqueue nesta fila (não bloqueante). Uma writer
   * thread dedicada drena as filas, coalesce todos os frames pendentes de um
   * cliente em um único send() não-bloqueante e trata partial send/backpressure.
   * Assim, um cliente lento/saturado nunca congela as threads de combate/tick
   * nem afeta o envio para os demais clientes.
   *
   * shared_ptr (como sendMu): a writer thread mantém referência viva mesmo se o
   * cliente for removido do mapa; `closed` sincroniza o fechamento do socket.
   */
  struct OutQueue {
    std::mutex mu;
    std::deque<std::vector<uint8_t>> frames;  // frames WS já codificados
    size_t headOffset = 0;                    // bytes já enviados do frame da frente
    size_t bytes = 0;                         // total de bytes enfileirados
    int64_t stalledSinceMs = 0;               // 0 = sem stall; senão, quando começou
    bool closed = false;                      // set no disconnect (sob mu)
    int socket = -1;
    uint32_t clientId = 0;
  };

  struct ClientState {
    uint32_t id = 0;
    int socket = -1;
    bool handshakeComplete = false;
    std::string address;
    uint16_t port = 0;
    /** Serializa todos os sends neste socket (evita frames intercalados entre threads). */
    std::shared_ptr<std::mutex> sendMu = std::make_shared<std::mutex>();
    /** Fila de saída assíncrona; a writer thread drena e envia. */
    std::shared_ptr<OutQueue> outQ;
    /** steady_clock ms do último frame recebido (PING/PONG/data). Detecta TCP zumbi. */
    int64_t lastRecvMs = 0;
    /** Falhas duras consecutivas (EPIPE/ECONNRESET). Após o limiar, força disconnect. */
    int consecutiveHardSendFails = 0;
  };

  enum class SendStatus : uint8_t { Ok = 0, WouldBlock = 1, HardFail = 2, Deadline = 3 };

  struct ClientSendTarget {
    uint32_t id = 0;
    int socket = -1;
    std::shared_ptr<std::mutex> sendMu;
  };
  
  uint16_t port_;
  int serverSocket_;
  std::atomic<bool> running_;
  std::unique_ptr<std::thread> acceptThread_;
  std::vector<std::unique_ptr<std::thread>> workerThreads_;
  
  std::map<uint32_t, ClientState> clients_;
  mutable std::mutex clientsMutex_;
  std::atomic<uint32_t> nextClientId_;

  // ---- Writer thread (envio assíncrono coalescido) ----
  std::unique_ptr<std::thread> writerThread_;
  std::mutex writerMu_;
  std::condition_variable writerCv_;
  bool writerWake_ = false;
  /** Limite de bytes enfileirados por cliente; acima disso = cliente lento => disconnect. */
  static constexpr size_t kMaxOutboundBytes = 1u * 1024u * 1024u;  // 1 MB
  /** Se o socket não aceitar NENHUM byte por mais que isso (buffer cheio) => disconnect. */
  static constexpr int64_t kSendStallDisconnectMs = 5000;
  /** Máx. de bytes coalescidos por send() (evita buffers gigantes). */
  static constexpr size_t kWriterCoalesceCap = 256u * 1024u;
  /** Período de varredura da writer thread (também janela p/ detectar stall). */
  static constexpr int64_t kWriterTickMs = 2;
  
  MessageCallback messageCallback_;
  BinaryCallback binaryCallback_;
  ConnectionCallback connectionCallback_;
  
  bool initializeSocket();
  void acceptLoop();
  void configureClientSocket(int clientSocket);
  void touchClientRecv(uint32_t clientId);
  void handleClient(int clientSocket, const std::string& address, uint16_t port);
  bool performHandshake(int clientSocket);
  WebSocketFrame receiveFrame(int clientSocket);
  /** Lê exatamente `len` bytes (loop). Linux pode devolver partial mesmo em socket blocking. */
  bool recvExact(int clientSocket, uint8_t* buffer, size_t len, int* outRounds = nullptr);
  /** Monta bytes do frame e envia por completo (caller deve segurar sendMu do cliente). */
  SendStatus sendFrame(int clientSocket, const WebSocketFrame& frame);
  /** Resolve socket+mutex e envia com lock por cliente. */
  bool sendFrameToClient(uint32_t clientId, const WebSocketFrame& frame);
  /** Envia buffer completo (loop em partial send). */
  SendStatus sendAll(int clientSocket, const uint8_t* data, size_t len);
  /** Após falhas duras repetidas, fecha o socket para desbloquear o recv do handleClient. */
  void noteHardSendFail(uint32_t clientId, int err);
  void noteSendOk(uint32_t clientId);
  static constexpr int kHardSendFailDisconnectThreshold = 3;
  std::vector<ClientSendTarget> snapshotSendTargets();
  std::string generateAcceptKey(const std::string& clientKey);
  void closeSocket(int socket);

  // ---- Envio assíncrono ----
  /** Codifica um frame WS (header + payload) em bytes prontos para o socket. */
  static void encodeFrame(const WebSocketFrame& frame, std::vector<uint8_t>& out);
  /** Enfileira frame já codificado na fila do cliente. Retorna false se overflow/inexistente. */
  bool enqueueEncodedFrame(uint32_t clientId, std::vector<uint8_t>&& bytes);
  /** Snapshot das filas ativas (sob clientsMutex_) para a writer drenar fora do lock. */
  std::vector<std::shared_ptr<OutQueue>> snapshotOutQueues();
  /** Drena/coalesce uma fila e envia 1 send() não-bloqueante. disconnectOut=true se morto. */
  void flushOutQueue(OutQueue& q, bool& disconnectOut);
  /** Loop da writer thread. */
  void writerLoop();
  /** Acorda a writer thread. */
  void signalWriter();
};

}  // namespace Network
}  // namespace Umbra
