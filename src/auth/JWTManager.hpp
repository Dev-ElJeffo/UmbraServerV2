#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <map>

namespace Umbra {
namespace Auth {

/**
 * @brief Payload do token JWT
 */
struct JWTPayload {
  uint64_t accountId = 0;
  uint64_t playerId = 0;
  std::string username;
  std::chrono::system_clock::time_point issuedAt;
  std::chrono::system_clock::time_point expiresAt;
  
  bool isExpired() const {
    return std::chrono::system_clock::now() >= expiresAt;
  }
};

/**
 * @brief Gerenciador de tokens JWT
 * 
 * Gera e valida tokens JWT usando HS256.
 * Mantém blacklist de tokens revogados no Redis.
 */
class JWTManager {
 public:
  /**
   * @brief Inicializa gerenciador JWT
   * @param secretKey Chave secreta para assinatura
   */
  explicit JWTManager(const std::string& secretKey);
  
  /**
   * @brief Gera token JWT
   * @param accountId ID da conta
   * @param playerId ID do personagem
   * @param username Nome de usuário
   * @param expirationMinutes Minutos até expiração
   * @return Token JWT
   */
  std::string generateToken(uint64_t accountId, 
                            uint64_t playerId,
                            const std::string& username,
                            uint32_t expirationMinutes = 60);
  
  /**
   * @brief Valida e decodifica token JWT
   * @param token Token a validar
   * @return Payload ou nullopt se inválido
   */
  std::optional<JWTPayload> validateToken(const std::string& token);
  
  /**
   * @brief Revoga token (adiciona à blacklist)
   * @param token Token a revogar
   * @return true se revogado com sucesso
   */
  bool revokeToken(const std::string& token);
  
  /**
   * @brief Verifica se token está revogado
   * @param token Token a verificar
   * @return true se revogado
   */
  bool isTokenRevoked(const std::string& token);
  
  /**
   * @brief Atualiza token (refresh)
   * @param oldToken Token existente
   * @param expirationMinutes Minutos até expiração do novo token
   * @return Novo token ou string vazia se falhou
   */
  std::string refreshToken(const std::string& oldToken, 
                           uint32_t expirationMinutes = 60);

 private:
  std::string secretKey_;
  std::map<std::string, std::chrono::system_clock::time_point> blacklist_;
  
  std::string base64UrlEncode(const std::string& input);
  std::string base64UrlDecode(const std::string& input);
  std::string hmacSha256(const std::string& data, const std::string& key);
  std::string createHeader();
  std::string createPayload(const JWTPayload& payload);
  std::optional<JWTPayload> parsePayload(const std::string& payloadStr);
};

}  // namespace Auth
}  // namespace Umbra

