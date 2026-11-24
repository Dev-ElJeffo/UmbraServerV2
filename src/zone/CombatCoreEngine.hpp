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
  bool restoresMana = false;
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
  bool restoresMana = false;
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
  uint8_t effectOrder = 0;
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
  uint8_t effectOrder = 0;
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

  /** Spawn (100) para a camada do NPC (etapa 4: mundo aberto ou instância). */
  void broadcastNpcSpawnToAll(const NpcRuntimeInstance& inst);
  /** Despawn (101) para a camada do NPC. */
  void broadcastNpcDespawnToAll(uint32_t npcInstanceId, uint8_t reason = 0);

  // ---- Instâncias (etapa 4) ----
  /** Clona os NPCs do molde na camada e manda o spawn a quem estiver nela (no combat worker). */
  void enqueueSpawnInstanceNpcs(uint32_t instanceTemplateId, uint32_t instanceId, float originX, float originY,
                                float originZ, float hpMult = 1.f, float damageMult = 1.f);
  /** Etapa 5: atacáveis/chefes do molde (totais do progresso da dungeon). */
  NpcManager::MoldeRoleCounts instanceMoldeRoleCounts(uint32_t instanceTemplateId) const;
  /**
   * Etapa 5: NPC de cópia morreu (instanceId, npcId, dungeonRole, killerPlayerId). Roda no combat worker;
   * não chamar de volta o CombatCoreEngine sob lock.
   */
  void setInstanceNpcKilledHook(std::function<void(uint32_t, uint32_t, uint8_t, uint32_t)> hook) {
    instanceNpcKilledHook_ = std::move(hook);
  }
  /** Remove os clones da camada e manda o despawn (no combat worker). */
  void enqueueRemoveInstanceNpcs(uint32_t instanceId);
  /** O player trocou de camada: despawn dos NPCs da antiga e snapshot da nova, só para ele (no worker). */
  void enqueueNpcLayerSwitch(uint32_t playerId, uint32_t oldLayer, uint32_t newLayer);
  /** Etapa 5b: tarefa serializada no combat worker (mudança estrutural de NPCs, fases de chefe). */
  void enqueueWorldOp(std::function<void()> op);
  /** Etapa 5b: chefes com fases (dano/morte de NPC e snapshot do 140). */
  void setBossController(class BossController* boss) { bossController_ = boss; }
  /** Etapa 5b: chefe reiniciou (wipe/leash) → remove DOTs e debuffs dos jogadores nele e avisa os clientes. */
  void clearNpcHostileEffects(uint32_t npcInstanceId);
  /** Recarrega os NPCs dos moldes (reload_instances). */
  size_t reloadInstanceMoldes();
  /** Dados do NPC portal (false se não for portal). */
  bool getNpcPortalInfo(uint32_t npcInstanceId, NpcManager::PortalInfo& out) const;
  size_t instanceMoldeNpcCount(uint32_t instanceTemplateId) const;

  const NpcManager* getNpcManager() const { return npcManager_.get(); }
  /** Etapa 5b: BossController muta NPCs (sempre no combat worker ou sob o lock do NpcManager). */
  NpcManager* getNpcManagerMutable() { return npcManager_.get(); }
  CharacterStateLoader* getCharacterStateLoader() { return stateLoader_.get(); }
  QuestProgressService* getQuestProgressService() { return questProgressService_.get(); }

  void processSkillCast(uint32_t sourcePlayerId, const SkillCastPayload& payload);
  /** Aplica só a resolução de hit (após castTimeMs). Não revalida CD/mana nem rebroadcast 97. */
  void finalizeSkillCastHit(uint32_t sourcePlayerId, const SkillCastPayload& payload);
  void processBasicAttack(uint32_t sourcePlayerId, const BasicAttackPayload& payload);
  /** Basic attack autoritativo do mob (NPC → player). */
  void processNpcBasicAttack(uint32_t npcInstanceId, uint32_t targetPlayerId);
  void processNpcSkillCast(uint32_t npcInstanceId, uint32_t targetPlayerId, uint32_t npcSkillId);
  /** Hit da skill do NPC (imediato ou no fim do cast agendado). */
  void resolveNpcSkillCast(uint32_t npcInstanceId, uint32_t targetPlayerId, uint32_t npcSkillId,
                           uint8_t rank, bool fromPendingCast);
  /**
   * Resolve path de montage para cast de skill do NPC:
   * skill.cast_anim_path → anim_states_json.casts[] → skill → attacks[].
   * Não usa vfx_key (Niagara separado).
   */
  static std::string resolveNpcSkillCastAnimPath(NpcRuntimeInstance& inst,
                                                  const std::string& skillCastAnimOverride = "");
  /** Round-robin em attacks[] para basic do NPC. */
  static void pickNpcBasicAttackAnim(NpcRuntimeInstance& inst, std::string& outPath,
                                      uint8_t& outAnimIndex);
  /** Broadcast opcode 102 (HP + posição) — usado pelo NpcAiSystem. */
  void broadcastNpcStatePublic(const NpcStatePayload& payload, uint32_t layer) { broadcastNpcState(payload, layer); }
  void cancelPendingSkillHit(uint32_t sourcePlayerId);
  /** Aplica AGORA todos os hits pendentes do jogador (animation cancel de combo).
   *  Mana/CD já foram cobrados no cast; commitar evita perder dano do cast anterior. */
  void commitPendingSkillHitsNow(uint32_t sourcePlayerId);
  void scheduleSkillHit(uint32_t sourcePlayerId, const SkillCastPayload& payload, uint32_t delayMs,
                        int32_t manaCharged = 0, bool canMoveWhileCasting = true);
  /** True se o player tem qualquer hit de cast pendente (sem a folga de isPlayerCasting). */
  bool hasPendingPlayerCast(uint32_t playerId);
  /**
   * Cancela o cast pendente do player e envia o opcode 122. Moved devolve mana e CD;
   * Interrupted/CrowdControl/Died não devolvem. Retorna true se havia cast.
   */
  bool interruptPlayerCast(uint32_t playerId, SkillCastInterruptReason reason);
  /** Cancela o cast pendente do NPC (opcode 122 com sourceType = NPC). */
  bool interruptNpcCast(uint32_t npcInstanceId, SkillCastInterruptReason reason);
  bool interruptCastOf(bool isNpc, uint32_t id, SkillCastInterruptReason reason);
  bool isNpcCasting(uint32_t npcInstanceId) const;
  /** True se o player tem cast em curso (hit pendente vencendo em mais de kCastGraceMs).
   *  Novo cast/ataque básico durante o cast é barrado. */
  bool isPlayerCasting(uint32_t playerId);

  /** Enfileira combate e retorna imediato (não bloqueia a thread de leitura WS). */
  void enqueueSkillCast(uint32_t sourcePlayerId, const SkillCastPayload& payload);
  void enqueueBasicAttack(uint32_t sourcePlayerId, const BasicAttackPayload& payload);
  /** Opcode 88 (ForeignVitalsNotify): enfileira o rebroadcast dos vitals do alvo.
   *  O worker relê HP/MP do DB (gravado pelo apply_vitals.php) e ignora os valores do cliente. */
  void enqueueForeignVitals(uint32_t senderPlayerId, const PlayerVitalsPayload& payload);

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
  float getPlayerMovementSpeedPercent(uint32_t playerId);
  bool canPlayerMove(uint32_t playerId);
  bool canNpcMove(uint32_t npcInstanceId);
  bool canNpcMove(const NpcRuntimeInstance& npc);
  bool canNpcAct(uint32_t npcInstanceId, bool usesSkill);
  bool canNpcAct(const NpcRuntimeInstance& npc, bool usesSkill);
  float getNpcMovementSpeedPercent(uint32_t npcInstanceId);
  float getNpcMovementSpeedPercent(const NpcRuntimeInstance& npc);
  /** Players com STEALTH ativo. A IA chama antes do forEachAlive (não aninha locks). */
  std::unordered_set<uint32_t> snapshotStealthedPlayers();

  /** Efeito de um consumível já validado e descontado pelo ConsumableService (opcode 127/elemental). */
  struct ConsumableEffectJob {
    uint32_t playerId = 0;
    uint8_t source = 0;  // ConsumableSource
    uint32_t itemTemplateId = 0;
    uint32_t inventoryId = 0;
    uint32_t quantityRemaining = 0;
    std::string subtype;
    std::string itemName;
    std::string iconPath;
    int32_t healthRestore = 0;
    int32_t manaRestore = 0;
    std::string buffKey;
    int32_t buffValue = 0;
    uint32_t buffDurationMs = 0;
    int64_t buffExpiresAtMs = 0;
    int64_t cooldownExpiresAtMs = 0;
    uint32_t cooldownTotalMs = 0;
  };
  /** Aplica no combat worker: vida/mana (87), número para grupo/AOI (95), buff (85) e o 128 ao dono. */
  void enqueueConsumableEffect(const ConsumableEffectJob& job);

  /** Reenvia o 87 com os vitals do cache (corrige a UI do dono quando o 86 é recusado). */
  void resyncPlayerVitals(uint32_t playerId) { broadcastPlayerVitals(playerId); }

private:
  void processConsumableEffect(const ConsumableEffectJob& job);
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
  void restorePlayerMana(uint32_t playerId, int32_t amount);
  /** Emite opcode 87. Caminho quente: usa valores conhecidos/cache (0 SELECT). */
  void broadcastPlayerVitals(uint32_t playerId, int32_t knownCurrentHealth = -1,
                             int32_t knownCurrentMana = -1);

  /** Aplica os efeitos DOT/HOT de uma skill no alvo (player via active_dots, NPC in-memory). */
  void applySkillEffects(uint32_t sourcePlayerId, uint8_t targetType, uint32_t targetId,
                         const Combat::SkillData& skill, const Combat::CharacterState& attacker,
                         bool haveAttacker, int32_t appliedDamage = 0);
  void insertPlayerDot(uint32_t sourcePlayerId, uint32_t targetPlayerId, uint32_t skillId,
                       const char* dotType, int32_t tickValue, uint32_t tickIntervalMs,
                       uint32_t ticksTotal, const Combat::SkillData& skill,
                       bool restoresMana = false);
  /** Aplica buff/debuff de player em memória + persistência write-behind (0 MySQL sync). */
  uint64_t applyPlayerBuffInMemory(uint32_t targetPlayerId, uint32_t sourcePlayerId,
                                   uint32_t skillId, const Combat::SkillEffect& effect,
                                   const Combat::SkillData& skill, uint8_t buffTypeCode);
  /** Duração do buff: efeito > skill > 5s, x (100 + buff_duration do caster)% em BUFF/SHIELD. */
  uint32_t effectiveBuffDurationMs(uint32_t sourcePlayerId, const Combat::SkillEffect& effect,
                                   const Combat::SkillData& skill, uint8_t buffTypeCode);
  /** Overlay dos buffs/debuffs/shield em memória sobre CharacterState (cálculo de combate). */
  void overlayRuntimeBuffs(uint32_t playerId, Combat::CharacterState& state);
  void overlayNpcRuntimeBuffs(uint32_t npcInstanceId, Combat::CharacterState& state);
  void hydratePlayerEffects(uint32_t playerId);
  bool hasNpcCrowdControl(uint32_t npcInstanceId, Combat::EffectType type);
  bool hasNpcCrowdControl(const NpcRuntimeInstance& npc, Combat::EffectType type);
  bool hasPlayerCrowdControl(uint32_t playerId, Combat::EffectType type);
  /** Consome pontos de SHIELD em playerBuffs_ após absorção no hit. */
  void consumePlayerShield(uint32_t playerId, int32_t absorbed);
  void tickNpcDots();
  void tickPlayerDots();
  void tickNpcBuffExpirations();
  void tickSummonExpirations();
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
  void broadcastMiss(uint8_t targetType, uint32_t targetId, uint32_t sourcePlayerId,
                     bool triggerDodgeReaction = true);
  void broadcastSkillCast(const SkillCastBroadcastPayload& payload);
  void broadcastBasicAttack(const BasicAttackBroadcastPayload& payload);
  void broadcastNpcCombatEvent(const NpcCombatEventPayload& payload);
  /** layer = instanceId do NPC (etapa 4). Não consulta o NpcManager: pode rodar sob o lock dele. */
  void broadcastNpcState(const NpcStatePayload& payload, uint32_t layer);
  void broadcastSkillBuffSync(const SkillBuffSyncPayload& payload);
  void broadcastCombatControlState(uint8_t targetType, uint32_t targetId);
  /** Opcode 104 action=0 para jogador: targetType=0, enrich e broadcast. */
  void broadcastPlayerSkillBuffApply(SkillBuffSyncPayload& sync);
  void preloadSkillAnimPaths();
  void loadSkillAnimPaths(uint32_t skillId, std::string& anim, std::string& vfx, std::string& sfx,
                           std::string& hitVfx);
  /** Path do opcode 99: skills.cast_anim_path (is_basic_attack) com fallback em basic_attacks. */
  std::string resolveBasicAttackAnimPath(uint32_t classId, const std::string& fallback);
  void handleNpcDamageResult(uint32_t npcInstanceId, int32_t applied, bool npcDied,
                             uint32_t killerPlayerId = 0);
  int32_t computeDoubleBonus(const Combat::CharacterState& attacker,
                             const Combat::CharacterState& defender, bool haveAttacker,
                             int32_t firstHitAbs) const;

  /** Inimigos num raio: NPCs atacáveis vivos + players fora do grupo do caster (caster excluído). */
  std::vector<std::pair<uint8_t, uint32_t>> collectAreaEnemies(uint32_t sourcePlayerId, float cx,
                                                               float cy, float cz, float radius,
                                                               size_t maxTargets);
  struct SkillDamageOutcome {
    bool missed = false;         // errou (hit roll) — efeitos no alvo não aplicam
    bool reactionDodge = false;  // reação do alvo anulou o golpe
    int32_t damage = 0;          // dano total aplicado (positivo)
  };
  /**
   * Resolve todos os efeitos DAMAGE da skill num alvo (1 hit por efeito DAMAGE).
   * rankFactor escala o value_percent dos DAMAGE pelo power_coef efetivo do rank.
   */
  SkillDamageOutcome resolveSkillDamageOnTarget(uint32_t sourcePlayerId, uint8_t targetType,
                                                uint32_t targetId, const Combat::SkillData& skill,
                                                float rankFactor,
                                                const Combat::CharacterState& source,
                                                bool reportMiss);
  /** Cura escalada pelo healer: + MAG_ATK x power_coef e x (100 + healing_bonus)%. */
  int32_t scaleHealAmount(int32_t baseAmount, const Combat::SkillData& skill,
                          const Combat::CharacterState& healer, bool haveHealer) const;
  /** Devolve lifesteal_percent do dano causado como HP ao caster (player). */
  void applyLifesteal(uint32_t sourcePlayerId, const Combat::CharacterState& source, int32_t damage);

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
    int64_t startedAtMs = 0;
    int64_t executeAtMs = 0;
    /** Mana cobrada no início: devolvida só no cancelamento por movimento. */
    int32_t manaCharged = 0;
    bool canMoveWhileCasting = false;
    /** Posição de referência, tirada após kCastMoveGraceMs. */
    bool hasBaseline = false;
    float baseX = 0.f;
    float baseY = 0.f;
  };
  std::mutex pendingSkillHitsMu_;
  std::vector<PendingSkillHit> pendingSkillHits_;

  /** Cast de NPC com cast_time_ms > 0: o NPC fica parado até executeAtMs (ou interrupt). */
  struct PendingNpcCast {
    uint32_t npcInstanceId = 0;
    uint32_t targetPlayerId = 0;
    uint32_t npcSkillId = 0;
    uint8_t rank = 1;
    int64_t executeAtMs = 0;
  };
  mutable std::mutex pendingNpcCastsMu_;
  std::vector<PendingNpcCast> pendingNpcCasts_;

  /** Fase B: andar mais que isto (uu, 2D) após a folga cancela o cast sem move cast. */
  static constexpr float kCastMoveCancelUu = 60.f;
  static constexpr int64_t kCastMoveGraceMs = 250;
  /** Chance do INTERRUPT implícito do checkbox legado is_interrupt. */
  static constexpr uint8_t kLegacyInterruptChance = 50;

  /** R4: remove o STEALTH do player (atacou ou usou skill hostil). True se havia. */
  bool breakPlayerStealth(uint32_t playerId);

  /** R3: retorno decrescente de CC por alvo e categoria (stun / silence / root+slow). */
  struct CcDrState {
    uint8_t count = 0;
    int64_t windowEndMs = 0;
    int64_t immuneUntilMs = 0;
  };
  std::mutex ccDrMu_;
  std::unordered_map<uint64_t, CcDrState> ccDr_;
  static constexpr int64_t kCcDrWindowMs = 15000;
  static constexpr int64_t kCcDrImmuneMs = 15000;
  /** Registra o CC e devolve a % da duração (100 ou 50) ou 0 = imune. */
  int32_t consumeCcDiminishing(bool targetIsNpc, uint32_t targetId, Combat::EffectType type);
  bool isNpcCcImmune(uint32_t npcInstanceId) const;
  /** Texto "Imune" no alvo (opcode 92 / 103 com reason 7). */
  void broadcastCcImmune(bool targetIsNpc, uint32_t targetId, uint32_t sourceId);

  void checkCastMovementCancels(int64_t nowMs);
  void tickPendingNpcCasts();
  void broadcastSkillCastInterrupted(uint8_t sourceType, uint32_t sourceId, uint32_t skillId,
                                     SkillCastInterruptReason reason);
  static void appendLegacyInterruptEffect(Combat::SkillData& skill);

  struct SkillAnimPaths {
    std::string anim;
    std::string vfx;
    std::string sfx;
    std::string hitVfx;
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
  struct ActiveSummon {
    uint32_t instanceId = 0;
    uint32_t ownerPlayerId = 0;
    uint32_t skillId = 0;
    std::chrono::steady_clock::time_point expiresAt{};
  };
  std::mutex summonsMu_;
  std::vector<ActiveSummon> activeSummons_;

  /** Evita reentrância ReactionEngine::mu_ quando dano vem de contra-ataque/reação. */
  bool inReactionDispatch_ = false;

  enum class CombatJobKind : uint8_t {
    SkillCast = 0,
    BasicAttack = 1,
    /** Hit adiado (castTimeMs): só aplica dano/efeitos, sem re-cobrar mana/CD/anim. */
    SkillCastHitOnly = 2,
    /** Opcode 88: rebroadcast de vitals do alvo lidos do DB (nunca do payload do cliente). */
    ForeignVitals = 3,
    /** Consumível validado pelo ConsumableService (127 / elemental). */
    ConsumableEffect = 4,
    /** Etapa 4: mudança estrutural na lista de NPCs (clones de instância) serializada no worker. */
    WorldOp = 5
  };
  struct CombatJob {
    CombatJobKind kind = CombatJobKind::SkillCast;
    uint32_t sourcePlayerId = 0;
    SkillCastPayload skill{};
    BasicAttackPayload basic{};
    PlayerVitalsPayload vitals{};
    ConsumableEffectJob consumable{};
    std::function<void()> worldOp;
  };
  /** Roda no combat worker: valida o sender, lê HP/MP do alvo no DB e faz o broadcast. */
  void processForeignVitals(uint32_t senderPlayerId, const PlayerVitalsPayload& payload);
  void combatWorkerLoop();
  void enqueueJob(CombatJob job);
  /** Enfileira hit adiado no combat worker (nunca processa no tick do zone). */
  void enqueueSkillCastHitOnly(uint32_t sourcePlayerId, const SkillCastPayload& payload);
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
  std::function<void(uint32_t, uint32_t, uint8_t, uint32_t)> instanceNpcKilledHook_;
  class BossController* bossController_ = nullptr;
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
