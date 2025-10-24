#include "AuthConnectionPool.hpp"
#include "core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <random>

namespace Umbra {
namespace Gateway {

AuthConnectionPool::AuthConnectionPool(const Config& config)
    : config_(config),
      running_(false),
      nextConnectionIndex_(0),
      totalRequests_(0),
      successfulRequests_(0),
      failedRequests_(0),
      poolHits_(0),
      poolMisses_(0) {
}

AuthConnectionPool::~AuthConnectionPool() {
  stop();
}

bool AuthConnectionPool::start() {
  if (running_) {
    Core::Logger::getInstance().warn("AuthConnectionPool already running");
    return false;
  }
  
  running_ = true;
  
  // Initialize connections
  initializeConnections();
  
  // Start health check thread
  healthCheckThread_ = std::make_unique<std::thread>(&AuthConnectionPool::healthCheckLoop, this);
  
  Core::Logger::getInstance().info("AuthConnectionPool started with {} connections", 
                                   connections_.size());
  
  return true;
}

void AuthConnectionPool::stop() {
  if (!running_) {
    return;
  }
  
  running_ = false;
  
  // Stop health check thread
  if (healthCheckThread_ && healthCheckThread_->joinable()) {
    healthCheckThread_->join();
  }
  
  // Stop all connections
  {
    std::lock_guard<std::mutex> lock(poolMutex_);
    for (auto& connection : connections_) {
      connection->stop();
    }
    connections_.clear();
    
    // Clear available connections queue
    while (!availableConnections_.empty()) {
      availableConnections_.pop();
    }
  }
  
  Core::Logger::getInstance().info("AuthConnectionPool stopped");
}

bool AuthConnectionPool::hasActiveConnections() const {
  std::lock_guard<std::mutex> lock(poolMutex_);
  
  for (const auto& connection : connections_) {
    if (connection->isConnected()) {
      return true;
    }
  }
  
  return false;
}

bool AuthConnectionPool::validateToken(const std::string& token, ResponseCallback callback) {
  auto connection = getAvailableConnection();
  if (!connection) {
    Core::Logger::getInstance().warn("No available connections in pool");
    poolMisses_++;
    return false;
  }
  
  poolHits_++;
  
  // Wrap callback to return connection to pool
  auto wrappedCallback = [this, connection, callback](const AuthResponse& response) {
    returnConnection(connection);
    
    if (response.success) {
      successfulRequests_++;
    } else {
      failedRequests_++;
    }
    
    if (callback) {
      callback(response);
    }
  };
  
  bool result = connection->validateToken(token, wrappedCallback);
  
  if (!result) {
    returnConnection(connection);
    poolMisses_++;
  }
  
  totalRequests_++;
  return result;
}

std::optional<AuthResponse> AuthConnectionPool::validateTokenSync(const std::string& token) {
  auto connection = getAvailableConnection();
  if (!connection) {
    Core::Logger::getInstance().warn("No available connections in pool");
    poolMisses_++;
    return std::nullopt;
  }
  
  poolHits_++;
  
  auto response = connection->validateTokenSync(token);
  
  returnConnection(connection);
  
  if (response) {
    if (response->success) {
      successfulRequests_++;
    } else {
      failedRequests_++;
    }
  } else {
    failedRequests_++;
  }
  
  totalRequests_++;
  return response;
}

bool AuthConnectionPool::checkSession(uint64_t accountId, ResponseCallback callback) {
  auto connection = getAvailableConnection();
  if (!connection) {
    Core::Logger::getInstance().warn("No available connections in pool");
    poolMisses_++;
    return false;
  }
  
  poolHits_++;
  
  // Wrap callback to return connection to pool
  auto wrappedCallback = [this, connection, callback](const AuthResponse& response) {
    returnConnection(connection);
    
    if (response.success) {
      successfulRequests_++;
    } else {
      failedRequests_++;
    }
    
    if (callback) {
      callback(response);
    }
  };
  
  bool result = connection->checkSession(accountId, wrappedCallback);
  
  if (!result) {
    returnConnection(connection);
    poolMisses_++;
  }
  
  totalRequests_++;
  return result;
}

bool AuthConnectionPool::revokeToken(const std::string& token, ResponseCallback callback) {
  auto connection = getAvailableConnection();
  if (!connection) {
    Core::Logger::getInstance().warn("No available connections in pool");
    poolMisses_++;
    return false;
  }
  
  poolHits_++;
  
  // Wrap callback to return connection to pool
  auto wrappedCallback = [this, connection, callback](const AuthResponse& response) {
    returnConnection(connection);
    
    if (response.success) {
      successfulRequests_++;
    } else {
      failedRequests_++;
    }
    
    if (callback) {
      callback(response);
    }
  };
  
  bool result = connection->revokeToken(token, wrappedCallback);
  
  if (!result) {
    returnConnection(connection);
    poolMisses_++;
  }
  
  totalRequests_++;
  return result;
}

std::string AuthConnectionPool::getStats() const {
  nlohmann::json stats;
  
  {
    std::lock_guard<std::mutex> lock(poolMutex_);
    stats["total_connections"] = connections_.size();
    stats["available_connections"] = availableConnections_.size();
    
    uint32_t activeConnections = 0;
    for (const auto& connection : connections_) {
      if (connection->isConnected()) {
        activeConnections++;
      }
    }
    stats["active_connections"] = activeConnections;
  }
  
  stats["total_requests"] = totalRequests_.load();
  stats["successful_requests"] = successfulRequests_.load();
  stats["failed_requests"] = failedRequests_.load();
  stats["pool_hits"] = poolHits_.load();
  stats["pool_misses"] = poolMisses_.load();
  
  // Calculate hit rate
  uint32_t totalPoolRequests = poolHits_.load() + poolMisses_.load();
  if (totalPoolRequests > 0) {
    stats["hit_rate"] = static_cast<double>(poolHits_.load()) / totalPoolRequests;
  } else {
    stats["hit_rate"] = 0.0;
  }
  
  return stats.dump();
}

void AuthConnectionPool::initializeConnections() {
  std::lock_guard<std::mutex> lock(poolMutex_);
  
  // Create connections for each host/port combination
  for (size_t i = 0; i < config_.authHosts.size(); ++i) {
    std::string host = config_.authHosts[i];
    uint16_t port = (i < config_.authPorts.size()) ? config_.authPorts[i] : config_.authPorts[0];
    
    // Create multiple connections per host
    for (uint32_t j = 0; j < config_.maxConnectionsPerHost; ++j) {
      AuthClient::Config clientConfig;
      clientConfig.host = host;
      clientConfig.port = port;
      clientConfig.connectionTimeoutMs = config_.connectionTimeoutMs;
      clientConfig.requestTimeoutMs = config_.requestTimeoutMs;
      clientConfig.maxRetries = config_.maxRetries;
      clientConfig.reconnectIntervalMs = config_.reconnectIntervalMs;
      
      auto connection = std::make_unique<AuthClient>(clientConfig);
      
      if (connection->start()) {
        connections_.push_back(std::move(connection));
        Core::Logger::getInstance().info("Created connection to {}:{}", host, port);
      } else {
        Core::Logger::getInstance().warn("Failed to create connection to {}:{}", host, port);
      }
    }
  }
  
  // Initialize available connections queue
  for (auto& connection : connections_) {
    availableConnections_.push(connection.get());
  }
  
  Core::Logger::getInstance().info("Initialized {} connections", connections_.size());
}

void AuthConnectionPool::healthCheckLoop() {
  while (running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(config_.healthCheckIntervalMs));
    
    if (!running_) {
      break;
    }
    
    cleanupUnhealthyConnections();
  }
}

AuthClient* AuthConnectionPool::getAvailableConnection() {
  std::lock_guard<std::mutex> lock(poolMutex_);
  
  if (availableConnections_.empty()) {
    return nullptr;
  }
  
  AuthClient* connection = availableConnections_.front();
  availableConnections_.pop();
  
  return connection;
}

void AuthConnectionPool::returnConnection(AuthClient* connection) {
  if (!connection) {
    return;
  }
  
  std::lock_guard<std::mutex> lock(poolMutex_);
  
  // Check if connection is still healthy before returning to pool
  if (isConnectionHealthy(connection)) {
    availableConnections_.push(connection);
  } else {
    Core::Logger::getInstance().warn("Connection unhealthy, not returning to pool");
  }
}

bool AuthConnectionPool::isConnectionHealthy(AuthClient* connection) {
  if (!connection) {
    return false;
  }
  
  return connection->isConnected();
}

void AuthConnectionPool::cleanupUnhealthyConnections() {
  std::lock_guard<std::mutex> lock(poolMutex_);
  
  // Remove unhealthy connections from available queue
  std::queue<AuthClient*> healthyConnections;
  
  while (!availableConnections_.empty()) {
    AuthClient* connection = availableConnections_.front();
    availableConnections_.pop();
    
    if (isConnectionHealthy(connection)) {
      healthyConnections.push(connection);
    } else {
      Core::Logger::getInstance().warn("Removing unhealthy connection from pool");
    }
  }
  
  // Replace queue with healthy connections
  availableConnections_ = std::move(healthyConnections);
  
  // Log pool status
  Core::Logger::getInstance().debug("Pool status: {} total, {} available, {} active", 
                                    connections_.size(), 
                                    availableConnections_.size(),
                                    [this]() {
                                      uint32_t active = 0;
                                      for (const auto& conn : connections_) {
                                        if (conn->isConnected()) active++;
                                      }
                                      return active;
                                    }());
}

}  // namespace Gateway
}  // namespace Umbra
