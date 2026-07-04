#include "zone/CombatCoreEngine.hpp"
#include "zone/MovementServer.hpp"
#include "zone/CombatRange.hpp"
#include "zone/QuestProgressService.hpp"
#include "SkillTypes.hpp"
#include "core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <unordered_set>

namespace Umbra {
namespace Zone {

namespace {
// Regeneracao passiva: intervalo do tick e fracao do maximo regenerada por tick.
constexpr float kRegenIntervalSeconds = 2.0f;
constexpr float kRegenHealthFraction = 0.02f;  // ~2% do max HP por tick
constexpr float kRegenManaFraction = 0.03f;    // ~3% do max MP por tick
constexpr int32_t kDoubleAttackDamagePercent = 80;

SkillCastRejectReason rejectReasonFromErrorCode(const std::string& code) {
  if (code == "NO_MANA") return SkillCastRejectReason::NoMana;
  if (code == "ON_COOLDOWN") return SkillCastRejectReason::OnCooldown;
  if (code == "CANNOT_CAST") return SkillCastRejectReason::CannotCast;
  if (code == "SKILL_NOT_FOUND") return SkillCastRejectReason::SkillNotFound;
  return SkillCastRejectReason::Unknown;
}

uint8_t buffTypeFromDbString(const std::string& value) {
  if (value == "DEBUFF") return static_cast<uint8_t>(Combat::BuffType::DEBUFF);
  if (value == "AURA") return static_cast<uint8_t>(Combat::BuffType::AURA);
  if (value == "DOT") return static_cast<uint8_t>(Combat::BuffType::DOT);
  if (value == "HOT") return static_cast<uint8_t>(Combat::BuffType::HOT);
  if (value == "SHIELD") return static_cast<uint8_t>(Combat::BuffType::SHIELD);
  return static_cast<uint8_t>(Combat::BuffType::BUFF);
}

std::string defaultRejectMessage(SkillCastRejectReason reason) {
  switch (reason) {
    case SkillCastRejectReason::RangeExceeded:
      return "Alvo muito distante";
    case SkillCastRejectReason::NoMana:
      return "Mana insuficiente";
    case SkillCastRejectReason::OnCooldown:
      return "Skill em cooldown";
    case SkillCastRejectReason::CannotCast:
      return "Nao e possivel usar skills agora";
    case SkillCastRejectReason::NoTarget:
      return "Alvo invalido";
    case SkillCastRejectReason::SkillNotFound:
      return "Skill nao encontrada";
    default:
      return "Skill rejeitada";
  }
}
}  // namespace

bool CombatCoreEngine::initialize(uint32_t zoneId,
                                  std::shared_ptr<Database::MySQLConnector> db,
                                  MovementServer* movementServer) {
  zoneId_ = zoneId;
  db_ = std::move(db);
  movementServer_ = movementServer;

  skillService_ = std::make_unique<Combat::SkillService>(db_);
  npcManager_ = std::make_unique<NpcManager>(db_, zoneId_);
  stateLoader_ = std::make_unique<CharacterStateLoader>(db_);
  reactionEngine_ = std::make_unique<ReactionEngine>();
  reactionEngine_->setDatabase(db_);
  reactionEngine_->setCombatEngine(this);
  questProgressService_ = std::make_unique<QuestProgressService>(db_, zoneId_);

  if (!loadBasicAttacks()) {
    Core::Logger::getInstance().warn("[CombatCoreEngine] basic_attacks não carregadas (tabela ausente?)");
  } else {
    Core::Logger::getInstance().info("[CombatCoreEngine] basic_attacks carregadas: {}",
                                       basicAttacksByClass_.size());
  }

  skillService_->loadSkillsFromDatabase();
  npcManager_->reloadFromDatabase();
  return movementServer_ != nullptr;
}

bool CombatCoreEngine::loadBasicAttacks() {
  if (!db_ || !db_->isConnected()) return false;
  basicAttacksByClass_.clear();

  auto rows = db_->executePreparedQuery(
      "SELECT class_id, power_coef, cooldown_ms, range_max, COALESCE(cast_anim_path,'') "
      "FROM basic_attacks",
      {});

  for (const auto& row : rows) {
    if (row.size() < 5) continue;
    try {
      BasicAttackDef def;
      def.classId = static_cast<uint32_t>(std::stoul(row[0]));
      def.powerCoef = static_cast<uint16_t>(std::stoul(row[1]));
      def.cooldownMs = static_cast<uint32_t>(std::stoul(row[2]));
      def.rangeMax = static_cast<uint16_t>(std::stoul(row[3]));
      def.castAnimPath = row[4];
      basicAttacksByClass_[def.classId] = def;
    } catch (...) {
    }
  }
  return !basicAttacksByClass_.empty();
}

void CombatCoreEngine::tick(float deltaSeconds) {
  respawnTickAccum_ += deltaSeconds;
  if (respawnTickAccum_ >= 1.f && npcManager_) {
    const auto respawned = npcManager_->tickRespawns(respawnTickAccum_);
    for (const uint32_t id : respawned) {
      if (const NpcRuntimeInstance* inst = npcManager_->findInstance(id)) {
        broadcastNpcSpawnToAll(*inst);
      }
    }
    respawnTickAccum_ = 0.f;
  }

  // DOT/HOT de NPC (in-memory) sao processados todo frame por steady_clock.
  tickNpcDots();
  tickNpcBuffExpirations();
}

void CombatCoreEngine::tickRegen(float deltaSeconds) {
  regenTickAccum_ += deltaSeconds;
  if (regenTickAccum_ < kRegenIntervalSeconds) return;
  regenTickAccum_ = 0.f;

  if (!db_ || !db_->isConnected() || !movementServer_) return;

  const auto players = movementServer_->getPlayerStates();
  for (const auto& [playerId, state] : players) {
    if (state.isDead) continue;

    const std::string pid = std::to_string(playerId);
    auto healthOpt = db_->executePreparedScalar("SELECT health FROM players WHERE id = ? LIMIT 1", {pid});
    auto manaOpt = db_->executePreparedScalar("SELECT mana FROM players WHERE id = ? LIMIT 1", {pid});
    if (!healthOpt || !manaOpt) continue;

    int32_t curHealth = 0, curMana = 0;
    try {
      curHealth = std::stoi(*healthOpt);
      curMana = std::stoi(*manaOpt);
    } catch (...) {
      continue;
    }

    // Max TOTAL (base + nivel + itens/buffs) para clamp coerente com o HUD.
    int32_t maxHealth = 100, maxMana = 50;
    Combat::CharacterState st;
    if (stateLoader_ && stateLoader_->loadPlayerState(playerId, st)) {
      maxHealth = std::max(1, st.buffedStats.maxHealth);
      maxMana = std::max(1, st.buffedStats.maxMana);
    } else {
      auto maxHOpt = db_->executePreparedScalar(
          "SELECT COALESCE(max_health, 100) FROM players WHERE id = ? LIMIT 1", {pid});
      auto maxMOpt = db_->executePreparedScalar(
          "SELECT COALESCE(max_mana, 50) FROM players WHERE id = ? LIMIT 1", {pid});
      try {
        if (maxHOpt) maxHealth = std::max(1, std::stoi(*maxHOpt));
        if (maxMOpt) maxMana = std::max(1, std::stoi(*maxMOpt));
      } catch (...) {
      }
    }

    // Mortos (health<=0) nao regeneram; respawn cuida disso.
    if (curHealth <= 0) continue;

    const int32_t healthRegen = std::max(1, static_cast<int32_t>(maxHealth * kRegenHealthFraction));
    const int32_t manaRegen = std::max(1, static_cast<int32_t>(maxMana * kRegenManaFraction));
    const int32_t newHealth = std::min(maxHealth, curHealth + healthRegen);
    const int32_t newMana = std::min(maxMana, curMana + manaRegen);

    if (newHealth == curHealth && newMana == curMana) continue;  // nada mudou

    db_->executePreparedInsert("UPDATE players SET health = ?, mana = ? WHERE id = ?",
                               {std::to_string(newHealth), std::to_string(newMana), pid});
    if (stateLoader_) stateLoader_->invalidate(playerId);
    broadcastPlayerVitals(playerId);
  }
}

void CombatCoreEngine::tickBuffExpirations() {
  if (!skillService_) return;
  const auto expired = skillService_->processBuffExpirations();
  if (expired.empty()) return;

  for (const auto& entry : expired) {
    if (stateLoader_) {
      stateLoader_->invalidate(static_cast<uint32_t>(entry.targetPlayerId));
    }
    SkillBuffSyncPayload sync;
    sync.action = 1;
    sync.targetPlayerId = static_cast<uint32_t>(entry.targetPlayerId);
    sync.buffId = entry.buffId;
    sync.skillId = entry.skillId;
    sync.buffType = entry.buffType;
    enrichSkillBuffSyncPayload(sync);
    broadcastSkillBuffSync(sync);
    Core::Logger::getInstance().debug(
        "[CombatCoreEngine] buff expirado target={} buff_id={} skill={}", entry.targetPlayerId,
        entry.buffId, entry.skillId);
  }
}

void CombatCoreEngine::applySkillEffects(uint32_t sourcePlayerId, uint8_t targetType,
                                         uint32_t targetId, const Combat::SkillData& skill,
                                         const Combat::CharacterState& attacker, bool haveAttacker) {
  if (skill.effects.empty()) return;

  const bool targetIsNpc = (targetType == static_cast<uint8_t>(CombatTargetType::Npc));
  bool targetIsPlayer = (targetType == static_cast<uint8_t>(CombatTargetType::Player));
  uint32_t effectPlayerId = targetId;
  const bool effectOnSelf = (skill.target == Combat::TargetType::SELF ||
                             skill.target == Combat::TargetType::PARTY);

  if (effectOnSelf) {
    effectPlayerId = sourcePlayerId;
    targetIsPlayer = true;
  } else if (targetIsPlayer && effectPlayerId == 0) {
    effectPlayerId = sourcePlayerId;
  }
  if (!targetIsNpc && !targetIsPlayer && effectPlayerId > 0) {
    targetIsPlayer = true;
  }
  if (!effectOnSelf && !targetIsNpc && !targetIsPlayer && effectPlayerId == 0) return;

  for (const auto& eff : skill.effects) {
    if (eff.effectType == Combat::EffectType::HEAL) continue;
    if (eff.conditions.is_object() && eff.conditions.contains("trigger")) continue;

    const bool isDot = (eff.effectType == Combat::EffectType::DOT);
    const bool isHot = (eff.effectType == Combat::EffectType::HOT);
    const bool isBuffStat = (eff.effectType == Combat::EffectType::BUFF_STAT);
    const bool isDebuffStat = (eff.effectType == Combat::EffectType::DEBUFF_STAT);
    const bool isShield = (eff.effectType == Combat::EffectType::SHIELD);
    const bool isCcDebuff = (eff.effectType == Combat::EffectType::STUN ||
                             eff.effectType == Combat::EffectType::SILENCE ||
                             eff.effectType == Combat::EffectType::ROOT ||
                             eff.effectType == Combat::EffectType::SLOW);

    if (isBuffStat || isDebuffStat || isShield || isCcDebuff) {
      if (effectPlayerId == 0) continue;
      if (eff.chancePercent < 100) {
        const int32_t roll = std::rand() % 100;
        if (roll >= eff.chancePercent) continue;
      }

      if (!effectOnSelf && targetIsNpc) {
        uint32_t durationMs = eff.durationMs > 0 ? eff.durationMs : skill.durationMs;
        if (durationMs == 0) durationMs = 5000;
        uint8_t buffTypeCode = static_cast<uint8_t>(Combat::BuffType::BUFF);
        if (isShield) {
          buffTypeCode = static_cast<uint8_t>(Combat::BuffType::SHIELD);
        } else if (isDebuffStat || isCcDebuff) {
          buffTypeCode = static_cast<uint8_t>(Combat::BuffType::DEBUFF);
        }
        const uint64_t buffId = applyNpcSkillBuff(targetId, sourcePlayerId, skill.skillId,
                                                  buffTypeCode, eff, skill);
        if (buffId > 0) {
          const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count();
          SkillBuffSyncPayload sync;
          sync.action = 0;
          sync.targetType = 1;
          sync.targetPlayerId = targetId;
          sync.buffId = buffId;
          sync.skillId = skill.skillId;
          sync.buffType = buffTypeCode;
          sync.stacks = 1;
          sync.valueFlat = eff.valueFlat;
          sync.valuePercent = eff.valuePercent;
          sync.expiresAtMs = nowMs + static_cast<int64_t>(durationMs);
          sync.durationMs = durationMs;
          sync.targetStat = eff.targetStat;
          sync.skillName = skill.skillName;
          sync.iconPath = skill.iconPath;
          enrichSkillBuffSyncPayload(sync);
          broadcastSkillBuffSync(sync);
        }
        continue;
      }

      if (!skillService_) continue;
      const uint64_t buffId = skillService_->applyBuff(effectPlayerId, sourcePlayerId, skill.skillId,
                                                       eff, attacker);
      if (buffId > 0) {
        if (stateLoader_) {
          stateLoader_->invalidate(static_cast<uint32_t>(effectPlayerId));
        }
        uint32_t durationMs = eff.durationMs > 0 ? eff.durationMs : skill.durationMs;
        if (durationMs == 0) durationMs = 5000;
        const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::system_clock::now().time_since_epoch())
                                  .count();
        SkillBuffSyncPayload sync;
        sync.targetPlayerId = static_cast<uint32_t>(effectPlayerId);
        sync.buffId = buffId;
        sync.skillId = skill.skillId;
        if (isShield) {
          sync.buffType = static_cast<uint8_t>(Combat::BuffType::SHIELD);
        } else if (isDebuffStat || isCcDebuff) {
          sync.buffType = static_cast<uint8_t>(Combat::BuffType::DEBUFF);
        } else {
          sync.buffType = static_cast<uint8_t>(Combat::BuffType::BUFF);
        }
        sync.stacks = 1;
        sync.valueFlat = eff.valueFlat;
        sync.valuePercent = eff.valuePercent;
        sync.expiresAtMs = nowMs + static_cast<int64_t>(durationMs);
        sync.durationMs = durationMs;
        sync.targetStat = eff.targetStat;
        sync.skillName = skill.skillName;
        sync.iconPath = skill.iconPath;
        broadcastPlayerSkillBuffApply(sync);
      } else {
        Core::Logger::getInstance().warn(
            "[CombatCoreEngine] applyBuff retornou 0: skill={} target={} effectType={}",
            skill.skillId, effectPlayerId, static_cast<int>(eff.effectType));
      }
      continue;
    }

    if (!isDot && !isHot) continue;

    if (!targetIsNpc && !targetIsPlayer) continue;

    // Chance de aplicar.
    if (eff.chancePercent < 100) {
      const int32_t roll = std::rand() % 100;
      if (roll >= eff.chancePercent) continue;
    }

    // Valor de cada tick: flat tem prioridade; senao percentual do atk relevante.
    int32_t tickValue = eff.valueFlat;
    if (tickValue <= 0 && eff.valuePercent > 0) {
      int32_t base = 0;
      if (haveAttacker) {
        base = (skill.element == Combat::Element::PHYSICAL) ? attacker.buffedStats.physicalAttack
                                                            : attacker.buffedStats.magicAttack;
      }
      if (base <= 0) base = std::max<uint16_t>(1, skill.powerCoef);
      tickValue = std::max(1, base * eff.valuePercent / 100);
    }
    if (tickValue <= 0) tickValue = 1;

    const uint32_t interval = std::max<uint32_t>(200, eff.tickIntervalMs);
    uint32_t ticksTotal = (eff.durationMs > 0) ? (eff.durationMs / interval) : 1;
    if (ticksTotal == 0) ticksTotal = 1;
    ticksTotal = std::min<uint32_t>(255, ticksTotal);

    if (targetIsPlayer) {
      // HOT cura (positivo); DOT dano. active_dots usa tick_value positivo + dot_type.
      const char* dotType = isHot ? "HEAL" : "DAMAGE";
      insertPlayerDot(sourcePlayerId, targetId, skill.skillId, dotType, tickValue, interval, ticksTotal,
                      skill);
    } else {  // NPC -> in-memory
      NpcDotInstance inst;
      inst.npcInstanceId = targetId;
      inst.sourcePlayerId = sourcePlayerId;
      inst.skillId = skill.skillId;
      inst.dotBuffId =
          (static_cast<uint64_t>(targetId) << 32) |
          static_cast<uint64_t>(npcDotIdSeq_.fetch_add(1, std::memory_order_relaxed));
      inst.tickValue = isHot ? tickValue : -tickValue;
      inst.intervalMs = interval;
      inst.ticksRemaining = static_cast<uint8_t>(ticksTotal);
      inst.durationMs = interval * ticksTotal;
      const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch())
                                .count();
      inst.expiresAtMs = nowMs + static_cast<int64_t>(inst.durationMs);
      inst.nextTickAt = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
      {
        std::lock_guard<std::mutex> lock(npcDotsMu_);
        npcDots_.erase(std::remove_if(npcDots_.begin(), npcDots_.end(),
                                      [&](const NpcDotInstance& existing) {
                                        return existing.npcInstanceId == targetId &&
                                               existing.skillId == skill.skillId;
                                      }),
                         npcDots_.end());
        npcDots_.push_back(inst);
      }
      SkillBuffSyncPayload sync;
      sync.action = 0;
      sync.targetType = 1;
      sync.targetPlayerId = targetId;
      sync.buffId = inst.dotBuffId;
      sync.skillId = skill.skillId;
      sync.buffType = static_cast<uint8_t>(isHot ? Combat::BuffType::HOT : Combat::BuffType::DOT);
      sync.stacks = static_cast<uint8_t>(ticksTotal);
      sync.valueFlat = tickValue;
      sync.expiresAtMs = inst.expiresAtMs;
      sync.durationMs = inst.durationMs;
      sync.skillName = skill.skillName;
      sync.iconPath = skill.iconPath;
      enrichSkillBuffSyncPayload(sync);
      broadcastSkillBuffSync(sync);
      Core::Logger::getInstance().info(
          "[CombatCoreEngine] DOT/HOT NPC aplicado: npc={} src={} skill={} tick={} ticks={} interval={}ms",
          targetId, sourcePlayerId, skill.skillId, inst.tickValue, ticksTotal, interval);
    }
  }
}

void CombatCoreEngine::insertPlayerDot(uint32_t sourcePlayerId, uint32_t targetPlayerId,
                                       uint32_t skillId, const char* dotType, int32_t tickValue,
                                       uint32_t tickIntervalMs, uint32_t ticksTotal,
                                       const Combat::SkillData& skill) {
  if (!db_ || !db_->isConnected()) return;

  const uint32_t intervalSec = std::max<uint32_t>(1, (tickIntervalMs + 999) / 1000);
  const uint32_t totalDurationSec = intervalSec * ticksTotal;
  const std::string intervalUsStr =
      std::to_string(static_cast<uint64_t>(tickIntervalMs) * 1000ULL);

  if (!db_->executePreparedInsert(
      "INSERT INTO active_dots ("
      "target_player_id, source_player_id, skill_id, dot_type, tick_value, tick_interval_ms, "
      "ticks_remaining, next_tick_at, expires_at) VALUES ("
      "?, ?, ?, ?, ?, ?, ?, "
      "DATE_ADD(NOW(3), INTERVAL ? MICROSECOND), "
      "DATE_ADD(NOW(3), INTERVAL ? SECOND))",
      {std::to_string(targetPlayerId), std::to_string(sourcePlayerId), std::to_string(skillId),
       dotType, std::to_string(tickValue), std::to_string(tickIntervalMs),
       std::to_string(std::min<uint32_t>(255, ticksTotal)), intervalUsStr,
       std::to_string(totalDurationSec)})) {
    return;
  }

  const uint64_t dotId = db_->getLastInsertId();
  const uint32_t durationMs = tickIntervalMs * ticksTotal;
  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

  if (dotId > 0 && movementServer_) {
    SkillBuffSyncPayload sync;
    sync.targetPlayerId = targetPlayerId;
    sync.buffId = dotId;
    sync.skillId = skillId;
    const bool isHeal = dotType && std::strcmp(dotType, "HEAL") == 0;
    sync.buffType = static_cast<uint8_t>(isHeal ? Combat::BuffType::HOT : Combat::BuffType::DOT);
    sync.stacks = static_cast<uint8_t>(std::min<uint32_t>(255, ticksTotal));
    sync.valueFlat = tickValue;
    sync.expiresAtMs = nowMs + static_cast<int64_t>(durationMs);
    sync.durationMs = durationMs;
    sync.skillName = skill.skillName;
    sync.iconPath = skill.iconPath;
    broadcastPlayerSkillBuffApply(sync);
  }

  Core::Logger::getInstance().info(
      "[CombatCoreEngine] DOT/HOT player aplicado: target={} src={} skill={} type={} tick={} ticks={} dot_id={}",
      targetPlayerId, sourcePlayerId, skillId, dotType ? dotType : "?", tickValue, ticksTotal,
      dotId);
}

void CombatCoreEngine::tickNpcDots() {
  if (!npcManager_ || !movementServer_) return;

  std::vector<NpcDotInstance> due;
  std::vector<NpcDotInstance> expiredDots;
  const auto now = std::chrono::steady_clock::now();
  {
    std::lock_guard<std::mutex> lock(npcDotsMu_);
    if (npcDots_.empty()) return;
    for (auto& inst : npcDots_) {
      while (inst.ticksRemaining > 0 && inst.nextTickAt <= now) {
        due.push_back(inst);
        inst.ticksRemaining--;
        inst.nextTickAt += std::chrono::milliseconds(inst.intervalMs);
        if (inst.ticksRemaining == 0) {
          expiredDots.push_back(inst);
        }
      }
    }
    // Remove esgotados.
    npcDots_.erase(std::remove_if(npcDots_.begin(), npcDots_.end(),
                                  [](const NpcDotInstance& i) { return i.ticksRemaining == 0; }),
                   npcDots_.end());
  }

  for (const auto& inst : expiredDots) {
    if (inst.dotBuffId == 0) continue;
    SkillBuffSyncPayload sync;
    sync.action = 1;
    sync.targetType = 1;
    sync.targetPlayerId = inst.npcInstanceId;
    sync.buffId = inst.dotBuffId;
    sync.skillId = inst.skillId;
    sync.buffType = static_cast<uint8_t>(inst.tickValue >= 0 ? Combat::BuffType::HOT
                                                             : Combat::BuffType::DOT);
    sync.expiresAtMs = inst.expiresAtMs;
    sync.durationMs = inst.durationMs;
    enrichSkillBuffSyncPayload(sync);
    broadcastSkillBuffSync(sync);
  }

  for (const auto& inst : due) {
    bool dummyCrit = false;
    bool npcDied = false;
    const int32_t applied = npcManager_->applyDamage(inst.npcInstanceId, inst.tickValue, dummyCrit, &npcDied);
    if (applied != 0 || npcDied) {
      NpcCombatEventPayload evt;
      evt.npcId = inst.npcInstanceId;
      evt.sourcePlayerId = inst.sourcePlayerId;
      evt.delta = applied;
      evt.reason = static_cast<uint8_t>(CombatReason::Dot);
      evt.isCrit = 0;
      broadcastNpcCombatEvent(evt);
      handleNpcDamageResult(inst.npcInstanceId, applied, npcDied, inst.sourcePlayerId);
    }
    if (npcDied) {
      // Remove DOTs restantes deste NPC.
      std::lock_guard<std::mutex> lock(npcDotsMu_);
      npcDots_.erase(std::remove_if(npcDots_.begin(), npcDots_.end(),
                                    [&](const NpcDotInstance& i) {
                                      return i.npcInstanceId == inst.npcInstanceId;
                                    }),
                     npcDots_.end());
    }
  }
}

uint64_t CombatCoreEngine::applyNpcSkillBuff(uint32_t npcInstanceId, uint32_t sourcePlayerId,
                                             uint32_t skillId, uint8_t buffType,
                                             const Combat::SkillEffect& eff,
                                             const Combat::SkillData& skill) {
  if (npcInstanceId == 0) return 0;

  uint32_t durationMs = eff.durationMs > 0 ? eff.durationMs : skill.durationMs;
  if (durationMs == 0) durationMs = 5000;

  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
  const uint64_t buffId =
      (static_cast<uint64_t>(npcInstanceId) << 32) |
      static_cast<uint64_t>(npcBuffIdSeq_.fetch_add(1, std::memory_order_relaxed));

  NpcBuffInstance inst;
  inst.npcInstanceId = npcInstanceId;
  inst.buffId = buffId;
  inst.sourcePlayerId = sourcePlayerId;
  inst.skillId = skillId;
  inst.buffType = buffType;
  inst.stacks = 1;
  inst.valueFlat = eff.valueFlat;
  inst.valuePercent = eff.valuePercent;
  inst.expiresAtMs = nowMs + static_cast<int64_t>(durationMs);
  inst.durationMs = durationMs;
  inst.targetStat = eff.targetStat;
  inst.skillName = skill.skillName;
  inst.iconPath = skill.iconPath;

  std::vector<NpcBuffInstance> replaced;
  {
    std::lock_guard<std::mutex> lock(npcBuffsMu_);
    for (const auto& existing : npcBuffs_) {
      if (existing.npcInstanceId == npcInstanceId && existing.skillId == skillId &&
          existing.buffType == buffType) {
        replaced.push_back(existing);
      }
    }
    npcBuffs_.erase(std::remove_if(npcBuffs_.begin(), npcBuffs_.end(),
                                   [&](const NpcBuffInstance& existing) {
                                     return existing.npcInstanceId == npcInstanceId &&
                                            existing.skillId == skillId &&
                                            existing.buffType == buffType;
                                   }),
                    npcBuffs_.end());
    npcBuffs_.push_back(inst);
  }

  for (const auto& old : replaced) {
    SkillBuffSyncPayload sync;
    sync.action = 1;
    sync.targetType = 1;
    sync.targetPlayerId = old.npcInstanceId;
    sync.buffId = old.buffId;
    sync.skillId = old.skillId;
    sync.buffType = old.buffType;
    sync.stacks = old.stacks;
    sync.valueFlat = old.valueFlat;
    sync.valuePercent = old.valuePercent;
    sync.expiresAtMs = old.expiresAtMs;
    sync.durationMs = old.durationMs;
    sync.targetStat = old.targetStat;
    sync.skillName = old.skillName;
    sync.iconPath = old.iconPath;
    enrichSkillBuffSyncPayload(sync);
    broadcastSkillBuffSync(sync);
  }

  Core::Logger::getInstance().info(
      "[CombatCoreEngine] buff NPC aplicado: npc={} src={} skill={} buffType={} duration={}ms",
      npcInstanceId, sourcePlayerId, skillId, static_cast<int>(buffType), durationMs);
  return buffId;
}

void CombatCoreEngine::tickNpcBuffExpirations() {
  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

  std::vector<NpcBuffInstance> expired;
  {
    std::lock_guard<std::mutex> lock(npcBuffsMu_);
    for (const auto& inst : npcBuffs_) {
      if (inst.expiresAtMs > 0 && inst.expiresAtMs <= nowMs) {
        expired.push_back(inst);
      }
    }
    npcBuffs_.erase(std::remove_if(npcBuffs_.begin(), npcBuffs_.end(),
                                   [&](const NpcBuffInstance& inst) {
                                     return inst.expiresAtMs > 0 && inst.expiresAtMs <= nowMs;
                                   }),
                    npcBuffs_.end());
  }

  for (const auto& inst : expired) {
    SkillBuffSyncPayload sync;
    sync.action = 1;
    sync.targetType = 1;
    sync.targetPlayerId = inst.npcInstanceId;
    sync.buffId = inst.buffId;
    sync.skillId = inst.skillId;
    sync.buffType = inst.buffType;
    sync.stacks = inst.stacks;
    sync.valueFlat = inst.valueFlat;
    sync.valuePercent = inst.valuePercent;
    sync.expiresAtMs = inst.expiresAtMs;
    sync.durationMs = inst.durationMs;
    sync.targetStat = inst.targetStat;
    sync.skillName = inst.skillName;
    sync.iconPath = inst.iconPath;
    enrichSkillBuffSyncPayload(sync);
    broadcastSkillBuffSync(sync);
    Core::Logger::getInstance().debug(
        "[CombatCoreEngine] buff NPC expirado npc={} buff_id={} skill={}", inst.npcInstanceId,
        inst.buffId, inst.skillId);
  }
}

bool CombatCoreEngine::spawnNpcInstance(uint32_t npcInstanceId) {
  if (!npcManager_ || npcInstanceId == 0) return false;
  if (!npcManager_->loadInstanceById(npcInstanceId)) {
    if (!npcManager_->findInstance(npcInstanceId)) return false;
  }
  if (const NpcRuntimeInstance* inst = npcManager_->findInstance(npcInstanceId)) {
    if (inst->isDead) {
      Core::Logger::getInstance().warn("[CombatCoreEngine] spawn bloqueado: NPC {} está morto (aguardando respawn)",
                                       npcInstanceId);
      return false;
    }
    broadcastNpcSpawnToAll(*inst);
    Core::Logger::getInstance().info("[CombatCoreEngine] hot spawn NPC instance {} broadcast", npcInstanceId);
    return true;
  }
  return false;
}

size_t CombatCoreEngine::reloadMissingInstancesFromDatabase() {
  if (!npcManager_) return 0;
  const size_t before = npcManager_->getAllInstances().size();
  npcManager_->reloadMissingInstancesFromDatabase();
  const auto& all = npcManager_->getAllInstances();
  for (size_t i = before; i < all.size(); ++i) {
    broadcastNpcSpawnToAll(all[i]);
  }
  return all.size() - before;
}

void CombatCoreEngine::broadcastNpcSpawnToAll(const NpcRuntimeInstance& inst) {
  if (!movementServer_ || !npcManager_) return;
  movementServer_->broadcastToAll(encodeNpcSpawnNotify(npcManager_->toSpawnPayload(inst)));
}

void CombatCoreEngine::broadcastNpcDespawnToAll(uint32_t npcInstanceId, uint8_t reason) {
  if (!movementServer_) return;
  NpcDespawnPayload p;
  p.npcId = npcInstanceId;
  p.reason = reason;
  movementServer_->broadcastToAll(encodeNpcDespawnNotify(p));
}

void CombatCoreEngine::handleNpcDamageResult(uint32_t npcInstanceId, int32_t applied, bool npcDied,
                                             uint32_t killerPlayerId) {
  if (applied == 0 && !npcDied) return;
  if (npcDied) {
    if (questProgressService_ && killerPlayerId > 0) {
      const NpcRuntimeInstance* victim = npcManager_->findInstance(npcInstanceId);
      if (victim) {
        questProgressService_->onNpcKilled(killerPlayerId, victim->npcInstanceId, victim->templateId,
                                           victim->zoneId);
      }
    }
    {
      std::lock_guard<std::mutex> lock(npcBuffsMu_);
      npcBuffs_.erase(std::remove_if(npcBuffs_.begin(), npcBuffs_.end(),
                                      [&](const NpcBuffInstance& i) {
                                        return i.npcInstanceId == npcInstanceId;
                                      }),
                      npcBuffs_.end());
    }
    broadcastNpcDespawnToAll(npcInstanceId, 1);
    return;
  }
  if (const NpcRuntimeInstance* inst = npcManager_->findInstance(npcInstanceId)) {
    broadcastNpcState(npcManager_->toStatePayload(*inst));
  }
}

void CombatCoreEngine::sendNpcSnapshotToClient(uint32_t clientId) {
  if (!movementServer_ || !npcManager_) return;
  for (const auto& inst : npcManager_->getAllInstances()) {
    if (inst.isDead) continue;
    movementServer_->sendBinaryToClient(clientId, encodeNpcSpawnNotify(npcManager_->toSpawnPayload(inst)));
  }
}

void CombatCoreEngine::sendNpcBuffSnapshotForNpc(uint32_t clientId, uint32_t npcInstanceId) {
  if (!movementServer_) return;

  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

  std::vector<NpcBuffInstance> buffsCopy;
  std::vector<NpcDotInstance> dotsCopy;
  {
    std::lock_guard<std::mutex> lock(npcBuffsMu_);
    buffsCopy = npcBuffs_;
  }
  {
    std::lock_guard<std::mutex> lock(npcDotsMu_);
    dotsCopy = npcDots_;
  }

  size_t sentBuffs = 0;
  size_t sentDots = 0;

  for (const auto& inst : buffsCopy) {
    if (npcInstanceId != 0 && inst.npcInstanceId != npcInstanceId) continue;
    if (inst.expiresAtMs > 0 && inst.expiresAtMs <= nowMs) continue;

    SkillBuffSyncPayload sync;
    sync.action = 0;
    sync.targetType = 1;
    sync.targetPlayerId = inst.npcInstanceId;
    sync.buffId = inst.buffId;
    sync.skillId = inst.skillId;
    sync.buffType = inst.buffType;
    sync.stacks = inst.stacks;
    sync.valueFlat = inst.valueFlat;
    sync.valuePercent = inst.valuePercent;
    sync.expiresAtMs = inst.expiresAtMs;
    sync.durationMs = inst.durationMs;
    sync.targetStat = inst.targetStat;
    sync.skillName = inst.skillName;
    sync.iconPath = inst.iconPath;
    enrichSkillBuffSyncPayload(sync);
    movementServer_->sendBinaryToClient(clientId, encodeSkillBuffSync(sync));
    ++sentBuffs;
  }

  for (const auto& inst : dotsCopy) {
    if (npcInstanceId != 0 && inst.npcInstanceId != npcInstanceId) continue;
    if (inst.ticksRemaining == 0 || inst.dotBuffId == 0) continue;

    SkillBuffSyncPayload sync;
    sync.action = 0;
    sync.targetType = 1;
    sync.targetPlayerId = inst.npcInstanceId;
    sync.buffId = inst.dotBuffId;
    sync.skillId = inst.skillId;
    sync.buffType = static_cast<uint8_t>(inst.tickValue >= 0 ? Combat::BuffType::HOT
                                                             : Combat::BuffType::DOT);
    sync.stacks = inst.ticksRemaining;
    sync.valueFlat = inst.tickValue >= 0 ? inst.tickValue : -inst.tickValue;
    sync.expiresAtMs = inst.expiresAtMs;
    sync.durationMs = inst.durationMs;
    enrichSkillBuffSyncPayload(sync);
    movementServer_->sendBinaryToClient(clientId, encodeSkillBuffSync(sync));
    ++sentDots;
  }

  Core::Logger::getInstance().info(
      "[CombatCoreEngine] sendNpcBuffSnapshot client={} npc={} buffs={} dots={}", clientId,
      npcInstanceId, sentBuffs, sentDots);
}

void CombatCoreEngine::sendNpcBuffSnapshotToClient(uint32_t clientId) {
  sendNpcBuffSnapshotForNpc(clientId, 0);
}

void CombatCoreEngine::sendPlayerBuffSnapshotToClient(uint32_t clientId) {
  if (!movementServer_ || !db_ || !db_->isConnected()) return;

  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

  const auto onlinePlayers = movementServer_->getPlayerStates();
  if (onlinePlayers.empty()) return;

  std::unordered_set<uint32_t> onlineIds;
  for (const auto& [pid, st] : onlinePlayers) {
    if (pid > 0) onlineIds.insert(pid);
  }

  size_t sentBuffs = 0;
  size_t sentDots = 0;

  for (uint32_t playerId : onlineIds) {
    auto buffRows = db_->executePreparedQuery(
        "SELECT buff_id, skill_id, buff_type, current_stacks, value_snapshot, "
        "COALESCE(snapshot_json,''), "
        "GREATEST(0, TIMESTAMPDIFF(MICROSECOND, NOW(3), expires_at) / 1000), is_permanent "
        "FROM active_buffs WHERE target_player_id = ? AND (expires_at > NOW(3) OR is_permanent = 1)",
        {std::to_string(playerId)});

    for (const auto& row : buffRows) {
      if (row.size() < 7) continue;
      try {
        SkillBuffSyncPayload sync;
        sync.action = 0;
        sync.targetType = 0;
        sync.targetPlayerId = playerId;
        sync.buffId = std::stoull(row[0]);
        sync.skillId = static_cast<uint32_t>(std::stoul(row[1]));
        sync.buffType = buffTypeFromDbString(row[2]);
        sync.stacks = static_cast<uint8_t>(std::max(1, std::stoi(row[3])));
        sync.valueFlat = std::stoi(row[4]);
        sync.valuePercent = 0;

        const nlohmann::json snap = nlohmann::json::parse(row[5], nullptr, false);
        if (snap.is_object()) {
          if (snap.contains("value_percent")) {
            sync.valuePercent = static_cast<int16_t>(snap["value_percent"].get<int>());
          }
          if (snap.contains("value_flat") && sync.valueFlat == 0) {
            sync.valueFlat = snap["value_flat"].get<int>();
          }
          if (snap.contains("target_stat") && snap["target_stat"].is_string()) {
            sync.targetStat = snap["target_stat"].get<std::string>();
          }
        }

        const int64_t remainingMs = std::stoll(row[6]);
        const bool isPermanent = (row.size() > 7 && row[7] == "1");
        sync.expiresAtMs = isPermanent ? 0 : (nowMs + remainingMs);
        sync.durationMs = isPermanent ? 0
                                      : static_cast<uint32_t>(std::max<int64_t>(0, remainingMs));

        enrichSkillBuffSyncPayload(sync);
        movementServer_->sendBinaryToClient(clientId, encodeSkillBuffSync(sync));
        ++sentBuffs;
      } catch (...) {
        continue;
      }
    }

    auto dotRows = db_->executePreparedQuery(
        "SELECT dot_id, skill_id, dot_type, tick_value, tick_interval_ms, ticks_remaining, "
        "GREATEST(0, TIMESTAMPDIFF(MICROSECOND, NOW(3), expires_at) / 1000) "
        "FROM active_dots WHERE target_player_id = ? AND expires_at > NOW(3)",
        {std::to_string(playerId)});

    for (const auto& row : dotRows) {
      if (row.size() < 7) continue;
      try {
        const uint32_t skillId = static_cast<uint32_t>(std::stoul(row[1]));
        const bool isHeal = row[2] == "HEAL";
        const int32_t tickValue = std::stoi(row[3]);
        const uint32_t intervalMs = static_cast<uint32_t>(std::stoul(row[4]));
        const uint8_t ticksRemaining = static_cast<uint8_t>(std::stoul(row[5]));
        const int64_t remainingMs = std::stoll(row[6]);

        SkillBuffSyncPayload sync;
        sync.action = 0;
        sync.targetType = 0;
        sync.targetPlayerId = playerId;
        sync.buffId = std::stoull(row[0]);
        sync.skillId = skillId;
        sync.buffType =
            static_cast<uint8_t>(isHeal ? Combat::BuffType::HOT : Combat::BuffType::DOT);
        sync.stacks = ticksRemaining;
        sync.valueFlat = tickValue;
        sync.expiresAtMs = nowMs + remainingMs;
        sync.durationMs = intervalMs * static_cast<uint32_t>(ticksRemaining);

        enrichSkillBuffSyncPayload(sync);
        movementServer_->sendBinaryToClient(clientId, encodeSkillBuffSync(sync));
        ++sentDots;
      } catch (...) {
        continue;
      }
    }
  }

  Core::Logger::getInstance().info(
      "[CombatCoreEngine] sendPlayerBuffSnapshot client={} players={} buffs={} dots={}", clientId,
      onlineIds.size(), sentBuffs, sentDots);
}

void CombatCoreEngine::loadSkillAnimPaths(uint32_t skillId,
                                          std::string& anim, std::string& vfx, std::string& sfx) {
  anim.clear();
  vfx.clear();
  sfx.clear();
  if (!db_ || !db_->isConnected()) return;

  auto rows = db_->executePreparedQuery(
      "SELECT COALESCE(cast_anim_path,''), COALESCE(vfx_path,''), COALESCE(sfx_path,'') "
      "FROM skills WHERE skill_id = ? LIMIT 1",
      {std::to_string(skillId)});
  if (rows.empty() || rows[0].size() < 3) return;
  anim = rows[0][0];
  vfx = rows[0][1];
  sfx = rows[0][2];
}

bool CombatCoreEngine::loadPlayerClassId(uint32_t playerId, uint32_t& outClassId) {
  outClassId = 0;
  if (!db_ || !db_->isConnected()) return false;
  auto opt = db_->executePreparedScalar("SELECT class_id FROM players WHERE id = ? LIMIT 1",
                                        {std::to_string(playerId)});
  if (!opt || opt->empty()) return false;
  try {
    outClassId = static_cast<uint32_t>(std::stoul(*opt));
    return true;
  } catch (...) {
    return false;
  }
}

uint8_t CombatCoreEngine::loadSkillRank(uint32_t playerId, uint32_t skillId) {
  if (!db_ || !db_->isConnected()) return 1;
  auto opt = db_->executePreparedScalar(
      "SELECT current_rank FROM player_skills WHERE player_id = ? AND skill_id = ? LIMIT 1",
      {std::to_string(playerId), std::to_string(skillId)});
  if (!opt || opt->empty()) return 1;
  try {
    const int rank = std::stoi(*opt);
    return static_cast<uint8_t>(std::max(1, rank));
  } catch (...) {
    return 1;
  }
}

void CombatCoreEngine::deductPlayerMana(uint32_t playerId, int32_t cost) {
  if (!db_ || !db_->isConnected() || cost <= 0) return;
  db_->executePreparedInsert(
      "UPDATE players SET mana = GREATEST(0, mana - ?) WHERE id = ?",
      {std::to_string(cost), std::to_string(playerId)});
  if (stateLoader_) stateLoader_->invalidate(playerId);
  // Sincroniza a mana real no cliente (opcode 87), inclusive quando o alvo e NPC.
  broadcastPlayerVitals(playerId);
}

void CombatCoreEngine::broadcastPlayerVitals(uint32_t playerId) {
  if (!db_ || !movementServer_ || playerId == 0) return;

  const std::string pid = std::to_string(playerId);
  // Current sempre fresco do DB (regen/cast acabaram de gravar).
  auto healthOpt = db_->executePreparedScalar("SELECT health FROM players WHERE id = ? LIMIT 1", {pid});
  auto manaOpt = db_->executePreparedScalar("SELECT mana FROM players WHERE id = ? LIMIT 1", {pid});
  if (!healthOpt || !manaOpt) return;

  int32_t curHealth = 0, curMana = 0;
  try {
    curHealth = std::stoi(*healthOpt);
    curMana = std::stoi(*manaOpt);
  } catch (...) {
    return;
  }

  // Max TOTAL (base + nivel + itens/buffs), igual ao character_info que o HUD usa.
  int32_t maxHealth = 100, maxMana = 50;
  Combat::CharacterState st;
  if (stateLoader_ && stateLoader_->loadPlayerState(playerId, st)) {
    maxHealth = std::max(1, st.buffedStats.maxHealth);
    maxMana = std::max(1, st.buffedStats.maxMana);
  } else {
    auto maxHOpt = db_->executePreparedScalar(
        "SELECT COALESCE(max_health, 100) FROM players WHERE id = ? LIMIT 1", {pid});
    auto maxMOpt = db_->executePreparedScalar(
        "SELECT COALESCE(max_mana, 50) FROM players WHERE id = ? LIMIT 1", {pid});
    try {
      if (maxHOpt) maxHealth = std::max(1, std::stoi(*maxHOpt));
      if (maxMOpt) maxMana = std::max(1, std::stoi(*maxMOpt));
    } catch (...) {
    }
  }

  PlayerVitalsPayload vitals;
  vitals.playerId = playerId;
  vitals.currentHealth = std::clamp(curHealth, 0, maxHealth);
  vitals.maxHealth = maxHealth;
  vitals.currentMana = std::clamp(curMana, 0, maxMana);
  vitals.maxMana = maxMana;
  vitals.sourcePlayerId = playerId;
  vitals.reason = static_cast<uint8_t>(CombatReason::Unknown);

  // delta=0 -> handleVitalsBroadcastUnlocked envia apenas o opcode 87 (sem CombatEvent).
  movementServer_->broadcastVitalsAndCombat(playerId, vitals, playerId, /*delta*/ 0,
                                            /*triggerDeath*/ false);
}

void CombatCoreEngine::writeCombatLog(uint32_t sourcePlayerId, uint32_t targetPlayerId,
                                      uint32_t skillId, const char* actionType, int32_t value,
                                      bool isCrit, int32_t overkill) {
  if (!db_ || !db_->isConnected() || targetPlayerId == 0) return;
  db_->executePreparedInsert(
      "INSERT INTO combat_log (source_player_id, target_player_id, skill_id, action_type, "
      "value, is_critical, overkill, zone_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
      {std::to_string(sourcePlayerId), std::to_string(targetPlayerId),
       std::to_string(skillId), actionType, std::to_string(value),
       isCrit ? "1" : "0", std::to_string(overkill), std::to_string(zoneId_)});
}

bool CombatCoreEngine::tryGetPlayerPosition(uint32_t playerId, float& outX, float& outY,
                                            float& outZ) const {
  if (!movementServer_) return false;
  const auto players = movementServer_->getPlayerStates();
  const auto it = players.find(playerId);
  if (it == players.end()) return false;
  outX = it->second.x;
  outY = it->second.y;
  outZ = it->second.z;
  return true;
}

bool CombatCoreEngine::tryGetTargetPosition(uint8_t targetType, uint32_t targetId, float& outX,
                                            float& outY, float& outZ) const {
  if (targetId == 0) return false;
  if (targetType == static_cast<uint8_t>(CombatTargetType::Player)) {
    return tryGetPlayerPosition(targetId, outX, outY, outZ);
  }
  if (targetType == static_cast<uint8_t>(CombatTargetType::Npc)) {
    if (!npcManager_) return false;
    const NpcRuntimeInstance* inst = npcManager_->findInstance(targetId);
    if (!inst || inst->isDead) return false;
    outX = inst->x;
    outY = inst->y;
    outZ = inst->z;
    return true;
  }
  return false;
}

bool CombatCoreEngine::validateSkillRange(uint32_t sourcePlayerId, const Combat::SkillData& skill,
                                          const SkillCastPayload& payload,
                                          SkillCastRejectReason* outFailReason) const {
  using TT = Combat::TargetType;

  auto fail = [&](SkillCastRejectReason reason) {
    if (outFailReason) *outFailReason = reason;
    return false;
  };

  if (skill.target == TT::SELF || skill.target == TT::PARTY) {
    return true;
  }

  float sx = 0.f, sy = 0.f, sz = 0.f;
  if (!tryGetPlayerPosition(sourcePlayerId, sx, sy, sz)) {
    Core::Logger::getInstance().warn(
        "[CombatCoreEngine] RANGE: posicao do caster {} desconhecida", sourcePlayerId);
    return fail(SkillCastRejectReason::CannotCast);
  }

  float tx = 0.f, ty = 0.f, tz = 0.f;
  bool haveTargetPos = false;

  if (skill.target == TT::AREA || skill.target == TT::AREA_ALLY) {
    tx = payload.targetX;
    ty = payload.targetY;
    tz = payload.targetZ;
    if (tx != 0.f || ty != 0.f || tz != 0.f) {
      haveTargetPos = true;
    } else if (payload.targetId > 0) {
      haveTargetPos = tryGetTargetPosition(payload.targetType, payload.targetId, tx, ty, tz);
    }
    if (!haveTargetPos) {
      Core::Logger::getInstance().warn("[CombatCoreEngine] RANGE: skill AREA {} sem ponto alvo",
                                       skill.skillId);
      return fail(SkillCastRejectReason::NoTarget);
    }
  } else {
    if (!skill.requiresTarget && payload.targetId == 0) {
      return true;
    }
    if (payload.targetId == 0) {
      Core::Logger::getInstance().warn("[CombatCoreEngine] RANGE: skill {} sem targetId",
                                       skill.skillId);
      return fail(SkillCastRejectReason::NoTarget);
    }
    haveTargetPos = tryGetTargetPosition(payload.targetType, payload.targetId, tx, ty, tz);
    if (!haveTargetPos) {
      Core::Logger::getInstance().warn(
          "[CombatCoreEngine] RANGE: alvo type={} id={} posicao desconhecida", payload.targetType,
          payload.targetId);
      return fail(SkillCastRejectReason::NoTarget);
    }
  }

  const float maxR = effectiveMaxRange(static_cast<float>(skill.rangeMax));
  if (!isInRange3D(sx, sy, sz, tx, ty, tz, maxR)) {
    Core::Logger::getInstance().info(
        "[CombatCoreEngine] RANGE_EXCEEDED skill={} player={} max={:.1f}", skill.skillId,
        sourcePlayerId, maxR);
    return fail(SkillCastRejectReason::RangeExceeded);
  }
  return true;
}

void CombatCoreEngine::sendSkillCastRejected(uint32_t playerId, uint32_t skillId,
                                           SkillCastRejectReason reason,
                                           const std::string& message) {
  if (!movementServer_) return;
  SkillCastRejectedPayload payload;
  payload.playerId = playerId;
  payload.skillId = skillId;
  payload.reason = static_cast<uint8_t>(reason);
  payload.message = message.empty() ? defaultRejectMessage(reason) : message;
  movementServer_->sendToPlayer(playerId, encodeSkillCastRejected(payload));
  Core::Logger::getInstance().info("[CombatCoreEngine] SkillCastRejected player={} skill={} reason={}",
                                   playerId, skillId, static_cast<int>(reason));
}

bool CombatCoreEngine::validateBasicAttackRange(uint32_t sourcePlayerId, uint8_t targetType,
                                                uint32_t targetId, uint16_t rangeMax) const {
  if (targetId == 0) return false;
  float sx = 0.f, sy = 0.f, sz = 0.f;
  float tx = 0.f, ty = 0.f, tz = 0.f;
  if (!tryGetPlayerPosition(sourcePlayerId, sx, sy, sz)) return false;
  if (!tryGetTargetPosition(targetType, targetId, tx, ty, tz)) return false;
  const float maxR = effectiveMaxRange(static_cast<float>(rangeMax));
  if (!isInRange3D(sx, sy, sz, tx, ty, tz, maxR)) {
    Core::Logger::getInstance().info(
        "[CombatCoreEngine] RANGE_EXCEEDED basic_attack player={} target={} max={:.1f}",
        sourcePlayerId, targetId, maxR);
    return false;
  }
  return true;
}

bool CombatCoreEngine::buildDefenderState(uint8_t targetType, uint32_t targetId,
                                          Combat::CharacterState& out, bool& outIsPlayer) {
  outIsPlayer = (targetType == static_cast<uint8_t>(CombatTargetType::Player));
  if (outIsPlayer) {
    if (targetId == 0 || !stateLoader_) return false;
    return stateLoader_->loadPlayerState(targetId, out);
  }
  if (!npcManager_) return false;
  const NpcRuntimeInstance* inst = npcManager_->findInstance(targetId);
  if (!inst || inst->isDead) return false;
  if (!inst->isAttackable) {
    Core::Logger::getInstance().warn("[CombatCoreEngine] ataque bloqueado em NPC pacífico id={}", targetId);
    return false;
  }
  out = CharacterStateLoader::makeNpcDefenderState(*inst);
  return true;
}

void CombatCoreEngine::broadcastMiss(uint8_t targetType, uint32_t targetId, uint32_t sourcePlayerId) {
  if (!movementServer_) return;
  if (targetType == static_cast<uint8_t>(CombatTargetType::Player) && targetId > 0 &&
      reactionEngine_) {
    reactionEngine_->onPlayerDodge(targetId, sourcePlayerId);
  }
  if (targetType == static_cast<uint8_t>(CombatTargetType::Npc)) {
    NpcCombatEventPayload evt;
    evt.npcId = targetId;
    evt.sourcePlayerId = sourcePlayerId;
    evt.delta = 0;
    evt.reason = static_cast<uint8_t>(CombatReason::Miss);
    evt.isCrit = 0;
    broadcastNpcCombatEvent(evt);
  } else if (targetId > 0) {
    CombatEventPayload combat;
    combat.targetId = targetId;
    combat.sourceId = sourcePlayerId;
    combat.delta = 0;
    combat.reason = static_cast<uint8_t>(CombatReason::Miss);
    combat.isCrit = 0;
    movementServer_->broadcastToAll(encodeCombatEventNotify(combat));
  }
}

bool CombatCoreEngine::checkAndStampBasicCooldown(uint32_t playerId, uint32_t cooldownMs) {
  if (cooldownMs == 0) return true;
  const auto now = std::chrono::steady_clock::now();
  std::lock_guard<std::mutex> lock(basicCdMu_);
  auto it = basicAttackReadyAt_.find(playerId);
  if (it != basicAttackReadyAt_.end() && now < it->second) {
    return false;
  }
  basicAttackReadyAt_[playerId] = now + std::chrono::milliseconds(cooldownMs);
  return true;
}

bool CombatCoreEngine::applyPlayerDamage(uint32_t sourcePlayerId, uint32_t targetPlayerId,
                                         int32_t delta, uint8_t reason, bool isCrit, bool isDouble) {
  if (!db_ || !movementServer_ || delta == 0) return false;

  const std::string tid = std::to_string(targetPlayerId);
  auto healthOpt = db_->executePreparedScalar("SELECT health FROM players WHERE id = ? LIMIT 1", {tid});
  auto maxHOpt = db_->executePreparedScalar(
      "SELECT COALESCE(max_health, 100) FROM players WHERE id = ? LIMIT 1", {tid});
  auto manaOpt = db_->executePreparedScalar("SELECT mana FROM players WHERE id = ? LIMIT 1", {tid});
  auto maxMOpt = db_->executePreparedScalar(
      "SELECT COALESCE(max_mana, 50) FROM players WHERE id = ? LIMIT 1", {tid});
  if (!healthOpt || !manaOpt) return false;

  int32_t curHealth = std::stoi(*healthOpt);
  int32_t maxHealth = maxHOpt ? std::max(1, std::stoi(*maxHOpt)) : 100;
  int32_t curMana = std::stoi(*manaOpt);
  int32_t maxMana = maxMOpt ? std::max(1, std::stoi(*maxMOpt)) : 50;

  if (stateLoader_) {
    Combat::CharacterState st;
    if (stateLoader_->loadPlayerState(targetPlayerId, st)) {
      maxHealth = std::max(1, st.buffedStats.maxHealth);
      maxMana = std::max(1, st.buffedStats.maxMana);
    }
  }

  if (reactionEngine_ && delta < 0 && !inReactionDispatch_) {
    int32_t adjustedDelta = delta;
    reactionEngine_->onAllyDamaged(targetPlayerId, sourcePlayerId, adjustedDelta);
    delta = adjustedDelta;
    reactionEngine_->onPlayerDamaged(targetPlayerId, sourcePlayerId, delta, isCrit);
  }

  const int32_t clampedHealth = std::max(0, std::min(maxHealth, curHealth + delta));
  const bool deadAfter = (clampedHealth <= 0);

  if (deadAfter) {
    db_->executePreparedInsert(
        "UPDATE players SET health = ?, is_dead = 1, last_death_at = CURRENT_TIMESTAMP WHERE id = ?",
        {std::to_string(clampedHealth), tid});
  } else {
    db_->executePreparedInsert(
        "UPDATE players SET health = ?, is_dead = 0 WHERE id = ?",
        {std::to_string(clampedHealth), tid});
  }

  if (stateLoader_) stateLoader_->invalidate(targetPlayerId);

  PlayerVitalsPayload vitals;
  vitals.playerId = targetPlayerId;
  vitals.currentHealth = clampedHealth;
  vitals.maxHealth = maxHealth;
  vitals.currentMana = curMana;
  vitals.maxMana = maxMana;
  vitals.sourcePlayerId = sourcePlayerId;
  vitals.reason = reason;

  movementServer_->broadcastVitalsAndCombat(targetPlayerId, vitals, sourcePlayerId,
                                            clampedHealth - curHealth, deadAfter, isCrit, isDouble);
  return true;
}

void CombatCoreEngine::broadcastSkillBuffSync(const SkillBuffSyncPayload& payload) {
  if (!movementServer_) return;
  if (payload.targetType == 1) {
    Core::Logger::getInstance().info(
        "[CombatCoreEngine] SkillBuffSync broadcast npc={} skill={} action={} buff_id={}",
        payload.targetPlayerId, payload.skillId, static_cast<int>(payload.action), payload.buffId);
  } else {
    Core::Logger::getInstance().info(
        "[CombatCoreEngine] SkillBuffSync broadcast player={} skill={} action={} buff_id={} type={}",
        payload.targetPlayerId, payload.skillId, static_cast<int>(payload.action), payload.buffId,
        static_cast<int>(payload.buffType));
  }
  movementServer_->broadcastToAll(encodeSkillBuffSync(payload));
}

void CombatCoreEngine::broadcastPlayerSkillBuffApply(SkillBuffSyncPayload& sync) {
  sync.action = 0;
  sync.targetType = 0;
  enrichSkillBuffSyncPayload(sync);
  broadcastSkillBuffSync(sync);
}

void CombatCoreEngine::broadcastSkillBuffSyncPublic(const SkillBuffSyncPayload& payload) {
  broadcastSkillBuffSync(payload);
}

void CombatCoreEngine::enrichSkillBuffSyncPayload(SkillBuffSyncPayload& payload) {
  if (!skillService_ || payload.skillId == 0) return;
  const Combat::SkillData* skill = skillService_->getSkillData(payload.skillId);
  if (!skill) return;
  if (payload.skillName.empty()) payload.skillName = skill->skillName;
  if (payload.iconPath.empty()) payload.iconPath = skill->iconPath;
}

bool CombatCoreEngine::skillHasEffectType(const Combat::SkillData& skill,
                                          Combat::EffectType type) const {
  return std::any_of(skill.effects.begin(), skill.effects.end(),
                     [type](const Combat::SkillEffect& e) { return e.effectType == type; });
}

int32_t CombatCoreEngine::computeInstantHealDelta(const Combat::SkillData& skill, uint8_t rank,
                                                  const Combat::CharacterState& sourceState,
                                                  const Combat::CharacterState& healTarget,
                                                  bool haveSource) const {
  int32_t totalHeal = 0;
  for (const auto& eff : skill.effects) {
    if (eff.effectType != Combat::EffectType::HEAL) continue;
    if (eff.valueFlat > 0) {
      totalHeal += eff.valueFlat;
      continue;
    }
    if (eff.valuePercent > 0) {
      const int32_t maxHp = std::max(1, healTarget.buffedStats.maxHealth);
      totalHeal += std::max(1, maxHp * eff.valuePercent / 100);
    }
  }
  if (totalHeal > 0) return totalHeal;
  if (!haveSource) {
    return static_cast<int32_t>(std::max<uint16_t>(1, skill.powerCoef / 2));
  }
  return std::max(1, Combat::CombatCalculator::getInstance().calculateHeal(sourceState, healTarget,
                                                                           skill, rank));
}

void CombatCoreEngine::armReactionSkill(uint32_t sourcePlayerId, const Combat::SkillData& skill) {
  if (!reactionEngine_) return;

  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

  for (const auto& eff : skill.effects) {
    if (!eff.conditions.is_object() || !eff.conditions.contains("trigger")) continue;

    const uint64_t buffId =
        reactionEngine_->armReaction(sourcePlayerId, sourcePlayerId, skill.skillId, skill, eff);
    if (buffId == 0) continue;

    if (stateLoader_) stateLoader_->invalidate(sourcePlayerId);

    uint32_t durationMs = eff.durationMs > 0 ? eff.durationMs : skill.durationMs;
    if (durationMs == 0) durationMs = 30000;

    SkillBuffSyncPayload sync;
    sync.targetPlayerId = sourcePlayerId;
    sync.buffId = buffId;
    sync.skillId = skill.skillId;
    sync.buffType = static_cast<uint8_t>(Combat::BuffType::AURA);
    sync.stacks = 1;
    sync.valueFlat = eff.valueFlat;
    sync.valuePercent = eff.valuePercent;
    sync.expiresAtMs = nowMs + static_cast<int64_t>(durationMs);
    sync.durationMs = durationMs;
    sync.targetStat = eff.targetStat.empty() ? "reaction" : eff.targetStat;
    sync.skillName = skill.skillName;
    sync.iconPath = skill.iconPath;
    broadcastPlayerSkillBuffApply(sync);
  }
}

void CombatCoreEngine::applyReactionBuff(uint32_t targetPlayerId, uint32_t sourcePlayerId,
                                         uint32_t skillId, const Combat::SkillEffect& effect) {
  if (!skillService_) return;
  Combat::CharacterState dummy;
  const uint64_t buffId =
      skillService_->applyBuff(targetPlayerId, sourcePlayerId, skillId, effect, dummy);
  if (buffId == 0) return;
  if (stateLoader_) stateLoader_->invalidate(targetPlayerId);

  uint32_t durationMs = effect.durationMs > 0 ? effect.durationMs : 500;
  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
  const Combat::SkillData* skill = skillService_->getSkillData(skillId);

  SkillBuffSyncPayload sync;
  sync.targetPlayerId = targetPlayerId;
  sync.buffId = buffId;
  sync.skillId = skillId;
  sync.buffType = static_cast<uint8_t>(Combat::BuffType::BUFF);
  sync.stacks = 1;
  sync.valueFlat = effect.valueFlat;
  sync.valuePercent = effect.valuePercent;
  sync.expiresAtMs = nowMs + static_cast<int64_t>(durationMs);
  sync.durationMs = durationMs;
  sync.targetStat = effect.targetStat;
  sync.skillName = skill ? skill->skillName : std::string{};
  sync.iconPath = skill ? skill->iconPath : std::string{};
  broadcastPlayerSkillBuffApply(sync);
}

void CombatCoreEngine::applyReactionCounterDamage(uint32_t ownerPlayerId, uint32_t targetPlayerId,
                                                  uint32_t skillId, const Combat::SkillEffect& effect,
                                                  int32_t fixedDamage) {
  if (targetPlayerId == 0 || ownerPlayerId == targetPlayerId) return;

  int32_t damage = fixedDamage;
  if (damage <= 0) {
    Combat::CharacterState owner;
    if (stateLoader_ && stateLoader_->loadPlayerState(ownerPlayerId, owner)) {
      const int32_t base = owner.buffedStats.physicalAttack;
      const int32_t pct = effect.valuePercent > 0 ? effect.valuePercent : 100;
      damage = std::max(1, base * pct / 100);
    } else {
      damage = std::max(1, effect.valueFlat);
    }
  }

  struct ReactionDispatchGuard {
    bool& flag;
    explicit ReactionDispatchGuard(bool& f) : flag(f) { flag = true; }
    ~ReactionDispatchGuard() { flag = false; }
  };
  ReactionDispatchGuard guard(inReactionDispatch_);
  applyPlayerDamage(ownerPlayerId, targetPlayerId, -damage, static_cast<uint8_t>(CombatReason::Skill),
                    false);
  writeCombatLog(ownerPlayerId, targetPlayerId, skillId, "REACTION", damage, false, 0);
}

void CombatCoreEngine::applyDirectPlayerDamage(uint32_t sourcePlayerId, uint32_t targetPlayerId,
                                               int32_t damage, uint8_t reason) {
  if (damage <= 0 || targetPlayerId == 0) return;
  applyPlayerDamage(sourcePlayerId, targetPlayerId, -damage, reason, false);
}

void CombatCoreEngine::broadcastSkillCast(const SkillCastBroadcastPayload& payload) {
  if (!movementServer_) return;
  movementServer_->broadcastToAll(encodeSkillCastBroadcast(payload));
}

void CombatCoreEngine::broadcastBasicAttack(const BasicAttackBroadcastPayload& payload) {
  if (!movementServer_) return;
  movementServer_->broadcastToAll(encodeBasicAttackBroadcast(payload));
}

void CombatCoreEngine::broadcastNpcCombatEvent(const NpcCombatEventPayload& payload) {
  if (!movementServer_) return;
  movementServer_->broadcastToAll(encodeNpcCombatEvent(payload));
}

void CombatCoreEngine::broadcastNpcState(const NpcStatePayload& payload) {
  if (!movementServer_) return;
  movementServer_->broadcastToAll(encodeNpcStateUpdate(payload));
}

void CombatCoreEngine::processSkillCast(uint32_t sourcePlayerId, const SkillCastPayload& payload) {
  if (!skillService_ || !movementServer_) return;

  const Combat::SkillData* skill = skillService_->getSkillData(payload.skillId);
  if (!skill) {
    Core::Logger::getInstance().warn("[CombatCoreEngine] skill {} não encontrada", payload.skillId);
    sendSkillCastRejected(sourcePlayerId, payload.skillId, SkillCastRejectReason::SkillNotFound);
    return;
  }

  Combat::CharacterState sourceState;
  const bool haveSource = stateLoader_ && stateLoader_->loadPlayerState(sourcePlayerId, sourceState);
  if (!haveSource) {
    sourceState = Combat::CharacterState{};
    sourceState.playerId = sourcePlayerId;
    sourceState.isAlive = true;
  }

  SkillCastRejectReason rangeFail = SkillCastRejectReason::Unknown;
  if (!validateSkillRange(sourcePlayerId, *skill, payload, &rangeFail)) {
    sendSkillCastRejected(sourcePlayerId, payload.skillId, rangeFail);
    return;
  }

  Combat::SkillUseRequest req;
  req.skillId = payload.skillId;
  req.sourcePlayerId = sourcePlayerId;
  if (payload.targetType == static_cast<uint8_t>(CombatTargetType::Player)) {
    req.targetPlayerId = payload.targetId;
  }

  const auto validation = skillService_->validateSkillUse(sourceState, req);
  if (!validation.isValid) {
    Core::Logger::getInstance().warn("[CombatCoreEngine] skill cast rejeitado: {}", validation.errorCode);
    sendSkillCastRejected(sourcePlayerId, payload.skillId,
                          rejectReasonFromErrorCode(validation.errorCode), validation.errorMessage);
    return;
  }

  const uint8_t rank = loadSkillRank(sourcePlayerId, payload.skillId);

  const bool isHealPrecheck = (skill->type == Combat::SkillType::HOT) ||
                              skillHasEffectType(*skill, Combat::EffectType::HEAL) ||
                              skillHasEffectType(*skill, Combat::EffectType::HOT);
  if (isHealPrecheck && skill->target == Combat::TargetType::SELF && haveSource &&
      sourceState.buffedStats.currentHealth >= sourceState.buffedStats.maxHealth) {
    Core::Logger::getInstance().info(
        "[CombatCoreEngine] HEAL rejeitado player={} skill={} motivo=HP_cheio", sourcePlayerId,
        payload.skillId);
    sendSkillCastRejected(sourcePlayerId, payload.skillId, SkillCastRejectReason::CannotCast,
                          "HP cheio");
    return;
  }

  skillService_->startCooldown(sourcePlayerId, payload.skillId, skill->cooldownMs);

  if (skill->resourceType == Combat::ResourceType::MANA && skill->resourceCost > 0) {
    deductPlayerMana(sourcePlayerId, static_cast<int32_t>(skill->resourceCost));
  }

  std::string anim, vfx, sfx;
  loadSkillAnimPaths(payload.skillId, anim, vfx, sfx);

  SkillCastBroadcastPayload castBroadcast;
  castBroadcast.sourcePlayerId = sourcePlayerId;
  castBroadcast.skillId = payload.skillId;
  castBroadcast.targetId = payload.targetId;
  castBroadcast.castTimeMs = skill->castTimeMs;
  castBroadcast.castAnimPath = anim;
  castBroadcast.vfxPath = vfx;
  castBroadcast.sfxPath = sfx;
  broadcastSkillCast(castBroadcast);

  if (skill->type == Combat::SkillType::REACTION) {
    armReactionSkill(sourcePlayerId, *skill);
    Core::Logger::getInstance().info(
        "[CombatCoreEngine] SkillCast REACTION armada player={} skill={}", sourcePlayerId,
        payload.skillId);
    return;
  }

  const bool isHeal = (skill->type == Combat::SkillType::HOT) ||
                      skillHasEffectType(*skill, Combat::EffectType::HEAL) ||
                      skillHasEffectType(*skill, Combat::EffectType::HOT);
  const bool isBuffOnly =
      !isHeal && !skillHasEffectType(*skill, Combat::EffectType::DAMAGE) &&
      (skillHasEffectType(*skill, Combat::EffectType::BUFF_STAT) ||
       skillHasEffectType(*skill, Combat::EffectType::DEBUFF_STAT) ||
       skillHasEffectType(*skill, Combat::EffectType::SHIELD) ||
       skillHasEffectType(*skill, Combat::EffectType::STUN) ||
       skillHasEffectType(*skill, Combat::EffectType::SILENCE) ||
       skillHasEffectType(*skill, Combat::EffectType::ROOT) ||
       skillHasEffectType(*skill, Combat::EffectType::SLOW));

  const bool effectOnSelf = (skill->target == Combat::TargetType::SELF ||
                             skill->target == Combat::TargetType::PARTY);

  Combat::CharacterState defender;
  bool defenderIsPlayer = false;
  bool haveDefender =
      buildDefenderState(payload.targetType, payload.targetId, defender, defenderIsPlayer);
  if (effectOnSelf || (isHeal && payload.targetId == 0)) {
    defender = sourceState;
    defenderIsPlayer = true;
    haveDefender = haveSource;
  }

  int32_t delta = 0;
  bool isCrit = false;
  int32_t overkill = 0;

  if (isHeal) {
    const Combat::CharacterState& healTarget = haveDefender ? defender : sourceState;
    delta = computeInstantHealDelta(*skill, rank, sourceState, healTarget, haveSource);
    delta = std::max(1, delta);
  } else if (!isBuffOnly) {
    if (!haveDefender) {
      Core::Logger::getInstance().warn(
          "[CombatCoreEngine] skill {} sem alvo válido (target={})", payload.skillId, payload.targetId);
      return;
    }
    if (haveSource) {
      const bool isPvP = defenderIsPlayer;
      {
        const int32_t hitChance =
            Combat::CombatCalculator::getInstance().calculateHitChance(sourceState, defender);
        if (!Combat::CombatCalculator::getInstance().rollHit(hitChance)) {
          broadcastMiss(payload.targetType, payload.targetId, sourcePlayerId);
          Core::Logger::getInstance().info("[CombatCoreEngine] SkillCast MISS player={} skill={} target={}",
                                           sourcePlayerId, payload.skillId, payload.targetId);
          return;
        }
      }
      if (defenderIsPlayer && payload.targetId > 0 && reactionEngine_ &&
          reactionEngine_->onPlayerHitReceived(payload.targetId, sourcePlayerId)) {
        broadcastMiss(payload.targetType, payload.targetId, sourcePlayerId);
        Core::Logger::getInstance().info(
            "[CombatCoreEngine] SkillCast REACTION miss player={} skill={} target={}", sourcePlayerId,
            payload.skillId, payload.targetId);
        applySkillEffects(sourcePlayerId, payload.targetType, payload.targetId, *skill, sourceState,
                          haveSource);
        return;
      }
      const Combat::DamageBreakdown bd =
          (skill->element == Combat::Element::PHYSICAL)
              ? Combat::CombatCalculator::getInstance().calculatePhysicalDamage(sourceState, defender, *skill, rank, isPvP)
              : Combat::CombatCalculator::getInstance().calculateMagicDamage(sourceState, defender, *skill, rank, isPvP);
      delta = -bd.finalDamage;
      isCrit = (bd.critMultiplier != 100);
      overkill = bd.overkill;
    } else {
      delta = -static_cast<int32_t>(std::max<uint16_t>(1, skill->getEffectivePowerCoef(rank) / 2));
    }
  }

  const uint32_t healTargetId =
      (isHeal && (effectOnSelf || payload.targetId == 0)) ? sourcePlayerId : payload.targetId;

  if (payload.targetType == static_cast<uint8_t>(CombatTargetType::Npc) && !isHeal && !isBuffOnly) {
    int32_t doubleBonus = 0;
    bool isDouble = false;
    if (haveSource && haveDefender && delta < 0) {
      doubleBonus =
          computeDoubleBonus(sourceState, defender, true, std::abs(delta));
      isDouble = doubleBonus > 0;
    }
    const int32_t totalDelta = delta - doubleBonus;

    bool dummyCrit = false;
    bool npcDied = false;
    const int32_t applied = npcManager_->applyDamage(payload.targetId, totalDelta, dummyCrit, &npcDied);
    if (applied != 0 || npcDied) {
      NpcCombatEventPayload evt;
      evt.npcId = payload.targetId;
      evt.sourcePlayerId = sourcePlayerId;
      evt.delta = applied;
      evt.reason = static_cast<uint8_t>(CombatReason::Skill);
      evt.isCrit = isCrit ? 1 : 0;
      evt.isDouble = isDouble ? 1 : 0;
      broadcastNpcCombatEvent(evt);
      handleNpcDamageResult(payload.targetId, applied, npcDied, sourcePlayerId);
      if (isDouble) {
        Core::Logger::getInstance().info(
            "[CombatCoreEngine] DOUBLE player={} targetType={} target={} totalDamage={} skill={}",
            sourcePlayerId, payload.targetType, payload.targetId, std::abs(applied), payload.skillId);
      }
    }
  } else if (isHeal) {
    std::vector<uint32_t> healTargets;
    using TT = Combat::TargetType;
    if (skill->target == TT::PARTY) {
      if (resolvePartyMembers_) {
        healTargets = resolvePartyMembers_(sourcePlayerId);
      }
      if (healTargets.empty()) {
        healTargets.push_back(sourcePlayerId);
      }
    } else if (skill->target == TT::AREA_ALLY) {
      std::vector<uint32_t> candidates;
      if (resolvePartyMembers_) {
        candidates = resolvePartyMembers_(sourcePlayerId);
      }
      if (candidates.empty()) {
        candidates.push_back(sourcePlayerId);
      }
      float cx = 0.f, cy = 0.f, cz = 0.f;
      if (!tryGetPlayerPosition(sourcePlayerId, cx, cy, cz)) {
        cx = payload.targetX;
        cy = payload.targetY;
        cz = payload.targetZ;
      }
      const float radius = skill->areaRadius > 0.f ? skill->areaRadius : 500.f;
      const float radiusSq = radius * radius;
      for (uint32_t memberId : candidates) {
        float px = 0.f, py = 0.f, pz = 0.f;
        if (tryGetPlayerPosition(memberId, px, py, pz)) {
          const float dx = px - cx;
          const float dy = py - cy;
          const float dz = pz - cz;
          if (dx * dx + dy * dy + dz * dz <= radiusSq) {
            healTargets.push_back(memberId);
          }
        }
      }
      if (healTargets.empty()) {
        healTargets.push_back(sourcePlayerId);
      }
    } else if (healTargetId > 0) {
      healTargets.push_back(healTargetId);
    } else {
      healTargets.push_back(sourcePlayerId);
    }

    bool anyHealApplied = false;
    for (uint32_t tgtId : healTargets) {
      Combat::CharacterState tgtState;
      if (!stateLoader_ || !stateLoader_->loadPlayerState(tgtId, tgtState)) {
        continue;
      }
      if (tgtState.buffedStats.currentHealth >= tgtState.buffedStats.maxHealth) {
        continue;
      }
      int32_t healDelta =
          computeInstantHealDelta(*skill, rank, sourceState, tgtState, haveSource);
      healDelta = std::max(1, healDelta);
      if (applyPlayerDamage(sourcePlayerId, tgtId, healDelta, static_cast<uint8_t>(CombatReason::Heal),
                            false)) {
        writeCombatLog(sourcePlayerId, tgtId, payload.skillId, "HEAL", healDelta, false, 0);
        Core::Logger::getInstance().info(
            "[CombatCoreEngine] HEAL player={} delta={} target={} skill={}", sourcePlayerId,
            healDelta, tgtId, payload.skillId);
        anyHealApplied = true;
      }
    }
    if (!anyHealApplied) {
      Core::Logger::getInstance().warn(
          "[CombatCoreEngine] HEAL sem efeito player={} skill={} (alvos com HP cheio ou invalidos)",
          sourcePlayerId, payload.skillId);
    }
  } else if (payload.targetType == static_cast<uint8_t>(CombatTargetType::Player) &&
             payload.targetId > 0 && !isHeal && !isBuffOnly) {
    int32_t doubleBonus = 0;
    bool isDouble = false;
    if (haveSource && haveDefender && delta < 0) {
      doubleBonus =
          computeDoubleBonus(sourceState, defender, true, std::abs(delta));
      isDouble = doubleBonus > 0;
    }
    const int32_t totalDelta = delta - doubleBonus;

    if (applyPlayerDamage(sourcePlayerId, payload.targetId, totalDelta,
                          static_cast<uint8_t>(CombatReason::Skill), isCrit, isDouble)) {
      writeCombatLog(sourcePlayerId, payload.targetId, payload.skillId,
                     isDouble ? "DOUBLE" : "DAMAGE", std::abs(totalDelta), isCrit, overkill);
      if (isDouble) {
        Core::Logger::getInstance().info(
            "[CombatCoreEngine] DOUBLE player={} targetType={} target={} totalDamage={} skill={}",
            sourcePlayerId, payload.targetType, payload.targetId, std::abs(totalDelta),
            payload.skillId);
      }
    }
  }

  // DOT/HOT e demais efeitos persistentes da skill (effects_json).
  applySkillEffects(sourcePlayerId, payload.targetType, payload.targetId, *skill, sourceState, haveSource);

  Core::Logger::getInstance().info(
      "[CombatCoreEngine] SkillCast player={} skill={} rank={} targetType={} target={} delta={} crit={}",
      sourcePlayerId, payload.skillId, static_cast<int>(rank), payload.targetType, payload.targetId,
      delta, isCrit ? 1 : 0);
}

void CombatCoreEngine::processBasicAttack(uint32_t sourcePlayerId, const BasicAttackPayload& payload) {
  if (!movementServer_) return;

  uint32_t classId = 0;
  if (!loadPlayerClassId(sourcePlayerId, classId)) {
    Core::Logger::getInstance().warn("[CombatCoreEngine] class_id não encontrado para player {}", sourcePlayerId);
    return;
  }

  auto it = basicAttacksByClass_.find(classId);
  if (it == basicAttacksByClass_.end()) {
    Core::Logger::getInstance().warn("[CombatCoreEngine] basic attack não definido para class {}", classId);
    return;
  }

  const BasicAttackDef& basic = it->second;

  if (!validateBasicAttackRange(sourcePlayerId, payload.targetType, payload.targetId,
                                basic.rangeMax)) {
    return;
  }

  if (!checkAndStampBasicCooldown(sourcePlayerId, basic.cooldownMs)) {
    Core::Logger::getInstance().debug("[CombatCoreEngine] basic attack em cooldown (player {})", sourcePlayerId);
    return;
  }

  BasicAttackBroadcastPayload atkBroadcast;
  atkBroadcast.sourcePlayerId = sourcePlayerId;
  atkBroadcast.classId = classId;
  atkBroadcast.targetId = payload.targetId;
  atkBroadcast.hitWindowMs = 300;
  atkBroadcast.castAnimPath = basic.castAnimPath;
  broadcastBasicAttack(atkBroadcast);

  // SkillData sintética representando o ataque básico (físico, pode critar).
  Combat::SkillData synthetic;
  synthetic.element = Combat::Element::PHYSICAL;
  synthetic.scalingStat = Combat::ScalingStat::PHYS_ATK;
  synthetic.powerCoef = basic.powerCoef;
  synthetic.canCrit = true;
  synthetic.ignoresDefense = false;

  Combat::CharacterState attacker;
  const bool haveAttacker = stateLoader_ && stateLoader_->loadPlayerState(sourcePlayerId, attacker);

  Combat::CharacterState defender;
  bool defenderIsPlayer = false;
  const bool haveDefender =
      buildDefenderState(payload.targetType, payload.targetId, defender, defenderIsPlayer);
  if (!haveDefender) {
    Core::Logger::getInstance().warn(
        "[CombatCoreEngine] basic attack sem alvo válido (target={})", payload.targetId);
    return;
  }

  // Hit/miss para qualquer alvo (player ou NPC). NPC tem dodge=0, entao a
  // frequencia de miss depende da accuracy do atacante.
  if (haveAttacker) {
    const int32_t hitChance =
        Combat::CombatCalculator::getInstance().calculateHitChance(attacker, defender);
    if (!Combat::CombatCalculator::getInstance().rollHit(hitChance)) {
      broadcastMiss(payload.targetType, payload.targetId, sourcePlayerId);
      Core::Logger::getInstance().info("[CombatCoreEngine] BasicAttack MISS player={} target={}",
                                       sourcePlayerId, payload.targetId);
      return;
    }
    if (defenderIsPlayer && payload.targetId > 0 && reactionEngine_ &&
        reactionEngine_->onPlayerHitReceived(payload.targetId, sourcePlayerId)) {
      broadcastMiss(payload.targetType, payload.targetId, sourcePlayerId);
      Core::Logger::getInstance().info(
          "[CombatCoreEngine] BasicAttack REACTION miss player={} target={}", sourcePlayerId,
          payload.targetId);
      return;
    }
  }

  int32_t delta = 0;
  bool isCrit = false;
  int32_t overkill = 0;
  if (haveAttacker) {
    const Combat::DamageBreakdown bd =
        Combat::CombatCalculator::getInstance().calculatePhysicalDamage(attacker, defender, synthetic, /*rank*/ 1, defenderIsPlayer);
    delta = -bd.finalDamage;
    isCrit = (bd.critMultiplier != 100);
    overkill = bd.overkill;
  } else {
    delta = -static_cast<int32_t>(std::max<uint16_t>(1, basic.powerCoef / 2));
  }

  if (payload.targetType == static_cast<uint8_t>(CombatTargetType::Npc)) {
    int32_t doubleBonus = 0;
    bool isDouble = false;
    if (haveAttacker && haveDefender && delta < 0) {
      doubleBonus = computeDoubleBonus(attacker, defender, true, std::abs(delta));
      isDouble = doubleBonus > 0;
    }
    const int32_t totalDelta = delta - doubleBonus;

    bool dummyCrit = false;
    bool npcDied = false;
    const int32_t applied = npcManager_->applyDamage(payload.targetId, totalDelta, dummyCrit, &npcDied);
    if (applied != 0 || npcDied) {
      NpcCombatEventPayload evt;
      evt.npcId = payload.targetId;
      evt.sourcePlayerId = sourcePlayerId;
      evt.delta = applied;
      evt.reason = static_cast<uint8_t>(CombatReason::Damage);
      evt.isCrit = isCrit ? 1 : 0;
      evt.isDouble = isDouble ? 1 : 0;
      broadcastNpcCombatEvent(evt);
      handleNpcDamageResult(payload.targetId, applied, npcDied, sourcePlayerId);
      if (isDouble) {
        Core::Logger::getInstance().info(
            "[CombatCoreEngine] DOUBLE player={} targetType={} target={} totalDamage={} skill=0",
            sourcePlayerId, payload.targetType, payload.targetId, std::abs(applied));
      }
    }
  } else if (payload.targetType == static_cast<uint8_t>(CombatTargetType::Player) && payload.targetId > 0) {
    int32_t doubleBonus = 0;
    bool isDouble = false;
    if (haveAttacker && haveDefender && delta < 0) {
      doubleBonus = computeDoubleBonus(attacker, defender, haveAttacker, std::abs(delta));
      isDouble = doubleBonus > 0;
    }
    const int32_t totalDelta = delta - doubleBonus;

    if (applyPlayerDamage(sourcePlayerId, payload.targetId, totalDelta,
                          static_cast<uint8_t>(CombatReason::Damage), isCrit, isDouble)) {
      writeCombatLog(sourcePlayerId, payload.targetId, 0, isDouble ? "DOUBLE" : "DAMAGE",
                     std::abs(totalDelta), isCrit, overkill);
      if (isDouble) {
        Core::Logger::getInstance().info(
            "[CombatCoreEngine] DOUBLE player={} targetType={} target={} totalDamage={} skill=0",
            sourcePlayerId, payload.targetType, payload.targetId, std::abs(totalDelta));
      }
    }
  }

  Core::Logger::getInstance().info(
      "[CombatCoreEngine] BasicAttack player={} class={} targetType={} target={} delta={} crit={}",
      sourcePlayerId, classId, payload.targetType, payload.targetId, delta, isCrit ? 1 : 0);
}

void CombatCoreEngine::onPlayerJoinedZone(uint32_t playerId) {
  if (!reactionEngine_ || playerId == 0) return;
  reactionEngine_->reloadArmedForPlayer(playerId);
  Core::Logger::getInstance().info("[CombatCoreEngine] jogador {} entrou na zone — reações recarregadas",
                                   playerId);
}

void CombatCoreEngine::setResolvePartyMembersCallback(
    std::function<std::vector<uint32_t>(uint32_t playerId)> cb) {
  resolvePartyMembers_ = std::move(cb);
}

float CombatCoreEngine::getPlayerMovementSpeedPercent(uint32_t playerId) const {
  if (!stateLoader_ || playerId == 0) return 100.f;
  Combat::CharacterState st;
  if (!stateLoader_->loadPlayerState(playerId, st)) return 100.f;
  return static_cast<float>(std::max(50, st.buffedStats.movementSpeed));
}

int32_t CombatCoreEngine::computeDoubleBonus(const Combat::CharacterState& attacker,
                                             const Combat::CharacterState& defender,
                                             bool haveAttacker, int32_t firstHitAbs) const {
  if (!haveAttacker || firstHitAbs <= 0) return 0;

  const int32_t chance =
      Combat::CombatCalculator::getInstance().calculateDoubleAttackChance(attacker, defender);
  if (!Combat::CombatCalculator::getInstance().rollDoubleAttack(chance)) {
    return 0;
  }

  return std::max(1, firstHitAbs * kDoubleAttackDamagePercent / 100);
}

}  // namespace Zone
}  // namespace Umbra
