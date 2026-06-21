#include "zone/CombatCoreEngine.hpp"
#include "zone/MovementServer.hpp"
#include "SkillTypes.hpp"
#include "core/Logger.hpp"
#include <cmath>

namespace Umbra {
namespace Zone {

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
      // Hit/miss apenas em PvP (ver nota no ataque básico).
      if (isPvP) {
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

  // Hit/miss apenas em PvP (alvo jogador). PvE é determinístico para não gerar
  // "misses" silenciosos em alvos de treino.
  if (haveAttacker && defenderIsPlayer) {
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
