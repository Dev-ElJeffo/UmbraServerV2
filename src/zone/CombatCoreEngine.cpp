#include "zone/CombatCoreEngine.hpp"
#include "zone/MovementServer.hpp"
#include "SkillTypes.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Umbra {
namespace Zone {

namespace {
// Regeneracao passiva: intervalo do tick e fracao do maximo regenerada por tick.
constexpr float kRegenIntervalSeconds = 2.0f;
constexpr float kRegenHealthFraction = 0.02f;  // ~2% do max HP por tick
constexpr float kRegenManaFraction = 0.03f;    // ~3% do max MP por tick
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

void CombatCoreEngine::applySkillEffects(uint32_t sourcePlayerId, uint8_t targetType,
                                         uint32_t targetId, const Combat::SkillData& skill,
                                         const Combat::CharacterState& attacker, bool haveAttacker) {
  if (skill.effects.empty() || targetId == 0) return;

  const bool targetIsNpc = (targetType == static_cast<uint8_t>(CombatTargetType::Npc));
  const bool targetIsPlayer = (targetType == static_cast<uint8_t>(CombatTargetType::Player));
  if (!targetIsNpc && !targetIsPlayer) return;

  for (const auto& eff : skill.effects) {
    const bool isDot = (eff.effectType == Combat::EffectType::DOT);
    const bool isHot = (eff.effectType == Combat::EffectType::HOT);
    if (!isDot && !isHot) continue;  // demais efeitos (buff/stun/...) ficam para depois

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
      insertPlayerDot(sourcePlayerId, targetId, skill.skillId, dotType, tickValue, interval, ticksTotal);
    } else {  // NPC -> in-memory
      NpcDotInstance inst;
      inst.npcInstanceId = targetId;
      inst.sourcePlayerId = sourcePlayerId;
      inst.skillId = skill.skillId;
      inst.tickValue = isHot ? tickValue : -tickValue;
      inst.intervalMs = interval;
      inst.ticksRemaining = static_cast<uint8_t>(ticksTotal);
      inst.nextTickAt = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
      {
        std::lock_guard<std::mutex> lock(npcDotsMu_);
        npcDots_.push_back(inst);
      }
      Core::Logger::getInstance().info(
          "[CombatCoreEngine] DOT/HOT NPC aplicado: npc={} src={} skill={} tick={} ticks={} interval={}ms",
          targetId, sourcePlayerId, skill.skillId, inst.tickValue, ticksTotal, interval);
    }
  }
}

void CombatCoreEngine::insertPlayerDot(uint32_t sourcePlayerId, uint32_t targetPlayerId,
                                       uint32_t skillId, const char* dotType, int32_t tickValue,
                                       uint32_t tickIntervalMs, uint32_t ticksTotal) {
  if (!db_ || !db_->isConnected()) return;

  const uint32_t intervalSec = std::max<uint32_t>(1, (tickIntervalMs + 999) / 1000);
  const uint32_t totalDurationSec = intervalSec * ticksTotal;

  db_->executePreparedInsert(
      "INSERT INTO active_dots ("
      "target_player_id, source_player_id, skill_id, dot_type, tick_value, tick_interval_ms, "
      "ticks_remaining, next_tick_at, expires_at) VALUES ("
      "?, ?, ?, ?, ?, ?, ?, "
      "DATE_ADD(NOW(3), INTERVAL ? MILLISECOND), "
      "DATE_ADD(NOW(3), INTERVAL ? SECOND))",
      {std::to_string(targetPlayerId), std::to_string(sourcePlayerId), std::to_string(skillId),
       dotType, std::to_string(tickValue), std::to_string(tickIntervalMs),
       std::to_string(std::min<uint32_t>(255, ticksTotal)), std::to_string(tickIntervalMs),
       std::to_string(totalDurationSec)});

  Core::Logger::getInstance().info(
      "[CombatCoreEngine] DOT/HOT player aplicado: target={} src={} skill={} type={} tick={} ticks={}",
      targetPlayerId, sourcePlayerId, skillId, dotType, tickValue, ticksTotal);
}

void CombatCoreEngine::tickNpcDots() {
  if (!npcManager_ || !movementServer_) return;

  std::vector<NpcDotInstance> due;
  const auto now = std::chrono::steady_clock::now();
  {
    std::lock_guard<std::mutex> lock(npcDotsMu_);
    if (npcDots_.empty()) return;
    for (auto& inst : npcDots_) {
      while (inst.ticksRemaining > 0 && inst.nextTickAt <= now) {
        due.push_back(inst);
        inst.ticksRemaining--;
        inst.nextTickAt += std::chrono::milliseconds(inst.intervalMs);
      }
    }
    // Remove esgotados.
    npcDots_.erase(std::remove_if(npcDots_.begin(), npcDots_.end(),
                                  [](const NpcDotInstance& i) { return i.ticksRemaining == 0; }),
                   npcDots_.end());
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
      handleNpcDamageResult(inst.npcInstanceId, applied, npcDied);
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

bool CombatCoreEngine::spawnNpcInstance(uint32_t npcInstanceId) {
  if (!npcManager_ || npcInstanceId == 0) return false;
  if (!npcManager_->loadInstanceById(npcInstanceId)) {
    if (!npcManager_->findInstance(npcInstanceId)) return false;
  }
  if (const NpcRuntimeInstance* inst = npcManager_->findInstance(npcInstanceId)) {
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

void CombatCoreEngine::handleNpcDamageResult(uint32_t npcInstanceId, int32_t applied, bool npcDied) {
  if (applied == 0 && !npcDied) return;
  if (npcDied) {
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
    movementServer_->sendBinaryToClient(clientId, encodeNpcSpawnNotify(npcManager_->toSpawnPayload(inst)));
  }
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
  out = CharacterStateLoader::makeNpcDefenderState(*inst);
  return true;
}

void CombatCoreEngine::broadcastMiss(uint8_t targetType, uint32_t targetId, uint32_t sourcePlayerId) {
  if (!movementServer_) return;
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
                                         int32_t delta, uint8_t reason, bool isCrit) {
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

  const int32_t newHealth = std::max(0, std::min(maxHealth, curHealth + delta));
  const bool isDead = (newHealth <= 0);

  if (isDead) {
    db_->executePreparedInsert(
        "UPDATE players SET health = ?, is_dead = 1, last_death_at = CURRENT_TIMESTAMP WHERE id = ?",
        {std::to_string(newHealth), tid});
  } else {
    db_->executePreparedInsert(
        "UPDATE players SET health = ?, is_dead = 0 WHERE id = ?",
        {std::to_string(newHealth), tid});
  }

  PlayerVitalsPayload vitals;
  vitals.playerId = targetPlayerId;
  vitals.currentHealth = newHealth;
  vitals.maxHealth = maxHealth;
  vitals.currentMana = curMana;
  vitals.maxMana = maxMana;
  vitals.sourcePlayerId = sourcePlayerId;
  vitals.reason = reason;

  movementServer_->broadcastVitalsAndCombat(targetPlayerId, vitals, sourcePlayerId,
                                            newHealth - curHealth, isDead, isCrit);
  return true;
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
    return;
  }

  Combat::CharacterState sourceState;
  const bool haveSource = stateLoader_ && stateLoader_->loadPlayerState(sourcePlayerId, sourceState);
  if (!haveSource) {
    sourceState = Combat::CharacterState{};
    sourceState.playerId = sourcePlayerId;
    sourceState.isAlive = true;
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
    return;
  }

  const uint8_t rank = loadSkillRank(sourcePlayerId, payload.skillId);

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

  const bool isHeal = (skill->type == Combat::SkillType::HOT) ||
                      skill->target == Combat::TargetType::SELF ||
                      skill->target == Combat::TargetType::ALLY ||
                      skill->target == Combat::TargetType::PARTY ||
                      skill->target == Combat::TargetType::AREA_ALLY;

  Combat::CharacterState defender;
  bool defenderIsPlayer = false;
  const bool haveDefender =
      buildDefenderState(payload.targetType, payload.targetId, defender, defenderIsPlayer);

  int32_t delta = 0;
  bool isCrit = false;
  int32_t overkill = 0;

  if (isHeal) {
    const Combat::CharacterState& healTarget = haveDefender ? defender : sourceState;
    const int32_t heal = haveSource ? Combat::CombatCalculator::getInstance().calculateHeal(sourceState, healTarget, *skill, rank)
                                    : static_cast<int32_t>(std::max<uint16_t>(1, skill->powerCoef / 2));
    delta = std::max(1, heal);
  } else {
    if (!haveDefender) {
      Core::Logger::getInstance().warn(
          "[CombatCoreEngine] skill {} sem alvo válido (target={})", payload.skillId, payload.targetId);
      return;
    }
    if (haveSource) {
      const bool isPvP = defenderIsPlayer;
      // Hit/miss para qualquer alvo (player ou NPC). NPC tem dodge=0, entao a
      // frequencia de miss depende da accuracy do atacante.
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

  if (payload.targetType == static_cast<uint8_t>(CombatTargetType::Npc) && !isHeal) {
    bool dummyCrit = false;
    bool npcDied = false;
    const int32_t applied = npcManager_->applyDamage(payload.targetId, delta, dummyCrit, &npcDied);
    if (applied != 0 || npcDied) {
      NpcCombatEventPayload evt;
      evt.npcId = payload.targetId;
      evt.sourcePlayerId = sourcePlayerId;
      evt.delta = applied;
      evt.reason = static_cast<uint8_t>(CombatReason::Skill);
      evt.isCrit = isCrit ? 1 : 0;
      broadcastNpcCombatEvent(evt);
      handleNpcDamageResult(payload.targetId, applied, npcDied);
    }
  } else if (payload.targetType == static_cast<uint8_t>(CombatTargetType::Player) && payload.targetId > 0) {
    const uint8_t reason =
        isHeal ? static_cast<uint8_t>(CombatReason::Heal) : static_cast<uint8_t>(CombatReason::Skill);
    if (applyPlayerDamage(sourcePlayerId, payload.targetId, delta, reason, isCrit)) {
      writeCombatLog(sourcePlayerId, payload.targetId, payload.skillId,
                     isHeal ? "HEAL" : "DAMAGE", std::abs(delta), isCrit, overkill);
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
    bool dummyCrit = false;
    bool npcDied = false;
    const int32_t applied = npcManager_->applyDamage(payload.targetId, delta, dummyCrit, &npcDied);
    if (applied != 0 || npcDied) {
      NpcCombatEventPayload evt;
      evt.npcId = payload.targetId;
      evt.sourcePlayerId = sourcePlayerId;
      evt.delta = applied;
      evt.reason = static_cast<uint8_t>(CombatReason::Damage);
      evt.isCrit = isCrit ? 1 : 0;
      broadcastNpcCombatEvent(evt);
      handleNpcDamageResult(payload.targetId, applied, npcDied);
    }
  } else if (payload.targetType == static_cast<uint8_t>(CombatTargetType::Player) && payload.targetId > 0) {
    if (applyPlayerDamage(sourcePlayerId, payload.targetId, delta,
                          static_cast<uint8_t>(CombatReason::Damage), isCrit)) {
      writeCombatLog(sourcePlayerId, payload.targetId, 0, "DAMAGE", std::abs(delta), isCrit, overkill);
    }
  }

  Core::Logger::getInstance().info(
      "[CombatCoreEngine] BasicAttack player={} class={} targetType={} target={} delta={} crit={}",
      sourcePlayerId, classId, payload.targetType, payload.targetId, delta, isCrit ? 1 : 0);
}

}  // namespace Zone
}  // namespace Umbra
