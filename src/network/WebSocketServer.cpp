#include "WebSocketServer.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include <cstring>
#include <sstream>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
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
  
  Core::Logger::getInstance().info("WebSocketServer started on port {}", port_);
  return true;
}

void WebSocketServer::stop() {
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
  std::lock_guard<std::mutex> lock(clientsMutex_);
  
  auto it = clients_.find(clientId);
  if (it == clients_.end() || !it->second.handshakeComplete) {
    return false;
  }
  
  WebSocketFrame frame;
  frame.opcode = WebSocketFrame::OpCode::TEXT;
  frame.fin = true;
  frame.payload.assign(message.begin(), message.end());
  
  return sendFrame(it->second.socket, frame);
}

bool WebSocketServer::sendBinary(uint32_t clientId, const std::vector<uint8_t>& data) {
  std::lock_guard<std::mutex> lock(clientsMutex_);
  
  auto it = clients_.find(clientId);
  if (it == clients_.end() || !it->second.handshakeComplete) {
    return false;
  }
  
  WebSocketFrame frame;
  frame.opcode = WebSocketFrame::OpCode::BINARY;
  frame.fin = true;
  frame.payload = data;
  
  return sendFrame(it->second.socket, frame);
}

void WebSocketServer::broadcastText(const std::string& message) {
  std::lock_guard<std::mutex> lock(clientsMutex_);
  
  WebSocketFrame frame;
  frame.opcode = WebSocketFrame::OpCode::TEXT;
  frame.fin = true;
  frame.payload.assign(message.begin(), message.end());
  
  for (auto& [id, client] : clients_) {
    if (client.handshakeComplete) {
      sendFrame(client.socket, frame);
    }
  }
}

void WebSocketServer::broadcastBinary(const std::vector<uint8_t>& data) {
  // Se não houver clients, apenas retornar (não causar erro)
  // Isso é seguro e evita trabalho desnecessário
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    if (clients_.empty()) {
      return;
    }
  }
  
  WebSocketFrame frame;
  frame.opcode = WebSocketFrame::OpCode::BINARY;
  frame.fin = true;
  frame.payload = data;
  
  // Criar cópia dos client IDs e sockets para evitar problemas se a lista mudar durante o broadcast
  std::vector<std::pair<uint32_t, int>> clientsToSend;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    clientsToSend.reserve(clients_.size());
    for (const auto& [id, client] : clients_) {
      if (client.handshakeComplete) {
        clientsToSend.push_back({id, client.socket});
      }
    }
  }
  
  // Enviar para cada client (sem lock para evitar deadlock)
  for (const auto& [clientId, socket] : clientsToSend) {
    try {
      sendFrame(socket, frame);
    } catch (const std::exception& e) {
      Core::Logger::getInstance().warn("Exception while broadcasting to client {}: {}", clientId, e.what());
      // Continuar com outros clients mesmo se um falhar
    } catch (...) {
      Core::Logger::getInstance().warn("Unknown exception while broadcasting to client {}", clientId);
      // Continuar com outros clients mesmo se um falhar
    }
  }
}

void WebSocketServer::disconnect(uint32_t clientId) {
  std::lock_guard<std::mutex> lock(clientsMutex_);
  
  auto it = clients_.find(clientId);
  if (it != clients_.end()) {
    closeSocket(it->second.socket);
    clients_.erase(it);
    
    if (connectionCallback_) {
      connectionCallback_(clientId, false);
    }
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
  if (!performHandshake(clientSocket)) {
    closeSocket(clientSocket);
    return;
  }
  
  uint32_t clientId = nextClientId_++;
  
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    ClientState state;
    state.id = clientId;
    state.socket = clientSocket;
    state.handshakeComplete = true;
    state.address = address;
    state.port = port;
    clients_[clientId] = state;
  }
  
  if (connectionCallback_) {
    connectionCallback_(clientId, true);
  }
  
  while (running_) {
    try {
      WebSocketFrame frame = receiveFrame(clientSocket);
      
      if (frame.opcode == WebSocketFrame::OpCode::CLOSE) {
        break;
      }
      
      // Verificar se o frame é válido (não apenas CLOSE por erro)
      // Se receiveFrame retornou CLOSE devido a erro de recv, sair do loop
      if (frame.opcode == WebSocketFrame::OpCode::CLOSE && frame.payload.empty()) {
        // Pode ser um erro de rede, verificar se o socket ainda é válido
        break;
      }
      
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
        sendFrame(clientSocket, pong);
      }
    } catch (const std::exception& e) {
      Core::Logger::getInstance().error("Exception in WebSocket client handler for client {}: {}", clientId, e.what());
      break;
    } catch (...) {
      Core::Logger::getInstance().error("Unknown exception in WebSocket client handler for client {}", clientId);
      break;
    }
  }
  
  disconnect(clientId);
  Core::Logger::getInstance().info("WebSocket client {} disconnected", clientId);
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

WebSocketFrame WebSocketServer::receiveFrame(int clientSocket) {
  WebSocketFrame frame;
  frame.opcode = WebSocketFrame::OpCode::CLOSE;
  
  uint8_t header[2];
  int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(header), 2, 0);
  if (bytesReceived <= 0) {
    // Erro ou conexão fechada
    return frame;
  }
  if (bytesReceived != 2) {
    // Header incompleto
    return frame;
  }
  
  frame.fin = (header[0] & 0x80) != 0;
  frame.opcode = static_cast<WebSocketFrame::OpCode>(header[0] & 0x0F);
  
  bool masked = (header[1] & 0x80) != 0;
  uint64_t payloadLen = header[1] & 0x7F;
  
  if (payloadLen == 126) {
    uint8_t len[2];
    bytesReceived = recv(clientSocket, reinterpret_cast<char*>(len), 2, 0);
    if (bytesReceived != 2) {
      frame.opcode = WebSocketFrame::OpCode::CLOSE;
      return frame;
    }
    payloadLen = (len[0] << 8) | len[1];
  } else if (payloadLen == 127) {
    uint8_t len[8];
    bytesReceived = recv(clientSocket, reinterpret_cast<char*>(len), 8, 0);
    if (bytesReceived != 8) {
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
    bytesReceived = recv(clientSocket, reinterpret_cast<char*>(maskKey), 4, 0);
    if (bytesReceived != 4) {
      frame.opcode = WebSocketFrame::OpCode::CLOSE;
      return frame;
    }
  }
  
  // Validar tamanho do payload (proteção contra valores inválidos)
  if (payloadLen > 1024 * 1024) {  // Limite de 1MB
    Core::Logger::getInstance().warn("WebSocket frame payload too large: {} bytes", payloadLen);
    frame.opcode = WebSocketFrame::OpCode::CLOSE;
    return frame;
  }
  
  frame.payload.resize(payloadLen);
  if (payloadLen > 0) {
    bytesReceived = recv(clientSocket, reinterpret_cast<char*>(frame.payload.data()), 
         static_cast<int>(payloadLen), 0);
    if (bytesReceived <= 0 || static_cast<uint64_t>(bytesReceived) != payloadLen) {
      // Erro ao receber payload completo
      frame.opcode = WebSocketFrame::OpCode::CLOSE;
      frame.payload.clear();
      return frame;
    }
    
    if (masked) {
      for (size_t i = 0; i < payloadLen; ++i) {
        frame.payload[i] ^= maskKey[i % 4];
      }
    }
  }
  
  return frame;
}

bool WebSocketServer::sendFrame(int clientSocket, const WebSocketFrame& frame) {
  // Verificar se o socket é válido antes de tentar enviar
  if (clientSocket == INVALID_SOCKET) {
    return false;
  }
  
  try {
    std::vector<uint8_t> data;
    
    uint8_t header = (frame.fin ? 0x80 : 0x00) | static_cast<uint8_t>(frame.opcode);
    data.push_back(header);
    
    size_t payloadLen = frame.payload.size();
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
    
    int result = send(clientSocket, 
                      reinterpret_cast<const char*>(data.data()), 
                      static_cast<int>(data.size()), 
                      0);
    
    // Se send falhou, o socket pode ter sido fechado - isso é normal e não deve parar o servidor
    if (result == SOCKET_ERROR) {
      // Log apenas em nível debug para não poluir os logs
      Core::Logger::getInstance().debug("Failed to send frame to socket {} (socket may be closed)", clientSocket);
      return false;
    }
    
    return true;
  } catch (const std::exception& e) {
    Core::Logger::getInstance().warn("Exception in sendFrame for socket {}: {}", clientSocket, e.what());
    return false;
  } catch (...) {
    Core::Logger::getInstance().warn("Unknown exception in sendFrame for socket {}", clientSocket);
    return false;
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

