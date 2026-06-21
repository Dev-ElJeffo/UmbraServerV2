#pragma once

#include "zone/MovementProtocol.hpp"
#include "zone/NpcManager.hpp"
#include "zone/CharacterStateLoader.hpp"
#include "SkillService.hpp"
#include "CombatCalculator.hpp"
#include "database/MySQLConnector.hpp"
#include <chrono>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace Umbra {
namespace Zone {

class MovementServer;

class CombatCoreEngine {
public:
  CombatCoreEngine() = default;

  bool initialize(uint32_t zoneId,
                    std::shared_ptr<Database::MySQLConnector> db,
                    MovementServer* movementServer);

  void tick(float deltaSeconds);
  void sendNpcSnapshotToClient(uint32_t clientId);

  /** Hot spawn: carrega instância do DB e broadcast opcode 100 a todos os clientes. */
  bool spawnNpcInstance(uint32_t npcInstanceId);
  /** Sincroniza instâncias novas do DB e broadcast spawn para cada uma adicionada. */
  size_t reloadMissingInstancesFromDatabase();

  void broadcastNpcSpawnToAll(const NpcRuntimeInstance& inst);
  void broadcastNpcDespawnToAll(uint32_t npcInstanceId, uint8_t reason = 0);

  const NpcManager* getNpcManager() const { return npcManager_.get(); }

  void processSkillCast(uint32_t sourcePlayerId, const SkillCastPayload& payload);
  void processBasicAttack(uint32_t sourcePlayerId, const BasicAttackPayload& payload);

private:
  bool loadBasicAttacks();
  bool loadPlayerClassId(uint32_t playerId, uint32_t& outClassId);
  uint8_t loadSkillRank(uint32_t playerId, uint32_t skillId);
  bool applyPlayerDamage(uint32_t sourcePlayerId, uint32_t targetPlayerId, int32_t delta, uint8_t reason,
                         bool isCrit = false);
  void deductPlayerMana(uint32_t playerId, int32_t cost);
  void writeCombatLog(uint32_t sourcePlayerId, uint32_t targetPlayerId, uint32_t skillId,
                      const char* actionType, int32_t value, bool isCrit, int32_t overkill);
  /** Resolve o defensor a partir do alvo. Retorna false se inválido/morto. */
  bool buildDefenderState(uint8_t targetType, uint32_t targetId, Combat::CharacterState& out,
                          bool& outIsPlayer);
  /** Cooldown server-side do ataque básico por jogador. */
  bool checkAndStampBasicCooldown(uint32_t playerId, uint32_t cooldownMs);
  /** Broadcast de "Miss" (dano 0) para alvo NPC ou player. */
  void broadcastMiss(uint8_t targetType, uint32_t targetId, uint32_t sourcePlayerId);
  void broadcastSkillCast(const SkillCastBroadcastPayload& payload);
  void broadcastBasicAttack(const BasicAttackBroadcastPayload& payload);
  void broadcastNpcCombatEvent(const NpcCombatEventPayload& payload);
  void broadcastNpcState(const NpcStatePayload& payload);
  void loadSkillAnimPaths(uint32_t skillId, std::string& anim, std::string& vfx, std::string& sfx);
  void handleNpcDamageResult(uint32_t npcInstanceId, int32_t applied, bool npcDied);

  uint32_t zoneId_ = 1;
  std::shared_ptr<Database::MySQLConnector> db_;
  MovementServer* movementServer_ = nullptr;
  std::unique_ptr<Combat::SkillService> skillService_;
  std::unique_ptr<NpcManager> npcManager_;
  std::unique_ptr<CharacterStateLoader> stateLoader_;
  std::unordered_map<uint32_t, Zone::BasicAttackDef> basicAttacksByClass_;
  std::mutex basicCdMu_;
  std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> basicAttackReadyAt_;
  float respawnTickAccum_ = 0.f;
};

}  // namespace Zone
}  // namespace Umbra
