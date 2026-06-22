#pragma once

#include "SkillTypes.hpp"
#include "zone/MovementProtocol.hpp"
#include "database/MySQLConnector.hpp"

#include <nlohmann/json.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace Umbra {
namespace Zone {

class CombatCoreEngine;

enum class ReactionTrigger : uint8_t {
  None = 0,
  OnCritReceived,
  OnDodge,
  OnAttackReceived,
  AllyDamaged
};

struct ArmedReaction {
  uint64_t buffId = 0;
  uint32_t ownerPlayerId = 0;
  uint32_t skillId = 0;
  Combat::SkillEffect effect;
  ReactionTrigger trigger = ReactionTrigger::None;
  int32_t usesRemaining = 1;
  bool redirectToSelf = false;
  uint32_t areaRadius = 0;
};

/** Gerencia skills REACTION armadas e dispara efeitos em eventos de combate. */
class ReactionEngine {
public:
  ReactionEngine() = default;

  void setDatabase(std::shared_ptr<Database::MySQLConnector> db) { db_ = std::move(db); }
  void setCombatEngine(CombatCoreEngine* engine) { combatEngine_ = engine; }

  /** Arma reação no cast; persiste em active_buffs (AURA + reaction_armed). */
  uint64_t armReaction(uint32_t ownerPlayerId, uint32_t sourcePlayerId, uint32_t skillId,
                       const Combat::SkillData& skill, const Combat::SkillEffect& effect);

  void reloadArmedForPlayer(uint32_t playerId);

  /** Retorna true se o hit deve virar miss (Reflexos Mortais). */
  bool onPlayerHitReceived(uint32_t targetPlayerId, uint32_t sourcePlayerId);

  void onPlayerDamaged(uint32_t targetPlayerId, uint32_t sourcePlayerId, int32_t delta, bool isCrit);

  void onPlayerDodge(uint32_t targetPlayerId, uint32_t sourcePlayerId);

  /** Modifica delta se aliado protegido (Interposição). */
  void onAllyDamaged(uint32_t allyPlayerId, uint32_t sourcePlayerId, int32_t& delta);

private:
  static ReactionTrigger parseTrigger(const nlohmann::json& conditions);
  void disarm(uint32_t ownerPlayerId, ArmedReaction& reaction, bool consumeUse);
  void executeReaction(uint32_t ownerPlayerId, uint32_t triggerSourceId, ArmedReaction& reaction);
  void broadcastDisarm(const ArmedReaction& reaction);
  std::vector<ArmedReaction>* findArmedList(uint32_t playerId);

  std::shared_ptr<Database::MySQLConnector> db_;
  CombatCoreEngine* combatEngine_ = nullptr;
  std::mutex mu_;
  std::unordered_map<uint32_t, std::vector<ArmedReaction>> armedByPlayer_;
};

}  // namespace Zone
}  // namespace Umbra
