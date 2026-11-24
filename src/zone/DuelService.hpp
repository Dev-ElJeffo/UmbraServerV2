// DuelService.hpp — duelo 1x1 na zone (etapa 6, opcodes 50-54) e regras de PvP (pvp_settings).
#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Umbra {
namespace Database {
class MySQLConnector;
}
namespace Zone {

class MovementServer;
class CombatCoreEngine;

/** Linha única de pvp_settings (Manager → aba PvP). */
struct PvpSettings {
  bool openWorldPvp = false;
  float requestDistance = 1500.f;
  uint32_t requestTimeoutSec = 30;
  float radius = 3000.f;
  uint32_t countdownSec = 3;
  uint32_t maxDurationSec = 180;
  uint32_t outOfBoundsSec = 5;
  bool restoreOnEnd = false;
  std::string flagMeshPath;
  std::string flagVfxPath;
};

/** Relação de duelo entre dois jogadores (consultada pelo combate). */
enum class DuelRelation : uint8_t {
  None = 0,         // nenhum dos dois em duelo
  Fighting = 1,     // duelando entre si, contagem encerrada
  Countdown = 2,    // duelando entre si, ainda na contagem
  Interfering = 3,  // pelo menos um está em duelo com outra pessoa
};

/** Regras puras (testadas em tests/test_duel_rules.cpp). */
namespace DuelRules {

inline bool isOutOfBounds(float flagX, float flagY, float px, float py, float radius) {
  const float dx = px - flagX;
  const float dy = py - flagY;
  return dx * dx + dy * dy > radius * radius;
}

/** Dano letal entre duelistas: devolve o delta que deixa o alvo com 1 de HP e marca a derrota. */
inline std::pair<int32_t, bool> clampLethal(int32_t currentHealth, int32_t delta) {
  if (delta >= 0 || currentHealth + delta > 0) return {delta, false};
  return {currentHealth > 1 ? 1 - currentHealth : 0, true};
}

/**
 * Pode afetar o outro jogador? hostile = dano/debuff; false = cura/buff.
 * Futuro: a facção e as safezones entram aqui (mesma facção só em duelo).
 */
inline bool canAffect(DuelRelation rel, bool hostile, bool openWorldPvp) {
  switch (rel) {
    case DuelRelation::Fighting:
      return hostile;  // oponente não recebe cura/buff do outro
    case DuelRelation::Countdown:
    case DuelRelation::Interfering:
      return false;
    case DuelRelation::None:
    default:
      return hostile ? openWorldPvp : true;
  }
}

}  // namespace DuelRules

class DuelService {
 public:
  DuelService(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId, MovementServer* movement,
              CombatCoreEngine* combat);

  /** Lê pvp_settings (reload_pvp_settings). false = tabela ausente (usa os padrões). */
  bool reload();
  PvpSettings settings() const;
  bool openWorldPvp() const;

  /** 50/52 (inbound worker). */
  void handleRequest(uint32_t fromPlayerId, uint32_t targetPlayerId);
  void handleResponse(uint32_t playerId, uint8_t action, uint32_t otherPlayerId);

  /** Tick da zone (~4x/s): expiração de desafios, contagem, área, tempo, morte, desconexão, camada. */
  void tick();

  /** Combat worker. */
  DuelRelation relation(uint32_t a, uint32_t b) const;
  bool isFightingOpponent(uint32_t a, uint32_t b) const { return relation(a, b) == DuelRelation::Fighting; }
  /** Golpe letal entre duelistas (o HP já foi limitado a 1): encerra com vitória de quem bateu. */
  void onLethalHit(uint32_t loserId, uint32_t winnerId);
  /**
   * Duelo entre os dois terminou há pouco (kPostDuelGraceMs): dano entre eles é descartado.
   * Cobre golpes já a caminho no fim (hit pendente, contra-ataque, reflexo) que matariam o perdedor.
   */
  bool isRecentlyEndedPair(uint32_t a, uint32_t b) const;
  static constexpr int64_t kPostDuelGraceMs = 5000;

  void onPlayerGone(uint32_t playerId);

 private:
  struct PendingRequest {
    uint32_t fromId = 0;
    uint32_t targetId = 0;
    std::chrono::steady_clock::time_point expiresAt;
  };
  struct ActiveDuel {
    uint32_t duelId = 0;
    uint32_t p1 = 0;  // quem desafiou
    uint32_t p2 = 0;
    uint32_t layer = 0;
    float flagX = 0.f, flagY = 0.f, flagZ = 0.f;
    std::chrono::steady_clock::time_point startedAt;  // aceite
    std::chrono::steady_clock::time_point fightAt;    // fim da contagem
    std::chrono::steady_clock::time_point oobSince1{};
    std::chrono::steady_clock::time_point oobSince2{};
    bool oob1 = false;
    bool oob2 = false;
  };
  struct EndAction {
    ActiveDuel duel;
    uint32_t winnerId = 0;
    uint32_t loserId = 0;
    uint8_t reason = 0;
    bool restore = false;  // pvp_settings.duel_restore_on_end no momento do fim
  };

  void sendNotice(uint32_t playerId, uint32_t otherId, uint8_t reason);
  /** Fora do lock: 54 para os dois, gravação, limpeza de efeitos e restauração. */
  void finish(const EndAction& end);
  /** Com mu_: tira o duelo do mapa e devolve o EndAction. */
  EndAction takeDuelLocked(uint32_t duelId, uint32_t winnerId, uint32_t loserId, uint8_t reason);
  const ActiveDuel* duelOfLocked(uint32_t playerId) const;
  bool busyLocked(uint32_t playerId) const;

  std::shared_ptr<Database::MySQLConnector> db_;
  uint32_t zoneId_ = 0;
  MovementServer* movement_ = nullptr;
  CombatCoreEngine* combat_ = nullptr;

  mutable std::mutex mu_;
  PvpSettings settings_;
  std::vector<PendingRequest> pending_;
  std::unordered_map<uint32_t, ActiveDuel> duels_;           // duelId -> duelo
  std::unordered_map<uint32_t, uint32_t> duelByPlayer_;      // playerId -> duelId
  struct EndedPair {
    uint32_t a = 0;
    uint32_t b = 0;
    std::chrono::steady_clock::time_point until;
  };
  std::vector<EndedPair> recentlyEnded_;  // podado no tick
  uint32_t nextDuelId_ = 1;
};

}  // namespace Zone
}  // namespace Umbra
