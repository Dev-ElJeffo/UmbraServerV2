#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>

namespace Umbra {
namespace Core {

/**
 * @brief Gerenciador de configurações com suporte a hot-reload
 * 
 * Carrega e valida arquivos JSON de configuração. Suporta
 * recarregamento dinâmico para configs não-críticas.
 */
class ConfigManager {
 public:
  static ConfigManager& getInstance();
  
  /**
   * @brief Carrega arquivo de configuração
   * @param filepath Caminho para o arquivo JSON
   * @return true se carregado com sucesso
   */
  bool loadConfig(const std::string& filepath);
  
  /**
   * @brief Obtém valor de configuração
   * @param key Chave no formato "section.subsection.key"
   * @param defaultValue Valor padrão se não encontrado
   * @return Valor da configuração
   */
  template<typename T>
  T get(const std::string& key, const T& defaultValue) const;
  
  /**
   * @brief Verifica se chave existe
   * @param key Chave no formato "section.subsection.key"
   * @return true se existe
   */
  bool hasKey(const std::string& key) const;
  
  /**
   * @brief Recarrega configuração do disco
   * @return true se recarregado com sucesso
   */
  bool reload();
  
  /**
   * @brief Obtém objeto JSON completo
   * @return Referência para o JSON
   */
  const nlohmann::json& getJson() const;
  
  /**
   * @brief Valida configuração contra schema
   * @param schema Schema JSON para validação
   * @return true se válido
   */
  bool validate(const nlohmann::json& schema) const;

 private:
  ConfigManager() = default;
  ~ConfigManager() = default;
  ConfigManager(const ConfigManager&) = delete;
  ConfigManager& operator=(const ConfigManager&) = delete;
  
  nlohmann::json config_;
  std::string filepath_;
  mutable std::mutex mutex_;
  
  nlohmann::json navigateToKey(const std::string& key) const;
};

// Template implementation
template<typename T>
T ConfigManager::get(const std::string& key, const T& defaultValue) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    auto value = navigateToKey(key);
    if (value.is_null()) {
      return defaultValue;
    }
    return value.get<T>();
  } catch (const std::exception&) {
    return defaultValue;
  }
}

}  // namespace Core
}  // namespace Umbra

