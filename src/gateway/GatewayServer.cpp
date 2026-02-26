#include "GatewayServer.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <cwchar>
#include <string>
#include <algorithm>

namespace Umbra {
namespace Gateway {

GatewayServer::GatewayServer(const Config& config)
    : config_(config), running_(false) {
  loadBalancer_ = std::make_unique<LoadBalancer>();
  jwtManager_ = std::make_unique<Auth::JWTManager>(config.jwtSecret);
  
  if (config.useConnectionPool) {
    // Configure Auth Connection Pool
    AuthConnectionPool::Config poolConfig;
    poolConfig.authHosts = {config.authHost};
    poolConfig.authPorts = {config.authPort};
    poolConfig.maxConnectionsPerHost = config.maxConnectionsPerHost;
    poolConfig.connectionTimeoutMs = config.authTimeoutMs;
    poolConfig.requestTimeoutMs = 3000;
    poolConfig.maxRetries = 3;
    poolConfig.reconnectIntervalMs = 1000;
    poolConfig.healthCheckIntervalMs = config.healthCheckIntervalMs;
    
    authPool_ = std::make_unique<AuthConnectionPool>(poolConfig);
  } else {
    // Configure single Auth Client
    AuthClient::Config authConfig;
    authConfig.host = config.authHost;
    authConfig.port = config.authPort;
    authConfig.connectionTimeoutMs = config.authTimeoutMs;
    authConfig.requestTimeoutMs = 3000;
    authConfig.maxRetries = 3;
    authConfig.reconnectIntervalMs = 1000;
    
    authClient_ = std::make_unique<AuthClient>(authConfig);
  }
  
  // Configure Network Server
  networkServer_ = std::make_unique<Network::SocketServer>(
    Network::ProtocolType::TCP, config.port);
  
  networkServer_->setMessageCallback(
    [this](uint32_t clientId, const std::vector<uint8_t>& data) {
      handleClientMessage(clientId, data);
    });
  
  networkServer_->setConnectionCallback(
    [this](uint32_t clientId, bool connected) {
      if (connected) {
        Core::Logger::getInstance().info("Client {} connected", clientId);
      } else {
        Core::Logger::getInstance().info("Client {} disconnected", clientId);
      }
    });
  
  networkServer_->setRateLimit(config.rateLimitPerSecond);
}

GatewayServer::~GatewayServer() {
  stop();
}

bool GatewayServer::start() {
  if (running_) {
    Core::Logger::getInstance().warn("GatewayServer already running");
    return false;
  }
  
  // Start Auth Client or Pool
  if (config_.useConnectionPool) {
    if (!authPool_->start()) {
      Core::Logger::getInstance().error("Failed to start Auth Connection Pool");
      return false;
    }
    
    // Wait for pool to have active connections
    int retries = 0;
    while (!authPool_->hasActiveConnections() && retries < 10) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      retries++;
    }
    
    if (!authPool_->hasActiveConnections()) {
      Core::Logger::getInstance().warn("Auth Pool has no active connections, continuing anyway");
    }
  } else {
    if (!authClient_->start()) {
      Core::Logger::getInstance().error("Failed to start Auth Client");
      return false;
    }
    
    // Wait for auth client connection
    int retries = 0;
    while (!authClient_->isConnected() && retries < 10) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      retries++;
    }
    
    if (!authClient_->isConnected()) {
      Core::Logger::getInstance().warn("Auth Client not connected, continuing anyway");
    }
  }
  
  // Start Network Server
  if (!networkServer_->start()) {
    Core::Logger::getInstance().error("Failed to start Network Server");
    if (config_.useConnectionPool) {
      authPool_->stop();
    } else {
      authClient_->stop();
    }
    return false;
  }
  
  running_ = true;
  
  // Start session cleanup thread
  std::thread cleanupThread([this]() {
    while (running_) {
      cleanupExpiredSessions();
      std::this_thread::sleep_for(std::chrono::minutes(5));
    }
  });
  cleanupThread.detach();
  
  Core::Logger::getInstance().info("GatewayServer started on port {}", config_.port);
  return true;
}

void GatewayServer::stop() {
  if (!running_) {
    return;
  }
  
  running_ = false;
  
  if (networkServer_) {
    networkServer_->stop();
  }
  
  if (config_.useConnectionPool && authPool_) {
    authPool_->stop();
  } else if (authClient_) {
    authClient_->stop();
  }
  
  Core::Logger::getInstance().info("GatewayServer stopped");
}

bool GatewayServer::isRunning() const {
  return running_;
}

LoadBalancer& GatewayServer::getLoadBalancer() {
  return *loadBalancer_;
}

bool GatewayServer::validateToken(const std::string& token) {
  bool hasConnection = false;
  
  if (config_.useConnectionPool) {
    hasConnection = authPool_->hasActiveConnections();
  } else {
    hasConnection = authClient_->isConnected();
  }
  
  if (!hasConnection) {
    Core::Logger::getInstance().warn("Auth client not connected, using local validation");
    
    // Fallback to local JWT validation
    auto payload = jwtManager_->validateToken(token);
    return payload.has_value() && !payload->isExpired();
  }
  
  // Use TCP validation
  std::optional<AuthResponse> response;
  
  if (config_.useConnectionPool) {
    response = authPool_->validateTokenSync(token);
  } else {
    response = authClient_->validateTokenSync(token);
  }
  
  if (!response) {
    return false;
  }
  
  // Cache valid sessions
  if (response->success && response->valid) {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    clientSessions_[token] = *response;
  }
  
  return response->success && response->valid;
}

std::optional<AuthResponse> GatewayServer::getClientInfo(const std::string& token) {
  // Check cached session first
  {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    auto it = clientSessions_.find(token);
    if (it != clientSessions_.end()) {
      return it->second;
    }
  }
  
  // Validate via TCP
  bool hasConnection = false;
  
  if (config_.useConnectionPool) {
    hasConnection = authPool_->hasActiveConnections();
  } else {
    hasConnection = authClient_->isConnected();
  }
  
  if (!hasConnection) {
    return std::nullopt;
  }
  
  std::optional<AuthResponse> response;
  
  if (config_.useConnectionPool) {
    response = authPool_->validateTokenSync(token);
  } else {
    response = authClient_->validateTokenSync(token);
  }
  
  if (response && response->success && response->valid) {
    // Cache the response
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    clientSessions_[token] = *response;
  }
  
  return response;
}

void GatewayServer::handleConnection(uint32_t clientId) {
  Core::Logger::getInstance().debug("Handling connection for client {}", clientId);
  
  // Send welcome message
  nlohmann::json welcome;
  welcome["type"] = "welcome";
  welcome["message"] = "Connected to UmbraEternum Gateway";
  welcome["client_id"] = clientId;
  
  sendResponse(clientId, welcome.dump());
}

void GatewayServer::handleClientMessage(uint32_t clientId, 
                                        const std::vector<uint8_t>& data) {
  try {
    // Verificar se há dados recebidos
    if (data.empty()) {
      Core::Logger::getInstance().warn("Received empty message from client {}", clientId);
      nlohmann::json response;
      response["success"] = false;
      response["message"] = "Empty message";
      sendResponse(clientId, response.dump());
      return;
    }
    
    std::string message(data.begin(), data.end());
    
    // Debug: logar o tamanho e conteúdo da mensagem
    Core::Logger::getInstance().debug("Received message from client {}: size={}, content='{}'", 
                                       clientId, message.size(), message);
    
    // Verificar se a mensagem está vazia ou só contém espaços
    if (message.empty() || message.find_first_not_of(" \t\n\r") == std::string::npos) {
      Core::Logger::getInstance().warn("Received empty or whitespace-only message from client {}", clientId);
      nlohmann::json response;
      response["success"] = false;
      response["message"] = "Invalid message: empty or whitespace only";
      sendResponse(clientId, response.dump());
      return;
    }
    
    // Tentar fazer parse do JSON
    nlohmann::json json;
    try {
      json = nlohmann::json::parse(message);
    } catch (const nlohmann::json::parse_error&) {
      // Se falhar, tentar decodificar Base64 (caso o cliente tenha enviado dados criptografados)
      Core::Logger::getInstance().debug("First parse failed for client {}, trying Base64 decode", clientId);
      
      try {
        // Limpar a mensagem removendo caracteres inválidos de Base64
        // Primeiro, remover caracteres não imprimíveis no início/fim
        std::string cleaned = message;
        
        // Remover caracteres não Base64 válidos (incluindo caracteres estranhos como ñ, ¿, etc)
        cleaned.erase(std::remove_if(cleaned.begin(), cleaned.end(), 
                                     [](char c) { 
                                       unsigned char uc = static_cast<unsigned char>(c);
                                       // Permitir apenas Base64 válido: A-Z, a-z, 0-9, +, /, =
                                       return !((uc >= 'A' && uc <= 'Z') || 
                                                (uc >= 'a' && uc <= 'z') || 
                                                (uc >= '0' && uc <= '9') || 
                                                c == '+' || c == '/' || c == '=');
                                     }), 
                     cleaned.end());
        
        // Remover padding/whitespace no início e fim (mas preservar dentro da string Base64)
        if (!cleaned.empty()) {
          size_t first = cleaned.find_first_not_of(" \t\n\r");
          if (first != std::string::npos) {
            cleaned = cleaned.substr(first);
          } else {
            cleaned.clear();
          }
          
          if (!cleaned.empty()) {
            size_t lastChar = cleaned.find_last_not_of(" \t\n\r");
            if (lastChar != std::string::npos && lastChar < cleaned.length() - 1) {
              cleaned = cleaned.substr(0, lastChar + 1);
            }
          }
        }
        
        if (cleaned.empty()) {
          throw std::runtime_error("Cleaned Base64 message is empty");
        }
        
        Core::Logger::getInstance().debug("Cleaned Base64 message: size={}, prefix='{}'", 
                                           cleaned.size(), cleaned.substr(0, std::min(static_cast<size_t>(50), cleaned.size())));
        
        std::string decoded = Core::Utils::base64Decode(cleaned);
        Core::Logger::getInstance().debug("Decoded Base64 message from client {}: size={}", clientId, decoded.size());
        
        // Log dos primeiros bytes decodificados
        std::string hexPreview;
        for (size_t i = 0; i < std::min(static_cast<size_t>(20), decoded.size()); i++) {
          char hex[4];
          snprintf(hex, sizeof(hex), "%02X ", static_cast<unsigned char>(decoded[i]));
          hexPreview += hex;
        }
        Core::Logger::getInstance().debug("First 20 bytes (hex): {}", hexPreview);
        
        if (decoded.empty()) {
          throw std::runtime_error("Decoded message is empty");
        }
        
        // Descriptografar XOR
        // O Base64 do UE5 retorna bytes, então descriptografamos byte por byte
        std::string encryptionKey = "UmbraEternum2025SecretKey123456789";
        std::string decrypted;
        
        // Descriptografar XOR byte por byte
        decrypted.reserve(decoded.length());
        for (size_t i = 0; i < decoded.length(); i++) {
          char decryptedChar = decoded[i] ^ encryptionKey[i % encryptionKey.length()];
          // Apenas adicionar caracteres válidos (não nulos e imprimíveis ou whitespace)
          if (decryptedChar != '\0' && (decryptedChar >= 32 || decryptedChar == '\n' || decryptedChar == '\r' || decryptedChar == '\t')) {
            decrypted += decryptedChar;
          }
        }
        
        Core::Logger::getInstance().debug("Decrypted message from client {}: size={}, first_100_chars='{}'", 
                                          clientId, decrypted.size(), 
                                          decrypted.substr(0, std::min(static_cast<size_t>(100), decrypted.size())));
        
        if (decrypted.empty()) {
          throw std::runtime_error("Decrypted message is empty after XOR");
        }
        
        // Log completo da mensagem descriptografada (apenas primeiros 500 chars para não poluir logs)
        Core::Logger::getInstance().debug("Decrypted message from client {} (first 500 chars): {}", 
                                          clientId, decrypted.substr(0, std::min(static_cast<size_t>(500), decrypted.size())));
        
        // Extrair apenas o JSON válido (pode haver lixo após o JSON devido ao padding da criptografia)
        // Procurar pelo primeiro '{' e último '}' válido
        size_t jsonStart = decrypted.find('{');
        if (jsonStart == std::string::npos) {
          throw std::runtime_error("No JSON object found in decrypted message");
        }
        
        // Encontrar o último '}' válido (contando braces balanceadas)
        size_t jsonEnd = jsonStart;
        int braceCount = 0;
        for (size_t i = jsonStart; i < decrypted.size(); ++i) {
          if (decrypted[i] == '{') {
            braceCount++;
          } else if (decrypted[i] == '}') {
            braceCount--;
            if (braceCount == 0) {
              jsonEnd = i;
              break;
            }
          }
        }
        
        if (braceCount != 0) {
          throw std::runtime_error("Unbalanced JSON braces in decrypted message");
        }
        
        if (jsonEnd <= jsonStart) {
          throw std::runtime_error("Invalid JSON boundaries");
        }
        
        std::string jsonOnly = decrypted.substr(jsonStart, jsonEnd - jsonStart + 1);
        
        // Validar que não está vazio
        if (jsonOnly.empty()) {
          throw std::runtime_error("Extracted JSON is empty");
        }
        
        // Limpar o JSON removendo apenas caracteres de controle problemáticos (mantendo JSON válido)
        // Não remover caracteres válidos do JSON como espaços, tabs, newlines dentro de strings
        jsonOnly.erase(std::remove_if(jsonOnly.begin(), jsonOnly.end(),
                                     [](char c) {
                                       // Remover apenas caracteres nulos e DEL (127)
                                       // Manter todos os outros caracteres, incluindo espaços e quebras de linha válidos no JSON
                                       return c == '\0' || c == 127;
                                     }),
                     jsonOnly.end());
        
        // Validar novamente após limpeza
        if (jsonOnly.empty() || jsonOnly.find('{') == std::string::npos) {
          throw std::runtime_error("JSON is invalid after cleaning");
        }
        
        Core::Logger::getInstance().debug("Extracted JSON from client {}: size={}, preview='{}'", 
                                          clientId, jsonOnly.size(),
                                          jsonOnly.substr(0, std::min(static_cast<size_t>(200), jsonOnly.size())));
        
        // Tentar fazer parse do JSON
        json = nlohmann::json::parse(jsonOnly);
      } catch (const std::exception& e) {
        Core::Logger::getInstance().warn("Failed to decode and parse message from client {}: {}", clientId, e.what());
        nlohmann::json response;
        response["success"] = false;
        response["message"] = "Invalid message format";
        sendResponse(clientId, response.dump());
        return;
      }
    }
    
    std::string action = json.value("action", "");
    
    if (action == "authenticate") {
      std::string token = json.value("token", "");
      
      if (token.empty()) {
        nlohmann::json response;
        response["success"] = false;
        response["message"] = "Token required";
        sendResponse(clientId, response.dump());
        return;
      }
      
      bool isValid = validateToken(token);
      
      nlohmann::json response;
      response["success"] = isValid;
      response["message"] = isValid ? "Authentication successful" : "Invalid token";
      
      if (isValid) {
        auto clientInfo = getClientInfo(token);
        if (clientInfo) {
          response["account_id"] = clientInfo->accountId;
          response["player_id"] = clientInfo->playerId;
          response["username"] = clientInfo->username;
        }
      }
      
      sendResponse(clientId, response.dump());
      
    } else if (action == "get_server_info") {
      nlohmann::json response;
      response["success"] = true;
      response["server_count"] = loadBalancer_->getServerCount();
      
      if (config_.useConnectionPool) {
        response["auth_connected"] = authPool_->hasActiveConnections();
        response["auth_stats"] = nlohmann::json::parse(authPool_->getStats());
      } else {
        response["auth_connected"] = authClient_->isConnected();
        response["auth_stats"] = nlohmann::json::parse(authClient_->getStats());
      }
      
      sendResponse(clientId, response.dump());
      
    } else if (action == "ping") {
      nlohmann::json response;
      response["type"] = "pong";
      response["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
      
      sendResponse(clientId, response.dump());
      
    } else {
      nlohmann::json response;
      response["success"] = false;
      response["message"] = "Unknown action: " + action;
      sendResponse(clientId, response.dump());
    }
    
  } catch (const std::exception& e) {
    Core::Logger::getInstance().error("Error handling client message: {}", e.what());
    
    nlohmann::json response;
    response["success"] = false;
    response["message"] = "Invalid message format";
    sendResponse(clientId, response.dump());
  }
}

void GatewayServer::sendResponse(uint32_t clientId, const std::string& response) {
  std::vector<uint8_t> data(response.begin(), response.end());
  networkServer_->sendToClient(clientId, data);
}

void GatewayServer::cleanupExpiredSessions() {
  std::lock_guard<std::mutex> lock(sessionsMutex_);
  
  auto now = std::chrono::system_clock::now();
  auto it = clientSessions_.begin();
  
  while (it != clientSessions_.end()) {
    // Simple cleanup - remove sessions older than 1 hour
    // In a real implementation, you'd check JWT expiration
    if (now - std::chrono::system_clock::time_point{} > std::chrono::hours(1)) {
      it = clientSessions_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace Gateway
}  // namespace Umbra

