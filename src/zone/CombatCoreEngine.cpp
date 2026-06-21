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

bool CombatCoreEngine::applyPlayerDamage(uint32_t sourcePlayerId, uint32_t targetPlayerId,
                                         int32_t delta, uint8_t reason) {
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
                                            newHealth - curHealth, isDead);
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
  sourceState.playerId = sourcePlayerId;
  sourceState.isAlive = true;

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

  skillService_->startCooldown(sourcePlayerId, payload.skillId, skill->cooldownMs);

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

  const int32_t delta = -50;

  if (payload.targetType == static_cast<uint8_t>(CombatTargetType::Npc)) {
    bool isCrit = false;
    bool npcDied = false;
    const int32_t applied = npcManager_->applyDamage(payload.targetId, delta, isCrit, &npcDied);
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
    applyPlayerDamage(sourcePlayerId, payload.targetId, delta,
                      static_cast<uint8_t>(CombatReason::Skill));
  }

  Core::Logger::getInstance().info("[CombatCoreEngine] SkillCast player={} skill={} targetType={} target={} delta={}",
                                   sourcePlayerId, payload.skillId, payload.targetType, payload.targetId, delta);
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
  const int32_t delta = -static_cast<int32_t>(std::max(1, basic.powerCoef / 2));

  BasicAttackBroadcastPayload atkBroadcast;
  atkBroadcast.sourcePlayerId = sourcePlayerId;
  atkBroadcast.classId = classId;
  atkBroadcast.targetId = payload.targetId;
  atkBroadcast.hitWindowMs = 300;
  atkBroadcast.castAnimPath = basic.castAnimPath;
  broadcastBasicAttack(atkBroadcast);

  if (payload.targetType == static_cast<uint8_t>(CombatTargetType::Npc)) {
    bool isCrit = false;
    bool npcDied = false;
    const int32_t applied = npcManager_->applyDamage(payload.targetId, delta, isCrit, &npcDied);
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
    applyPlayerDamage(sourcePlayerId, payload.targetId, delta,
                      static_cast<uint8_t>(CombatReason::Damage));
  }

  Core::Logger::getInstance().info("[CombatCoreEngine] BasicAttack player={} class={} targetType={} target={} delta={}",
                                   sourcePlayerId, classId, payload.targetType, payload.targetId, delta);
}

}  // namespace Zone
}  // namespace Umbra
