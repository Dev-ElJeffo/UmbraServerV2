#include <algorithm>
#include "SocketServer.hpp"
#include "core/Logger.hpp"
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

SocketServer::SocketServer(ProtocolType type, uint16_t port)
    : type_(type),
      port_(port),
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
  
  if (acceptThread_ && acceptThread_->joinable()) {
    acceptThread_->join();
  }
  
  for (auto& thread : workerThreads_) {
    if (thread && thread->joinable()) {
      thread->join();
    }
  }
  workerThreads_.clear();
  
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    for (auto& client : clients_) {
      closeSocket(client.socket);
    }
    clients_.clear();
  }
  
  if (serverSocket_ != INVALID_SOCKET) {
    closeSocket(serverSocket_);
    serverSocket_ = INVALID_SOCKET;
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
  std::lock_guard<std::mutex> lock(clientsMutex_);
  
  auto* client = findClient(clientId);
  if (!client || !client->connected) {
    return false;
  }
  
  int result = send(client->socket, 
                    reinterpret_cast<const char*>(data.data()), 
                    static_cast<int>(data.size()), 
                    0);
  
  return result != SOCKET_ERROR;
}

void SocketServer::broadcast(const std::vector<uint8_t>& data) {
  std::lock_guard<std::mutex> lock(clientsMutex_);
  
  for (auto& client : clients_) {
    if (client.connected) {
      send(client.socket, 
           reinterpret_cast<const char*>(data.data()), 
           static_cast<int>(data.size()), 
           0);
    }
  }
}

void SocketServer::disconnectClient(uint32_t clientId) {
  std::lock_guard<std::mutex> lock(clientsMutex_);
  
  auto it = std::find_if(clients_.begin(), clients_.end(),
    [clientId](const ClientConnection& c) { return c.id == clientId; });
  
  if (it != clients_.end()) {
    closeSocket(it->socket);
    clients_.erase(it);
    
    if (connectionCallback_) {
      connectionCallback_(clientId, false);
    }
  }
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
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port_);
  
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
    
    auto thread = std::make_unique<std::thread>(
      &SocketServer::handleTcpClient, this, clientSocket, clientAddress, clientPort);
    workerThreads_.push_back(std::move(thread));
  }
}

void SocketServer::handleTcpClient(int clientSocket, 
                                   const std::string& address, 
                                   uint16_t port) {
  uint32_t clientId = nextClientId_++;
  
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    ClientConnection conn;
    conn.id = clientId;
    conn.address = address;
    conn.port = port;
    conn.connected = true;
    conn.socket = clientSocket;
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

