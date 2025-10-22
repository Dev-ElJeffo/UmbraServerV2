#include "AuthClient.hpp"
#include "core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <cstring>
#include <chrono>
#include <future>
#include <algorithm>

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
namespace Gateway {

AuthClient::AuthClient(const Config& config)
    : config_(config),
      socket_(static_cast<int>(INVALID_SOCKET)),
      running_(false),
      connected_(false),
      nextRequestId_(1),
      totalRequests_(0),
      successfulRequests_(0),
      failedRequests_(0) {
#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

AuthClient::~AuthClient() {
  stop();
#ifdef _WIN32
  WSACleanup();
#endif
}

bool AuthClient::start() {
  if (running_) {
    Core::Logger::getInstance().warn("AuthClient already running");
    return false;
  }
  
  running_ = true;
  
  // Start connection thread
  connectionThread_ = std::make_unique<std::thread>(&AuthClient::connectionLoop, this);
  
  // Start response thread
  responseThread_ = std::make_unique<std::thread>(&AuthClient::responseLoop, this);
  
  Core::Logger::getInstance().info("AuthClient started, connecting to {}:{}", 
                                   config_.host, config_.port);
  
  return true;
}

void AuthClient::stop() {
  if (!running_) {
    return;
  }
  
  running_ = false;
  connected_ = false;
  
  // Notify response thread
  {
    std::lock_guard<std::mutex> lock(pendingMutex_);
    pendingCondition_.notify_all();
  }
  
  if (connectionThread_ && connectionThread_->joinable()) {
    connectionThread_->join();
  }
  
  if (responseThread_ && responseThread_->joinable()) {
    responseThread_->join();
  }
  
  closeSocket();
  
  Core::Logger::getInstance().info("AuthClient stopped");
}

bool AuthClient::isConnected() const {
  return connected_;
}

bool AuthClient::validateToken(const std::string& token, ResponseCallback callback) {
  if (!connected_) {
    Core::Logger::getInstance().warn("AuthClient not connected");
    return false;
  }
  
  nlohmann::json requestData;
  requestData["token"] = token;
  
  std::string request = createRequest("validate_token", requestData);
  
  if (!sendRequest(request)) {
    return false;
  }
  
  // Store callback for response
  {
    std::lock_guard<std::mutex> lock(pendingMutex_);
    PendingRequest pending;
    pending.requestId = nextRequestId_++;
    pending.callback = callback;
    pending.timestamp = std::chrono::system_clock::now();
    pendingRequests_.push(pending);
  }
  
  pendingCondition_.notify_one();
  totalRequests_++;
  
  return true;
}

std::optional<AuthResponse> AuthClient::validateTokenSync(const std::string& token) {
  std::promise<AuthResponse> promise;
  std::future<AuthResponse> future = promise.get_future();
  
  auto callback = [&promise](const AuthResponse& response) {
    promise.set_value(response);
  };
  
  if (!validateToken(token, callback)) {
    return std::nullopt;
  }
  
  // Wait for response with timeout
  auto status = future.wait_for(std::chrono::milliseconds(config_.requestTimeoutMs));
  
  if (status == std::future_status::timeout) {
    Core::Logger::getInstance().warn("Token validation timeout");
    return std::nullopt;
  }
  
  return future.get();
}

bool AuthClient::checkSession(uint64_t accountId, ResponseCallback callback) {
  if (!connected_) {
    Core::Logger::getInstance().warn("AuthClient not connected");
    return false;
  }
  
  nlohmann::json requestData;
  requestData["account_id"] = accountId;
  
  std::string request = createRequest("check_session", requestData);
  
  if (!sendRequest(request)) {
    return false;
  }
  
  // Store callback for response
  {
    std::lock_guard<std::mutex> lock(pendingMutex_);
    PendingRequest pending;
    pending.requestId = nextRequestId_++;
    pending.callback = callback;
    pending.timestamp = std::chrono::system_clock::now();
    pendingRequests_.push(pending);
  }
  
  pendingCondition_.notify_one();
  totalRequests_++;
  
  return true;
}

bool AuthClient::revokeToken(const std::string& token, ResponseCallback callback) {
  if (!connected_) {
    Core::Logger::getInstance().warn("AuthClient not connected");
    return false;
  }
  
  nlohmann::json requestData;
  requestData["token"] = token;
  
  std::string request = createRequest("revoke_token", requestData);
  
  if (!sendRequest(request)) {
    return false;
  }
  
  // Store callback for response
  {
    std::lock_guard<std::mutex> lock(pendingMutex_);
    PendingRequest pending;
    pending.requestId = nextRequestId_++;
    pending.callback = callback;
    pending.timestamp = std::chrono::system_clock::now();
    pendingRequests_.push(pending);
  }
  
  pendingCondition_.notify_one();
  totalRequests_++;
  
  return true;
}

std::string AuthClient::getStats() const {
  nlohmann::json stats;
  stats["connected"] = connected_.load();
  stats["host"] = config_.host;
  stats["port"] = config_.port;
  stats["total_requests"] = totalRequests_.load();
  stats["successful_requests"] = successfulRequests_.load();
  stats["failed_requests"] = failedRequests_.load();
  
  return stats.dump();
}

bool AuthClient::initializeSocket() {
  socket_ = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
  
  if (socket_ == INVALID_SOCKET) {
    Core::Logger::getInstance().error("Failed to create socket");
    return false;
  }
  
  // Set socket options
  int opt = 1;
  setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, 
             reinterpret_cast<const char*>(&opt), sizeof(opt));
  
  return true;
}

void AuthClient::connectionLoop() {
  while (running_) {
    if (!connected_) {
      if (initializeSocket()) {
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(config_.port);
        
        if (inet_pton(AF_INET, config_.host.c_str(), &serverAddr.sin_addr) <= 0) {
          // Try to resolve localhost to 127.0.0.1
          if (config_.host == "localhost") {
            inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
          } else {
            Core::Logger::getInstance().error("Invalid server address: {}", config_.host);
            closeSocket();
            std::this_thread::sleep_for(std::chrono::milliseconds(config_.reconnectIntervalMs));
            continue;
          }
        }
        
        if (connect(socket_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
          Core::Logger::getInstance().warn("Failed to connect to auth server {}:{}", 
                                           config_.host, config_.port);
          closeSocket();
          std::this_thread::sleep_for(std::chrono::milliseconds(config_.reconnectIntervalMs));
          continue;
        }
        
        connected_ = true;
        Core::Logger::getInstance().info("Connected to auth server {}:{}", 
                                         config_.host, config_.port);
      }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void AuthClient::responseLoop() {
  const size_t bufferSize = 4096;
  std::vector<char> buffer(bufferSize);
  
  while (running_) {
    if (!connected_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    
    // Check for incoming data
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socket_, &readfds);
    
    timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000; // 100ms
    
    int result = select(socket_ + 1, &readfds, nullptr, nullptr, &timeout);
    
    if (result > 0 && FD_ISSET(socket_, &readfds)) {
      int bytesReceived = recv(socket_, buffer.data(), bufferSize, 0);
      
      if (bytesReceived > 0) {
        std::string response(buffer.data(), bytesReceived);
        handleResponse(response);
      } else if (bytesReceived == 0) {
        Core::Logger::getInstance().info("Auth server disconnected");
        connected_ = false;
        closeSocket();
      } else {
        Core::Logger::getInstance().warn("Error receiving data from auth server");
        connected_ = false;
        closeSocket();
      }
    }
    
    // Process pending requests timeout
    {
      std::lock_guard<std::mutex> lock(pendingMutex_);
      auto now = std::chrono::system_clock::now();
      
      while (!pendingRequests_.empty()) {
        auto& pending = pendingRequests_.front();
        
        if (now - pending.timestamp > std::chrono::milliseconds(config_.requestTimeoutMs)) {
          AuthResponse timeoutResponse;
          timeoutResponse.success = false;
          timeoutResponse.message = "Request timeout";
          
          if (pending.callback) {
            pending.callback(timeoutResponse);
          }
          
          pendingRequests_.pop();
          failedRequests_++;
        } else {
          break;
        }
      }
    }
  }
}

bool AuthClient::sendRequest(const std::string& request) {
  if (!connected_) {
    return false;
  }
  
  int result = send(socket_, request.c_str(), static_cast<int>(request.length()), 0);
  
  if (result == SOCKET_ERROR) {
    Core::Logger::getInstance().error("Failed to send request to auth server");
    connected_ = false;
    return false;
  }
  
  return true;
}

void AuthClient::handleResponse(const std::string& response) {
  try {
    auto json = nlohmann::json::parse(response);
    
    AuthResponse authResponse;
    authResponse.success = json.value("success", false);
    authResponse.message = json.value("message", "");
    authResponse.token = json.value("token", "");
    authResponse.accountId = json.value("account_id", 0ULL);
    authResponse.playerId = json.value("player_id", 0ULL);
    authResponse.username = json.value("username", "");
    authResponse.valid = json.value("valid", false);
    
    // Find and execute callback
    {
      std::lock_guard<std::mutex> lock(pendingMutex_);
      
      if (!pendingRequests_.empty()) {
        auto pending = pendingRequests_.front();
        pendingRequests_.pop();
        
        if (pending.callback) {
          pending.callback(authResponse);
        }
        
        if (authResponse.success) {
          successfulRequests_++;
        } else {
          failedRequests_++;
        }
      }
    }
    
  } catch (const std::exception& e) {
    Core::Logger::getInstance().error("Error parsing auth response: {}", e.what());
    failedRequests_++;
  }
}

void AuthClient::closeSocket() {
  if (socket_ != INVALID_SOCKET) {
    CLOSE_SOCKET(socket_);
    socket_ = static_cast<int>(INVALID_SOCKET);
  }
}

bool AuthClient::reconnect() {
  closeSocket();
  connected_ = false;
  
  // Wait before reconnecting
  std::this_thread::sleep_for(std::chrono::milliseconds(config_.reconnectIntervalMs));
  
  return initializeSocket();
}

std::string AuthClient::createRequest(const std::string& action, const nlohmann::json& data) {
  nlohmann::json request;
  request["action"] = action;
  request["data"] = data;
  request["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  
  return request.dump() + "\n";
}

AuthResponse AuthClient::parseResponse(const std::string& response) {
  AuthResponse authResponse;
  
  try {
    auto json = nlohmann::json::parse(response);
    
    authResponse.success = json.value("success", false);
    authResponse.message = json.value("message", "");
    authResponse.token = json.value("token", "");
    authResponse.accountId = json.value("account_id", 0ULL);
    authResponse.playerId = json.value("player_id", 0ULL);
    authResponse.username = json.value("username", "");
    authResponse.valid = json.value("valid", false);
    
  } catch (const std::exception& e) {
    Core::Logger::getInstance().error("Error parsing response: {}", e.what());
    authResponse.success = false;
    authResponse.message = "Parse error";
  }
  
  return authResponse;
}

}  // namespace Gateway
}  // namespace Umbra
