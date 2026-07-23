#include "WebSocketServer.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
  #include <chrono>
#include <cstring>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #define CLOSE_SOCKET closesocket
  // Non-blocking via FIONBIO em configureClientSocket.
  #define WS_SEND_FLAGS 0
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <errno.h>
  #define SOCKET int
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
  #define CLOSE_SOCKET close
  #ifndef MSG_NOSIGNAL
    #define MSG_NOSIGNAL 0
  #endif
  // MSG_DONTWAIT só no send: recv do handleClient continua bloqueante.
  #define WS_SEND_FLAGS (MSG_NOSIGNAL | MSG_DONTWAIT)
#endif

namespace Umbra {
namespace Network {

namespace {
// #region agent log
void agentLogSend(const char* message, const std::string& dataJson,
                  const char* hypothesisId = "H-SEND") {
  static std::mutex logMu;
  static std::ofstream f;
  std::lock_guard<std::mutex> lock(logMu);
  if (!f.is_open()) {
    f.open("/root/UmbraServerV2/debug-f24ed2.log", std::ios::app);
    if (!f.is_open()) {
      f.open("debug-f24ed2.log", std::ios::app);
    }
  }
  if (!f.is_open()) return;
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();
  f << "{\"sessionId\":\"f24ed2\",\"runId\":\"post-fix\",\"hypothesisId\":\"" << hypothesisId
    << "\",\"location\":\"WebSocketServer.cpp\",\"message\":\"" << message
    << "\",\"data\":" << dataJson << ",\"timestamp\":" << ms << "}\n";
}

inline int64_t agentSendNowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

// Orçamento para completar frame parcial (nunca bloquear tick por segundos).
constexpr int64_t kWsSendDeadlineMs = 50;
// #endregion
}  // namespace

WebSocketServer::WebSocketServer(uint16_t port)
    : port_(port),
      serverSocket_(INVALID_SOCKET),
      running_(false),
      nextClientId_(1) {
#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

WebSocketServer::~WebSocketServer() {
  stop();
#ifdef _WIN32
  WSACleanup();
#endif
}

bool WebSocketServer::start() {
  if (running_) {
    Core::Logger::getInstance().warn("WebSocketServer already running");
    return false;
  }
  
  if (!initializeSocket()) {
    return false;
  }
  
  running_ = true;
  acceptThread_ = std::make_unique<std::thread>(&WebSocketServer::acceptLoop, this);
  writerThread_ = std::make_unique<std::thread>(&WebSocketServer::writerLoop, this);
  
  Core::Logger::getInstance().info("WebSocketServer started on port {}", port_);
  return true;
}

void WebSocketServer::stop() {
  if (!running_) {
    return;
  }
  
  running_ = false;
  signalWriter();
  
  if (acceptThread_ && acceptThread_->joinable()) {
    acceptThread_->join();
  }

  if (writerThread_ && writerThread_->joinable()) {
    writerThread_->join();
  }
  writerThread_.reset();
  
  for (auto& thread : workerThreads_) {
    if (thread && thread->joinable()) {
      thread->join();
    }
  }
  workerThreads_.clear();
  
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    for (auto& [id, client] : clients_) {
      closeSocket(client.socket);
    }
    clients_.clear();
  }
  
  if (serverSocket_ != INVALID_SOCKET) {
    closeSocket(serverSocket_);
    serverSocket_ = INVALID_SOCKET;
  }
  
  Core::Logger::getInstance().info("WebSocketServer stopped");
}

bool WebSocketServer::isRunning() const {
  return running_;
}

void WebSocketServer::setMessageCallback(MessageCallback callback) {
  messageCallback_ = callback;
}

void WebSocketServer::setBinaryCallback(BinaryCallback callback) {
  binaryCallback_ = callback;
}

void WebSocketServer::setConnectionCallback(ConnectionCallback callback) {
  connectionCallback_ = callback;
}

bool WebSocketServer::sendText(uint32_t clientId, const std::string& message) {
  WebSocketFrame frame;
  frame.opcode = WebSocketFrame::OpCode::TEXT;
  frame.fin = true;
  frame.payload.assign(message.begin(), message.end());
  return sendFrameToClient(clientId, frame);
}

bool WebSocketServer::sendBinary(uint32_t clientId, const std::vector<uint8_t>& data) {
  WebSocketFrame frame;
  frame.opcode = WebSocketFrame::OpCode::BINARY;
  frame.fin = true;
  frame.payload = data;
  return sendFrameToClient(clientId, frame);
}

std::vector<WebSocketServer::ClientSendTarget> WebSocketServer::snapshotSendTargets() {
  std::vector<ClientSendTarget> targets;
  std::lock_guard<std::mutex> lock(clientsMutex_);
  targets.reserve(clients_.size());
  for (const auto& [id, client] : clients_) {
    if (client.handshakeComplete && client.sendMu) {
      ClientSendTarget t;
      t.id = id;
      t.socket = client.socket;
      t.sendMu = client.sendMu;
      targets.push_back(std::move(t));
    }
  }
  return targets;
}

bool WebSocketServer::sendFrameToClient(uint32_t clientId, const WebSocketFrame& frame) {
  // Envio assíncrono: codifica o frame e enfileira. A writer thread coalesce e
  // envia via send() não-bloqueante. Nunca bloqueia o chamador (combat/tick/recv).
  std::vector<uint8_t> bytes;
  encodeFrame(frame, bytes);
  return enqueueEncodedFrame(clientId, std::move(bytes));
}

void WebSocketServer::encodeFrame(const WebSocketFrame& frame, std::vector<uint8_t>& out) {
  out.clear();
  uint8_t header = (frame.fin ? 0x80 : 0x00) | static_cast<uint8_t>(frame.opcode);
  out.push_back(header);

  const size_t payloadLen = frame.payload.size();
  if (payloadLen < 126) {
    out.push_back(static_cast<uint8_t>(payloadLen));
  } else if (payloadLen < 65536) {
    out.push_back(126);
    out.push_back(static_cast<uint8_t>((payloadLen >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(payloadLen & 0xFF));
  } else {
    out.push_back(127);
    for (int i = 7; i >= 0; --i) {
      out.push_back(static_cast<uint8_t>((payloadLen >> (i * 8)) & 0xFF));
    }
  }
  out.insert(out.end(), frame.payload.begin(), frame.payload.end());
}

bool WebSocketServer::enqueueEncodedFrame(uint32_t clientId, std::vector<uint8_t>&& bytes) {
  if (bytes.empty()) return true;
  std::shared_ptr<OutQueue> q;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    auto it = clients_.find(clientId);
    if (it == clients_.end() || !it->second.handshakeComplete || !it->second.outQ) {
      return false;
    }
    q = it->second.outQ;
  }

  bool overflow = false;
  {
    std::lock_guard<std::mutex> lk(q->mu);
    if (q->closed) return false;
    if (q->bytes + bytes.size() > kMaxOutboundBytes) {
      overflow = true;  // cliente lento: fila estourou
    } else {
      q->bytes += bytes.size();
      q->frames.push_back(std::move(bytes));
    }
  }

  if (overflow) {
    // #region agent log
    agentLogSend("ws_outq_overflow",
                 std::string("{\"clientId\":") + std::to_string(clientId) +
                     ",\"cap\":" + std::to_string(kMaxOutboundBytes) + "}",
                 "H-ZOMBIE");
    // #endregion
    Core::Logger::getInstance().warn(
        "WebSocket client {} outbound queue overflow (>{}B) — disconnecting slow client",
        clientId, kMaxOutboundBytes);
    disconnect(clientId);
    return false;
  }

  signalWriter();
  return true;
}

bool WebSocketServer::sendBinaryBatch(
    uint32_t clientId, const std::vector<const std::vector<uint8_t>*>& messages) {
  if (messages.empty()) return true;

  // Codifica todos os frames fora dos locks.
  std::vector<std::vector<uint8_t>> encoded;
  encoded.reserve(messages.size());
  size_t addBytes = 0;
  for (const auto* m : messages) {
    if (m == nullptr || m->empty()) continue;
    WebSocketFrame frame;
    frame.opcode = WebSocketFrame::OpCode::BINARY;
    frame.fin = true;
    frame.payload = *m;
    std::vector<uint8_t> b;
    encodeFrame(frame, b);
    addBytes += b.size();
    encoded.push_back(std::move(b));
  }
  if (encoded.empty()) return true;

  std::shared_ptr<OutQueue> q;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    auto it = clients_.find(clientId);
    if (it == clients_.end() || !it->second.handshakeComplete || !it->second.outQ) {
      return false;
    }
    q = it->second.outQ;
  }

  bool overflow = false;
  {
    std::lock_guard<std::mutex> lk(q->mu);
    if (q->closed) return false;
    if (q->bytes + addBytes > kMaxOutboundBytes) {
      overflow = true;
    } else {
      for (auto& b : encoded) {
        q->bytes += b.size();
        q->frames.push_back(std::move(b));
      }
    }
  }

  if (overflow) {
    // #region agent log
    agentLogSend("ws_outq_overflow",
                 std::string("{\"clientId\":") + std::to_string(clientId) +
                     ",\"cap\":" + std::to_string(kMaxOutboundBytes) + ",\"batch\":true}",
                 "H-ZOMBIE");
    // #endregion
    Core::Logger::getInstance().warn(
        "WebSocket client {} outbound queue overflow (batch) — disconnecting slow client",
        clientId);
    disconnect(clientId);
    return false;
  }

  signalWriter();
  return true;
}

std::vector<std::shared_ptr<WebSocketServer::OutQueue>> WebSocketServer::snapshotOutQueues() {
  std::vector<std::shared_ptr<OutQueue>> out;
  std::lock_guard<std::mutex> lock(clientsMutex_);
  out.reserve(clients_.size());
  for (const auto& [id, client] : clients_) {
    if (client.handshakeComplete && client.outQ) {
      out.push_back(client.outQ);
    }
  }
  return out;
}

void WebSocketServer::signalWriter() {
  {
    std::lock_guard<std::mutex> lk(writerMu_);
    writerWake_ = true;
  }
  writerCv_.notify_one();
}

void WebSocketServer::flushOutQueue(OutQueue& q, bool& disconnectOut) {
  std::lock_guard<std::mutex> lk(q.mu);
  if (q.closed || q.socket == INVALID_SOCKET) {
    return;
  }
  if (q.frames.empty()) {
    q.stalledSinceMs = 0;
    return;
  }

  // Coalesce todos os frames pendentes (até o cap) em um único buffer, honrando
  // o offset do frame parcialmente enviado na frente.
  thread_local std::vector<uint8_t> buf;
  buf.clear();
  for (size_t i = 0; i < q.frames.size(); ++i) {
    const auto& f = q.frames[i];
    const size_t start = (i == 0) ? q.headOffset : 0;
    if (start >= f.size()) continue;
    buf.insert(buf.end(), f.begin() + start, f.end());
    if (buf.size() >= kWriterCoalesceCap) break;
  }
  if (buf.empty()) {
    q.stalledSinceMs = 0;
    return;
  }

  const int n = ::send(q.socket, reinterpret_cast<const char*>(buf.data()),
                       static_cast<int>(buf.size()), WS_SEND_FLAGS);
  if (n > 0) {
    // Avança a fila consumindo `n` bytes a partir do headOffset atual.
    size_t adv = static_cast<size_t>(n);
    while (adv > 0 && !q.frames.empty()) {
      auto& front = q.frames.front();
      const size_t rem = front.size() - q.headOffset;
      if (adv >= rem) {
        adv -= rem;
        q.bytes -= rem;
        q.frames.pop_front();
        q.headOffset = 0;
      } else {
        q.headOffset += adv;
        q.bytes -= adv;
        adv = 0;
      }
    }
    q.stalledSinceMs = 0;  // houve progresso
    return;
  }

  // n <= 0: WouldBlock (buffer cheio) ou erro duro.
#ifdef _WIN32
  const int err = WSAGetLastError();
  const bool wouldBlock = (err == WSAEWOULDBLOCK);
  const bool intr = (err == WSAEINTR);
#else
  const int err = errno;
  const bool wouldBlock = (err == EAGAIN || err == EWOULDBLOCK);
  const bool intr = (err == EINTR);
#endif
  if (intr) {
    return;  // tenta de novo no próximo ciclo
  }
  if (wouldBlock) {
    const int64_t now = agentSendNowMs();
    if (q.stalledSinceMs == 0) {
      q.stalledSinceMs = now;
    } else if (now - q.stalledSinceMs > kSendStallDisconnectMs) {
      // #region agent log
      agentLogSend("ws_send_stall",
                   std::string("{\"clientId\":") + std::to_string(q.clientId) +
                       ",\"stalledMs\":" + std::to_string(now - q.stalledSinceMs) +
                       ",\"queued\":" + std::to_string(q.bytes) + "}",
                   "H-ZOMBIE");
      // #endregion
      disconnectOut = true;
    }
    return;
  }
  // Erro duro (EPIPE/ECONNRESET/EBADF...): socket morto.
  // #region agent log
  agentLogSend("ws_send_errno",
               std::string("{\"clientId\":") + std::to_string(q.clientId) +
                   ",\"err\":" + std::to_string(err) + "}",
               "H-SEND");
  // #endregion
  disconnectOut = true;
}

void WebSocketServer::writerLoop() {
  while (running_) {
    {
      std::unique_lock<std::mutex> lk(writerMu_);
      writerCv_.wait_for(lk, std::chrono::milliseconds(kWriterTickMs),
                         [this] { return !running_ || writerWake_; });
      writerWake_ = false;
    }
    if (!running_) break;

    auto queues = snapshotOutQueues();
    std::vector<uint32_t> toDisconnect;
    for (auto& q : queues) {
      bool disc = false;
      flushOutQueue(*q, disc);
      if (disc) toDisconnect.push_back(q->clientId);
    }
    // disconnect() adquire clientsMutex_; chamado fora de qualquer q->mu.
    for (uint32_t id : toDisconnect) {
      disconnect(id);
    }
  }
}

void WebSocketServer::noteSendOk(uint32_t clientId) {
  std::lock_guard<std::mutex> lock(clientsMutex_);
  auto it = clients_.find(clientId);
  if (it != clients_.end()) {
    it->second.consecutiveHardSendFails = 0;
  }
}

void WebSocketServer::noteHardSendFail(uint32_t clientId, int err) {
  bool shouldDisconnect = false;
  int fails = 0;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    auto it = clients_.find(clientId);
    if (it == clients_.end()) return;
    fails = ++it->second.consecutiveHardSendFails;
    if (fails >= kHardSendFailDisconnectThreshold) {
      shouldDisconnect = true;
    }
  }
  // #region agent log
  agentLogSend("ws_hard_send_fail",
               std::string("{\"clientId\":") + std::to_string(clientId) +
                   ",\"fails\":" + std::to_string(fails) +
                   ",\"err\":" + std::to_string(err) +
                   ",\"disconnect\":" + (shouldDisconnect ? "true" : "false") + "}",
               "H-ZOMBIE");
  // #endregion
  if (shouldDisconnect) {
    // Fecha o socket: o handleClient (recv bloqueante) recebe EOF e limpa a sessão.
    Core::Logger::getInstance().warn(
        "WebSocket client {} hard-send fail x{} — forcing disconnect", clientId, fails);
    disconnect(clientId);
  }
}

void WebSocketServer::broadcastText(const std::string& message) {
  WebSocketFrame frame;
  frame.opcode = WebSocketFrame::OpCode::TEXT;
  frame.fin = true;
  frame.payload.assign(message.begin(), message.end());
  std::vector<uint8_t> bytes;
  encodeFrame(frame, bytes);

  std::vector<uint32_t> ids;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    ids.reserve(clients_.size());
    for (const auto& [id, client] : clients_) {
      if (client.handshakeComplete) ids.push_back(id);
    }
  }
  for (uint32_t id : ids) {
    enqueueEncodedFrame(id, std::vector<uint8_t>(bytes));
  }
}

void WebSocketServer::broadcastBinary(const std::vector<uint8_t>& data) {
  WebSocketFrame frame;
  frame.opcode = WebSocketFrame::OpCode::BINARY;
  frame.fin = true;
  frame.payload = data;
  std::vector<uint8_t> bytes;
  encodeFrame(frame, bytes);

  std::vector<uint32_t> ids;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    ids.reserve(clients_.size());
    for (const auto& [id, client] : clients_) {
      if (client.handshakeComplete) ids.push_back(id);
    }
  }
  for (uint32_t id : ids) {
    enqueueEncodedFrame(id, std::vector<uint8_t>(bytes));
  }
}

void WebSocketServer::broadcastPing() {
  // PING RFC + BINARY app-level: mantém o receive do cliente vivo.
  // Agora só enfileira: a writer thread envia e detecta socket morto (stall/erro).
  WebSocketFrame pingFrame;
  pingFrame.opcode = WebSocketFrame::OpCode::PING;
  pingFrame.fin = true;
  pingFrame.payload.clear();

  WebSocketFrame binaryFrame;
  binaryFrame.opcode = WebSocketFrame::OpCode::BINARY;
  binaryFrame.fin = true;
  binaryFrame.payload = {static_cast<uint8_t>(250)};  // MovementMsgType::WsKeepalive

  std::vector<uint8_t> pingBytes, binBytes;
  encodeFrame(pingFrame, pingBytes);
  encodeFrame(binaryFrame, binBytes);

  std::vector<uint32_t> ids;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    ids.reserve(clients_.size());
    for (const auto& [id, client] : clients_) {
      if (client.handshakeComplete) ids.push_back(id);
    }
  }
  for (uint32_t id : ids) {
    enqueueEncodedFrame(id, std::vector<uint8_t>(pingBytes));
    enqueueEncodedFrame(id, std::vector<uint8_t>(binBytes));
  }
}

void WebSocketServer::disconnect(uint32_t clientId) {
  bool shouldNotify = false;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    auto it = clients_.find(clientId);
    if (it != clients_.end()) {
      // Sincroniza com a writer thread: marca closed sob outQ->mu ANTES de fechar
      // o fd. Se a writer estiver no meio de um send(), esperamos ela terminar;
      // depois disso ela verá closed e não tocará mais no socket.
      if (it->second.outQ) {
        std::lock_guard<std::mutex> lk(it->second.outQ->mu);
        it->second.outQ->closed = true;
        it->second.outQ->socket = INVALID_SOCKET;
      }
      closeSocket(it->second.socket);
      clients_.erase(it);
      shouldNotify = (connectionCallback_ != nullptr);
    }
  }
  // IMPORTANTE: Chamar callback FORA do lock - o callback pode chamar broadcastBinary
  // que precisa adquirir clientsMutex_. Manter o lock causaria deadlock.
  if (shouldNotify && connectionCallback_) {
    connectionCallback_(clientId, false);
  }
}

size_t WebSocketServer::getClientCount() const {
  std::lock_guard<std::mutex> lock(clientsMutex_);
  return clients_.size();
}

bool WebSocketServer::initializeSocket() {
  serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
  
  if (serverSocket_ == INVALID_SOCKET) {
    Core::Logger::getInstance().error("Failed to create WebSocket server socket");
    return false;
  }
  
  int opt = 1;
  setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, 
             reinterpret_cast<const char*>(&opt), sizeof(opt));
  
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port_);
  
  if (bind(serverSocket_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
    Core::Logger::getInstance().error("Failed to bind WebSocket server to port {}", port_);
    closeSocket(serverSocket_);
    return false;
  }
  
  if (listen(serverSocket_, SOMAXCONN) == SOCKET_ERROR) {
    Core::Logger::getInstance().error("Failed to listen on WebSocket server");
    closeSocket(serverSocket_);
    return false;
  }
  
  return true;
}

void WebSocketServer::configureClientSocket(int clientSocket) {
  if (clientSocket == INVALID_SOCKET) return;

  // SNDTIMEO curto + keepalive. O_NONBLOCK só APÓS handshake (senão HTTP falha).
#ifdef _WIN32
  DWORD sndTimeoutMs = 50;
  setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO,
             reinterpret_cast<const char*>(&sndTimeoutMs), sizeof(sndTimeoutMs));
  // Buffer de envio maior: absorve rajadas de combate (evita EWOULDBLOCK precoce).
  int sndBuf = 1024 * 1024;
  setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF,
             reinterpret_cast<const char*>(&sndBuf), sizeof(sndBuf));
  BOOL keepAlive = TRUE;
  setsockopt(clientSocket, SOL_SOCKET, SO_KEEPALIVE,
             reinterpret_cast<const char*>(&keepAlive), sizeof(keepAlive));
  // Desliga Nagle: frames de combate são pequenos (26-30B) e latência importa
  // mais que agrupar. Sem isso, casts/vitals em sequência sofrem ~40ms de delay.
  BOOL noDelay = TRUE;
  setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY,
             reinterpret_cast<const char*>(&noDelay), sizeof(noDelay));
#else
  timeval sndTv{};
  sndTv.tv_sec = 0;
  sndTv.tv_usec = 50000;  // 50ms
  setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &sndTv, sizeof(sndTv));

  // Buffer de envio maior: absorve rajadas de combate (evita EWOULDBLOCK precoce).
  int sndBuf = 1024 * 1024;
  setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, &sndBuf, sizeof(sndBuf));

  int yes = 1;
  setsockopt(clientSocket, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));
  int idle = 10;
  int intvl = 3;
  int cnt = 3;
  setsockopt(clientSocket, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
  setsockopt(clientSocket, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));
  setsockopt(clientSocket, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
  // Desliga Nagle: frames de combate são pequenos (26-30B) e latência importa
  // mais que agrupar. Sem isso, casts/vitals em sequência sofrem ~40ms de delay.
  int noDelay = 1;
  setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay));
#endif
}

void WebSocketServer::touchClientRecv(uint32_t clientId) {
  const int64_t now = agentSendNowMs();
  std::lock_guard<std::mutex> lock(clientsMutex_);
  auto it = clients_.find(clientId);
  if (it != clients_.end()) {
    it->second.lastRecvMs = now;
  }
}

void WebSocketServer::acceptLoop() {
  while (running_) {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    
    int clientSocket = accept(serverSocket_, 
                               reinterpret_cast<sockaddr*>(&clientAddr), 
                               &clientLen);
    
    if (clientSocket == INVALID_SOCKET) {
      if (running_) {
        Core::Logger::getInstance().warn("WebSocket accept failed");
      }
      continue;
    }
    
    std::string clientAddress = inet_ntoa(clientAddr.sin_addr);
    uint16_t clientPort = ntohs(clientAddr.sin_port);
    
    Core::Logger::getInstance().info("New WebSocket connection from {}:{}", 
                                     clientAddress, clientPort);
    
    auto thread = std::make_unique<std::thread>(
      &WebSocketServer::handleClient, this, clientSocket, clientAddress, clientPort);
    workerThreads_.push_back(std::move(thread));
  }
}

void WebSocketServer::handleClient(int clientSocket, 
                                   const std::string& address, 
                                   uint16_t port) {
  configureClientSocket(clientSocket);
  if (!performHandshake(clientSocket)) {
    closeSocket(clientSocket);
    return;
  }
  // Socket permanece blocking no recv; sends usam MSG_DONTWAIT (Linux) + try_lock.
  
  uint32_t clientId = nextClientId_++;
  
  size_t clientsAfter = 0;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    ClientState state;
    state.id = clientId;
    state.socket = clientSocket;
    state.handshakeComplete = true;
    state.address = address;
    state.port = port;
    state.lastRecvMs = agentSendNowMs();
    state.outQ = std::make_shared<OutQueue>();
    state.outQ->socket = clientSocket;
    state.outQ->clientId = clientId;
    clients_[clientId] = std::move(state);
    clientsAfter = clients_.size();
  }

  // #region agent log
  {
    static std::mutex logMu;
    std::lock_guard<std::mutex> lock(logMu);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();
    std::ofstream f("/root/UmbraServerV2/debug-f24ed2.log", std::ios::app);
    if (!f.is_open()) {
      f.open("debug-f24ed2.log", std::ios::app);
    }
    if (f.is_open()) {
      f << "{\"sessionId\":\"f24ed2\",\"runId\":\"dual-login\",\"hypothesisId\":\"H-OV\","
        << "\"location\":\"WebSocketServer.cpp:handleClient\",\"message\":\"ws_client_added\","
        << "\"data\":{\"cid\":" << clientId << ",\"clients\":" << clientsAfter
        << "},\"timestamp\":" << ms << "}\n";
    }
  }
  // #endregion
  
  if (connectionCallback_) {
    connectionCallback_(clientId, true);
  }
  
  const char* exitReason = "running_false";
  while (running_) {
    try {
      WebSocketFrame frame = receiveFrame(clientSocket);
      
      if (frame.opcode == WebSocketFrame::OpCode::CLOSE) {
        exitReason = frame.payload.empty() ? "recv_eof_or_error" : "peer_close_frame";
        break;
      }

      // Qualquer frame inbound (data/PING/PONG) prova que o peer está vivo.
      touchClientRecv(clientId);
      
      if (frame.opcode == WebSocketFrame::OpCode::TEXT && messageCallback_) {
        std::string message(frame.payload.begin(), frame.payload.end());
        messageCallback_(clientId, message);
      } else if (frame.opcode == WebSocketFrame::OpCode::BINARY && binaryCallback_) {
        binaryCallback_(clientId, frame.payload);
      } else if (frame.opcode == WebSocketFrame::OpCode::PING) {
        WebSocketFrame pong;
        pong.opcode = WebSocketFrame::OpCode::PONG;
        pong.fin = true;
        pong.payload = frame.payload;
        sendFrameToClient(clientId, pong);
      }
      // PONG do cliente (resposta ao nosso keepalive): já contou em touchClientRecv
    } catch (const std::exception& e) {
      Core::Logger::getInstance().error("Exception in WebSocket client handler for client {}: {}", clientId, e.what());
      exitReason = "exception";
      break;
    } catch (...) {
      Core::Logger::getInstance().error("Unknown exception in WebSocket client handler for client {}", clientId);
      exitReason = "exception_unknown";
      break;
    }
  }
  
  // #region agent log
  {
    static std::mutex logMu;
    std::lock_guard<std::mutex> lock(logMu);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();
    const size_t clientsBefore = getClientCount();
    std::ofstream f("/root/UmbraServerV2/debug-f24ed2.log", std::ios::app);
    if (!f.is_open()) {
      f.open("debug-f24ed2.log", std::ios::app);
    }
    if (f.is_open()) {
      f << "{\"sessionId\":\"f24ed2\",\"runId\":\"dual-login\",\"hypothesisId\":\"H-J\","
        << "\"location\":\"WebSocketServer.cpp:handleClient\",\"message\":\"ws_handler_exit\","
        << "\"data\":{\"cid\":" << clientId << ",\"reason\":\"" << exitReason
        << "\",\"clientsBefore\":" << clientsBefore
        << "},\"timestamp\":" << ms << "}\n";
    }
  }
  // #endregion

  disconnect(clientId);
  Core::Logger::getInstance().info("WebSocket client {} disconnected ({})", clientId, exitReason);
}

bool WebSocketServer::performHandshake(int clientSocket) {
  char buffer[4096];
  int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
  
  if (bytesReceived <= 0) {
    return false;
  }
  
  buffer[bytesReceived] = '\0';
  std::string request(buffer);
  
  // Extract Sec-WebSocket-Key
  size_t keyPos = request.find("Sec-WebSocket-Key: ");
  if (keyPos == std::string::npos) {
    return false;
  }
  
  keyPos += 19;
  size_t keyEnd = request.find("\r\n", keyPos);
  std::string clientKey = request.substr(keyPos, keyEnd - keyPos);
  
  std::string acceptKey = generateAcceptKey(clientKey);
  
  std::ostringstream response;
  response << "HTTP/1.1 101 Switching Protocols\r\n"
           << "Upgrade: websocket\r\n"
           << "Connection: Upgrade\r\n"
           << "Sec-WebSocket-Accept: " << acceptKey << "\r\n"
           << "\r\n";
  
  std::string responseStr = response.str();
  send(clientSocket, responseStr.c_str(), static_cast<int>(responseStr.size()), 0);
  
  return true;
}

bool WebSocketServer::recvExact(int clientSocket, uint8_t* buffer, size_t len, int* outRounds) {
  if (clientSocket == INVALID_SOCKET || buffer == nullptr) {
    return len == 0;
  }
  size_t got = 0;
  int rounds = 0;
  while (got < len) {
    const int n = ::recv(clientSocket,
                         reinterpret_cast<char*>(buffer + got),
                         static_cast<int>(len - got),
                         0);
    ++rounds;
    if (n <= 0) {
      if (outRounds) {
        *outRounds = rounds;
      }
      return false;  // EOF / erro real
    }
    got += static_cast<size_t>(n);
  }
  if (outRounds) {
    *outRounds = rounds;
  }
  // #region agent log
  // Prova: Linux devolveu partial e o loop juntou o frame (antes isso virava "disconnect").
  if (rounds > 1) {
    agentLogSend("ws_recv_partial_recovered",
                 std::string("{\"socket\":") + std::to_string(clientSocket) +
                     ",\"len\":" + std::to_string(len) +
                     ",\"rounds\":" + std::to_string(rounds) + "}");
  }
  // #endregion
  return true;
}

WebSocketFrame WebSocketServer::receiveFrame(int clientSocket) {
  WebSocketFrame frame;
  frame.opcode = WebSocketFrame::OpCode::CLOSE;

  // CRÍTICO: em Linux, recv() em socket blocking PODE devolver menos que o pedido.
  // Tratar partial como CLOSE derrubava o cliente UE (MoveUpdate contínuo) no Proxmox.
  uint8_t header[2];
  if (!recvExact(clientSocket, header, 2)) {
    return frame;
  }

  frame.fin = (header[0] & 0x80) != 0;
  frame.opcode = static_cast<WebSocketFrame::OpCode>(header[0] & 0x0F);

  bool masked = (header[1] & 0x80) != 0;
  uint64_t payloadLen = header[1] & 0x7F;

  if (payloadLen == 126) {
    uint8_t len[2];
    if (!recvExact(clientSocket, len, 2)) {
      frame.opcode = WebSocketFrame::OpCode::CLOSE;
      return frame;
    }
    payloadLen = (static_cast<uint64_t>(len[0]) << 8) | len[1];
  } else if (payloadLen == 127) {
    uint8_t len[8];
    if (!recvExact(clientSocket, len, 8)) {
      frame.opcode = WebSocketFrame::OpCode::CLOSE;
      return frame;
    }
    payloadLen = 0;
    for (int i = 0; i < 8; ++i) {
      payloadLen = (payloadLen << 8) | len[i];
    }
  }

  uint8_t maskKey[4] = {0};
  if (masked) {
    if (!recvExact(clientSocket, maskKey, 4)) {
      frame.opcode = WebSocketFrame::OpCode::CLOSE;
      return frame;
    }
  }

  if (payloadLen > 1024 * 1024) {
    Core::Logger::getInstance().warn("WebSocket frame payload too large: {} bytes", payloadLen);
    frame.opcode = WebSocketFrame::OpCode::CLOSE;
    return frame;
  }

  frame.payload.resize(static_cast<size_t>(payloadLen));
  if (payloadLen > 0) {
    if (!recvExact(clientSocket, frame.payload.data(), static_cast<size_t>(payloadLen))) {
      frame.opcode = WebSocketFrame::OpCode::CLOSE;
      frame.payload.clear();
      return frame;
    }
    if (masked) {
      for (size_t i = 0; i < static_cast<size_t>(payloadLen); ++i) {
        frame.payload[i] ^= maskKey[i % 4];
      }
    }
  }

  return frame;
}

WebSocketServer::SendStatus WebSocketServer::sendAll(int clientSocket, const uint8_t* data,
                                                     size_t len) {
  if (clientSocket == INVALID_SOCKET || data == nullptr || len == 0) {
    return (len == 0) ? SendStatus::Ok : SendStatus::HardFail;
  }

  size_t sentTotal = 0;
  int partialRounds = 0;
  const int64_t t0 = agentSendNowMs();
  const int64_t deadline = t0 + kWsSendDeadlineMs;
  while (sentTotal < len) {
    if (agentSendNowMs() > deadline) {
      // #region agent log
      agentLogSend("ws_send_deadline",
                   std::string("{\"socket\":") + std::to_string(clientSocket) +
                       ",\"len\":" + std::to_string(len) +
                       ",\"sent\":" + std::to_string(sentTotal) +
                       ",\"ms\":" + std::to_string(agentSendNowMs() - t0) + "}");
      // #endregion
      return SendStatus::Deadline;
    }
    const int toSend = static_cast<int>(len - sentTotal);
    const int n = ::send(clientSocket,
                         reinterpret_cast<const char*>(data + sentTotal),
                         toSend,
                         WS_SEND_FLAGS);
    if (n > 0) {
      if (static_cast<size_t>(n) < static_cast<size_t>(toSend)) {
        ++partialRounds;
      }
      sentTotal += static_cast<size_t>(n);
      continue;
    }
#ifdef _WIN32
    const int err = WSAGetLastError();
    if (err == WSAEINTR) {
      continue;
    }
    if (err == WSAEWOULDBLOCK) {
      if (sentTotal == 0) {
        return SendStatus::WouldBlock;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    // #region agent log
    agentLogSend("ws_send_errno",
                 std::string("{\"socket\":") + std::to_string(clientSocket) +
                     ",\"err\":" + std::to_string(err) +
                     ",\"sent\":" + std::to_string(sentTotal) + "}",
                 "H-SEND");
    // #endregion
    Core::Logger::getInstance().debug("sendAll failed socket {} err={}", clientSocket, err);
    return SendStatus::HardFail;
#else
    if (n < 0 && errno == EINTR) {
      continue;
    }
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      if (sentTotal == 0) {
        return SendStatus::WouldBlock;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    const int err = (n < 0) ? errno : 0;
    // #region agent log
    agentLogSend("ws_send_errno",
                 std::string("{\"socket\":") + std::to_string(clientSocket) +
                     ",\"err\":" + std::to_string(err) +
                     ",\"sent\":" + std::to_string(sentTotal) + "}",
                 "H-SEND");
    // #endregion
    Core::Logger::getInstance().debug("sendAll failed socket {} errno={}", clientSocket, err);
    return SendStatus::HardFail;
#endif
  }

  // #region agent log
  if (partialRounds > 0) {
    agentLogSend("ws_send_partial",
                 std::string("{\"socket\":") + std::to_string(clientSocket) +
                     ",\"len\":" + std::to_string(len) +
                     ",\"partialRounds\":" + std::to_string(partialRounds) + "}");
  }
  // #endregion
  return SendStatus::Ok;
}

WebSocketServer::SendStatus WebSocketServer::sendFrame(int clientSocket,
                                                       const WebSocketFrame& frame) {
  if (clientSocket == INVALID_SOCKET) {
    return SendStatus::HardFail;
  }

  try {
    thread_local std::vector<uint8_t> data;
    data.clear();

    uint8_t header = (frame.fin ? 0x80 : 0x00) | static_cast<uint8_t>(frame.opcode);
    data.push_back(header);

    size_t payloadLen = frame.payload.size();
    // #region agent log
    if (payloadLen >= 8192) {
      agentLogSend("ws_send_large",
                   std::string("{\"socket\":") + std::to_string(clientSocket) +
                       ",\"payload\":" + std::to_string(payloadLen) +
                       ",\"opcode\":" + std::to_string(static_cast<int>(frame.opcode)) + "}");
    }
    // #endregion
    if (payloadLen < 126) {
      data.push_back(static_cast<uint8_t>(payloadLen));
    } else if (payloadLen < 65536) {
      data.push_back(126);
      data.push_back(static_cast<uint8_t>((payloadLen >> 8) & 0xFF));
      data.push_back(static_cast<uint8_t>(payloadLen & 0xFF));
    } else {
      data.push_back(127);
      for (int i = 7; i >= 0; --i) {
        data.push_back(static_cast<uint8_t>((payloadLen >> (i * 8)) & 0xFF));
      }
    }

    data.insert(data.end(), frame.payload.begin(), frame.payload.end());
    return sendAll(clientSocket, data.data(), data.size());
  } catch (const std::exception& e) {
    Core::Logger::getInstance().warn("Exception in sendFrame for socket {}: {}", clientSocket, e.what());
    return SendStatus::HardFail;
  } catch (...) {
    Core::Logger::getInstance().warn("Unknown exception in sendFrame for socket {}", clientSocket);
    return SendStatus::HardFail;
  }
}

std::string WebSocketServer::generateAcceptKey(const std::string& clientKey) {
  static const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  std::string combined = clientKey + magic;
  
  // WebSocket RFC 6455: SHA-1 hash followed by Base64 encoding
  std::string sha1Hash = Core::Utils::sha1(combined);
  if (sha1Hash.empty()) {
    Core::Logger::getInstance().error("Failed to generate SHA-1 hash for WebSocket handshake");
    return "";
  }
  
  return Core::Utils::base64Encode(sha1Hash);
}

void WebSocketServer::closeSocket(int socket) {
  if (socket != INVALID_SOCKET) {
    CLOSE_SOCKET(socket);
  }
}

}  // namespace Network
}  // namespace Umbra

