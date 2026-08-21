#pragma once

#include "zone/MovementProtocol.hpp"
#include "zone/NpcManager.hpp"
#include "zone/NpcAiSystem.hpp"
#include "zone/CharacterStateLoader.hpp"
#include "zone/ReactionEngine.hpp"
#include "zone/QuestProgressService.hpp"
#include "zone/LootService.hpp"
#include "SkillService.hpp"
#include "CombatCalculator.hpp"
#include "database/MySQLConnector.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
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

/** DOT/HOT em jogador (in-memory). Evita poll MySQL active_dots no tick da zone (spikes 100–300ms). */
struct PlayerDotInstance {
  uint32_t targetPlayerId = 0;
  uint32_t sourcePlayerId = 0;
  uint32_t skillId = 0;
  uint64_t dotBuffId = 0;
  int32_t tickValue = 0;  // sempre positivo; isHeal decide o sinal
  bool isHeal = false;
  uint32_t intervalMs = 1000;
  uint8_t ticksRemaining = 0;
  int64_t expiresAtMs = 0;
  uint32_t durationMs = 0;
  std::chrono::steady_clock::time_point nextTickAt{};
};

/** Buff/debuff de skill em jogador (in-memory). Evita SELECT/INSERT sync em active_buffs
 *  no combat worker (media 1–3s com expire concorrente → fila → skills em cascata). */
struct PlayerBuffInstance {
  uint32_t targetPlayerId = 0;
  uint32_t sourcePlayerId = 0;
  uint32_t skillId = 0;
  uint64_t buffId = 0;
  uint8_t buffType = 0;
  uint8_t stacks = 1;
  int32_t valueFlat = 0;
  int16_t valuePercent = 0;
  int64_t expiresAtMs = 0;
  uint32_t durationMs = 0;
  std::string targetStat;
  std::string skillName;
  std::string iconPath;
  std::string effectTypeStr;
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
  std::string effectTypeStr;
};

class CombatCoreEngine {
public:
  CombatCoreEngine() = default;
  ~CombatCoreEngine();

  bool initialize(uint32_t zoneId,
                    std::shared_ptr<Database::MySQLConnector> db,
                    MovementServer* movementServer);
  /** Para o worker de combate e drena a fila (idempotente). */
  void shutdown();

  void tick(float deltaSeconds);
  /** Aplica hits adiados por castTimeMs (animation cancel cancela o pendente). */
  void tickPendingSkillHits();
  /** Regeneracao passiva de HP/MP dos jogadores online (tick lento). */
  void tickRegen(float deltaSeconds);
  /** Expira buffs de skill em memória (sem poll MySQL active_buffs). */
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
  bool despawnNpcInstance(uint32_t npcInstanceId, uint8_t reason = 0);
  /** Move NPC em runtime e rebroadcast spawn (opcode 100). Não grava MySQL (Manager/PHP). */
  bool moveNpcInstance(uint32_t npcInstanceId, float x, float y, float z, float yaw);
  /** Sincroniza instâncias novas do DB e broadcast spawn para cada uma adicionada. */
  size_t reloadMissingInstancesFromDatabase();
  /** Despawn todos → reload MySQL → spawn todos (corrige pos/home/roam em runtime). */
  size_t reloadAllNpcInstancesFromDatabase();
  /** Recarrega definitions de skills (+ skill_rank_scaling) do MySQL em runtime. */
  bool reloadSkills();

  void broadcastNpcSpawnToAll(const NpcRuntimeInstance& inst);
  void broadcastNpcDespawnToAll(uint32_t npcInstanceId, uint8_t reason = 0);

  const NpcManager* getNpcManager() const { return npcManager_.get(); }
  CharacterStateLoader* getCharacterStateLoader() { return stateLoader_.get(); }
  QuestProgressService* getQuestProgressService() { return questProgressService_.get(); }

  void processSkillCast(uint32_t sourcePlayerId, const SkillCastPayload& payload);
  /** Aplica só a resolução de hit (após castTimeMs). Não revalida CD/mana nem rebroadcast 97. */
  void finalizeSkillCastHit(uint32_t sourcePlayerId, const SkillCastPayload& payload);
  void processBasicAttack(uint32_t sourcePlayerId, const BasicAttackPayload& payload);
  /** Basic attack autoritativo do mob (NPC → player). */
  void processNpcBasicAttack(uint32_t npcInstanceId, uint32_t targetPlayerId);
  /** Broadcast opcode 102 (HP + posição) — usado pelo NpcAiSystem. */
  void broadcastNpcStatePublic(const NpcStatePayload& payload) { broadcastNpcState(payload); }
  void cancelPendingSkillHit(uint32_t sourcePlayerId);
  /** Aplica AGORA todos os hits pendentes do jogador (animation cancel de combo).
   *  Mana/CD já foram cobrados no cast; commitar evita perder dano do cast anterior. */
  void commitPendingSkillHitsNow(uint32_t sourcePlayerId);
  void scheduleSkillHit(uint32_t sourcePlayerId, const SkillCastPayload& payload, uint32_t delayMs);

  /** Enfileira combate e retorna imediato (não bloqueia a thread de leitura WS). */
  void enqueueSkillCast(uint32_t sourcePlayerId, const SkillCastPayload& payload);
  void enqueueBasicAttack(uint32_t sourcePlayerId, const BasicAttackPayload& payload);

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
  /** Write-behind SQL (ReactionEngine / hot path sem bloquear no MySQL). */
  void enqueueDbWritePublic(std::string sql) { enqueueDbWrite(std::move(sql)); }

  /** Recarrega reações armadas quando jogador entra na zone. */
  void onPlayerJoinedZone(uint32_t playerId);
  /** Equip/unequip: recarrega stats do MySQL preservando HP/MP em memória. */
  void onPlayerEquipmentOrStatsChanged(uint32_t playerId);
  /** Se o jogador entrou já morto no DB/cache, força isDead + notify (sem reload de reações). */
  void syncJoinDeathState(uint32_t playerId);

  void setResolvePartyMembersCallback(std::function<std::vector<uint32_t>(uint32_t playerId)> cb);
  void setLootService(LootService* loot) { lootService_ = loot; }

  /** Multiplicador de velocidade (100 = base) para validação de movimento. */
  float getPlayerMovementSpeedPercent(uint32_t playerId) const;

private:
  bool skillHasEffectType(const Combat::SkillData& skill, Combat::EffectType type,
                          uint8_t skillRank = 1) const;
  void armReactionSkill(uint32_t sourcePlayerId, const Combat::SkillData& skill);
  int32_t computeInstantHealDelta(const Combat::SkillData& skill, uint8_t rank,
                                  const Combat::CharacterState& sourceState,
                                  const Combat::CharacterState& healTarget, bool haveSource) const;
  bool tryGetPlayerPosition(uint32_t playerId, float& outX, float& outY, float& outZ) const;
  bool tryGetTargetPosition(uint8_t targetType, uint32_t targetId, float& outX, float& outY,
                            float& outZ) const;
  /**
   * Corrige targetType quando player_id e npc_instance_id colidem.
   * Se existir NPC vivo com o mesmo id e o caster estiver mais perto dele (ou o
   * "player" for stub em 0,0,0), reinterpreta como Npc.
   */
  void disambiguateTargetType(uint32_t sourcePlayerId, uint8_t& targetType, uint32_t targetId) const;
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
  void deductPlayerMana(uint32_t playerId, int32_t cost, int32_t knownCurrentHealth = -1,
                        int32_t knownCurrentMana = -1);
  /** Emite opcode 87. Caminho quente: usa valores conhecidos/cache (0 SELECT). */
  void broadcastPlayerVitals(uint32_t playerId, int32_t knownCurrentHealth = -1,
                             int32_t knownCurrentMana = -1);

  /** Aplica os efeitos DOT/HOT de uma skill no alvo (player via active_dots, NPC in-memory). */
  void applySkillEffects(uint32_t sourcePlayerId, uint8_t targetType, uint32_t targetId,
                         const Combat::SkillData& skill, const Combat::CharacterState& attacker,
                         bool haveAttacker);
  void insertPlayerDot(uint32_t sourcePlayerId, uint32_t targetPlayerId, uint32_t skillId,
                       const char* dotType, int32_t tickValue, uint32_t tickIntervalMs,
                       uint32_t ticksTotal, const Combat::SkillData& skill);
  /** Aplica buff/debuff de player em memória + persistência write-behind (0 MySQL sync). */
  uint64_t applyPlayerBuffInMemory(uint32_t targetPlayerId, uint32_t sourcePlayerId,
                                   uint32_t skillId, const Combat::SkillEffect& effect,
                                   const Combat::SkillData& skill, uint8_t buffTypeCode);
  void tickNpcDots();
  void tickPlayerDots();
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
  void preloadSkillAnimPaths();
  void loadSkillAnimPaths(uint32_t skillId, std::string& anim, std::string& vfx, std::string& sfx);
  void handleNpcDamageResult(uint32_t npcInstanceId, int32_t applied, bool npcDied,
                             uint32_t killerPlayerId = 0);
  int32_t computeDoubleBonus(const Combat::CharacterState& attacker,
                             const Combat::CharacterState& defender, bool haveAttacker,
                             int32_t firstHitAbs) const;

  std::function<std::vector<uint32_t>(uint32_t)> resolvePartyMembers_;

  uint32_t zoneId_ = 1;
  std::shared_ptr<Database::MySQLConnector> db_;
  MovementServer* movementServer_ = nullptr;
  std::unique_ptr<Combat::SkillService> skillService_;
  std::unique_ptr<NpcManager> npcManager_;
  std::unique_ptr<NpcAiSystem> npcAi_;
  std::unique_ptr<CharacterStateLoader> stateLoader_;
  std::unique_ptr<ReactionEngine> reactionEngine_;
  std::unique_ptr<QuestProgressService> questProgressService_;
  LootService* lootService_ = nullptr;
  std::unordered_map<uint32_t, Zone::BasicAttackDef> basicAttacksByClass_;
  std::mutex basicCdMu_;
  std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> basicAttackReadyAt_;
  float respawnTickAccum_ = 0.f;
  float regenTickAccum_ = 0.f;

  /** Hit adiado: castTimeMs > 0 agenda apply; novo cast do mesmo player cancela (animation cancel). */
  struct PendingSkillHit {
    uint32_t sourcePlayerId = 0;
    SkillCastPayload payload{};
    int64_t executeAtMs = 0;
  };
  std::mutex pendingSkillHitsMu_;
  std::vector<PendingSkillHit> pendingSkillHits_;

  struct SkillAnimPaths {
    std::string anim;
    std::string vfx;
    std::string sfx;
  };
  std::mutex skillAnimCacheMu_;
  std::unordered_map<uint32_t, SkillAnimPaths> skillAnimCache_;

  std::mutex npcDotsMu_;
  std::vector<NpcDotInstance> npcDots_;

  std::mutex playerDotsMu_;
  std::vector<PlayerDotInstance> playerDots_;
  std::atomic<uint32_t> playerDotIdSeq_{1};

  std::mutex playerBuffsMu_;
  std::vector<PlayerBuffInstance> playerBuffs_;
  std::atomic<uint32_t> playerBuffIdSeq_{1};

  std::mutex npcBuffsMu_;
  std::vector<NpcBuffInstance> npcBuffs_;
  std::atomic<uint32_t> npcBuffIdSeq_{1};
  std::atomic<uint32_t> npcDotIdSeq_{1};

  /** Evita reentrância ReactionEngine::mu_ quando dano vem de contra-ataque/reação. */
  bool inReactionDispatch_ = false;

  enum class CombatJobKind : uint8_t { SkillCast = 0, BasicAttack = 1 };
  struct CombatJob {
    CombatJobKind kind = CombatJobKind::SkillCast;
    uint32_t sourcePlayerId = 0;
    SkillCastPayload skill{};
    BasicAttackPayload basic{};
    /** steady_clock ms no enqueue — mede espera na fila (H-BURST). */
    int64_t enqueuedAtMs = 0;
  };
  void combatWorkerLoop();
  void enqueueJob(CombatJob job);
  static uint64_t skillCastKey(uint32_t playerId, uint32_t skillId) {
    return (static_cast<uint64_t>(playerId) << 32) | static_cast<uint64_t>(skillId);
  }

  static constexpr size_t kCombatQueueCap = 512;
  std::mutex combatQueueMu_;
  std::condition_variable combatQueueCv_;
  std::deque<CombatJob> combatQueue_;
  /** Skills em processamento (fora da fila) — evita spam re-enfileirar sem carimbar CD cedo. */
  std::unordered_set<uint64_t> skillCastInFlight_;
  std::thread combatWorker_;
  std::atomic<bool> workerRunning_{false};

  // ---- Escrita assíncrona (write-behind) ----
  // Tira os UPDATE/INSERT do hotpath de combate: o worker enfileira SQL (com valores
  // numéricos já embutidos, injection-safe) e uma thread dedicada grava no MySQL remoto.
  // O cache em memória é atualizado de forma síncrona, então cliente/cálculo veem o valor
  // correto imediatamente; o DB apenas persiste com pequeno atraso.
  void enqueueDbWrite(std::string sql);
  // Igual ao enqueueDbWrite, mas insere na FRENTE da fila. Usado na morte: o
  // is_dead precisa persistir antes do backlog de updates de HP para o respawn/
  // relog não ler estado defasado, sem bloquear o combat worker com write síncrono.
  void enqueueDbWriteFront(std::string sql);
  void dbWriterLoop();
  static constexpr size_t kDbWriteQueueCap = 4096;
  std::mutex dbWriteMu_;
  std::condition_variable dbWriteCv_;
  std::deque<std::string> dbWriteQueue_;
  std::thread dbWriter_;
  std::atomic<bool> dbWriterRunning_{false};

  // ---- Cache de rank de skill (evita 1 SELECT por cast) ----
  // Rank raramente muda; TTL curto o suficiente para refletir rank up sem ir ao DB por cast.
  std::mutex rankCacheMu_;
  struct CachedRank {
    uint8_t rank = 1;
    std::chrono::steady_clock::time_point expiresAt{};
  };
  std::unordered_map<uint64_t, CachedRank> rankCache_;
  static constexpr int kRankCacheTtlMs = 30000;
};

}  // namespace Zone
}  // namespace Umbra
