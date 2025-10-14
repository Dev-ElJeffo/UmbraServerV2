#pragma once

#include "database/Models.hpp"
#include <string>
#include <map>
#include <mutex>
#include <optional>
#include <chrono>

namespace Umbra {
namespace Auth {

/**
 * @brief Gerenciador de sessões de usuários
 * 
 * Mantém sessões ativas em memória com cache Redis opcional.
 * Suporta invalidação e verificação de sessões.
 */
class SessionManager {
 public:
  SessionManager();
  
  /**
   * @brief Cria nova sessão
   * @param accountId ID da conta
   * @param playerId ID do personagem
   * @param ipAddress IP do cliente
   * @param durationMinutes Duração da sessão em minutos
   * @return Token da sessão
   */
  std::string createSession(uint64_t accountId,
                            uint64_t playerId,
                            const std::string& ipAddress,
                            uint32_t durationMinutes = 60);
  
  /**
   * @brief Valida sessão
   * @param token Token da sessão
   * @return Session ou nullopt se inválida
   */
  std::optional<Database::Session> validateSession(const std::string& token);
  
  /**
   * @brief Invalida sessão
   * @param token Token da sessão
   * @return true se invalidada com sucesso
   */
  bool invalidateSession(const std::string& token);
  
  /**
   * @brief Invalida todas as sessões de uma conta
   * @param accountId ID da conta
   * @return Número de sessões invalidadas
   */
  uint32_t invalidateAccountSessions(uint64_t accountId);
  
  /**
   * @brief Atualiza tempo de expiração da sessão
   * @param token Token da sessão
   * @param additionalMinutes Minutos adicionais
   * @return true se atualizado com sucesso
   */
  bool extendSession(const std::string& token, uint32_t additionalMinutes);
  
  /**
   * @brief Obtém número de sessões ativas
   * @return Contagem de sessões
   */
  size_t getActiveSessionCount() const;
  
  /**
   * @brief Obtém sessões de uma conta
   * @param accountId ID da conta
   * @return Vector de sessões
   */
  std::vector<Database::Session> getAccountSessions(uint64_t accountId);
  
  /**
   * @brief Limpa sessões expiradas
   * @return Número de sessões removidas
   */
  uint32_t cleanupExpiredSessions();

 private:
  std::map<std::string, Database::Session> sessions_;
  mutable std::mutex mutex_;
  
  std::string generateSessionToken();
};

}  // namespace Auth
}  // namespace Umbra

