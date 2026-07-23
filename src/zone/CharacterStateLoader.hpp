#pragma once

#include "database/MySQLConnector.hpp"
#include "zone/NpcManager.hpp"
#include "SkillTypes.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>

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
  ~CharacterStateLoader();

  /** Carrega o estado completo do jogador. Retorna false se o jogador não existir. */
  bool loadPlayerState(uint32_t playerId, Combat::CharacterState& out);

  /**
   * Hot path do worker de combate: lê SÓ do cache; em miss, agenda um warm
   * assíncrono (thread dedicada) e retorna false SEM ir ao MySQL. Assim um
   * cache miss nunca bloqueia a fila de combate com a carga pesada (6 queries).
   * O estado é aquecido no join, então miss aqui é raro (o cast perdido pode ser
   * repetido pelo cliente e o próximo já encontra o cache quente).
   */
  bool getCachedOrWarm(uint32_t playerId, Combat::CharacterState& out);

  /** Enfileira um warm assíncrono do estado do jogador (idempotente). */
  void requestWarm(uint32_t playerId);

  /** Invalida o cache de um jogador (ex.: troca de equipamento). */
  void invalidate(uint32_t playerId);

  /** Atualiza só o vital no cache (sem invalidate/reload). No-op se não houver entrada. */
  void patchCachedMana(uint32_t playerId, int32_t newMana);
  void patchCachedHealth(uint32_t playerId, int32_t newHealth);

  /** Lê estado do cache sem ir ao DB (ignora TTL). Retorna false se não houver entrada. */
  bool tryGetCachedState(uint32_t playerId, Combat::CharacterState& out) const;

  /** Constrói um defensor a partir de uma instância de NPC (stats do template). */
  static Combat::CharacterState makeNpcDefenderState(const NpcRuntimeInstance& inst);

private:
  bool loadPlayerStateFromDb(uint32_t playerId, Combat::CharacterState& out);
  void warmLoop();

  std::shared_ptr<Database::MySQLConnector> db_;

  // Thread de warm assíncrono: carrega o estado pesado fora do worker de combate.
  std::thread warmThread_;
  std::atomic<bool> warmRunning_{false};
  std::mutex warmMu_;
  std::condition_variable warmCv_;
  std::deque<uint32_t> warmQueue_;
  std::unordered_set<uint32_t> warmPending_;

  struct CachedState {
    Combat::CharacterState state;
    std::chrono::steady_clock::time_point expiresAt;
  };
  mutable std::mutex cacheMu_;
  std::unordered_map<uint32_t, CachedState> cache_;
  // TTL alto: o cache é invalidado em TODA mudança de stat (dano, mana, buff/debuff,
  // regen, equipamento, level up), então stats ficam corretos entre eventos sem ir ao
  // MySQL remoto a cada cast. Isto elimina o represamento da fila de combate (lag).
  static constexpr int kCacheTtlMs = 10000;
};

}  // namespace Zone
}  // namespace Umbra
