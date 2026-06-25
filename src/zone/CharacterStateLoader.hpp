#pragma once

#include "database/MySQLConnector.hpp"
#include "zone/NpcManager.hpp"
#include "SkillTypes.hpp"
#include <chrono>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace Umbra {
namespace Zone {

/**
 * Carrega Combat::CharacterState (stats completos) a partir do MySQL, espelhando a
 * lógica de www/umbra_api/helpers/character_info_helper.php (classe + level + atributos
 * + equipamento + buffs de poção). É a fonte de stats autoritativa para o cálculo de
 * dano no zone server (CombatCalculator).
 *
 * Possui cache leve (TTL curto) por playerId para evitar query a cada hit em spam de
 * ataque básico. O HP/mana atuais ficam no estado cacheado e podem estar levemente
 * defasados; a aplicação de dano real (applyPlayerDamage) relê o HP do DB.
 */
class CharacterStateLoader {
public:
  explicit CharacterStateLoader(std::shared_ptr<Database::MySQLConnector> db);

  /** Carrega o estado completo do jogador. Retorna false se o jogador não existir. */
  bool loadPlayerState(uint32_t playerId, Combat::CharacterState& out);

  /** Invalida o cache de um jogador (ex.: troca de equipamento). */
  void invalidate(uint32_t playerId);

  /** Constrói um defensor a partir de uma instância de NPC (stats do template). */
  static Combat::CharacterState makeNpcDefenderState(const NpcRuntimeInstance& inst);

private:
  bool loadPlayerStateFromDb(uint32_t playerId, Combat::CharacterState& out);

  std::shared_ptr<Database::MySQLConnector> db_;

  struct CachedState {
    Combat::CharacterState state;
    std::chrono::steady_clock::time_point expiresAt;
  };
  std::mutex cacheMu_;
  std::unordered_map<uint32_t, CachedState> cache_;
  static constexpr int kCacheTtlMs = 1000;
};

}  // namespace Zone
}  // namespace Umbra
