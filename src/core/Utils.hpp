#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <random>

namespace Umbra {
namespace Core {

/**
 * @brief Funções utilitárias gerais
 */
class Utils {
 public:
  /**
   * @brief Divide string por delimitador
   * @param str String a ser dividida
   * @param delimiter Delimitador
   * @return Vector de substrings
   */
  static std::vector<std::string> split(const std::string& str, 
                                        char delimiter);
  
  /**
   * @brief Remove espaços em branco do início e fim
   * @param str String a ser trimada
   * @return String trimada
   */
  static std::string trim(const std::string& str);
  
  /**
   * @brief Converte string para lowercase
   * @param str String a ser convertida
   * @return String em lowercase
   */
  static std::string toLower(const std::string& str);
  
  /**
   * @brief Converte string para uppercase
   * @param str String a ser convertida
   * @return String em uppercase
   */
  static std::string toUpper(const std::string& str);
  
  /**
   * @brief Valida formato de email
   * @param email Email a ser validado
   * @return true se válido
   */
  static bool isValidEmail(const std::string& email);
  
  /**
   * @brief Gera string aleatória
   * @param length Comprimento da string
   * @return String aleatória
   */
  static std::string generateRandomString(size_t length);
  
  /**
   * @brief Gera UUID v4
   * @return UUID como string
   */
  static std::string generateUUID();
  
  /**
   * @brief Obtém timestamp atual em milissegundos
   * @return Timestamp em ms desde epoch
   */
  static int64_t getCurrentTimestamp();
  
  /**
   * @brief Formata timestamp para string ISO 8601
   * @param timestamp Timestamp em ms
   * @return String formatada
   */
  static std::string formatTimestamp(int64_t timestamp);
  
  /**
   * @brief Hash de senha com bcrypt-style (simplificado)
   * @param password Senha em texto plano
   * @param salt Salt para o hash
   * @return Hash da senha
   */
  static std::string hashPassword(const std::string& password, 
                                   const std::string& salt = "");
  
  /**
   * @brief Verifica senha contra hash
   * @param password Senha em texto plano
   * @param hash Hash armazenado
   * @return true se correspondem
   */
  static bool verifyPassword(const std::string& password, 
                             const std::string& hash);
  
  /**
   * @brief Sanitiza string para prevenir SQL injection
   * @param input String a ser sanitizada
   * @return String sanitizada
   */
  static std::string sanitizeInput(const std::string& input);
  
  /**
   * @brief Codifica string em Base64
   * @param input String a ser codificada
   * @return String em Base64
   */
  static std::string base64Encode(const std::string& input);
  
  /**
   * @brief Decodifica string de Base64
   * @param input String em Base64
   * @return String decodificada
   */
  static std::string base64Decode(const std::string& input);

 private:
  static std::mt19937& getRandomGenerator();
  
  // Função auxiliar para hash simples (fallback)
  static std::string hashPasswordSimple(const std::string& password, 
                                        const std::string& salt = "");
};

}  // namespace Core
}  // namespace Umbra

