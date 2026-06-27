#pragma once

#include "zone/MovementProtocol.hpp"
#include "zone/NpcManager.hpp"
#include "zone/CharacterStateLoader.hpp"
#include "zone/ReactionEngine.hpp"
#include "SkillService.hpp"
#include "CombatCalculator.hpp"
#include "database/MySQLConnector.hpp"
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace Umbra {
namespace Zone {

class MovementServer;

/** DOT/HOT ativo sobre uma instancia de NPC (in-memory; NPC nao usa active_dots). */
struct NpcDotInstance {
  uint32_t npcInstanceId = 0;
  uint32_t sourcePlayerId = 0;
  uint32_t skillId = 0;
  uint64_t dotBuffId = 0;
  int32_t tickValue = 0;   // negativo = dano, positivo = cura
  uint32_t intervalMs = 1000;
  uint8_t ticksRemaining = 0;
  int64_t expiresAtMs = 0;
  uint32_t durationMs = 0;
  std::chrono::steady_clock::time_point nextTickAt{};
};

/** Buff/debuff de skill em NPC (in-memory; NPC nao usa active_buffs MySQL). */
struct NpcBuffInstance {
  uint32_t npcInstanceId = 0;
  uint64_t buffId = 0;
  uint32_t sourcePlayerId = 0;
  uint32_t skillId = 0;
  uint8_t buffType = 0;
  uint8_t stacks = 1;
  int32_t valueFlat = 0;
  int16_t valuePercent = 0;
  int64_t expiresAtMs = 0;
  uint32_t durationMs = 0;
  std::string targetStat;
  std::string skillName;
  std::string iconPath;
};

class CombatCoreEngine {
public:
  CombatCoreEngine() = default;

  bool initialize(uint32_t zoneId,
                    std::shared_ptr<Database::MySQLConnector> db,
                    MovementServer* movementServer);

  void tick(float deltaSeconds);
  /** Regeneracao passiva de HP/MP dos jogadores online (tick lento). */
  void tickRegen(float deltaSeconds);
  /** Expira buffs de skill (active_buffs) e invalida cache de stats. */
  void tickBuffExpirations();
  void sendNpcSnapshotToClient(uint32_t clientId);
  /** Reenvia buffs/DOTs ativos de NPCs (opcode 104) a um cliente que acabou de conectar. */
  void sendNpcBuffSnapshotToClient(uint32_t clientId);
  /** Reenvia buffs/DOTs de jogadores online (opcode 104) a um cliente que acabou de conectar. */
  void sendPlayerBuffSnapshotToClient(uint32_t clientId);
  /** Reenvia buffs/DOTs de um NPC (ou todos se npcInstanceId==0) via opcode 104. */
  void sendNpcBuffSnapshotForNpc(uint32_t clientId, uint32_t npcInstanceId);

  /** Hot spawn: carrega instância do DB e broadcast opcode 100 a todos os clientes. */
  bool spawnNpcInstance(uint32_t npcInstanceId);
  /** Sincroniza instâncias novas do DB e broadcast spawn para cada uma adicionada. */
  size_t reloadMissingInstancesFromDatabase();

  void broadcastNpcSpawnToAll(const NpcRuntimeInstance& inst);
  void broadcastNpcDespawnToAll(uint32_t npcInstanceId, uint8_t reason = 0);

  const NpcManager* getNpcManager() const { return npcManager_.get(); }
  CharacterStateLoader* getCharacterStateLoader() { return stateLoader_.get(); }

  void processSkillCast(uint32_t sourcePlayerId, const SkillCastPayload& payload);
  void processBasicAttack(uint32_t sourcePlayerId, const BasicAttackPayload& payload);

  /** Usado por ReactionEngine para opcode 104. */
  void broadcastSkillBuffSyncPublic(const SkillBuffSyncPayload& payload);
  void applyReactionBuff(uint32_t targetPlayerId, uint32_t sourcePlayerId, uint32_t skillId,
                         const Combat::SkillEffect& effect);
  void applyReactionCounterDamage(uint32_t ownerPlayerId, uint32_t targetPlayerId, uint32_t skillId,
                                  const Combat::SkillEffect& effect, int32_t fixedDamage = 0);
  void applyDirectPlayerDamage(uint32_t sourcePlayerId, uint32_t targetPlayerId, int32_t damage,
                               uint8_t reason);
  /** Preenche skillName/iconPath em payloads 104 incompletos (ex.: expiração). */
  void enrichSkillBuffSyncPayload(SkillBuffSyncPayload& payload);

  /** Recarrega reações armadas quando jogador entra na zone. */
  void onPlayerJoinedZone(uint32_t playerId);

  void setResolvePartyMembersCallback(std::function<std::vector<uint32_t>(uint32_t playerId)> cb);

  /** Multiplicador de velocidade (100 = base) para validação de movimento. */
  float getPlayerMovementSpeedPercent(uint32_t playerId) const;

private:
  bool skillHasEffectType(const Combat::SkillData& skill, Combat::EffectType type) const;
  void armReactionSkill(uint32_t sourcePlayerId, const Combat::SkillData& skill);
  int32_t computeInstantHealDelta(const Combat::SkillData& skill, uint8_t rank,
                                  const Combat::CharacterState& sourceState,
                                  const Combat::CharacterState& healTarget, bool haveSource) const;
  bool tryGetPlayerPosition(uint32_t playerId, float& outX, float& outY, float& outZ) const;
  bool tryGetTargetPosition(uint8_t targetType, uint32_t targetId, float& outX, float& outY,
                            float& outZ) const;
  bool validateSkillRange(uint32_t sourcePlayerId, const Combat::SkillData& skill,
                          const SkillCastPayload& payload,
                          SkillCastRejectReason* outFailReason = nullptr) const;
  void sendSkillCastRejected(uint32_t playerId, uint32_t skillId, SkillCastRejectReason reason,
                             const std::string& message = {});
  bool validateBasicAttackRange(uint32_t sourcePlayerId, uint8_t targetType, uint32_t targetId,
                                uint16_t rangeMax) const;

  bool loadBasicAttacks();
  bool loadPlayerClassId(uint32_t playerId, uint32_t& outClassId);
  uint8_t loadSkillRank(uint32_t playerId, uint32_t skillId);
  bool applyPlayerDamage(uint32_t sourcePlayerId, uint32_t targetPlayerId, int32_t delta, uint8_t reason,
                         bool isCrit = false, bool isDouble = false);
  void deductPlayerMana(uint32_t playerId, int32_t cost);
  /** Le vitals atuais do DB e envia opcode 87 (sem floating text) ao jogador e AOI. */
  void broadcastPlayerVitals(uint32_t playerId);

  /** Aplica os efeitos DOT/HOT de uma skill no alvo (player via active_dots, NPC in-memory). */
  void applySkillEffects(uint32_t sourcePlayerId, uint8_t targetType, uint32_t targetId,
                         const Combat::SkillData& skill, const Combat::CharacterState& attacker,
                         bool haveAttacker);
  void insertPlayerDot(uint32_t sourcePlayerId, uint32_t targetPlayerId, uint32_t skillId,
                       const char* dotType, int32_t tickValue, uint32_t tickIntervalMs,
                       uint32_t ticksTotal, const Combat::SkillData& skill);
  void tickNpcDots();
  void tickNpcBuffExpirations();
  uint64_t applyNpcSkillBuff(uint32_t npcInstanceId, uint32_t sourcePlayerId, uint32_t skillId,
                             uint8_t buffType, const Combat::SkillEffect& eff,
                             const Combat::SkillData& skill);
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
  void broadcastSkillBuffSync(const SkillBuffSyncPayload& payload);
  /** Opcode 104 action=0 para jogador: targetType=0, enrich e broadcast. */
  void broadcastPlayerSkillBuffApply(SkillBuffSyncPayload& sync);
  void loadSkillAnimPaths(uint32_t skillId, std::string& anim, std::string& vfx, std::string& sfx);
  void handleNpcDamageResult(uint32_t npcInstanceId, int32_t applied, bool npcDied);
  int32_t computeDoubleBonus(const Combat::CharacterState& attacker,
                             const Combat::CharacterState& defender, bool haveAttacker,
                             int32_t firstHitAbs) const;

  std::function<std::vector<uint32_t>(uint32_t)> resolvePartyMembers_;

  uint32_t zoneId_ = 1;
  std::shared_ptr<Database::MySQLConnector> db_;
  MovementServer* movementServer_ = nullptr;
  std::unique_ptr<Combat::SkillService> skillService_;
  std::unique_ptr<NpcManager> npcManager_;
  std::unique_ptr<CharacterStateLoader> stateLoader_;
  std::unique_ptr<ReactionEngine> reactionEngine_;
  std::unordered_map<uint32_t, Zone::BasicAttackDef> basicAttacksByClass_;
  std::mutex basicCdMu_;
  std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> basicAttackReadyAt_;
  float respawnTickAccum_ = 0.f;
  float regenTickAccum_ = 0.f;

  std::mutex npcDotsMu_;
  std::vector<NpcDotInstance> npcDots_;

  std::mutex npcBuffsMu_;
  std::vector<NpcBuffInstance> npcBuffs_;
  std::atomic<uint32_t> npcBuffIdSeq_{1};
  std::atomic<uint32_t> npcDotIdSeq_{1};

  /** Evita reentrância ReactionEngine::mu_ quando dano vem de contra-ataque/reação. */
  bool inReactionDispatch_ = false;
};

}  // namespace Zone
}  // namespace Umbra
