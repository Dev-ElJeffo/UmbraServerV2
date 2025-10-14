#include "JWTManager.hpp"
#include "core/Logger.hpp"
#include "core/Utils.hpp"
#include <sstream>
#include <nlohmann/json.hpp>

namespace Umbra {
namespace Auth {

JWTManager::JWTManager(const std::string& secretKey)
    : secretKey_(secretKey) {
  if (secretKey_.empty()) {
    Core::Logger::getInstance().warn("JWT secret key is empty!");
  }
}

std::string JWTManager::generateToken(uint64_t accountId, 
                                      uint64_t playerId,
                                      const std::string& username,
                                      uint32_t expirationMinutes) {
  JWTPayload payload;
  payload.accountId = accountId;
  payload.playerId = playerId;
  payload.username = username;
  payload.issuedAt = std::chrono::system_clock::now();
  payload.expiresAt = payload.issuedAt + std::chrono::minutes(expirationMinutes);
  
  std::string header = createHeader();
  std::string payloadStr = createPayload(payload);
  
  std::string headerEncoded = base64UrlEncode(header);
  std::string payloadEncoded = base64UrlEncode(payloadStr);
  
  std::string signature = hmacSha256(headerEncoded + "." + payloadEncoded, secretKey_);
  std::string signatureEncoded = base64UrlEncode(signature);
  
  std::string token = headerEncoded + "." + payloadEncoded + "." + signatureEncoded;
  
  Core::Logger::getInstance().debug("Generated JWT token for user: {}", username);
  
  return token;
}

std::optional<JWTPayload> JWTManager::validateToken(const std::string& token) {
  if (isTokenRevoked(token)) {
    Core::Logger::getInstance().warn("Token is revoked");
    return std::nullopt;
  }
  
  // Split token
  size_t firstDot = token.find('.');
  size_t secondDot = token.find('.', firstDot + 1);
  
  if (firstDot == std::string::npos || secondDot == std::string::npos) {
    Core::Logger::getInstance().warn("Invalid token format");
    return std::nullopt;
  }
  
  std::string headerEncoded = token.substr(0, firstDot);
  std::string payloadEncoded = token.substr(firstDot + 1, secondDot - firstDot - 1);
  std::string signatureEncoded = token.substr(secondDot + 1);
  
  // Verify signature
  std::string expectedSignature = hmacSha256(headerEncoded + "." + payloadEncoded, secretKey_);
  std::string expectedSignatureEncoded = base64UrlEncode(expectedSignature);
  
  if (signatureEncoded != expectedSignatureEncoded) {
    Core::Logger::getInstance().warn("Invalid token signature");
    return std::nullopt;
  }
  
  // Decode payload
  std::string payloadStr = base64UrlDecode(payloadEncoded);
  auto payload = parsePayload(payloadStr);
  
  if (!payload) {
    Core::Logger::getInstance().warn("Failed to parse token payload");
    return std::nullopt;
  }
  
  if (payload->isExpired()) {
    Core::Logger::getInstance().warn("Token is expired");
    return std::nullopt;
  }
  
  return payload;
}

bool JWTManager::revokeToken(const std::string& token) {
  auto payload = validateToken(token);
  if (!payload) {
    return false;
  }
  
  blacklist_[token] = payload->expiresAt;
  
  Core::Logger::getInstance().info("Revoked token for user: {}", payload->username);
  return true;
}

bool JWTManager::isTokenRevoked(const std::string& token) {
  auto it = blacklist_.find(token);
  if (it == blacklist_.end()) {
    return false;
  }
  
  // Clean up expired entries
  if (std::chrono::system_clock::now() >= it->second) {
    blacklist_.erase(it);
    return false;
  }
  
  return true;
}

std::string JWTManager::refreshToken(const std::string& oldToken, 
                                     uint32_t expirationMinutes) {
  auto payload = validateToken(oldToken);
  if (!payload) {
    return "";
  }
  
  revokeToken(oldToken);
  
  return generateToken(payload->accountId, 
                       payload->playerId, 
                       payload->username, 
                       expirationMinutes);
}

std::string JWTManager::base64UrlEncode(const std::string& input) {
  std::string encoded = Core::Utils::base64Encode(input);
  
  // Convert to URL-safe format
  for (char& c : encoded) {
    if (c == '+') c = '-';
    if (c == '/') c = '_';
  }
  
  // Remove padding
  encoded.erase(std::find(encoded.begin(), encoded.end(), '='), encoded.end());
  
  return encoded;
}

std::string JWTManager::base64UrlDecode(const std::string& input) {
  std::string decoded = input;
  
  // Convert from URL-safe format
  for (char& c : decoded) {
    if (c == '-') c = '+';
    if (c == '_') c = '/';
  }
  
  // Add padding
  while (decoded.size() % 4 != 0) {
    decoded += '=';
  }
  
  return Core::Utils::base64Decode(decoded);
}

std::string JWTManager::hmacSha256(const std::string& data, const std::string& key) {
  // TODO: Implement proper HMAC-SHA256
  // This is a placeholder using simple hash
  std::string combined = data + key;
  return Core::Utils::hashPassword(combined);
}

std::string JWTManager::createHeader() {
  nlohmann::json header;
  header["alg"] = "HS256";
  header["typ"] = "JWT";
  
  return header.dump();
}

std::string JWTManager::createPayload(const JWTPayload& payload) {
  nlohmann::json json;
  json["account_id"] = payload.accountId;
  json["player_id"] = payload.playerId;
  json["username"] = payload.username;
  
  auto iat = std::chrono::duration_cast<std::chrono::seconds>(
    payload.issuedAt.time_since_epoch()).count();
  auto exp = std::chrono::duration_cast<std::chrono::seconds>(
    payload.expiresAt.time_since_epoch()).count();
  
  json["iat"] = iat;
  json["exp"] = exp;
  
  return json.dump();
}

std::optional<JWTPayload> JWTManager::parsePayload(const std::string& payloadStr) {
  try {
    auto json = nlohmann::json::parse(payloadStr);
    
    JWTPayload payload;
    payload.accountId = json["account_id"];
    payload.playerId = json["player_id"];
    payload.username = json["username"];
    
    int64_t iat = json["iat"];
    int64_t exp = json["exp"];
    
    payload.issuedAt = std::chrono::system_clock::time_point(std::chrono::seconds(iat));
    payload.expiresAt = std::chrono::system_clock::time_point(std::chrono::seconds(exp));
    
    return payload;
  } catch (const std::exception& e) {
    Core::Logger::getInstance().error("Failed to parse JWT payload: {}", e.what());
    return std::nullopt;
  }
}

}  // namespace Auth
}  // namespace Umbra

