#pragma once

#include "Models.hpp"
#include "MySQLConnector.hpp"
#include <memory>
#include <optional>
#include <vector>

namespace Umbra {
namespace Database {

/**
 * @brief Data Access Object para personagens de jogadores
 */
class PlayerDAO {
 public:
  explicit PlayerDAO(std::shared_ptr<MySQLConnector> connector);
  
  /**
   * @brief Cria novo personagem
   * @param player Dados do personagem
   * @return ID do personagem criado ou 0 se falhou
   */
  uint64_t createPlayer(const Player& player);
  
  /**
   * @brief Busca personagem por ID
   * @param id ID do personagem
   * @return Player ou nullopt se não encontrado
   */
  std::optional<Player> getPlayerById(uint64_t id);
  
  /**
   * @brief Busca personagem por nome
   * @param characterName Nome do personagem
   * @return Player ou nullopt se não encontrado
   */
  std::optional<Player> getPlayerByName(const std::string& characterName);
  
  /**
   * @brief Lista todos os personagens de uma conta
   * @param accountId ID da conta
   * @return Vector de personagens
   */
  std::vector<Player> getPlayersByAccountId(uint64_t accountId);
  
  /**
   * @brief Atualiza dados do personagem
   * @param player Dados atualizados
   * @return true se atualizado com sucesso
   */
  bool updatePlayer(const Player& player);
  
  /**
   * @brief Atualiza posição do personagem
   * @param id ID do personagem
   * @param x Posição X
   * @param y Posição Y
   * @param z Posição Z
   * @param zone Zona atual
   * @return true se atualizado com sucesso
   */
  bool updatePosition(uint64_t id, float x, float y, float z, 
                      const std::string& zone);
  
  /**
   * @brief Atualiza stats do personagem
   * @param id ID do personagem
   * @param health HP atual
   * @param mana Mana atual
   * @param stamina Stamina atual
   * @return true se atualizado com sucesso
   */
  bool updateStats(uint64_t id, uint32_t health, uint32_t mana, uint32_t stamina);
  
  /**
   * @brief Adiciona experiência ao personagem
   * @param id ID do personagem
   * @param expAmount Quantidade de XP
   * @return true se atualizado com sucesso
   */
  bool addExperience(uint64_t id, uint64_t expAmount);
  
  /**
   * @brief Deleta personagem
   * @param id ID do personagem
   * @return true se deletado com sucesso
   */
  bool deletePlayer(uint64_t id);
  
  /**
   * @brief Verifica se nome de personagem já existe
   * @param characterName Nome do personagem
   * @return true se existe
   */
  bool characterNameExists(const std::string& characterName);
  
  /**
   * @brief Atualiza último tempo jogado
   * @param id ID do personagem
   * @return true se atualizado com sucesso
   */
  bool updateLastPlayed(uint64_t id);

 private:
  std::shared_ptr<MySQLConnector> connector_;
  
  Player resultToPlayer(const std::string& result);
};

}  // namespace Database
}  // namespace Umbra

