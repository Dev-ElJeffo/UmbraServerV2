#include <algorithm>
#include "SocketServer.hpp"
#include "core/Logger.hpp"
#include <cerrno>
#include <cstring>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  #define CLOSE_SOCKET closesocket
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #define SOCKET int
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
  #define CLOSE_SOCKET close
#endif

namespace Umbra {
namespace Network {

SocketServer::SocketServer(ProtocolType type, uint16_t port, std::string bindHost)
    : type_(type),
      port_(port),
      bindHost_(std::move(bindHost)),
      serverSocket_(INVALID_SOCKET),
      running_(false),
      nextClientId_(1),
      rateLimitPerSecond_(1000) {
#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

SocketServer::~SocketServer() {
  stop();
#ifdef _WIN32
  WSACleanup();
#endif
}

bool SocketServer::start() {
  if (running_) {
    Core::Logger::getInstance().warn("SocketServer already running");
    return false;
  }
  
  if (!initializeSocket()) {
    return false;
  }
  
  running_ = true;
  
  if (type_ == ProtocolType::TCP) {
    acceptThread_ = std::make_unique<std::thread>(&SocketServer::acceptLoop, this);
  } else {
    acceptThread_ = std::make_unique<std::thread>(&SocketServer::handleUdpMessages, this);
  }
  
  Core::Logger::getInstance().info("SocketServer started on port {} ({})", 
                                   port_, 
                                   type_ == ProtocolType::TCP ? "TCP" : "UDP");
  return true;
}

void SocketServer::stop() {
  if (!running_) {
    return;
  }
  
  running_ = false;

  // 1) Fechar o socket de escuta antes do join: accept() bloqueado só retorna assim.
  if (serverSocket_ != INVALID_SOCKET) {
    shutdownAndClose(serverSocket_);
    serverSocket_ = INVALID_SOCKET;
  }
  if (acceptThread_ && acceptThread_->joinable()) {
    acceptThread_->join();
  }

  // 2) Acordar os recv() dos workers; cada worker fecha o próprio socket em disconnectClient.
  std::vector<std::shared_ptr<ClientIo>> ios;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    for (auto& client : clients_) {
      if (client.io) ios.push_back(client.io);
    }
  }
  // Windows só acorda recv() bloqueado com closesocket(); Linux só com shutdown().
  // shutdownAndClose faz os dois; closed=true evita close duplo em disconnectClient.
  for (auto& io : ios) {
    std::lock_guard<std::mutex> ioLock(io->sendMu);
    if (!io->closed) {
      io->closed = true;
      shutdownAndClose(io->socket);
    }
  }

  // 3) Join dos workers. Se stop() vier de dentro de um callback (ex.: comando admin
  //    "shutdown"), a própria thread não pode se dar join.
  const auto selfId = std::this_thread::get_id();
  for (auto& worker : workerThreads_) {
    if (!worker.thread || !worker.thread->joinable()) continue;
    if (worker.thread->get_id() == selfId) {
      worker.thread->detach();
    } else {
      worker.thread->join();
    }
  }
  workerThreads_.clear();

  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    for (auto& client : clients_) {
      if (client.io) {
        std::lock_guard<std::mutex> ioLock(client.io->sendMu);
        if (!client.io->closed) {
          client.io->closed = true;
          shutdownAndClose(client.io->socket);
        }
      }
    }
    clients_.clear();
  }

  Core::Logger::getInstance().info("SocketServer stopped");
}

bool SocketServer::isRunning() const {
  return running_;
}

void SocketServer::setMessageCallback(MessageCallback callback) {
  messageCallback_ = callback;
}

void SocketServer::setConnectionCallback(ConnectionCallback callback) {
  connectionCallback_ = callback;
}

bool SocketServer::sendToClient(uint32_t clientId, const std::vector<uint8_t>& data) {
  std::shared_ptr<ClientIo> io;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    auto* client = findClient(clientId);
    if (!client || !client->connected || !client->io) {
      return false;
    }
    io = client->io;
  }
  // Envio fora do clientsMutex_: cliente lento não trava os demais.
  return sendToIo(io, data);
}

void SocketServer::broadcast(const std::vector<uint8_t>& data) {
  std::vector<std::shared_ptr<ClientIo>> ios;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    ios.reserve(clients_.size());
    for (auto& client : clients_) {
      if (client.connected && client.io) ios.push_back(client.io);
    }
  }
  for (auto& io : ios) {
    sendToIo(io, data);
  }
}

bool SocketServer::sendToIo(const std::shared_ptr<ClientIo>& io,
                            const std::vector<uint8_t>& data) {
  std::lock_guard<std::mutex> ioLock(io->sendMu);
  if (io->closed) {
    return false;
  }
  if (data.empty()) {
    return true;
  }
  if (!sendAll(*io, data.data(), data.size())) {
    // Frame parcial corrompe o stream: derruba a conexão; o worker acorda do recv()
    // e faz a limpeza (disconnectClient + callback).
    io->closed = true;
    shutdownAndClose(io->socket);
    Core::Logger::getInstance().warn("SocketServer: send failed on port {}, closing client", port_);
    return false;
  }
  return true;
}

bool SocketServer::sendAll(ClientIo& io, const uint8_t* data, size_t size) {
  size_t sent = 0;
  while (sent < size) {
    const size_t remaining = size - sent;
    const int chunk = static_cast<int>(std::min<size_t>(remaining, 1 << 20));
#ifdef _WIN32
    const int result = send(static_cast<SOCKET>(io.socket),
                            reinterpret_cast<const char*>(data + sent), chunk, 0);
#else
    const int result = static_cast<int>(
        send(io.socket, reinterpret_cast<const char*>(data + sent), chunk, MSG_NOSIGNAL));
    if (result < 0 && errno == EINTR) {
      continue;
    }
#endif
    if (result == SOCKET_ERROR || result <= 0) {
      return false;  // erro ou SO_SNDTIMEO estourado
    }
    sent += static_cast<size_t>(result);
  }
  return true;
}

bool SocketServer::disconnectClient(uint32_t clientId) {
  bool found = false;
  std::shared_ptr<ClientIo> io;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);

    auto it = std::find_if(clients_.begin(), clients_.end(),
      [clientId](const ClientConnection& c) { return c.id == clientId; });

    if (it != clients_.end()) {
      io = it->io;
      if (!io) {
        closeSocket(it->socket);
      }
      clients_.erase(it);
      found = true;
    }
  }

  if (io) {
    // Espera um send em andamento terminar (limitado por SO_SNDTIMEO) antes de fechar.
    std::lock_guard<std::mutex> ioLock(io->sendMu);
    if (!io->closed) {
      io->closed = true;
      shutdownAndClose(io->socket);
    }
  }

  if (found && connectionCallback_) {
    connectionCallback_(clientId, false);
  }
  return found;
}

size_t SocketServer::getClientCount() const {
  std::lock_guard<std::mutex> lock(clientsMutex_);
  return clients_.size();
}

void SocketServer::setRateLimit(uint32_t maxMessagesPerSecond) {
  rateLimitPerSecond_ = maxMessagesPerSecond;
}

void SocketServer::setMaxConnections(uint32_t maxConnections) {
  maxConnections_ = maxConnections;
}

bool SocketServer::initializeSocket() {
  int socketType = (type_ == ProtocolType::TCP) ? SOCK_STREAM : SOCK_DGRAM;
  serverSocket_ = socket(AF_INET, socketType, 0);
  
  if (serverSocket_ == INVALID_SOCKET) {
    Core::Logger::getInstance().error("Failed to create socket");
    return false;
  }
  
  // Set socket options
  int opt = 1;
  setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, 
             reinterpret_cast<const char*>(&opt), sizeof(opt));
  
  // Bind
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(port_);
  if (bindHost_.empty() || bindHost_ == "0.0.0.0") {
    address.sin_addr.s_addr = INADDR_ANY;
  } else if (inet_pton(AF_INET, bindHost_.c_str(), &address.sin_addr) != 1) {
    Core::Logger::getInstance().error("Failed to parse bind host {}", bindHost_);
    closeSocket(serverSocket_);
    return false;
  }
  
  if (bind(serverSocket_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
    Core::Logger::getInstance().error("Failed to bind socket to port {}", port_);
    closeSocket(serverSocket_);
    return false;
  }
  
  // Listen (TCP only)
  if (type_ == ProtocolType::TCP) {
    if (listen(serverSocket_, SOMAXCONN) == SOCKET_ERROR) {
      Core::Logger::getInstance().error("Failed to listen on socket");
      closeSocket(serverSocket_);
      return false;
    }
  }
  
  return true;
}

void SocketServer::acceptLoop() {
  while (running_) {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    
    int clientSocket = accept(serverSocket_, 
                               reinterpret_cast<sockaddr*>(&clientAddr), 
                               &clientLen);
    
    if (clientSocket == INVALID_SOCKET) {
      if (running_) {
        Core::Logger::getInstance().warn("Accept failed");
      }
      continue;
    }
    
    std::string clientAddress = inet_ntoa(clientAddr.sin_addr);
    uint16_t clientPort = ntohs(clientAddr.sin_port);

    {
      std::lock_guard<std::mutex> lock(clientsMutex_);
      if (clients_.size() >= maxConnections_) {
        Core::Logger::getInstance().warn("Max connections ({}) reached, rejecting {}:{}",
                                         maxConnections_, clientAddress, clientPort);
        CLOSE_SOCKET(clientSocket);
        continue;
      }
    }

    Core::Logger::getInstance().info("New connection from {}:{}",
                                     clientAddress, clientPort);

    // Libera threads de conexões já encerradas (antes ficavam até o stop()).
    reapFinishedWorkers();

    Worker worker;
    worker.done = std::make_shared<std::atomic<bool>>(false);
    worker.thread = std::make_unique<std::thread>(
      &SocketServer::handleTcpClient, this, clientSocket, clientAddress, clientPort,
      worker.done);
    workerThreads_.push_back(std::move(worker));
  }
}

void SocketServer::reapFinishedWorkers() {
  auto it = workerThreads_.begin();
  while (it != workerThreads_.end()) {
    if (it->done && it->done->load(std::memory_order_acquire)) {
      if (it->thread && it->thread->joinable()) {
        it->thread->join();  // já saiu de handleTcpClient: retorno imediato
      }
      it = workerThreads_.erase(it);
    } else {
      ++it;
    }
  }
}

void SocketServer::handleTcpClient(int clientSocket,
                                   const std::string& address,
                                   uint16_t port,
                                   std::shared_ptr<std::atomic<bool>> done) {
  uint32_t clientId = nextClientId_++;

  // Timeout de envio: um cliente que não lê não segura o sendMu para sempre.
#ifdef _WIN32
  const DWORD sendTimeoutMs = 5000;
  setsockopt(static_cast<SOCKET>(clientSocket), SOL_SOCKET, SO_SNDTIMEO,
             reinterpret_cast<const char*>(&sendTimeoutMs), sizeof(sendTimeoutMs));
#else
  timeval sendTimeout{};
  sendTimeout.tv_sec = 5;
  setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &sendTimeout, sizeof(sendTimeout));
#endif

  auto io = std::make_shared<ClientIo>();
  io->socket = clientSocket;

  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    ClientConnection conn;
    conn.id = clientId;
    conn.address = address;
    conn.port = port;
    conn.connected = true;
    conn.socket = clientSocket;
    conn.io = io;
    clients_.push_back(conn);
  }
  
  if (connectionCallback_) {
    connectionCallback_(clientId, true);
  }
  
  const size_t bufferSize = 4096;
  std::vector<uint8_t> buffer(bufferSize);
  
  while (running_) {
    int bytesReceived = recv(clientSocket, 
                             reinterpret_cast<char*>(buffer.data()), 
                             bufferSize, 
                             0);
    
    if (bytesReceived <= 0) {
      break;
    }
    
    if (!checkRateLimit(clientId)) {
      Core::Logger::getInstance().warn("Rate limit exceeded for client {}, disconnecting", clientId);
      break;
    }
    
    if (messageCallback_) {
      std::vector<uint8_t> data(buffer.begin(), buffer.begin() + bytesReceived);
      messageCallback_(clientId, data);
    }
  }
  
  {
    std::lock_guard<std::mutex> rateLock(rateMutex_);
    clientRates_.erase(clientId);
  }
  
  disconnectClient(clientId);
  Core::Logger::getInstance().info("Client {} disconnected", clientId);
  done->store(true, std::memory_order_release);
}

void SocketServer::handleUdpMessages() {
  const size_t bufferSize = 4096;
  std::vector<uint8_t> buffer(bufferSize);
  
  while (running_) {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    
    int bytesReceived = recvfrom(serverSocket_, 
                                  reinterpret_cast<char*>(buffer.data()), 
                                  bufferSize, 
                                  0,
                                  reinterpret_cast<sockaddr*>(&clientAddr), 
                                  &clientLen);
    
    if (bytesReceived <= 0) {
      continue;
    }
    
    // TODO: Track UDP "connections" and assign client IDs
    if (messageCallback_) {
      std::vector<uint8_t> data(buffer.begin(), buffer.begin() + bytesReceived);
      messageCallback_(0, data);
    }
  }
}

void SocketServer::closeSocket(int socket) {
  if (socket != INVALID_SOCKET) {
    CLOSE_SOCKET(socket);
  }
}

void SocketServer::shutdownAndClose(int socket) {
  if (socket == INVALID_SOCKET) {
    return;
  }
#ifdef _WIN32
  shutdown(static_cast<SOCKET>(socket), SD_BOTH);
#else
  shutdown(socket, SHUT_RDWR);
#endif
  CLOSE_SOCKET(socket);
}

bool SocketServer::checkRateLimit(uint32_t clientId) {
  if (rateLimitPerSecond_ == 0) return true;
  
  std::lock_guard<std::mutex> lock(rateMutex_);
  auto now = std::chrono::steady_clock::now();
  auto& info = clientRates_[clientId];
  
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - info.windowStart);
  if (elapsed.count() >= 1) {
    info.messageCount = 1;
    info.windowStart = now;
    return true;
  }
  
  ++info.messageCount;
  return info.messageCount <= rateLimitPerSecond_;
}

ClientConnection* SocketServer::findClient(uint32_t clientId) {
  auto it = std::find_if(clients_.begin(), clients_.end(),
    [clientId](const ClientConnection& c) { return c.id == clientId; });
  
  return (it != clients_.end()) ? &(*it) : nullptr;
}

}  // namespace Network
}  // namespace Umbra

