#include "zone/CombatCoreEngine.hpp"
#include "zone/MovementServer.hpp"
#include "zone/CombatRange.hpp"
#include "zone/QuestProgressService.hpp"
#include "zone/AgentDebugLog.hpp"
#include "SkillTypes.hpp"
#include "StatKeyMapping.hpp"
#include "core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_set>
#include <unordered_map>

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

const char* effectTypeToWireName(Combat::EffectType type) {
  switch (type) {
    case Combat::EffectType::DEBUFF_STAT: return "DEBUFF_STAT";
    case Combat::EffectType::SHIELD: return "SHIELD";
    case Combat::EffectType::STUN: return "STUN";
    case Combat::EffectType::SILENCE: return "SILENCE";
    case Combat::EffectType::ROOT: return "ROOT";
    case Combat::EffectType::SLOW: return "SLOW";
    case Combat::EffectType::DOT: return "DOT";
    case Combat::EffectType::HOT: return "HOT";
    case Combat::EffectType::HEAL: return "HEAL";
    default: return "BUFF_STAT";
  }
}

std::string resolveBuffTargetStat(const Combat::SkillEffect& effect) {
  if (!effect.targetStat.empty()) return effect.targetStat;
  switch (effect.effectType) {
    case Combat::EffectType::STUN: return "stun";
    case Combat::EffectType::SILENCE: return "silence";
    case Combat::EffectType::ROOT: return "root";
    case Combat::EffectType::SLOW: return "slow";
    default: return {};
  }
}

void inferEffectTypeFromTargetStat(SkillBuffSyncPayload& payload) {
  if (!payload.effectType.empty()) return;
  std::string ts = payload.targetStat;
  for (char& c : ts) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  if (ts == "stun") payload.effectType = "STUN";
  else if (ts == "silence") payload.effectType = "SILENCE";
  else if (ts == "root") payload.effectType = "ROOT";
  else if (ts == "slow") payload.effectType = "SLOW";
}

int32_t ccResistFromStats(const Combat::CharacterStats& stats, Combat::EffectType type) {
  switch (type) {
    case Combat::EffectType::STUN: return stats.stunResist;
    case Combat::EffectType::SILENCE: return stats.silenceResist;
    case Combat::EffectType::ROOT: return stats.rootResist;
    case Combat::EffectType::SLOW: return stats.slowResist;
    default: return 0;
  }
}

int32_t ccChanceFromStats(const Combat::CharacterStats& stats, Combat::EffectType type) {
  switch (type) {
    case Combat::EffectType::STUN: return stats.stunChance;
    case Combat::EffectType::SILENCE: return stats.silenceChance;
    case Combat::EffectType::ROOT: return stats.rootChance;
    case Combat::EffectType::SLOW: return stats.slowChance;
    default: return 0;
  }
}

int32_t computeCcApplyChance(const Combat::SkillEffect& effect, int32_t targetResist,
                             int32_t casterCcChance) {
  const int32_t reduced =
      std::max(0, targetResist - static_cast<int32_t>(effect.resistPenetration));
  return std::clamp(static_cast<int32_t>(effect.chancePercent) + casterCcChance - reduced, 0, 100);
}

bool isCasterCcResistBuff(const Combat::SkillEffect& effect) {
  if (effect.effectType != Combat::EffectType::BUFF_STAT) return false;
  std::string raw = effect.targetStat;
  for (char& c : raw) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  const std::string key = Combat::StatKeyMapping::mapTargetStatToCanonical(raw);
  return key == "stun_resist" || key == "silence_resist" || key == "root_resist" ||
         key == "slow_resist";
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
  preloadSkillAnimPaths();
  npcManager_->reloadFromDatabase();

  if (!dbWriterRunning_.exchange(true)) {
    dbWriter_ = std::thread([this]() { dbWriterLoop(); });
    Core::Logger::getInstance().info("[CombatCoreEngine] db writer (write-behind) iniciado");
  }

  if (npcManager_) {
    npcManager_->setAsyncDbWrite([this](std::string sql) { enqueueDbWrite(std::move(sql)); });
  }

  npcAi_ = std::make_unique<NpcAiSystem>(npcManager_.get(), movementServer_, this);

  if (!workerRunning_.exchange(true)) {
    combatWorker_ = std::thread([this]() { combatWorkerLoop(); });
    Core::Logger::getInstance().info("[CombatCoreEngine] combat worker iniciado (fila FIFO)");
  }

  return movementServer_ != nullptr;
}

CombatCoreEngine::~CombatCoreEngine() {
  shutdown();
}

void CombatCoreEngine::shutdown() {
  if (!workerRunning_.exchange(false)) {
    return;
  }
  combatQueueCv_.notify_all();
  if (combatWorker_.joinable()) {
    combatWorker_.join();
  }
  {
    std::lock_guard<std::mutex> lock(combatQueueMu_);
    combatQueue_.clear();
  }
  Core::Logger::getInstance().info("[CombatCoreEngine] combat worker parado");

  // Drenar as escritas pendentes ANTES de sair para não perder HP/mana/logs no shutdown.
  if (dbWriterRunning_.exchange(false)) {
    dbWriteCv_.notify_all();
    if (dbWriter_.joinable()) {
      dbWriter_.join();
    }
    Core::Logger::getInstance().info("[CombatCoreEngine] db writer parado (fila drenada)");
  }
}

void CombatCoreEngine::enqueueDbWrite(std::string sql) {
  if (sql.empty()) return;
  // Fallback síncrono se o writer não estiver ativo (ex.: shutdown em progresso).
  if (!dbWriterRunning_) {
    if (db_ && db_->isConnected()) db_->executeQuery(sql);
    return;
  }
  {
    std::lock_guard<std::mutex> lock(dbWriteMu_);
    while (dbWriteQueue_.size() >= kDbWriteQueueCap) {
      // Segurança: nunca deixar a fila crescer sem limite; descarta a mais antiga.
      dbWriteQueue_.pop_front();
    }
    dbWriteQueue_.push_back(std::move(sql));
  }
  dbWriteCv_.notify_one();
}

void CombatCoreEngine::enqueueDbWriteFront(std::string sql) {
  if (sql.empty()) return;
  if (!dbWriterRunning_) {
    if (db_ && db_->isConnected()) db_->executeQuery(sql);
    return;
  }
  {
    std::lock_guard<std::mutex> lock(dbWriteMu_);
    while (dbWriteQueue_.size() >= kDbWriteQueueCap) {
      dbWriteQueue_.pop_back();  // descarta a mais nova; a de morte tem prioridade
    }
    dbWriteQueue_.push_front(std::move(sql));
  }
  dbWriteCv_.notify_one();
}

void CombatCoreEngine::dbWriterLoop() {
  while (true) {
    std::string sql;
    {
      std::unique_lock<std::mutex> lock(dbWriteMu_);
      dbWriteCv_.wait(lock, [this]() {
        return !dbWriterRunning_ || !dbWriteQueue_.empty();
      });
      if (dbWriteQueue_.empty()) {
        if (!dbWriterRunning_) break;
        continue;
      }
      sql = std::move(dbWriteQueue_.front());
      dbWriteQueue_.pop_front();
    }
    try {
      if (db_ && db_->isConnected()) db_->executeQuery(sql);
    } catch (const std::exception& e) {
      Core::Logger::getInstance().error("[CombatCoreEngine] db writer exceção: {}", e.what());
    } catch (...) {
      Core::Logger::getInstance().error("[CombatCoreEngine] db writer exceção desconhecida");
    }
  }
}

void CombatCoreEngine::enqueueJob(CombatJob job) {
  if (!workerRunning_) {
    // Fallback síncrono se o worker ainda não subiu / já parou.
    if (job.kind == CombatJobKind::SkillCast) {
      processSkillCast(job.sourcePlayerId, job.skill);
    } else {
      processBasicAttack(job.sourcePlayerId, job.basic);
    }
    return;
  }

  {
    std::lock_guard<std::mutex> lock(combatQueueMu_);
    while (combatQueue_.size() >= kCombatQueueCap) {
      Core::Logger::getInstance().warn(
          "[CombatCoreEngine] fila de combate cheia ({}), dropando job antigo kind={}",
          combatQueue_.size(), static_cast<int>(combatQueue_.front().kind));
      combatQueue_.pop_front();
    }
    combatQueue_.push_back(std::move(job));
  }
  combatQueueCv_.notify_one();
}

void CombatCoreEngine::enqueueSkillCast(uint32_t sourcePlayerId, const SkillCastPayload& payload) {
  if (sourcePlayerId == 0 || payload.skillId == 0 || !skillService_) return;

  const Combat::SkillData* skill = skillService_->getSkillData(payload.skillId);
  if (!skill) return;

  // Cooldown de um cast JÁ processado: descarta spam.
  if (skillService_->isSkillOnCooldown(sourcePlayerId, payload.skillId)) {
    // #region agent log
    agentDebugLog("H-G", "CombatCoreEngine.cpp:enqueueSkillCast", "combat_enqueue_drop",
                  std::string("{\"reason\":\"cooldown\",\"playerId\":") +
                      std::to_string(sourcePlayerId) +
                      ",\"skillId\":" + std::to_string(payload.skillId) + "}",
                  "post-fix");
    // #endregion
    sendSkillCastRejected(sourcePlayerId, payload.skillId, SkillCastRejectReason::OnCooldown);
    return;
  }

  CombatJob job;
  job.kind = CombatJobKind::SkillCast;
  job.sourcePlayerId = sourcePlayerId;
  job.skill = payload;

  if (!workerRunning_) {
    processSkillCast(sourcePlayerId, payload);
    return;
  }

  const uint64_t key = skillCastKey(sourcePlayerId, payload.skillId);
  size_t queueSize = 0;
  bool droppedDup = false;
  {
    std::lock_guard<std::mutex> lock(combatQueueMu_);
    if (skillCastInFlight_.count(key) > 0) {
      droppedDup = true;
      queueSize = combatQueue_.size();
    } else {
      // Só dropa a MESMA skill já pendente/in-flight (spam). Combos com skills
      // diferentes entram na fila — sem latest-wins (isso engolia o combo).
      for (const auto& existing : combatQueue_) {
        if (existing.kind == CombatJobKind::SkillCast &&
            existing.sourcePlayerId == sourcePlayerId &&
            existing.skill.skillId == payload.skillId) {
          droppedDup = true;
          break;
        }
      }
      if (droppedDup) {
        queueSize = combatQueue_.size();
      } else {
        while (combatQueue_.size() >= kCombatQueueCap) {
          combatQueue_.pop_front();
        }
        job.enqueuedAtMs = agentNowMs();
        combatQueue_.push_back(std::move(job));
        queueSize = combatQueue_.size();
      }
    }
  }
  if (droppedDup) {
    // #region agent log
    agentDebugLog("H-Q", "CombatCoreEngine.cpp:enqueueSkillCast", "combat_enqueue_drop",
                  std::string("{\"reason\":\"same_skill_pending\",\"playerId\":") +
                      std::to_string(sourcePlayerId) +
                      ",\"skillId\":" + std::to_string(payload.skillId) +
                      ",\"queue\":" + std::to_string(queueSize) + "}",
                  "post-fix");
    // #endregion
    sendSkillCastRejected(sourcePlayerId, payload.skillId, SkillCastRejectReason::CannotCast,
                          "cast_pending");
    return;
  }
  combatQueueCv_.notify_one();
}

void CombatCoreEngine::enqueueBasicAttack(uint32_t sourcePlayerId, const BasicAttackPayload& payload) {
  if (sourcePlayerId == 0) return;

  CombatJob job;
  job.kind = CombatJobKind::BasicAttack;
  job.sourcePlayerId = sourcePlayerId;
  job.basic = payload;

  if (!workerRunning_) {
    processBasicAttack(sourcePlayerId, payload);
    return;
  }

  bool droppedDup = false;
  {
    std::lock_guard<std::mutex> lock(combatQueueMu_);
    for (const auto& existing : combatQueue_) {
      if (existing.kind == CombatJobKind::BasicAttack &&
          existing.sourcePlayerId == sourcePlayerId) {
        droppedDup = true;
        break;
      }
    }
    if (!droppedDup) {
      while (combatQueue_.size() >= kCombatQueueCap) {
        combatQueue_.pop_front();
      }
      job.enqueuedAtMs = agentNowMs();
      combatQueue_.push_back(std::move(job));
    }
  }
  if (droppedDup) {
    // #region agent log
    agentDebugLog("H-G", "CombatCoreEngine.cpp:enqueueBasicAttack", "combat_enqueue_drop",
                  std::string("{\"reason\":\"dup_basic\",\"playerId\":") +
                      std::to_string(sourcePlayerId) + "}",
                  "post-fix");
    // #endregion
    return;
  }
  combatQueueCv_.notify_one();
}

void CombatCoreEngine::combatWorkerLoop() {
  while (workerRunning_) {
    CombatJob job;
    size_t queueBefore = 0;
    size_t queueAfter = 0;
    uint64_t inFlightKey = 0;
    {
      std::unique_lock<std::mutex> lock(combatQueueMu_);
      combatQueueCv_.wait(lock, [this]() {
        return !workerRunning_ || !combatQueue_.empty();
      });
      if (!workerRunning_ && combatQueue_.empty()) {
        break;
      }
      if (combatQueue_.empty()) {
        continue;
      }
      queueBefore = combatQueue_.size();
      job = std::move(combatQueue_.front());
      combatQueue_.pop_front();
      queueAfter = combatQueue_.size();
      if (job.kind == CombatJobKind::SkillCast) {
        inFlightKey = skillCastKey(job.sourcePlayerId, job.skill.skillId);
        skillCastInFlight_.insert(inFlightKey);
      }
    }

    // #region agent log
    const int64_t jobT0 = agentNowMs();
    // #endregion
    try {
      if (job.kind == CombatJobKind::SkillCast) {
        processSkillCast(job.sourcePlayerId, job.skill);
      } else {
        processBasicAttack(job.sourcePlayerId, job.basic);
      }
    } catch (const std::exception& e) {
      Core::Logger::getInstance().error(
          "[CombatCoreEngine] exceção no combat worker: {}", e.what());
    } catch (...) {
      Core::Logger::getInstance().error("[CombatCoreEngine] exceção desconhecida no combat worker");
    }
    if (inFlightKey != 0) {
      std::lock_guard<std::mutex> lock(combatQueueMu_);
      skillCastInFlight_.erase(inFlightKey);
    }
    // #region agent log
    {
      const int64_t jobMs = agentNowMs() - jobT0;
      const int64_t queueWaitMs =
          (job.enqueuedAtMs > 0) ? (jobT0 - job.enqueuedAtMs) : 0;
      const uint32_t skillId =
          (job.kind == CombatJobKind::SkillCast) ? job.skill.skillId : 0;
      if (jobMs >= 20 || queueWaitMs >= 30 || queueBefore >= 2) {
        agentDebugLog("H-BURST", "CombatCoreEngine.cpp:combatWorkerLoop", "combat_job_ms",
                      std::string("{\"ms\":") + std::to_string(jobMs) +
                          ",\"queueWaitMs\":" + std::to_string(queueWaitMs) +
                          ",\"kind\":" + std::to_string(static_cast<int>(job.kind)) +
                          ",\"playerId\":" + std::to_string(job.sourcePlayerId) +
                          ",\"skillId\":" + std::to_string(skillId) +
                          ",\"queueBefore\":" + std::to_string(queueBefore) +
                          ",\"queueAfter\":" + std::to_string(queueAfter) + "}",
                      "burst");
      }
    }
    // #endregion
  }
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

  // Manager grava o path em skills (Ataque Básico). Overlay em cima de basic_attacks.
  auto skillRows = db_->executePreparedQuery(
      "SELECT class_id, COALESCE(cast_anim_path,'') FROM skills "
      "WHERE is_basic_attack = 1 AND is_enabled = 1",
      {});
  for (const auto& row : skillRows) {
    if (row.size() < 2 || row[1].empty()) continue;
    try {
      const uint32_t cid = static_cast<uint32_t>(std::stoul(row[0]));
      auto it = basicAttacksByClass_.find(cid);
      if (it != basicAttacksByClass_.end()) {
        it->second.castAnimPath = row[1];
      }
    } catch (...) {
    }
  }
  return !basicAttacksByClass_.empty();
}

void CombatCoreEngine::tick(float deltaSeconds) {
  if (npcManager_) {
    const auto deathReady = npcManager_->tickPendingDeathDespawns();
    for (const uint32_t id : deathReady) {
      broadcastNpcDespawnToAll(id, 1);
    }
  }

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

  // Hits adiados por castTimeMs (antes dos DOTs).
  tickPendingSkillHits();

  // DOT/HOT + expiração de buffs de player/NPC (in-memory) — sem poll MySQL.
  tickNpcDots();
  tickPlayerDots();
  tickNpcBuffExpirations();
  tickBuffExpirations();

  if (npcAi_) {
    npcAi_->tick(deltaSeconds);
  }
}

void CombatCoreEngine::cancelPendingSkillHit(uint32_t sourcePlayerId) {
  if (sourcePlayerId == 0) return;
  size_t removed = 0;
  {
    std::lock_guard<std::mutex> lock(pendingSkillHitsMu_);
    const auto before = pendingSkillHits_.size();
    pendingSkillHits_.erase(
        std::remove_if(pendingSkillHits_.begin(), pendingSkillHits_.end(),
                       [sourcePlayerId](const PendingSkillHit& h) {
                         return h.sourcePlayerId == sourcePlayerId;
                       }),
        pendingSkillHits_.end());
    removed = before - pendingSkillHits_.size();
  }
  if (removed > 0) {
    // #region agent log
    agentDebugLog("H-CAST-DELAY", "CombatCoreEngine.cpp:cancelPendingSkillHit", "skill_hit_cancelled",
                  std::string("{\"playerId\":") + std::to_string(sourcePlayerId) +
                      ",\"removed\":" + std::to_string(removed) + "}",
                  "post-fix");
    // #endregion
  }
}

void CombatCoreEngine::commitPendingSkillHitsNow(uint32_t sourcePlayerId) {
  if (sourcePlayerId == 0) return;
  std::vector<PendingSkillHit> due;
  {
    std::lock_guard<std::mutex> lock(pendingSkillHitsMu_);
    for (auto it = pendingSkillHits_.begin(); it != pendingSkillHits_.end();) {
      if (it->sourcePlayerId == sourcePlayerId) {
        due.push_back(*it);
        it = pendingSkillHits_.erase(it);
      } else {
        ++it;
      }
    }
  }
  for (const PendingSkillHit& hit : due) {
    // #region agent log
    agentDebugLog("H-CAST-DELAY", "CombatCoreEngine.cpp:commitPendingSkillHitsNow",
                  "skill_hit_committed",
                  std::string("{\"playerId\":") + std::to_string(hit.sourcePlayerId) +
                      ",\"skillId\":" + std::to_string(hit.payload.skillId) + "}",
                  "post-fix");
    // #endregion
    finalizeSkillCastHit(hit.sourcePlayerId, hit.payload);
  }
}

void CombatCoreEngine::scheduleSkillHit(uint32_t sourcePlayerId, const SkillCastPayload& payload,
                                        uint32_t delayMs) {
  PendingSkillHit hit;
  hit.sourcePlayerId = sourcePlayerId;
  hit.payload = payload;
  hit.executeAtMs = agentNowMs() + static_cast<int64_t>(delayMs);
  {
    std::lock_guard<std::mutex> lock(pendingSkillHitsMu_);
    pendingSkillHits_.push_back(hit);
  }
  // #region agent log
  agentDebugLog("H-CAST-DELAY", "CombatCoreEngine.cpp:scheduleSkillHit", "skill_hit_deferred",
                std::string("{\"playerId\":") + std::to_string(sourcePlayerId) +
                    ",\"skillId\":" + std::to_string(payload.skillId) +
                    ",\"delayMs\":" + std::to_string(delayMs) +
                    ",\"executeAtMs\":" + std::to_string(hit.executeAtMs) + "}",
                "post-fix");
  // #endregion
}

void CombatCoreEngine::tickPendingSkillHits() {
  const int64_t now = agentNowMs();
  std::vector<PendingSkillHit> due;
  size_t deferredOverdue = 0;
  {
    std::lock_guard<std::mutex> lock(pendingSkillHitsMu_);
    // Stagger: no máximo 1 hit overdue por jogador por tick (~3ms). Se o tick
    // atrasar, vários hits vencem juntos; disparar todos de uma vez = cascata.
    // Os restantes ficam para o próximo pumpInbound.
    std::unordered_set<uint32_t> firedPlayer;
    for (auto it = pendingSkillHits_.begin(); it != pendingSkillHits_.end();) {
      if (it->executeAtMs <= now) {
        if (firedPlayer.count(it->sourcePlayerId) > 0) {
          ++deferredOverdue;
          ++it;
          continue;
        }
        firedPlayer.insert(it->sourcePlayerId);
        due.push_back(*it);
        it = pendingSkillHits_.erase(it);
      } else {
        ++it;
      }
    }
  }
  // #region agent log
  if (due.size() >= 2 || deferredOverdue > 0) {
    int64_t maxLate = 0;
    for (const PendingSkillHit& hit : due) {
      maxLate = std::max(maxLate, now - hit.executeAtMs);
    }
    agentDebugLog("H-PENDING-FLUSH", "CombatCoreEngine.cpp:tickPendingSkillHits",
                  "pending_hit_flush",
                  std::string("{\"due\":") + std::to_string(due.size()) +
                      ",\"staggerLeft\":" + std::to_string(deferredOverdue) +
                      ",\"maxLateMs\":" + std::to_string(maxLate) + "}",
                  "post-fix");
  }
  // #endregion
  for (const PendingSkillHit& hit : due) {
    // #region agent log
    agentDebugLog("H-CAST-DELAY", "CombatCoreEngine.cpp:tickPendingSkillHits", "skill_hit_fire",
                  std::string("{\"playerId\":") + std::to_string(hit.sourcePlayerId) +
                      ",\"skillId\":" + std::to_string(hit.payload.skillId) +
                      ",\"lateMs\":" + std::to_string(now - hit.executeAtMs) + "}",
                  "post-fix");
    // #endregion
    finalizeSkillCastHit(hit.sourcePlayerId, hit.payload);
  }
}

namespace {
thread_local bool g_skillCastHitOnly = false;
}

void CombatCoreEngine::tickRegen(float deltaSeconds) {
  regenTickAccum_ += deltaSeconds;
  if (regenTickAccum_ < kRegenIntervalSeconds) return;
  regenTickAccum_ = 0.f;

  if (!movementServer_ || !stateLoader_) return;

  // #region agent log
  const int64_t regenT0 = agentNowMs();
  int cacheHits = 0;
  int cacheLoads = 0;
  int updated = 0;
  // #endregion
  const auto players = movementServer_->getPlayerStates();
  for (const auto& [playerId, state] : players) {
    if (state.isDead) continue;

    Combat::CharacterState st;
    // NUNCA loadPlayerState/DB aqui: um cache miss no Proxmox mediu 2829ms e congelou o tick.
    // Sem cache → pula este tick; onPlayerJoinedZone / combate reaquecem o estado.
    if (!stateLoader_->tryGetCachedState(playerId, st)) {
      continue;
    }
    // #region agent log
    ++cacheHits;
    // #endregion

    const int32_t curHealth = st.buffedStats.currentHealth;
    const int32_t curMana = st.buffedStats.currentMana;
    const int32_t maxHealth = std::max(1, st.buffedStats.maxHealth);
    const int32_t maxMana = std::max(1, st.buffedStats.maxMana);

    // Mortos (health<=0) nao regeneram; respawn cuida disso.
    if (curHealth <= 0) continue;

    const int32_t healthRegen = std::max(1, static_cast<int32_t>(maxHealth * kRegenHealthFraction));
    const int32_t manaRegen = std::max(1, static_cast<int32_t>(maxMana * kRegenManaFraction));
    const int32_t newHealth = std::min(maxHealth, curHealth + healthRegen);
    const int32_t newMana = std::min(maxMana, curMana + manaRegen);

    if (newHealth == curHealth && newMana == curMana) continue;  // nada mudou

    // Cache imediato + write-behind (não bloqueia o tick da zone no MySQL remoto).
    stateLoader_->patchCachedHealth(playerId, newHealth);
    stateLoader_->patchCachedMana(playerId, newMana);
    enqueueDbWrite("UPDATE players SET health = " + std::to_string(newHealth) +
                   ", mana = " + std::to_string(newMana) +
                   " WHERE id = " + std::to_string(playerId));
    broadcastPlayerVitals(playerId, newHealth, newMana);
    // #region agent log
    ++updated;
    // #endregion
  }
  // #region agent log
  // Só loga se custou algo ou houve update — evita I/O a cada 2s no idle (congelava a zone).
  if (agentNowMs() - regenT0 >= 5 || updated > 0 || cacheLoads > 0) {
    agentDebugLog("H-B", "CombatCoreEngine.cpp:tickRegen", "regen_tick_ms",
                  std::string("{\"ms\":") + std::to_string(agentNowMs() - regenT0) +
                      ",\"players\":" + std::to_string(players.size()) +
                      ",\"cacheHits\":" + std::to_string(cacheHits) +
                      ",\"cacheLoads\":" + std::to_string(cacheLoads) +
                      ",\"updated\":" + std::to_string(updated) + "}");
  }
  // #endregion
}

void CombatCoreEngine::tickBuffExpirations() {
  std::vector<PlayerBuffInstance> expired;
  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
  {
    std::lock_guard<std::mutex> lock(playerBuffsMu_);
    if (playerBuffs_.empty()) return;
    auto it = playerBuffs_.begin();
    while (it != playerBuffs_.end()) {
      if (it->expiresAtMs > 0 && it->expiresAtMs <= nowMs) {
        expired.push_back(*it);
        it = playerBuffs_.erase(it);
      } else {
        ++it;
      }
    }
  }
  // #region agent log
  if (!expired.empty()) {
    agentDebugLog("H-C", "CombatCoreEngine.cpp:tickBuffExpirations", "buff_expire_ms",
                  std::string("{\"ms\":0,\"expired\":") + std::to_string(expired.size()) +
                      ",\"thread\":\"mem\"}",
                  "post-fix");
  }
  // #endregion
  for (const auto& entry : expired) {
    const char* buffTypeStr = "BUFF";
    if (entry.buffType == static_cast<uint8_t>(Combat::BuffType::DEBUFF)) buffTypeStr = "DEBUFF";
    else if (entry.buffType == static_cast<uint8_t>(Combat::BuffType::SHIELD)) buffTypeStr = "SHIELD";
    else if (entry.buffType == static_cast<uint8_t>(Combat::BuffType::AURA)) buffTypeStr = "AURA";
    auto escapeSql = [](std::string s) {
      std::string out;
      for (char c : s) {
        if (c == '\\' || c == '\'') out.push_back('\\');
        out.push_back(c);
      }
      return out;
    };
    const std::string statKey = entry.targetStat.empty() ? "_default" : entry.targetStat;
    enqueueDbWrite(
        "DELETE FROM active_buffs WHERE target_player_id = " + std::to_string(entry.targetPlayerId) +
        " AND skill_id = " + std::to_string(entry.skillId) + " AND buff_type = '" + buffTypeStr +
        "' AND COALESCE(JSON_UNQUOTE(JSON_EXTRACT(snapshot_json, '$.target_stat')), '_default') = '" +
        escapeSql(statKey) + "'");
    SkillBuffSyncPayload sync;
    sync.action = 1;
    sync.targetPlayerId = entry.targetPlayerId;
    sync.buffId = entry.buffId;
    sync.skillId = entry.skillId;
    sync.buffType = entry.buffType;
    sync.skillName = entry.skillName;
    sync.iconPath = entry.iconPath;
    sync.targetStat = entry.targetStat;
    sync.effectType = entry.effectTypeStr;
    enrichSkillBuffSyncPayload(sync);
    broadcastSkillBuffSync(sync);
  }
}

uint64_t CombatCoreEngine::applyPlayerBuffInMemory(uint32_t targetPlayerId, uint32_t sourcePlayerId,
                                                   uint32_t skillId,
                                                   const Combat::SkillEffect& effect,
                                                   const Combat::SkillData& skill,
                                                   uint8_t buffTypeCode) {
  if (targetPlayerId == 0) return 0;

  uint32_t durationMs = effect.durationMs > 0 ? effect.durationMs : skill.durationMs;
  if (durationMs == 0) durationMs = 5000;
  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
  const std::string targetStat = resolveBuffTargetStat(effect);
  const int32_t valueSnapshot = effect.valueFlat != 0 ? effect.valueFlat : effect.valuePercent;

  const char* buffTypeStr = "BUFF";
  if (buffTypeCode == static_cast<uint8_t>(Combat::BuffType::DEBUFF)) buffTypeStr = "DEBUFF";
  else if (buffTypeCode == static_cast<uint8_t>(Combat::BuffType::SHIELD)) buffTypeStr = "SHIELD";
  else if (buffTypeCode == static_cast<uint8_t>(Combat::BuffType::AURA)) buffTypeStr = "AURA";

  const char* effectTypeStr = effectTypeToWireName(effect.effectType);

  uint64_t buffId = 0;
  uint8_t stacks = 1;
  {
    std::lock_guard<std::mutex> lock(playerBuffsMu_);
    // Refresh mesma skill+stat no alvo.
    for (auto& existing : playerBuffs_) {
      if (existing.targetPlayerId == targetPlayerId && existing.skillId == skillId &&
          existing.targetStat == targetStat && existing.buffType == buffTypeCode) {
        buffId = existing.buffId;
        if (skill.isStackable && existing.stacks < skill.maxStacks) {
          existing.stacks =
              static_cast<uint8_t>(std::min<int>(existing.stacks + 1, skill.maxStacks));
        }
        stacks = existing.stacks;
        existing.valueFlat = effect.valueFlat;
        existing.valuePercent = effect.valuePercent;
        existing.expiresAtMs = nowMs + static_cast<int64_t>(durationMs);
        existing.durationMs = durationMs;
        existing.sourcePlayerId = sourcePlayerId;
        existing.effectTypeStr = effectTypeStr;
        break;
      }
    }
    if (buffId == 0) {
      buffId = (static_cast<uint64_t>(targetPlayerId) << 32) |
               static_cast<uint64_t>(playerBuffIdSeq_.fetch_add(1, std::memory_order_relaxed));
      PlayerBuffInstance inst;
      inst.targetPlayerId = targetPlayerId;
      inst.sourcePlayerId = sourcePlayerId;
      inst.skillId = skillId;
      inst.buffId = buffId;
      inst.buffType = buffTypeCode;
      inst.stacks = 1;
      inst.valueFlat = effect.valueFlat;
      inst.valuePercent = effect.valuePercent;
      inst.expiresAtMs = nowMs + static_cast<int64_t>(durationMs);
      inst.durationMs = durationMs;
      inst.targetStat = targetStat;
      inst.skillName = skill.skillName;
      inst.iconPath = skill.iconPath;
      inst.effectTypeStr = effectTypeStr;
      playerBuffs_.push_back(inst);
    }
  }

  // Persistência write-behind (não bloqueia combat worker / zone).
  auto escapeSql = [](std::string s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
      if (c == '\\' || c == '\'') out.push_back('\\');
      out.push_back(c);
    }
    return out;
  };
  const std::string snapshotJson =
      std::string("{\"target_stat\":\"") + escapeSql(targetStat) +
      "\",\"value_flat\":" + std::to_string(effect.valueFlat) +
      ",\"value_percent\":" + std::to_string(effect.valuePercent) +
      ",\"effect_type\":\"" + effectTypeStr + "\"}";

  enqueueDbWrite(
      "DELETE FROM active_buffs WHERE target_player_id = " + std::to_string(targetPlayerId) +
      " AND skill_id = " + std::to_string(skillId) +
      " AND buff_type = '" + buffTypeStr +
      "' AND COALESCE(JSON_UNQUOTE(JSON_EXTRACT(snapshot_json, '$.target_stat')), '_default') = '" +
      escapeSql(targetStat.empty() ? "_default" : targetStat) + "'");
  enqueueDbWrite(
      "INSERT INTO active_buffs (target_player_id, source_player_id, skill_id, buff_type, "
      "current_stacks, value_snapshot, expires_at, snapshot_json) VALUES (" +
      std::to_string(targetPlayerId) + ", " + std::to_string(sourcePlayerId) + ", " +
      std::to_string(skillId) + ", '" + buffTypeStr + "', " + std::to_string(static_cast<unsigned>(stacks)) +
      ", " + std::to_string(valueSnapshot) +
      ", DATE_ADD(NOW(3), INTERVAL " + std::to_string(static_cast<uint64_t>(durationMs) * 1000ULL) +
      " MICROSECOND), '" + escapeSql(snapshotJson) + "')");

  Core::Logger::getInstance().debug(
      "[CombatCoreEngine] buff player (mem): target={} skill={} type={} stacks={} id={}",
      targetPlayerId, skillId, buffTypeStr, static_cast<unsigned>(stacks), buffId);
  return buffId;
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
    const bool casterResistBuff = isCasterCcResistBuff(eff);

    if (isBuffStat || isDebuffStat || isShield || isCcDebuff) {
      if (!casterResistBuff && effectPlayerId == 0) continue;
      int32_t applyChance = static_cast<int32_t>(eff.chancePercent);
      if (isCcDebuff) {
        int32_t targetResist = 0;
        int32_t casterCcChance = 0;
        if (haveAttacker) {
          casterCcChance = ccChanceFromStats(attacker.buffedStats, eff.effectType);
        }
        Combat::CharacterState defender;
        bool defenderIsPlayer = false;
        if (effectOnSelf) {
          if (haveAttacker) {
            targetResist = ccResistFromStats(attacker.buffedStats, eff.effectType);
          }
        } else if (buildDefenderState(targetIsNpc ? static_cast<uint8_t>(CombatTargetType::Npc)
                                                  : static_cast<uint8_t>(CombatTargetType::Player),
                                      targetIsNpc ? targetId : effectPlayerId, defender,
                                      defenderIsPlayer)) {
          targetResist = ccResistFromStats(defender.buffedStats, eff.effectType);
        }
        applyChance = computeCcApplyChance(eff, targetResist, casterCcChance);
      }
      if (applyChance < 100) {
        const int32_t roll = std::rand() % 100;
        if (roll >= applyChance) continue;
      }

      if (!effectOnSelf && targetIsNpc && !casterResistBuff) {
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
          sync.targetStat = resolveBuffTargetStat(eff);
          sync.effectType = effectTypeToWireName(eff.effectType);
          sync.skillName = skill.skillName;
          sync.iconPath = skill.iconPath;
          enrichSkillBuffSyncPayload(sync);
          broadcastSkillBuffSync(sync);
        }
        continue;
      }

      if (!skillService_) continue;
      uint8_t buffTypeCode = static_cast<uint8_t>(Combat::BuffType::BUFF);
      if (isShield) {
        buffTypeCode = static_cast<uint8_t>(Combat::BuffType::SHIELD);
      } else if (isDebuffStat || isCcDebuff) {
        buffTypeCode = static_cast<uint8_t>(Combat::BuffType::DEBUFF);
      }
      const uint32_t buffPlayerId = casterResistBuff ? sourcePlayerId : effectPlayerId;
      if (buffPlayerId == 0) continue;
      const uint64_t buffId =
          applyPlayerBuffInMemory(buffPlayerId, sourcePlayerId, skill.skillId, eff, skill,
                                  buffTypeCode);
      if (buffId > 0) {
        uint32_t durationMs = eff.durationMs > 0 ? eff.durationMs : skill.durationMs;
        if (durationMs == 0) durationMs = 5000;
        const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::system_clock::now().time_since_epoch())
                                  .count();
        SkillBuffSyncPayload sync;
        sync.targetPlayerId = buffPlayerId;
        sync.buffId = buffId;
        sync.skillId = skill.skillId;
        sync.buffType = buffTypeCode;
        sync.stacks = 1;
        sync.valueFlat = eff.valueFlat;
        sync.valuePercent = eff.valuePercent;
        sync.expiresAtMs = nowMs + static_cast<int64_t>(durationMs);
        sync.durationMs = durationMs;
        sync.targetStat = resolveBuffTargetStat(eff);
        sync.effectType = effectTypeToWireName(eff.effectType);
        sync.skillName = skill.skillName;
        sync.iconPath = skill.iconPath;
        broadcastPlayerSkillBuffApply(sync);
      } else {
        Core::Logger::getInstance().warn(
            "[CombatCoreEngine] applyPlayerBuffInMemory retornou 0: skill={} target={} effectType={}",
            skill.skillId, buffPlayerId, static_cast<int>(eff.effectType));
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
      Core::Logger::getInstance().debug(
          "[CombatCoreEngine] DOT/HOT NPC aplicado: npc={} src={} skill={} tick={} ticks={} interval={}ms",
          targetId, sourcePlayerId, skill.skillId, inst.tickValue, ticksTotal, interval);
    }
  }
}

void CombatCoreEngine::insertPlayerDot(uint32_t sourcePlayerId, uint32_t targetPlayerId,
                                       uint32_t skillId, const char* dotType, int32_t tickValue,
                                       uint32_t tickIntervalMs, uint32_t ticksTotal,
                                       const Combat::SkillData& skill) {
  if (targetPlayerId == 0 || tickValue <= 0 || ticksTotal == 0) return;

  const uint32_t interval = std::max<uint32_t>(200, tickIntervalMs);
  const uint8_t ticks = static_cast<uint8_t>(std::min<uint32_t>(255, ticksTotal));
  const uint32_t durationMs = interval * ticks;
  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
  const bool isHeal = (dotType && std::strcmp(dotType, "HEAL") == 0);
  const uint64_t dotBuffId =
      (static_cast<uint64_t>(targetPlayerId) << 32) |
      static_cast<uint64_t>(playerDotIdSeq_.fetch_add(1, std::memory_order_relaxed));

  PlayerDotInstance inst;
  inst.targetPlayerId = targetPlayerId;
  inst.sourcePlayerId = sourcePlayerId;
  inst.skillId = skillId;
  inst.dotBuffId = dotBuffId;
  inst.tickValue = tickValue;
  inst.isHeal = isHeal;
  inst.intervalMs = interval;
  inst.ticksRemaining = ticks;
  inst.expiresAtMs = nowMs + static_cast<int64_t>(durationMs);
  inst.durationMs = durationMs;
  inst.nextTickAt = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);

  {
    std::lock_guard<std::mutex> lock(playerDotsMu_);
    // Substitui DOT/HOT da mesma skill no mesmo alvo (refresh).
    playerDots_.erase(std::remove_if(playerDots_.begin(), playerDots_.end(),
                                     [&](const PlayerDotInstance& existing) {
                                       return existing.targetPlayerId == targetPlayerId &&
                                              existing.skillId == skillId;
                                     }),
                      playerDots_.end());
    playerDots_.push_back(inst);
  }

  // Persistência write-behind (ZoneCombatService não faz mais poll no tick).
  const uint32_t intervalSec = std::max<uint32_t>(1, (interval + 999) / 1000);
  const uint32_t totalDurationSec = intervalSec * ticks;
  enqueueDbWrite(
      "INSERT INTO active_dots ("
      "target_player_id, source_player_id, skill_id, dot_type, tick_value, tick_interval_ms, "
      "ticks_remaining, next_tick_at, expires_at) VALUES (" +
      std::to_string(targetPlayerId) + ", " + std::to_string(sourcePlayerId) + ", " +
      std::to_string(skillId) + ", '" + (isHeal ? "HEAL" : "DAMAGE") + "', " +
      std::to_string(tickValue) + ", " + std::to_string(interval) + ", " +
      std::to_string(static_cast<unsigned>(ticks)) +
      ", DATE_ADD(NOW(3), INTERVAL " + std::to_string(static_cast<uint64_t>(interval) * 1000ULL) +
      " MICROSECOND), DATE_ADD(NOW(3), INTERVAL " + std::to_string(totalDurationSec) +
      " SECOND))");

  if (movementServer_) {
    SkillBuffSyncPayload sync;
    sync.targetPlayerId = targetPlayerId;
    sync.buffId = dotBuffId;
    sync.skillId = skillId;
    sync.buffType = static_cast<uint8_t>(isHeal ? Combat::BuffType::HOT : Combat::BuffType::DOT);
    sync.stacks = ticks;
    sync.valueFlat = tickValue;
    sync.expiresAtMs = inst.expiresAtMs;
    sync.durationMs = durationMs;
    sync.skillName = skill.skillName;
    sync.iconPath = skill.iconPath;
    broadcastPlayerSkillBuffApply(sync);
  }

  Core::Logger::getInstance().debug(
      "[CombatCoreEngine] DOT/HOT player (mem): target={} src={} skill={} type={} tick={} ticks={} id={}",
      targetPlayerId, sourcePlayerId, skillId, isHeal ? "HEAL" : "DAMAGE", tickValue,
      static_cast<unsigned>(ticks), dotBuffId);
}

void CombatCoreEngine::tickPlayerDots() {
  if (!movementServer_) return;

  std::vector<PlayerDotInstance> due;
  std::vector<PlayerDotInstance> expiredDots;
  const auto now = std::chrono::steady_clock::now();
  {
    std::lock_guard<std::mutex> lock(playerDotsMu_);
    if (playerDots_.empty()) return;
    for (auto& inst : playerDots_) {
      while (inst.ticksRemaining > 0 && inst.nextTickAt <= now) {
        due.push_back(inst);
        inst.ticksRemaining--;
        inst.nextTickAt += std::chrono::milliseconds(inst.intervalMs);
        if (inst.ticksRemaining == 0) {
          expiredDots.push_back(inst);
        }
      }
    }
    playerDots_.erase(std::remove_if(playerDots_.begin(), playerDots_.end(),
                                     [](const PlayerDotInstance& i) { return i.ticksRemaining == 0; }),
                      playerDots_.end());
  }

  for (const auto& inst : expiredDots) {
    if (inst.dotBuffId == 0) continue;
    SkillBuffSyncPayload sync;
    sync.action = 1;
    sync.targetType = 0;
    sync.targetPlayerId = inst.targetPlayerId;
    sync.buffId = inst.dotBuffId;
    sync.skillId = inst.skillId;
    sync.buffType =
        static_cast<uint8_t>(inst.isHeal ? Combat::BuffType::HOT : Combat::BuffType::DOT);
    sync.expiresAtMs = inst.expiresAtMs;
    sync.durationMs = inst.durationMs;
    enrichSkillBuffSyncPayload(sync);
    broadcastSkillBuffSync(sync);
  }

  for (const auto& inst : due) {
    const int32_t delta = inst.isHeal ? inst.tickValue : -inst.tickValue;
    applyPlayerDamage(inst.sourcePlayerId, inst.targetPlayerId, delta,
                      static_cast<uint8_t>(CombatReason::Dot), false, false);
  }
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
  inst.targetStat = resolveBuffTargetStat(eff);
  inst.skillName = skill.skillName;
  inst.iconPath = skill.iconPath;
  inst.effectTypeStr = effectTypeToWireName(eff.effectType);

  std::vector<NpcBuffInstance> replaced;
  {
    std::lock_guard<std::mutex> lock(npcBuffsMu_);
    for (const auto& existing : npcBuffs_) {
      if (existing.npcInstanceId == npcInstanceId && existing.skillId == skillId &&
          existing.buffType == buffType && existing.targetStat == inst.targetStat) {
        replaced.push_back(existing);
      }
    }
    npcBuffs_.erase(std::remove_if(npcBuffs_.begin(), npcBuffs_.end(),
                                   [&](const NpcBuffInstance& existing) {
                                     return existing.npcInstanceId == npcInstanceId &&
                                            existing.skillId == skillId &&
                                            existing.buffType == buffType &&
                                            existing.targetStat == inst.targetStat;
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
    sync.effectType = old.effectTypeStr;
    enrichSkillBuffSyncPayload(sync);
    broadcastSkillBuffSync(sync);
  }

  Core::Logger::getInstance().debug(
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
    sync.effectType = inst.effectTypeStr;
    enrichSkillBuffSyncPayload(sync);
    broadcastSkillBuffSync(sync);
    Core::Logger::getInstance().debug(
        "[CombatCoreEngine] buff NPC expirado npc={} buff_id={} skill={}", inst.npcInstanceId,
        inst.buffId, inst.skillId);
  }
}

bool CombatCoreEngine::reloadSkills() {
  if (!skillService_) return false;
  const bool ok = skillService_->reloadSkills();
  if (ok) {
    preloadSkillAnimPaths();
    loadBasicAttacks();
    Core::Logger::getInstance().info("[CombatCoreEngine] skills recarregadas do DB");
  }
  return ok;
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

bool CombatCoreEngine::despawnNpcInstance(uint32_t npcInstanceId, uint8_t reason) {
  if (!npcManager_ || npcInstanceId == 0) return false;
  if (!npcManager_->findInstance(npcInstanceId)) return false;
  broadcastNpcDespawnToAll(npcInstanceId, reason);
  return npcManager_->removeInstance(npcInstanceId);
}

bool CombatCoreEngine::moveNpcInstance(uint32_t npcInstanceId, float x, float y, float z, float yaw) {
  if (!npcManager_ || npcInstanceId == 0) return false;

  if (!npcManager_->findInstance(npcInstanceId)) {
    if (!npcManager_->loadInstanceById(npcInstanceId)) {
      return false;
    }
  }

  if (!npcManager_->setInstanceTransform(npcInstanceId, x, y, z, yaw, true, true)) {
    return false;
  }

  if (const NpcRuntimeInstance* inst = npcManager_->findInstance(npcInstanceId)) {
    if (inst->isDead) {
      Core::Logger::getInstance().warn("[CombatCoreEngine] move: NPC {} está morto — posição atualizada, sem broadcast spawn",
                                       npcInstanceId);
      return true;
    }
    broadcastNpcSpawnToAll(*inst);
    Core::Logger::getInstance().info("[CombatCoreEngine] move NPC {} -> ({:.1f},{:.1f},{:.1f}) yaw={:.1f}",
                                     npcInstanceId, x, y, z, yaw);
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

size_t CombatCoreEngine::reloadAllNpcInstancesFromDatabase() {
  if (!npcManager_) return 0;

  std::vector<uint32_t> oldIds;
  oldIds.reserve(npcManager_->getAllInstances().size());
  for (const auto& inst : npcManager_->getAllInstances()) {
    oldIds.push_back(inst.npcInstanceId);
  }
  for (uint32_t id : oldIds) {
    broadcastNpcDespawnToAll(id, 0);
  }

  if (!npcManager_->reloadFromDatabase()) {
    return 0;
  }

  size_t spawned = 0;
  for (const auto& inst : npcManager_->getAllInstances()) {
    if (inst.isDead) continue;
    broadcastNpcSpawnToAll(inst);
    ++spawned;
  }
  Core::Logger::getInstance().info(
      "[CombatCoreEngine] reloadAllNpcInstances: despawned={} spawned_alive={}", oldIds.size(),
      spawned);
  return spawned;
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
    if (lootService_ && killerPlayerId > 0) {
      const NpcRuntimeInstance* victim = npcManager_->findInstance(npcInstanceId);
      if (victim) {
        lootService_->onNpcKilled(killerPlayerId, victim->npcInstanceId, victim->templateId,
                                   victim->zoneId, victim->x, victim->y, victim->z);
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

    uint16_t deathMs = 0;
    bool hasDeathAnim = false;
    if (const NpcRuntimeInstance* victim = npcManager_->findInstance(npcInstanceId)) {
      hasDeathAnim = !victim->deathAnimPath.empty();
      deathMs = victim->deathDurationMs;
      if (hasDeathAnim && deathMs == 0) deathMs = 1500;
      if (deathMs > 0 && deathMs < 500) deathMs = 500;
      if (deathMs > 5000) deathMs = 5000;
      broadcastNpcState(npcManager_->toStatePayload(*victim));
    }

    if (hasDeathAnim && deathMs > 0) {
      npcManager_->mutateInstance(npcInstanceId, [deathMs](NpcRuntimeInstance& inst) {
        inst.pendingDeathDespawn = true;
        inst.deathDespawnAt =
            std::chrono::steady_clock::now() + std::chrono::milliseconds(deathMs);
        inst.aiState = NpcAiState::Dying;
      });
      return;
    }

    broadcastNpcDespawnToAll(npcInstanceId, 1);
    return;
  }
  // Aggro por dano: hostil passa a perseguir o atacante.
  if (killerPlayerId > 0 && npcManager_) {
    npcManager_->setAggroTarget(npcInstanceId, killerPlayerId);
  }
  if (const NpcRuntimeInstance* inst = npcManager_->findInstance(npcInstanceId)) {
    broadcastNpcState(npcManager_->toStatePayload(*inst));
  }
}

void CombatCoreEngine::sendNpcSnapshotToClient(uint32_t clientId) {
  if (!movementServer_ || !npcManager_) return;
  size_t sent = 0;
  size_t skippedDead = 0;
  for (const auto& inst : npcManager_->getAllInstances()) {
    if (inst.isDead) {
      ++skippedDead;
      continue;
    }
    movementServer_->sendBinaryToClient(clientId, encodeNpcSpawnNotify(npcManager_->toSpawnPayload(inst)));
    ++sent;
  }
  // #region agent log
  agentDebugLog("H-NPC", "CombatCoreEngine.cpp:sendNpcSnapshotToClient", "npc_snapshot_sent",
                std::string("{\"clientId\":") + std::to_string(clientId) +
                    ",\"sent\":" + std::to_string(sent) +
                    ",\"skippedDead\":" + std::to_string(skippedDead) + "}",
                "mid-phase");
  // #endregion
  Core::Logger::getInstance().info(
      "[CombatCoreEngine] NpcSnapshot client={} sent={} skippedDead={}", clientId, sent,
      skippedDead);
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
    sync.effectType = inst.effectTypeStr;
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
          if (snap.contains("effect_type") && snap["effect_type"].is_string()) {
            sync.effectType = snap["effect_type"].get<std::string>();
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

void CombatCoreEngine::preloadSkillAnimPaths() {
  if (!db_ || !db_->isConnected()) return;

  auto rows = db_->executePreparedQuery(
      "SELECT skill_id, COALESCE(cast_anim_path,''), COALESCE(vfx_path,''), COALESCE(sfx_path,'') "
      "FROM skills WHERE is_enabled = 1",
      {});
  std::lock_guard<std::mutex> lock(skillAnimCacheMu_);
  size_t n = 0;
  for (const auto& row : rows) {
    if (row.size() < 4) continue;
    try {
      const uint32_t skillId = static_cast<uint32_t>(std::stoul(row[0]));
      skillAnimCache_[skillId] = SkillAnimPaths{row[1], row[2], row[3]};
      ++n;
    } catch (...) {
      continue;
    }
  }
  Core::Logger::getInstance().info("[CombatCoreEngine] skill anim paths preloaded: {}", n);
}

void CombatCoreEngine::loadSkillAnimPaths(uint32_t skillId,
                                          std::string& anim, std::string& vfx, std::string& sfx) {
  anim.clear();
  vfx.clear();
  sfx.clear();

  // Sempre relê o DB: o Manager altera cast_anim_path sem reiniciar a zone.
  if (db_ && db_->isConnected()) {
    auto rows = db_->executePreparedQuery(
        "SELECT COALESCE(cast_anim_path,''), COALESCE(vfx_path,''), COALESCE(sfx_path,'') "
        "FROM skills WHERE skill_id = ? LIMIT 1",
        {std::to_string(skillId)});
    if (!rows.empty() && rows[0].size() >= 3) {
      anim = rows[0][0];
      vfx = rows[0][1];
      sfx = rows[0][2];
      std::lock_guard<std::mutex> lock(skillAnimCacheMu_);
      skillAnimCache_[skillId] = SkillAnimPaths{anim, vfx, sfx};
      return;
    }
  }

  std::lock_guard<std::mutex> lock(skillAnimCacheMu_);
  auto it = skillAnimCache_.find(skillId);
  if (it != skillAnimCache_.end()) {
    anim = it->second.anim;
    vfx = it->second.vfx;
    sfx = it->second.sfx;
  }
}

std::string CombatCoreEngine::resolveBasicAttackAnimPath(uint32_t classId,
                                                       const std::string& fallback) {
  if (db_ && db_->isConnected()) {
    auto rows = db_->executePreparedQuery(
        "SELECT COALESCE(cast_anim_path,'') FROM skills "
        "WHERE class_id = ? AND is_basic_attack = 1 AND is_enabled = 1 LIMIT 1",
        {std::to_string(classId)});
    if (!rows.empty() && !rows[0].empty() && !rows[0][0].empty()) {
      return rows[0][0];
    }
  }
  return fallback;
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
  // Cache-first no hot path: o rank de todas as skills aprendidas é carregado no
  // warm do CharacterState (skillRanks). Assim o worker não faz SELECT por cast.
  if (stateLoader_) {
    Combat::CharacterState st;
    if (stateLoader_->tryGetCachedState(playerId, st)) {
      auto rit = st.skillRanks.find(skillId);
      if (rit != st.skillRanks.end()) {
        return std::max<uint8_t>(1, rit->second);
      }
    }
  }

  const uint64_t key = (static_cast<uint64_t>(playerId) << 32) | skillId;
  const auto now = std::chrono::steady_clock::now();
  {
    std::lock_guard<std::mutex> lock(rankCacheMu_);
    auto it = rankCache_.find(key);
    if (it != rankCache_.end() && now < it->second.expiresAt) {
      return it->second.rank;
    }
  }

  if (!db_ || !db_->isConnected()) return 1;
  auto opt = db_->executePreparedScalar(
      "SELECT current_rank FROM player_skills WHERE player_id = ? AND skill_id = ? LIMIT 1",
      {std::to_string(playerId), std::to_string(skillId)});
  uint8_t rank = 1;
  if (opt && !opt->empty()) {
    try {
      rank = static_cast<uint8_t>(std::max(1, std::stoi(*opt)));
    } catch (...) {
      rank = 1;
    }
  }
  {
    std::lock_guard<std::mutex> lock(rankCacheMu_);
    rankCache_[key] = CachedRank{rank, now + std::chrono::milliseconds(kRankCacheTtlMs)};
  }
  return rank;
}

void CombatCoreEngine::deductPlayerMana(uint32_t playerId, int32_t cost,
                                        int32_t knownCurrentHealth, int32_t knownCurrentMana) {
  if (!db_ || !db_->isConnected() || cost <= 0) return;
  // Write-behind: persiste a mana em background (valores numéricos, injection-safe).
  enqueueDbWrite("UPDATE players SET mana = GREATEST(0, mana - " + std::to_string(cost) +
                 ") WHERE id = " + std::to_string(playerId));

  int32_t newMana = knownCurrentMana;
  if (knownCurrentMana >= 0) {
    newMana = std::max(0, knownCurrentMana - cost);
    if (stateLoader_) stateLoader_->patchCachedMana(playerId, newMana);
  }

  // Sincroniza a mana no cliente (opcode 87) sem SELECT/reload no caminho quente.
  broadcastPlayerVitals(playerId, knownCurrentHealth, newMana);
}

void CombatCoreEngine::broadcastPlayerVitals(uint32_t playerId, int32_t knownCurrentHealth,
                                             int32_t knownCurrentMana) {
  if (!movementServer_ || playerId == 0) return;

  int32_t curHealth = knownCurrentHealth;
  int32_t curMana = knownCurrentMana;
  int32_t maxHealth = 100;
  int32_t maxMana = 50;
  bool haveMax = false;

  Combat::CharacterState st;
  if (stateLoader_ && stateLoader_->tryGetCachedState(playerId, st)) {
    maxHealth = std::max(1, st.buffedStats.maxHealth);
    maxMana = std::max(1, st.buffedStats.maxMana);
    if (curHealth < 0) curHealth = st.buffedStats.currentHealth;
    if (curMana < 0) curMana = st.buffedStats.currentMana;
    haveMax = true;
  }

  // Fallback DB só se não houver estado em cache / valores conhecidos.
  if ((curHealth < 0 || curMana < 0 || !haveMax) && db_ && db_->isConnected()) {
    const std::string pid = std::to_string(playerId);
    if (curHealth < 0 || curMana < 0) {
      auto healthOpt = db_->executePreparedScalar("SELECT health FROM players WHERE id = ? LIMIT 1", {pid});
      auto manaOpt = db_->executePreparedScalar("SELECT mana FROM players WHERE id = ? LIMIT 1", {pid});
      if (!healthOpt || !manaOpt) return;
      try {
        if (curHealth < 0) curHealth = std::stoi(*healthOpt);
        if (curMana < 0) curMana = std::stoi(*manaOpt);
      } catch (...) {
        return;
      }
    }
    if (!haveMax) {
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
    }
  }

  if (curHealth < 0 || curMana < 0) return;

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
  // Write-behind: combat_log é puramente histórico; nunca deve bloquear o cast.
  // actionType é literal fixo do próprio servidor ("DAMAGE"/"HEAL"/"DOUBLE"), injection-safe.
  enqueueDbWrite(
      std::string("INSERT INTO combat_log (source_player_id, target_player_id, skill_id, "
                  "action_type, value, is_critical, overkill, zone_id) VALUES (") +
      std::to_string(sourcePlayerId) + ", " + std::to_string(targetPlayerId) + ", " +
      std::to_string(skillId) + ", '" + actionType + "', " + std::to_string(value) + ", " +
      (isCrit ? "1" : "0") + ", " + std::to_string(overkill) + ", " + std::to_string(zoneId_) + ")");
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

void CombatCoreEngine::disambiguateTargetType(uint32_t sourcePlayerId, uint8_t& targetType,
                                              uint32_t targetId) const {
  if (targetId == 0 || !npcManager_ || !movementServer_) return;
  const bool asPlayer = (targetType == static_cast<uint8_t>(CombatTargetType::Player));
  const bool asNpc = (targetType == static_cast<uint8_t>(CombatTargetType::Npc));
  if (!asPlayer && !asNpc) return;

  float sx = 0.f, sy = 0.f, sz = 0.f;
  if (!tryGetPlayerPosition(sourcePlayerId, sx, sy, sz)) return;

  float px = 0.f, py = 0.f, pz = 0.f;
  const bool havePlayer = tryGetPlayerPosition(targetId, px, py, pz);
  const bool playerConnected = movementServer_->isPlayerConnected(targetId);
  const bool stubOrigin =
      havePlayer && (std::fabs(px) < 1.f && std::fabs(py) < 1.f && std::fabs(pz) < 1.f);
  const bool playerOnlineValid = playerConnected && havePlayer && !stubOrigin;

  const NpcRuntimeInstance* npc = npcManager_->findInstance(targetId);
  const bool aliveNpc = (npc != nullptr && !npc->isDead);
  const float distNpc =
      aliveNpc ? std::sqrt(distanceSquared2D(sx, sy, npc->x, npc->y)) : 1.0e30f;
  const float distPlayer =
      havePlayer ? std::sqrt(distanceSquared2D(sx, sy, px, py)) : 1.0e30f;

  // Defesa: cliente pode mandar Npc por colisão de ID em PvP. Reescreve para
  // Player se online com pos válida e (NPC ausente OU player claramente mais perto).
  // Mantém Npc quando o NPC está tão perto ou mais perto (PvE real no homônimo).
  if (asNpc && playerOnlineValid) {
    if (!aliveNpc || distPlayer + 200.f < distNpc) {
      Core::Logger::getInstance().info(
          "[CombatCoreEngine] disambiguateTarget id={} Npc->Player "
          "(playerOnline=1 haveNpc={} distP={:.1f} distN={:.1f} "
          "src=({:.0f},{:.0f},{:.0f}) tgt=({:.0f},{:.0f},{:.0f}))",
          targetId, aliveNpc, distPlayer, distNpc, sx, sy, sz, px, py, pz);
      targetType = static_cast<uint8_t>(CombatTargetType::Player);
    }
    return;
  }

  if (!asPlayer || !aliveNpc) return;

  // Player online com pos válida: nunca Player→Npc (IDs colidem com npc_instance).
  if (playerOnlineValid) {
    return;
  }

  // Só reescreve se offline / sem entry / stub (0,0,0) e existe NPC vivo com esse id.
  Core::Logger::getInstance().info(
      "[CombatCoreEngine] disambiguateTarget id={} Player->Npc "
      "(playerOnline={} havePlayer={} stub={} distP={:.1f} distN={:.1f} "
      "src=({:.0f},{:.0f},{:.0f}) tgt=({:.0f},{:.0f},{:.0f}))",
      targetId, playerConnected, havePlayer, stubOrigin, distPlayer, distNpc, sx, sy, sz, px, py,
      pz);
  targetType = static_cast<uint8_t>(CombatTargetType::Npc);
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

  const bool isNpcTarget =
      (payload.targetType == static_cast<uint8_t>(CombatTargetType::Npc));
  const bool isPlayerTarget =
      (payload.targetType == static_cast<uint8_t>(CombatTargetType::Player));
  // PvP e PvE usam XY (toast/reject já mostravam só XY); 3D só para ponto de área no chão.
  const bool useXy = isNpcTarget || isPlayerTarget;
  float npcMoveSpeed = 200.f;
  if (isNpcTarget && npcManager_) {
    if (const NpcRuntimeInstance* npc = npcManager_->findInstance(payload.targetId)) {
      npcMoveSpeed = npc->moveSpeed;
    }
  }
  const float maxR =
      isNpcTarget ? effectiveMaxRangeVsNpc(static_cast<float>(skill.rangeMax), npcMoveSpeed)
                   : effectiveMaxRange(static_cast<float>(skill.rangeMax));
  const float distXy = std::sqrt(distanceSquared2D(sx, sy, tx, ty));
  const float dist3D = std::sqrt(distanceSquared3D(sx, sy, sz, tx, ty, tz));
  const bool inRange =
      useXy ? isInRange2D(sx, sy, tx, ty, maxR) : isInRange3D(sx, sy, sz, tx, ty, tz, maxR);
  if (!inRange) {
    Core::Logger::getInstance().info(
        "[CombatCoreEngine] RANGE_EXCEEDED skill={} player={} targetType={} target={} "
        "src=({:.1f},{:.1f},{:.1f}) tgt=({:.1f},{:.1f},{:.1f}) distXY={:.1f} dist3D={:.1f} "
        "max={:.1f} mode={} distM={:.1f}",
        skill.skillId, sourcePlayerId, static_cast<int>(payload.targetType), payload.targetId, sx,
        sy, sz, tx, ty, tz, distXy, dist3D, maxR, useXy ? "xy" : "3d", distXy / 100.f);
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
  const bool isNpcTarget = (targetType == static_cast<uint8_t>(CombatTargetType::Npc));
  const bool isPlayerTarget = (targetType == static_cast<uint8_t>(CombatTargetType::Player));
  const bool useXy = isNpcTarget || isPlayerTarget;
  float npcMoveSpeed = 200.f;
  if (isNpcTarget && npcManager_) {
    if (const NpcRuntimeInstance* npc = npcManager_->findInstance(targetId)) {
      npcMoveSpeed = npc->moveSpeed;
    }
  }
  const float maxR =
      isNpcTarget ? effectiveMaxRangeVsNpc(static_cast<float>(rangeMax), npcMoveSpeed)
                   : effectiveMaxRange(static_cast<float>(rangeMax));
  const float distXy = std::sqrt(distanceSquared2D(sx, sy, tx, ty));
  const float dist3D = std::sqrt(distanceSquared3D(sx, sy, sz, tx, ty, tz));
  const bool inRange =
      useXy ? isInRange2D(sx, sy, tx, ty, maxR) : isInRange3D(sx, sy, sz, tx, ty, tz, maxR);
  if (!inRange) {
    Core::Logger::getInstance().info(
        "[CombatCoreEngine] RANGE_EXCEEDED basic_attack player={} targetType={} target={} "
        "src=({:.1f},{:.1f},{:.1f}) tgt=({:.1f},{:.1f},{:.1f}) distXY={:.1f} dist3D={:.1f} "
        "max={:.1f} mode={}",
        sourcePlayerId, static_cast<int>(targetType), targetId, sx, sy, sz, tx, ty, tz, distXy,
        dist3D, maxR, useXy ? "xy" : "3d");
    return false;
  }
  return true;
}

bool CombatCoreEngine::buildDefenderState(uint8_t targetType, uint32_t targetId,
                                          Combat::CharacterState& out, bool& outIsPlayer) {
  outIsPlayer = (targetType == static_cast<uint8_t>(CombatTargetType::Player));
  if (outIsPlayer) {
    if (targetId == 0 || !stateLoader_) return false;
    // Cache-only no worker: miss agenda warm assíncrono e aborta este hit (o alvo
    // é aquecido no join, então miss é raro e o próximo cast já acha o cache).
    return stateLoader_->getCachedOrWarm(targetId, out);
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
    movementServer_->broadcastNearPlayer(sourcePlayerId, encodeCombatEventNotify(combat));
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

  // Preferir cache (mesmo “stale” pós-TTL): loadPlayerState após TTL faz JOIN pesado no MySQL
  // remoto e congelava o tick (DOT/regen). Fallback a DB só se não houver entrada.
  int32_t curHealth = -1, maxHealth = 100, curMana = -1, maxMana = 50;
  bool haveState = false;
  if (stateLoader_) {
    Combat::CharacterState st;
    if (stateLoader_->tryGetCachedState(targetPlayerId, st) ||
        stateLoader_->loadPlayerState(targetPlayerId, st)) {
      curHealth = st.buffedStats.currentHealth;
      maxHealth = std::max(1, st.buffedStats.maxHealth);
      curMana = st.buffedStats.currentMana;
      maxMana = std::max(1, st.buffedStats.maxMana);
      haveState = true;
    }
  }
  if (!haveState) {
    auto healthOpt = db_->executePreparedScalar("SELECT health FROM players WHERE id = ? LIMIT 1", {tid});
    auto manaOpt = db_->executePreparedScalar("SELECT mana FROM players WHERE id = ? LIMIT 1", {tid});
    if (!healthOpt || !manaOpt) return false;
    try {
      curHealth = std::stoi(*healthOpt);
      curMana = std::stoi(*manaOpt);
    } catch (...) {
      return false;
    }
  }
  if (curHealth < 0 || curMana < 0) return false;

  if (reactionEngine_ && delta < 0 && !inReactionDispatch_) {
    int32_t adjustedDelta = delta;
    reactionEngine_->onAllyDamaged(targetPlayerId, sourcePlayerId, adjustedDelta);
    delta = adjustedDelta;
    reactionEngine_->onPlayerDamaged(targetPlayerId, sourcePlayerId, delta, isCrit);
  }

  const int32_t clampedHealth = std::max(0, std::min(maxHealth, curHealth + delta));
  const bool deadAfter = (clampedHealth <= 0);

  // Morte: write-behind COM PRIORIDADE (frente da fila). A fonte de verdade para o
  // respawn é o estado em memória (players_[pid].isDead + memorySaysDead), então não
  // precisamos mais bloquear o combat worker com UPDATE síncrono. O enqueueDbWriteFront
  // garante que o is_dead persista antes do backlog de HP (evita respawn/relog lendo
  // estado defasado, que era o motivo do write síncrono original). HP não-letal segue
  // write-behind normal.
  if (deadAfter) {
    const std::string deathSql =
        "UPDATE players SET health = " + std::to_string(clampedHealth) +
        ", is_dead = 1, last_death_at = CURRENT_TIMESTAMP WHERE id = " + tid;
    enqueueDbWriteFront(deathSql);
    // #region agent log
    agentDebugLog("H-RESPAWN", "CombatCoreEngine.cpp:applyPlayerDamage", "death_async_write",
                  std::string("{\"playerId\":") + std::to_string(targetPlayerId) +
                      ",\"health\":" + std::to_string(clampedHealth) + "}",
                  "post-fix");
    // #endregion
  } else {
    enqueueDbWrite("UPDATE players SET health = " + std::to_string(clampedHealth) +
                   ", is_dead = 0 WHERE id = " + tid);
  }

  // Atualiza só o HP no cache (sem invalidate → não força reload de stats no próximo cast).
  if (stateLoader_) stateLoader_->patchCachedHealth(targetPlayerId, clampedHealth);

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
    Core::Logger::getInstance().debug(
        "[CombatCoreEngine] SkillBuffSync npc={} skill={} action={} buff_id={}",
        payload.targetPlayerId, payload.skillId, static_cast<int>(payload.action), payload.buffId);
    if (npcManager_) {
      if (const NpcRuntimeInstance* inst = npcManager_->findInstance(payload.targetPlayerId)) {
        movementServer_->broadcastNearWorldXY(inst->x, inst->y, encodeSkillBuffSync(payload));
        return;
      }
    }
    movementServer_->broadcastToAll(encodeSkillBuffSync(payload));
  } else {
    Core::Logger::getInstance().debug(
        "[CombatCoreEngine] SkillBuffSync player={} skill={} action={} buff_id={} type={}",
        payload.targetPlayerId, payload.skillId, static_cast<int>(payload.action), payload.buffId,
        static_cast<int>(payload.buffType));
    movementServer_->broadcastNearPlayer(payload.targetPlayerId, encodeSkillBuffSync(payload));
  }
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
  inferEffectTypeFromTargetStat(payload);
  if (!skillService_ || payload.skillId == 0) return;
  const Combat::SkillData* skill = skillService_->getSkillData(payload.skillId);
  if (!skill) return;
  if (payload.skillName.empty()) payload.skillName = skill->skillName;
  if (payload.iconPath.empty()) payload.iconPath = skill->iconPath;
}

bool CombatCoreEngine::skillHasEffectType(const Combat::SkillData& skill,
                                          Combat::EffectType type, uint8_t skillRank) const {
  const auto effects = skill.buildEffectsForRank(skillRank);
  return std::any_of(effects.begin(), effects.end(),
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
    return 0;  // sem estado do caster: nao inventar cura via powerCoef/2
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

    // Não invalidar CharacterState: reaction é in-memory (ReactionEngine).
    // invalidate() forçava JOIN MySQL no próximo cast (~100ms no Proxmox) e gerava hitch/cascata.

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
  const Combat::SkillData* skill = skillService_->getSkillData(skillId);
  if (!skill) return;

  const uint8_t buffTypeCode = static_cast<uint8_t>(Combat::BuffType::BUFF);
  const uint64_t buffId =
      applyPlayerBuffInMemory(targetPlayerId, sourcePlayerId, skillId, effect, *skill, buffTypeCode);
  if (buffId == 0) return;

  uint32_t durationMs = effect.durationMs > 0 ? effect.durationMs : 500;
  const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

  SkillBuffSyncPayload sync;
  sync.targetPlayerId = targetPlayerId;
  sync.buffId = buffId;
  sync.skillId = skillId;
  sync.buffType = buffTypeCode;
  sync.stacks = 1;
  sync.valueFlat = effect.valueFlat;
  sync.valuePercent = effect.valuePercent;
  sync.expiresAtMs = nowMs + static_cast<int64_t>(durationMs);
  sync.durationMs = durationMs;
  sync.targetStat = resolveBuffTargetStat(effect);
  sync.effectType = effectTypeToWireName(effect.effectType);
  sync.skillName = skill->skillName;
  sync.iconPath = skill->iconPath;
  broadcastPlayerSkillBuffApply(sync);
}

void CombatCoreEngine::applyReactionCounterDamage(uint32_t ownerPlayerId, uint32_t targetPlayerId,
                                                  uint32_t skillId, const Combat::SkillEffect& effect,
                                                  int32_t fixedDamage) {
  if (targetPlayerId == 0 || ownerPlayerId == targetPlayerId) return;

  int32_t damage = fixedDamage;
  if (damage <= 0) {
    Combat::CharacterState owner;
    if (stateLoader_ && stateLoader_->getCachedOrWarm(ownerPlayerId, owner)) {
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
  // #region agent log
  const int64_t t0 = agentNowMs();
  // #endregion
  movementServer_->broadcastNearPlayer(payload.sourcePlayerId, encodeSkillCastBroadcast(payload));
  // #region agent log
  {
    const int64_t ms = agentNowMs() - t0;
    if (ms >= 15) {
      agentDebugLog("H-BURST", "CombatCoreEngine.cpp:broadcastSkillCast", "skill_bcast_ms",
                    std::string("{\"ms\":") + std::to_string(ms) +
                        ",\"playerId\":" + std::to_string(payload.sourcePlayerId) +
                        ",\"skillId\":" + std::to_string(payload.skillId) + "}",
                    "burst");
    }
  }
  // #endregion
}

void CombatCoreEngine::broadcastBasicAttack(const BasicAttackBroadcastPayload& payload) {
  if (!movementServer_) return;
  if (payload.sourceType == static_cast<uint8_t>(CombatTargetType::Npc)) {
    if (npcManager_) {
      if (const NpcRuntimeInstance* inst = npcManager_->findInstance(payload.sourcePlayerId)) {
        movementServer_->broadcastNearWorldXY(inst->x, inst->y, encodeBasicAttackBroadcast(payload));
        return;
      }
    }
    if (payload.targetId > 0) {
      movementServer_->broadcastNearPlayer(payload.targetId, encodeBasicAttackBroadcast(payload));
      return;
    }
    movementServer_->broadcastToAll(encodeBasicAttackBroadcast(payload));
    return;
  }
  movementServer_->broadcastNearPlayer(payload.sourcePlayerId, encodeBasicAttackBroadcast(payload));
}

void CombatCoreEngine::broadcastNpcCombatEvent(const NpcCombatEventPayload& payload) {
  if (!movementServer_) return;
  // #region agent log
  const int64_t t0 = agentNowMs();
  // #endregion
  if (payload.sourcePlayerId > 0) {
    movementServer_->broadcastNearPlayer(payload.sourcePlayerId, encodeNpcCombatEvent(payload));
  } else if (npcManager_) {
    if (const NpcRuntimeInstance* inst = npcManager_->findInstance(payload.npcId)) {
      movementServer_->broadcastNearWorldXY(inst->x, inst->y, encodeNpcCombatEvent(payload));
    } else {
      movementServer_->broadcastToAll(encodeNpcCombatEvent(payload));
    }
  } else {
    movementServer_->broadcastToAll(encodeNpcCombatEvent(payload));
  }
  // #region agent log
  {
    const int64_t ms = agentNowMs() - t0;
    if (ms >= 15) {
      agentDebugLog("H-BURST", "CombatCoreEngine.cpp:broadcastNpcCombatEvent", "npc_combat_bcast_ms",
                    std::string("{\"ms\":") + std::to_string(ms) +
                        ",\"npcId\":" + std::to_string(payload.npcId) +
                        ",\"src\":" + std::to_string(payload.sourcePlayerId) +
                        ",\"delta\":" + std::to_string(payload.delta) + "}",
                    "burst");
    }
  }
  // #endregion
}

void CombatCoreEngine::broadcastNpcState(const NpcStatePayload& payload) {
  if (!movementServer_) return;
  // Sempre zone-wide: AOI só na pos do NPC deixava "fantasma" no cliente quando o mob
  // voltava ao home (>radius) e o player não recebia mais o opcode 102.
  movementServer_->broadcastToAll(encodeNpcStateUpdate(payload));
}

void CombatCoreEngine::finalizeSkillCastHit(uint32_t sourcePlayerId, const SkillCastPayload& payload) {
  struct HitOnlyGuard {
    HitOnlyGuard() { g_skillCastHitOnly = true; }
    ~HitOnlyGuard() { g_skillCastHitOnly = false; }
  } guard;
  processSkillCast(sourcePlayerId, payload);
}

void CombatCoreEngine::processSkillCast(uint32_t sourcePlayerId, const SkillCastPayload& payloadIn) {
  if (!skillService_ || !movementServer_) return;

  SkillCastPayload payload = payloadIn;
  disambiguateTargetType(sourcePlayerId, payload.targetType, payload.targetId);

  const bool hitOnly = g_skillCastHitOnly;

  // #region agent log
  struct CastPhaseLog {
    int64_t t0 = agentNowMs();
    int64_t tLoad = 0;
    int64_t tRank = 0;
    int64_t tAnim = 0;
    int64_t tBcast = 0;
    int64_t tDef = 0;
    int64_t tHit = 0;
    int64_t tFx = 0;
    uint32_t playerId = 0;
    uint32_t skillId = 0;
    ~CastPhaseLog() {
      const int64_t total = agentNowMs() - t0;
      if (total < 20 && tLoad < 20 && tRank < 20 && tAnim < 20 && tBcast < 20 &&
          tDef < 20 && tHit < 20 && tFx < 20) {
        return;
      }
      agentDebugLog("H-F", "CombatCoreEngine.cpp:processSkillCast", "cast_phase_ms",
                    std::string("{\"ms\":") + std::to_string(total) +
                        ",\"load\":" + std::to_string(tLoad) +
                        ",\"rank\":" + std::to_string(tRank) +
                        ",\"anim\":" + std::to_string(tAnim) +
                        ",\"bcast\":" + std::to_string(tBcast) +
                        ",\"def\":" + std::to_string(tDef) +
                        ",\"hit\":" + std::to_string(tHit) +
                        ",\"fx\":" + std::to_string(tFx) +
                        ",\"playerId\":" + std::to_string(playerId) +
                        ",\"skillId\":" + std::to_string(skillId) + "}",
                    "mid-phase");
    }
  } phaseLog;
  phaseLog.playerId = sourcePlayerId;
  phaseLog.skillId = payload.skillId;
  // #endregion

  const Combat::SkillData* skill = skillService_->getSkillData(payload.skillId);
  if (!skill) {
    Core::Logger::getInstance().warn("[CombatCoreEngine] skill {} não encontrada", payload.skillId);
    if (!hitOnly) {
      sendSkillCastRejected(sourcePlayerId, payload.skillId, SkillCastRejectReason::SkillNotFound);
    }
    return;
  }

  Combat::CharacterState sourceState;
  // #region agent log
  const int64_t a0 = agentNowMs();
  // #endregion
  bool haveSource = stateLoader_ && stateLoader_->getCachedOrWarm(sourcePlayerId, sourceState);
  // #region agent log
  phaseLog.tLoad = agentNowMs() - a0;
  // #endregion
  if (!haveSource) {
    sourceState = Combat::CharacterState{};
    sourceState.playerId = sourcePlayerId;
    sourceState.isAlive = true;
  }

  // #region agent log
  const int64_t rankT0 = agentNowMs();
  // #endregion
  const uint8_t rank = loadSkillRank(sourcePlayerId, payload.skillId);
  // #region agent log
  phaseLog.tRank = agentNowMs() - rankT0;
  // #endregion

  if (!hitOnly) {
  SkillCastRejectReason rangeFail = SkillCastRejectReason::Unknown;
  if (!validateSkillRange(sourcePlayerId, *skill, payload, &rangeFail)) {
    std::string rangeMsg;
    if (rangeFail == SkillCastRejectReason::RangeExceeded) {
      float sx = 0.f, sy = 0.f, sz = 0.f, tx = 0.f, ty = 0.f, tz = 0.f;
      if (tryGetPlayerPosition(sourcePlayerId, sx, sy, sz) &&
          tryGetTargetPosition(payload.targetType, payload.targetId, tx, ty, tz)) {
        const float distM = std::sqrt(distanceSquared2D(sx, sy, tx, ty)) / 100.f;
        const float maxM =
            (payload.targetType == static_cast<uint8_t>(CombatTargetType::Npc)
                 ? effectiveMaxRangeVsNpc(static_cast<float>(skill->rangeMax))
                 : effectiveMaxRange(static_cast<float>(skill->rangeMax))) /
            100.f;
        char buf[96];
        std::snprintf(buf, sizeof(buf), "Alvo muito distante (%.0fm, alcance %.0fm)", distM, maxM);
        rangeMsg = buf;
      }
    }
    sendSkillCastRejected(sourcePlayerId, payload.skillId, rangeFail, rangeMsg);
    return;
  }

  Combat::SkillUseRequest req;
  req.skillId = payload.skillId;
  req.sourcePlayerId = sourcePlayerId;
  req.skillRank = rank;
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
  }  // !hitOnly

  Combat::SkillData rankedSkill = *skill;
  rankedSkill.effects = skill->buildEffectsForRank(rank);
  rankedSkill.durationMs = skill->getEffectiveDurationMs(rank);
  rankedSkill.powerCoef = skill->getEffectivePowerCoef(rank);
  rankedSkill.resourceCost = skill->getEffectiveResourceCost(rank);
  rankedSkill.cooldownMs = skill->getEffectiveCooldownMs(rank);

  if (!hitOnly) {
  const bool isHealPrecheck = (skill->type == Combat::SkillType::HOT) ||
                              skillHasEffectType(*skill, Combat::EffectType::HEAL, rank) ||
                              skillHasEffectType(*skill, Combat::EffectType::HOT, rank);
  if (isHealPrecheck && skill->target == Combat::TargetType::SELF && haveSource &&
      sourceState.buffedStats.currentHealth >= sourceState.buffedStats.maxHealth) {
    // Possível cache stale pós-equip: recarrega stats e reavalia uma vez.
    if (stateLoader_ && stateLoader_->reloadStatsPreservingVitals(sourcePlayerId) &&
        stateLoader_->tryGetCachedState(sourcePlayerId, sourceState)) {
      haveSource = true;
    }
    if (haveSource &&
        sourceState.buffedStats.currentHealth >= sourceState.buffedStats.maxHealth) {
      Core::Logger::getInstance().info(
          "[CombatCoreEngine] HEAL rejeitado player={} skill={} motivo=HP_cheio", sourcePlayerId,
          payload.skillId);
      sendSkillCastRejected(sourcePlayerId, payload.skillId, SkillCastRejectReason::CannotCast,
                            "HP cheio");
      return;
    }
  }

  skillService_->startCooldown(sourcePlayerId, payload.skillId, skill->getEffectiveCooldownMs(rank));

  const uint16_t effectiveManaCost = skill->getEffectiveResourceCost(rank);
  if (skill->resourceType == Combat::ResourceType::MANA && effectiveManaCost > 0) {
    const int32_t cost = static_cast<int32_t>(effectiveManaCost);
    const int32_t knownMana = haveSource ? sourceState.buffedStats.currentMana : -1;
    const int32_t knownHealth = haveSource ? sourceState.buffedStats.currentHealth : -1;
    deductPlayerMana(sourcePlayerId, cost, knownHealth, knownMana);
    if (haveSource) {
      sourceState.buffedStats.currentMana = std::max(0, sourceState.buffedStats.currentMana - cost);
      sourceState.baseStats.currentMana = sourceState.buffedStats.currentMana;
    }
  }

  std::string anim, vfx, sfx;
  // #region agent log
  const int64_t animT0 = agentNowMs();
  // #endregion
  loadSkillAnimPaths(payload.skillId, anim, vfx, sfx);
  // #region agent log
  phaseLog.tAnim = agentNowMs() - animT0;
  // #endregion

  SkillCastBroadcastPayload castBroadcast;
  castBroadcast.sourcePlayerId = sourcePlayerId;
  castBroadcast.skillId = payload.skillId;
  castBroadcast.targetId = payload.targetId;
  castBroadcast.castTimeMs = skill->castTimeMs;
  castBroadcast.castAnimPath = anim;
  castBroadcast.vfxPath = vfx;
  castBroadcast.sfxPath = sfx;
  // #region agent log
  {
    static std::mutex lastEmitMu;
    static std::unordered_map<uint32_t, int64_t> lastEmitByPlayer;
    int64_t since = -1;
    {
      std::lock_guard<std::mutex> lk(lastEmitMu);
      const int64_t now = agentNowMs();
      auto it = lastEmitByPlayer.find(sourcePlayerId);
      if (it != lastEmitByPlayer.end()) {
        since = now - it->second;
      }
      lastEmitByPlayer[sourcePlayerId] = now;
    }
    agentDebugLog("H-CASCADE", "CombatCoreEngine.cpp:processSkillCast", "skill_cast_emit",
                  std::string("{\"playerId\":") + std::to_string(sourcePlayerId) +
                      ",\"skillId\":" + std::to_string(payload.skillId) +
                      ",\"targetId\":" + std::to_string(payload.targetId) +
                      ",\"castTimeMs\":" + std::to_string(skill->castTimeMs) +
                      ",\"sinceLastEmitMs\":" + std::to_string(since) +
                      ",\"t\":" + std::to_string(agentNowMs()) + "}",
                  "cascade");
  }
  const int64_t bcastT0 = agentNowMs();
  // #endregion
  broadcastSkillCast(castBroadcast);
  // #region agent log
  phaseLog.tBcast = agentNowMs() - bcastT0;
  // #endregion

  if (skill->type == Combat::SkillType::REACTION) {
    armReactionSkill(sourcePlayerId, rankedSkill);
    Core::Logger::getInstance().debug(
        "[CombatCoreEngine] SkillCast REACTION armada player={} skill={}", sourcePlayerId,
        payload.skillId);
    return;
  }

  // Sem commit-early e sem cancel: cada hit pendente dispara no seu executeAtMs
  // (castTimeMs). Commitar cedo comprimia vários 103 no mesmo instante (rajada/cascata);
  // cancelar perdia dano. Deixar a fila resolver mantém os hits espaçados pelo cast time.
  if (skill->castTimeMs > 0) {
    scheduleSkillHit(sourcePlayerId, payload, skill->castTimeMs);
    return;
  }
  }  // !hitOnly

  const bool isHeal = (skill->type == Combat::SkillType::HOT) ||
                      skillHasEffectType(*skill, Combat::EffectType::HEAL, rank) ||
                      skillHasEffectType(*skill, Combat::EffectType::HOT, rank);
  const bool isBuffOnly =
      !isHeal && !skillHasEffectType(*skill, Combat::EffectType::DAMAGE, rank) &&
      (skillHasEffectType(*skill, Combat::EffectType::BUFF_STAT, rank) ||
       skillHasEffectType(*skill, Combat::EffectType::DEBUFF_STAT, rank) ||
       skillHasEffectType(*skill, Combat::EffectType::SHIELD, rank) ||
       skillHasEffectType(*skill, Combat::EffectType::STUN, rank) ||
       skillHasEffectType(*skill, Combat::EffectType::SILENCE, rank) ||
       skillHasEffectType(*skill, Combat::EffectType::ROOT, rank) ||
       skillHasEffectType(*skill, Combat::EffectType::SLOW, rank));

  const bool effectOnSelf = (skill->target == Combat::TargetType::SELF ||
                             skill->target == Combat::TargetType::PARTY);

  Combat::CharacterState defender;
  bool defenderIsPlayer = false;
  // #region agent log
  const int64_t defT0 = agentNowMs();
  // #endregion
  bool haveDefender =
      buildDefenderState(payload.targetType, payload.targetId, defender, defenderIsPlayer);
  // #region agent log
  phaseLog.tDef = agentNowMs() - defT0;
  // #endregion
  if (effectOnSelf || (isHeal && payload.targetId == 0)) {
    defender = sourceState;
    defenderIsPlayer = true;
    haveDefender = haveSource;
  }

  // #region agent log
  struct HitPhaseGuard {
    CastPhaseLog& log;
    int64_t t0 = agentNowMs();
    bool active = true;
    void finish() {
      if (!active) return;
      log.tHit = agentNowMs() - t0;
      active = false;
    }
    ~HitPhaseGuard() { finish(); }
  } hitPhase{phaseLog};
  // #endregion
  int32_t delta = 0;
  bool isCrit = false;
  int32_t overkill = 0;

  if (isHeal) {
    const Combat::CharacterState& healTarget = haveDefender ? defender : sourceState;
    delta = computeInstantHealDelta(rankedSkill, rank, sourceState, healTarget, haveSource);
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
          Core::Logger::getInstance().debug("[CombatCoreEngine] SkillCast MISS player={} skill={} target={}",
                                            sourcePlayerId, payload.skillId, payload.targetId);
          return;
        }
      }
      if (defenderIsPlayer && payload.targetId > 0 && reactionEngine_ &&
          reactionEngine_->onPlayerHitReceived(payload.targetId, sourcePlayerId)) {
        broadcastMiss(payload.targetType, payload.targetId, sourcePlayerId);
        Core::Logger::getInstance().debug(
            "[CombatCoreEngine] SkillCast REACTION miss player={} skill={} target={}", sourcePlayerId,
            payload.skillId, payload.targetId);
        applySkillEffects(sourcePlayerId, payload.targetType, payload.targetId, rankedSkill, sourceState,
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
      Core::Logger::getInstance().debug(
          "[CombatCoreEngine] SkillCast dmg player={} skill={} haveSource=1 physAtk={} magAtk={} str={} "
          "powerCoef={} finalDamage={} crit={}",
          sourcePlayerId, payload.skillId, sourceState.buffedStats.physicalAttack,
          sourceState.buffedStats.magicAttack, sourceState.buffedStats.strength,
          skill->getEffectivePowerCoef(rank), bd.finalDamage, isCrit ? 1 : 0);
    } else {
      Core::Logger::getInstance().warn(
          "[CombatCoreEngine] SkillCast ABORTADO: loadPlayerState falhou player={} skill={} "
          "(fallback powerCoef/2 desativado)",
          sourcePlayerId, payload.skillId);
      return;
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
      // #region agent log
      {
        static std::mutex dmgGapMu;
        static std::unordered_map<uint32_t, int64_t> lastDmgByPlayer;
        int64_t sinceDmg = -1;
        const int64_t nowD = agentNowMs();
        {
          std::lock_guard<std::mutex> lk(dmgGapMu);
          auto it = lastDmgByPlayer.find(sourcePlayerId);
          if (it != lastDmgByPlayer.end()) {
            sinceDmg = nowD - it->second;
          }
          lastDmgByPlayer[sourcePlayerId] = nowD;
        }
        agentDebugLog("H-CASCADE", "CombatCoreEngine.cpp:processSkillCast", "damage_emit_103",
                      std::string("{\"playerId\":") + std::to_string(sourcePlayerId) +
                          ",\"skillId\":" + std::to_string(payload.skillId) +
                          ",\"npcId\":" + std::to_string(payload.targetId) +
                          ",\"delta\":" + std::to_string(applied) +
                          ",\"sinceLastDmgMs\":" + std::to_string(sinceDmg) +
                          ",\"t\":" + std::to_string(nowD) + "}",
                      "cascade");
      }
      // #endregion
      broadcastNpcCombatEvent(evt);
      handleNpcDamageResult(payload.targetId, applied, npcDied, sourcePlayerId);
      if (isDouble) {
        Core::Logger::getInstance().debug(
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
      if (!stateLoader_ || !stateLoader_->getCachedOrWarm(tgtId, tgtState)) {
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
        Core::Logger::getInstance().debug(
            "[CombatCoreEngine] DOUBLE player={} targetType={} target={} totalDamage={} skill={}",
            sourcePlayerId, payload.targetType, payload.targetId, std::abs(totalDelta),
            payload.skillId);
      }
    }
  }

  // DOT/HOT e demais efeitos persistentes da skill (effects_json).
  // #region agent log
  hitPhase.finish();
  const int64_t fxT0 = agentNowMs();
  // #endregion
  applySkillEffects(sourcePlayerId, payload.targetType, payload.targetId, rankedSkill, sourceState, haveSource);
  // #region agent log
  phaseLog.tFx = agentNowMs() - fxT0;
  // #endregion

  Core::Logger::getInstance().debug(
      "[CombatCoreEngine] SkillCast player={} skill={} rank={} targetType={} target={} delta={} crit={}",
      sourcePlayerId, payload.skillId, static_cast<int>(rank), payload.targetType, payload.targetId,
      delta, isCrit ? 1 : 0);
}

void CombatCoreEngine::processBasicAttack(uint32_t sourcePlayerId, const BasicAttackPayload& payloadIn) {
  if (!movementServer_) return;

  BasicAttackPayload payload = payloadIn;
  disambiguateTargetType(sourcePlayerId, payload.targetType, payload.targetId);

  Combat::CharacterState attacker;
  const bool haveAttacker = stateLoader_ && stateLoader_->getCachedOrWarm(sourcePlayerId, attacker);
  if (!haveAttacker || attacker.classId == 0) {
    Core::Logger::getInstance().warn("[CombatCoreEngine] class_id não encontrado para player {}", sourcePlayerId);
    return;
  }

  const uint32_t classId = static_cast<uint32_t>(attacker.classId);

  auto it = basicAttacksByClass_.find(classId);
  if (it == basicAttacksByClass_.end()) {
    Core::Logger::getInstance().warn("[CombatCoreEngine] basic attack não definido para class {}", classId);
    return;
  }

  const BasicAttackDef& basic = it->second;

  if (!validateBasicAttackRange(sourcePlayerId, payload.targetType, payload.targetId,
                                basic.rangeMax)) {
    // Não reenviar opcode 100 (Destroy no cliente). Só 102 para corrigir XY sem sumir.
    if (payload.targetType == static_cast<uint8_t>(CombatTargetType::Npc) && npcManager_) {
      if (const NpcRuntimeInstance* inst = npcManager_->findInstance(payload.targetId)) {
        if (!inst->isDead) {
          broadcastNpcState(npcManager_->toStatePayload(*inst));
        }
      }
    }
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
  atkBroadcast.castAnimPath = resolveBasicAttackAnimPath(classId, basic.castAnimPath);
  Core::Logger::getInstance().debug(
      "[CombatCoreEngine] BasicAttack anim class={} path={}", classId, atkBroadcast.castAnimPath);
  broadcastBasicAttack(atkBroadcast);

  // SkillData sintética representando o ataque básico (físico, pode critar).
  Combat::SkillData synthetic;
  synthetic.element = Combat::Element::PHYSICAL;
  synthetic.scalingStat = Combat::ScalingStat::PHYS_ATK;
  synthetic.powerCoef = basic.powerCoef;
  synthetic.canCrit = true;
  synthetic.ignoresDefense = false;

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
      Core::Logger::getInstance().debug("[CombatCoreEngine] BasicAttack MISS player={} target={}",
                                       sourcePlayerId, payload.targetId);
      return;
    }
    if (defenderIsPlayer && payload.targetId > 0 && reactionEngine_ &&
        reactionEngine_->onPlayerHitReceived(payload.targetId, sourcePlayerId)) {
      broadcastMiss(payload.targetType, payload.targetId, sourcePlayerId);
      Core::Logger::getInstance().debug(
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
    Core::Logger::getInstance().info(
        "[CombatCoreEngine] BasicAttack dmg player={} haveAttacker=1 physAtk={} str={} powerCoef={} "
        "defPhys={} finalDamage={} crit={}",
        sourcePlayerId, attacker.buffedStats.physicalAttack, attacker.buffedStats.strength,
        basic.powerCoef, defender.buffedStats.physicalDefense, bd.finalDamage, isCrit ? 1 : 0);
  } else {
    Core::Logger::getInstance().warn(
        "[CombatCoreEngine] BasicAttack ABORTADO: loadPlayerState falhou player={} "
        "(fallback powerCoef/2 desativado)",
        sourcePlayerId);
    return;
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
        Core::Logger::getInstance().debug(
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
        Core::Logger::getInstance().debug(
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
  if (playerId == 0) return;
  // Aquece o cache de stats/HP/mana na entrada — regen e movimento não batem MySQL no 1º tick.
  if (stateLoader_) {
    Combat::CharacterState st;
    stateLoader_->loadPlayerState(playerId, st);
  }
  syncJoinDeathState(playerId);
  if (!reactionEngine_) return;
  reactionEngine_->reloadArmedForPlayer(playerId);
  Core::Logger::getInstance().info("[CombatCoreEngine] jogador {} entrou na zone — reações recarregadas",
                                   playerId);
}

void CombatCoreEngine::onPlayerEquipmentOrStatsChanged(uint32_t playerId) {
  if (playerId == 0 || !stateLoader_) return;
  if (!stateLoader_->reloadStatsPreservingVitals(playerId)) {
    stateLoader_->invalidate(playerId);
    stateLoader_->requestWarm(playerId);
  }
}

void CombatCoreEngine::syncJoinDeathState(uint32_t playerId) {
  if (playerId == 0 || !stateLoader_ || !movementServer_) return;
  Combat::CharacterState st;
  if (!stateLoader_->tryGetCachedState(playerId, st) && !stateLoader_->loadPlayerState(playerId, st)) {
    return;
  }
  if (st.isAlive && st.buffedStats.currentHealth > 0) return;

  // #region agent log
  agentDebugLog("H-RESPAWN", "CombatCoreEngine.cpp:syncJoinDeathState", "join_dead_detected",
                std::string("{\"playerId\":") + std::to_string(playerId) +
                    ",\"hp\":" + std::to_string(st.buffedStats.currentHealth) +
                    ",\"isAlive\":" + (st.isAlive ? "true" : "false") +
                    ",\"maxHp\":" + std::to_string(st.buffedStats.maxHealth) + "}",
                "post-fix");
  // #endregion

  // Login já morto: auto-respawn (senão fica 0 HP + isDead bloqueando movimento).
  if (movementServer_->autoRespawnOnJoin(playerId)) {
    return;
  }

  movementServer_->forcePlayerDeadState(playerId, st.buffedStats.currentHealth,
                                        std::max(1, st.buffedStats.maxHealth),
                                        st.buffedStats.currentMana,
                                        std::max(1, st.buffedStats.maxMana));
}

void CombatCoreEngine::setResolvePartyMembersCallback(
    std::function<std::vector<uint32_t>(uint32_t playerId)> cb) {
  resolvePartyMembers_ = std::move(cb);
}

float CombatCoreEngine::getPlayerMovementSpeedPercent(uint32_t playerId) const {
  if (!stateLoader_ || playerId == 0) return 100.f;
  // Hot path de movimento (~30 frames/s por cliente, na thread do cliente): usar SOMENTE
  // o cache, nunca ir ao MySQL. Em cache miss retorna 100% (o combate popula o cache no
  // primeiro cast/hit). Evita a tempestade de round-trips ao DB remoto que causava o lag.
  Combat::CharacterState st;
  if (!stateLoader_->tryGetCachedState(playerId, st)) return 100.f;
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

std::string CombatCoreEngine::resolveNpcSkillCastAnimPath(const NpcRuntimeInstance& inst,
                                                           const std::string& skillCastAnimOverride) {
  if (!skillCastAnimOverride.empty()) return skillCastAnimOverride;
  if (!inst.skillAnimPath.empty()) return inst.skillAnimPath;
  if (!inst.attackAnimPaths.empty()) return inst.attackAnimPaths.front();
  return {};
}

void CombatCoreEngine::processNpcBasicAttack(uint32_t npcInstanceId, uint32_t targetPlayerId) {
  if (!movementServer_ || !npcManager_ || npcInstanceId == 0 || targetPlayerId == 0) return;

  const NpcRuntimeInstance* inst = npcManager_->findInstance(npcInstanceId);
  if (!inst || inst->isDead || !inst->isHostile) return;

  float px = 0.f, py = 0.f, pz = 0.f;
  if (!tryGetPlayerPosition(targetPlayerId, px, py, pz)) return;

  // Melee do mob: range curto (sem margem grande de cápsula/AI — senão “bate de longe”).
  const float attackR = std::max(50.f, inst->attackRange) * 1.15f;
  if (!isInRange2D(inst->x, inst->y, px, py, attackR)) {
    return;
  }

  Combat::CharacterState attacker = CharacterStateLoader::makeNpcAttackerState(*inst);
  Combat::CharacterState defender;
  if (!stateLoader_ ||
      (!stateLoader_->getCachedOrWarm(targetPlayerId, defender) &&
       !stateLoader_->loadPlayerState(targetPlayerId, defender))) {
    return;
  }
  if (!defender.isAlive || defender.buffedStats.currentHealth <= 0) return;

  BasicAttackBroadcastPayload atkBroadcast;
  atkBroadcast.sourcePlayerId = npcInstanceId;
  atkBroadcast.classId = 0;
  atkBroadcast.targetId = targetPlayerId;
  atkBroadcast.hitWindowMs = 300;
  atkBroadcast.castAnimPath =
      inst->attackAnimPaths.empty() ? std::string() : inst->attackAnimPaths.front();
  atkBroadcast.sourceType = static_cast<uint8_t>(CombatTargetType::Npc);
  atkBroadcast.animIndex = 0;
  broadcastBasicAttack(atkBroadcast);

  Combat::SkillData synthetic;
  synthetic.element = Combat::Element::PHYSICAL;
  synthetic.scalingStat = Combat::ScalingStat::PHYS_ATK;
  synthetic.powerCoef = 100;
  synthetic.canCrit = true;
  synthetic.ignoresDefense = false;

  const int32_t hitChance =
      Combat::CombatCalculator::getInstance().calculateHitChance(attacker, defender);
  if (!Combat::CombatCalculator::getInstance().rollHit(hitChance)) {
    CombatEventPayload combat;
    combat.targetId = targetPlayerId;
    combat.sourceId = npcInstanceId;
    combat.delta = 0;
    combat.reason = static_cast<uint8_t>(CombatReason::Miss);
    combat.isCrit = 0;
    movementServer_->broadcastNearPlayer(targetPlayerId, encodeCombatEventNotify(combat));
    return;
  }

  const Combat::DamageBreakdown bd =
      Combat::CombatCalculator::getInstance().calculatePhysicalDamage(attacker, defender, synthetic,
                                                                      /*rank*/ 1, true);
  const int32_t delta = -bd.finalDamage;
  const bool isCrit = (bd.critMultiplier != 100);

  int32_t doubleBonus = 0;
  bool isDouble = false;
  if (delta < 0) {
    doubleBonus = computeDoubleBonus(attacker, defender, true, std::abs(delta));
    isDouble = doubleBonus > 0;
  }
  const int32_t totalDelta = delta - doubleBonus;

  // Evita ReactionEngine tratar npcId como player source.
  const bool prevReaction = inReactionDispatch_;
  inReactionDispatch_ = true;
  applyPlayerDamage(npcInstanceId, targetPlayerId, totalDelta,
                    static_cast<uint8_t>(CombatReason::Damage), isCrit, isDouble);
  inReactionDispatch_ = prevReaction;

  Core::Logger::getInstance().debug(
      "[CombatCoreEngine] NpcBasicAttack npc={} -> player={} dmg={} crit={}",
      npcInstanceId, targetPlayerId, std::abs(totalDelta), isCrit ? 1 : 0);
}

}  // namespace Zone
}  // namespace Umbra
