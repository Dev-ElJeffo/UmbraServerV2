#include "zone/NpcManager.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <ctime>

namespace Umbra {
namespace Zone {

namespace {
constexpr size_t kMinRowFields = 48;

std::chrono::system_clock::time_point respawnTimeFromUnix(uint64_t unixTs) {
  if (unixTs == 0) {
    return std::chrono::system_clock::now();
  }
  return std::chrono::system_clock::from_time_t(static_cast<std::time_t>(unixTs));
}

float parseFloatOr(const std::string& s, float fallback) {
  if (s.empty()) return fallback;
  try {
    return std::stof(s);
  } catch (...) {
    return fallback;
  }
}

bool parseOptionalFloat(const std::string& s, float& out) {
  if (s.empty()) return false;
  try {
    out = std::stof(s);
    return true;
  } catch (...) {
    return false;
  }
}
}  // namespace

const char* NpcManager::kInstanceSelectSql =
    "SELECT ni.npc_instance_id, ni.npc_template_id, ni.zone_id, ni.pos_x, ni.pos_y, ni.pos_z, ni.yaw, "
    "ni.current_health, ni.current_mana, ni.is_dead, "
    "COALESCE(UNIX_TIMESTAMP(ni.respawn_at), 0) AS respawn_at_unix, "
    "nt.npc_name, nt.level, nt.max_health, nt.max_mana, "
    "nt.physical_attack, nt.magic_attack, nt.physical_defense, nt.magic_defense, "
    "nt.accuracy, nt.dodge, nt.critical, nt.critical_resistance, "
    "nt.double_attack_rate, nt.double_attack_resistance, "
    "nt.skeletal_mesh_path, nt.anim_blueprint_path, "
    "COALESCE(nt.mesh_scale, 1.0) AS mesh_scale, "
    "nt.is_attackable, nt.interaction_radius, nt.has_vendor, nt.has_quest_dialog, "
    "COALESCE(nv.vendor_id, 0) AS vendor_id, "
    "COALESCE(nt.respawn_seconds, 30) AS respawn_seconds, "
    "COALESCE(nt.aggro_radius, 0) AS tpl_aggro, "
    "COALESCE(nt.leash_radius, 0) AS tpl_leash, "
    "COALESCE(nt.attack_range, 150) AS tpl_attack_range, "
    "COALESCE(nt.attack_cooldown_ms, 1500) AS tpl_attack_cd, "
    "COALESCE(nt.move_speed, 200) AS tpl_move_speed, "
    "COALESCE(nt.roam_radius, 0) AS tpl_roam, "
    "COALESCE(nt.is_hostile, 1) AS is_hostile, "
    "ni.home_x, ni.home_y, ni.home_z, "
    "ni.roam_radius AS inst_roam, ni.aggro_radius AS inst_aggro, "
    "ni.leash_radius AS inst_leash, ni.move_speed AS inst_move_speed "
    "FROM npc_instances ni "
    "JOIN npc_templates nt ON nt.npc_template_id = ni.npc_template_id "
    "LEFT JOIN npc_vendors nv ON nv.npc_template_id = nt.npc_template_id ";

std::string NpcManager::zoneWhereClause() {
  return "WHERE (ni.zone_id = ? OR ni.zone_id = 0) ";
}

NpcManager::NpcManager(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId)
    : db_(std::move(db)), zoneId_(zoneId) {}

void NpcManager::resetAiState(NpcRuntimeInstance& inst) {
  inst.aiState = NpcAiState::Idle;
  inst.targetPlayerId = 0;
  inst.hasWanderDest = false;
  inst.nextWanderAt = {};
  inst.lastAttackAt = {};
}

bool NpcManager::reloadFromDatabase() {
  if (!db_ || !db_->isConnected()) return false;

  std::lock_guard<std::mutex> lock(mu_);
  instances_.clear();
  indexById_.clear();

  const std::string zoneStr = std::to_string(zoneId_);
  auto rows = db_->executePreparedQuery(std::string(kInstanceSelectSql) + zoneWhereClause(), {zoneStr});

  for (const auto& row : rows) {
    loadInstanceFromRow(row);
  }

  Core::Logger::getInstance().info("[NpcManager] carregadas {} instancias de NPC para zone_id={}",
                                   instances_.size(), zoneId_);
  for (const auto& inst : instances_) {
    Core::Logger::getInstance().info(
        "[NpcManager] NPC id={} tpl={} name='{}' home=({:.1f},{:.1f},{:.1f}) roam={:.0f}",
        inst.npcInstanceId, inst.templateId, inst.npcName, inst.homeX, inst.homeY, inst.homeZ,
        inst.roamRadius);
  }
  return true;
}

bool NpcManager::loadInstanceById(uint32_t npcInstanceId) {
  if (npcInstanceId == 0 || !db_ || !db_->isConnected()) return false;

  {
    std::lock_guard<std::mutex> lock(mu_);
    if (indexById_.find(npcInstanceId) != indexById_.end()) return true;
  }

  const std::string idStr = std::to_string(npcInstanceId);
  const std::string zoneStr = std::to_string(zoneId_);
  auto rows = db_->executePreparedQuery(
      std::string(kInstanceSelectSql) + zoneWhereClause() + "AND ni.npc_instance_id = ? LIMIT 1",
      {zoneStr, idStr});

  if (rows.empty()) return false;

  std::lock_guard<std::mutex> lock(mu_);
  if (indexById_.find(npcInstanceId) != indexById_.end()) return true;
  loadInstanceFromRow(rows[0]);
  return indexById_.find(npcInstanceId) != indexById_.end();
}

size_t NpcManager::reloadMissingInstancesFromDatabase() {
  if (!db_ || !db_->isConnected()) return 0;

  const std::string zoneStr = std::to_string(zoneId_);
  auto rows = db_->executePreparedQuery(std::string(kInstanceSelectSql) + zoneWhereClause(), {zoneStr});

  size_t loaded = 0;
  std::lock_guard<std::mutex> lock(mu_);
  for (const auto& row : rows) {
    if (row.empty()) continue;
    try {
      const uint32_t id = static_cast<uint32_t>(std::stoul(row[0]));
      if (indexById_.find(id) != indexById_.end()) continue;
      const size_t before = instances_.size();
      loadInstanceFromRow(row);
      if (instances_.size() > before) ++loaded;
    } catch (...) {
    }
  }

  if (loaded > 0) {
    Core::Logger::getInstance().info("[NpcManager] {} instancia(s) NPC adicionada(s) em runtime zone_id={}",
                                     loaded, zoneId_);
  }
  return loaded;
}

void NpcManager::loadInstanceFromRow(const std::vector<std::string>& row) {
  if (row.size() < kMinRowFields) return;

  NpcRuntimeInstance inst;
  try {
    inst.npcInstanceId = static_cast<uint32_t>(std::stoul(row[0]));
    inst.templateId = static_cast<uint32_t>(std::stoul(row[1]));
    inst.zoneId = static_cast<uint32_t>(std::stoul(row[2]));
    inst.x = std::stof(row[3]);
    inst.y = std::stof(row[4]);
    inst.z = std::stof(row[5]);
    inst.yaw = std::stof(row[6]);
    inst.currentHealth = std::stoi(row[7]);
    inst.currentMana = std::stoi(row[8]);
    inst.isDead = (std::stoi(row[9]) != 0);
    const uint64_t respawnUnix = std::stoull(row[10]);
    inst.npcName = row[11];
    inst.level = static_cast<uint32_t>(std::stoul(row[12]));
    inst.maxHealth = std::stoi(row[13]);
    inst.maxMana = std::stoi(row[14]);
    inst.physicalAttack = std::stoi(row[15]);
    inst.magicAttack = std::stoi(row[16]);
    inst.physicalDefense = std::stoi(row[17]);
    inst.magicDefense = std::stoi(row[18]);
    inst.accuracy = std::stoi(row[19]);
    inst.dodge = std::stoi(row[20]);
    inst.critical = std::stoi(row[21]);
    inst.criticalResistance = std::stoi(row[22]);
    inst.doubleAttackRate = std::stoi(row[23]);
    inst.doubleAttackResistance = std::stoi(row[24]);
    inst.skeletalMeshPath = row[25];
    inst.animBlueprintPath = row[26];
    inst.meshScale = parseFloatOr(row[27], 1.f);
    if (inst.meshScale <= 0.01f) inst.meshScale = 1.f;
    inst.isAttackable = (std::stoi(row[28]) != 0);
    inst.interactionRadius = parseFloatOr(row[29], 300.f);
    inst.hasVendor = (std::stoi(row[30]) != 0);
    inst.hasQuestDialog = (std::stoi(row[31]) != 0);
    inst.vendorId = static_cast<uint32_t>(std::stoul(row[32]));
    inst.respawnSeconds = static_cast<uint32_t>(std::stoul(row[33]));
    if (inst.respawnSeconds == 0) inst.respawnSeconds = kDefaultRespawnSeconds;

    const float tplAggro = parseFloatOr(row[34], 0.f);
    const float tplLeash = parseFloatOr(row[35], 0.f);
    inst.attackRange = parseFloatOr(row[36], 150.f);
    inst.attackCooldownMs = static_cast<uint32_t>(std::max(1, std::stoi(row[37])));
    const float tplMove = parseFloatOr(row[38], 200.f);
    const float tplRoam = parseFloatOr(row[39], 0.f);
    inst.isHostile = (std::stoi(row[40]) != 0);

    // pos_* é a fonte da verdade do admin (MySQL / Manager).
    // Antes: spawn só em home_* → editar pos_* “não mudava nada”.
    const float posX = inst.x;
    const float posY = inst.y;
    const float posZ = inst.z;
    float oldHomeX = posX, oldHomeY = posY, oldHomeZ = posZ;
    const bool hadHome = parseOptionalFloat(row[41], oldHomeX) && parseOptionalFloat(row[42], oldHomeY) &&
                         parseOptionalFloat(row[43], oldHomeZ);
    const float homePosDistSq =
        (oldHomeX - posX) * (oldHomeX - posX) + (oldHomeY - posY) * (oldHomeY - posY) +
        (oldHomeZ - posZ) * (oldHomeZ - posZ);
    inst.homeX = posX;
    inst.homeY = posY;
    inst.homeZ = posZ;
    inst.x = posX;
    inst.y = posY;
    inst.z = posZ;
    if (hadHome && homePosDistSq > 1.f) {
      Core::Logger::getInstance().warn(
          "[NpcManager] NPC id={} pos!=home — usando pos=({:.1f},{:.1f},{:.1f}) e sync home (home antigo={:.1f},{:.1f},{:.1f})",
          inst.npcInstanceId, posX, posY, posZ, oldHomeX, oldHomeY, oldHomeZ);
      persistNpcSql(
          "UPDATE npc_instances SET home_x = " + std::to_string(posX) + ", home_y = " + std::to_string(posY) +
          ", home_z = " + std::to_string(posZ) + " WHERE npc_instance_id = " +
          std::to_string(inst.npcInstanceId));
    }

    float ov = 0.f;
    inst.roamRadius = parseOptionalFloat(row[44], ov) ? ov : tplRoam;
    inst.aggroRadius = parseOptionalFloat(row[45], ov) ? ov : tplAggro;
    inst.leashRadius = parseOptionalFloat(row[46], ov) ? ov : tplLeash;
    inst.moveSpeed = parseOptionalFloat(row[47], ov) ? ov : tplMove;
    if (inst.moveSpeed <= 0.f) inst.moveSpeed = 200.f;
    if (inst.attackRange <= 0.f) inst.attackRange = 150.f;

    inst.lastBroadcastX = inst.x;
    inst.lastBroadcastY = inst.y;
    inst.lastBroadcastYaw = inst.yaw;

    if (inst.isDead) {
      inst.respawnAt = respawnTimeFromUnix(respawnUnix);
    }
    resetAiState(inst);
  } catch (...) {
    return;
  }

  indexById_[inst.npcInstanceId] = instances_.size();
  instances_.push_back(std::move(inst));
}

const NpcRuntimeInstance* NpcManager::findInstance(uint32_t npcInstanceId) const {
  std::lock_guard<std::mutex> lock(mu_);
  auto it = indexById_.find(npcInstanceId);
  if (it == indexById_.end()) return nullptr;
  return &instances_[it->second];
}

void NpcManager::forEachAlive(const std::function<void(NpcRuntimeInstance&)>& fn) {
  std::lock_guard<std::mutex> lock(mu_);
  for (auto& inst : instances_) {
    if (inst.isDead) continue;
    fn(inst);
  }
}

bool NpcManager::mutateInstance(uint32_t npcInstanceId, const std::function<void(NpcRuntimeInstance&)>& fn) {
  std::lock_guard<std::mutex> lock(mu_);
  auto it = indexById_.find(npcInstanceId);
  if (it == indexById_.end()) return false;
  fn(instances_[it->second]);
  return true;
}

void NpcManager::persistNpcSql(const std::string& sql) {
  if (sql.empty()) return;
  if (asyncDbWrite_) {
    asyncDbWrite_(sql);
    return;
  }
  if (db_ && db_->isConnected()) {
    db_->executeQuery(sql);
  }
}

bool NpcManager::respawnInstance(NpcRuntimeInstance& inst) {
  inst.isDead = false;
  inst.currentHealth = inst.maxHealth;
  inst.currentMana = inst.maxMana;
  inst.respawnAt = {};
  // Volta ao home (não à última posição de chase).
  inst.x = inst.homeX;
  inst.y = inst.homeY;
  inst.z = inst.homeZ;
  resetAiState(inst);
  inst.lastBroadcastX = inst.x;
  inst.lastBroadcastY = inst.y;
  inst.lastBroadcastYaw = inst.yaw;

  persistNpcSql(
      "UPDATE npc_instances SET current_health = " + std::to_string(inst.currentHealth) +
      ", current_mana = " + std::to_string(inst.currentMana) +
      ", is_dead = 0, respawn_at = NULL" +
      ", pos_x = " + std::to_string(inst.x) +
      ", pos_y = " + std::to_string(inst.y) +
      ", pos_z = " + std::to_string(inst.z) +
      " WHERE npc_instance_id = " + std::to_string(inst.npcInstanceId));
  return true;
}

int32_t NpcManager::applyDamage(uint32_t npcInstanceId, int32_t delta, bool& outIsCrit, bool* outNpcDied) {
  outIsCrit = false;
  if (outNpcDied) *outNpcDied = false;
  if (delta == 0) return 0;

  if (!findInstance(npcInstanceId)) {
    loadInstanceById(npcInstanceId);
  }

  std::lock_guard<std::mutex> lock(mu_);
  auto it = indexById_.find(npcInstanceId);
  if (it == indexById_.end()) return 0;

  NpcRuntimeInstance& inst = instances_[it->second];
  if (inst.isDead) return 0;
  if (!inst.isAttackable && delta < 0) return 0;

  const int32_t before = inst.currentHealth;
  inst.currentHealth = std::max(0, std::min(inst.maxHealth, inst.currentHealth + delta));
  const int32_t applied = inst.currentHealth - before;

  if (inst.currentHealth <= 0) {
    inst.isDead = true;
    resetAiState(inst);
    const uint32_t respawnSec = std::max(1u, inst.respawnSeconds);
    inst.respawnAt = std::chrono::system_clock::now() + std::chrono::seconds(respawnSec);
    if (outNpcDied) *outNpcDied = true;

    persistNpcSql(
        "UPDATE npc_instances SET current_health = 0, is_dead = 1, respawn_at = DATE_ADD(NOW(), "
        "INTERVAL " +
        std::to_string(respawnSec) +
        " SECOND), last_combat_at = CURRENT_TIMESTAMP WHERE npc_instance_id = " +
        std::to_string(npcInstanceId));
    return applied;
  }

  persistNpcSql(
      "UPDATE npc_instances SET current_health = " + std::to_string(inst.currentHealth) +
      ", is_dead = 0, last_combat_at = CURRENT_TIMESTAMP WHERE npc_instance_id = " +
      std::to_string(npcInstanceId));

  return applied;
}

std::vector<uint32_t> NpcManager::tickRespawns(float deltaSeconds) {
  (void)deltaSeconds;
  std::vector<uint32_t> respawned;
  const auto now = std::chrono::system_clock::now();

  std::lock_guard<std::mutex> lock(mu_);
  for (auto& inst : instances_) {
    if (!inst.isDead) continue;
    if (inst.respawnAt.time_since_epoch().count() == 0) continue;
    if (now < inst.respawnAt) continue;

    const uint32_t id = inst.npcInstanceId;
    if (respawnInstance(inst)) {
      respawned.push_back(id);
    }
  }

  if (!respawned.empty()) {
    Core::Logger::getInstance().info("[NpcManager] {} NPC(s) respawned", respawned.size());
  }
  return respawned;
}

bool NpcManager::removeInstance(uint32_t npcInstanceId) {
  std::lock_guard<std::mutex> lock(mu_);
  auto it = indexById_.find(npcInstanceId);
  if (it == indexById_.end()) {
    return false;
  }

  const size_t idx = it->second;
  instances_.erase(instances_.begin() + static_cast<long long>(idx));
  indexById_.erase(it);
  for (size_t i = idx; i < instances_.size(); ++i) {
    indexById_[instances_[i].npcInstanceId] = i;
  }
  return true;
}

bool NpcManager::setInstanceTransform(uint32_t npcInstanceId, float x, float y, float z, float yaw,
                                      bool persistToDb, bool updateHome) {
  {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = indexById_.find(npcInstanceId);
    if (it == indexById_.end()) {
      return false;
    }
    NpcRuntimeInstance& inst = instances_[it->second];
    inst.x = x;
    inst.y = y;
    inst.z = z;
    inst.yaw = yaw;
    if (updateHome) {
      inst.homeX = x;
      inst.homeY = y;
      inst.homeZ = z;
      resetAiState(inst);
    }
    inst.lastBroadcastX = x;
    inst.lastBroadcastY = y;
    inst.lastBroadcastYaw = yaw;
  }

  if (persistToDb) {
    std::string sql =
        "UPDATE npc_instances SET pos_x = " + std::to_string(x) + ", pos_y = " + std::to_string(y) +
        ", pos_z = " + std::to_string(z) + ", yaw = " + std::to_string(yaw);
    if (updateHome) {
      sql += ", home_x = " + std::to_string(x) + ", home_y = " + std::to_string(y) +
             ", home_z = " + std::to_string(z);
    }
    sql += " WHERE npc_instance_id = " + std::to_string(npcInstanceId);
    persistNpcSql(sql);
  }
  return true;
}

bool NpcManager::setAggroTarget(uint32_t npcInstanceId, uint32_t playerId) {
  if (npcInstanceId == 0 || playerId == 0) return false;
  std::lock_guard<std::mutex> lock(mu_);
  auto it = indexById_.find(npcInstanceId);
  if (it == indexById_.end()) return false;
  NpcRuntimeInstance& inst = instances_[it->second];
  if (inst.isDead || !inst.isHostile) return false;
  // Dummy/estátua: sem roam e sem aggro → nunca entra em chase por dano.
  if (inst.aggroRadius <= 0.f && inst.roamRadius <= 0.f) return false;
  if (inst.aiState == NpcAiState::Return) return false;
  inst.targetPlayerId = playerId;
  inst.aiState = NpcAiState::Chase;  // força perseguir (não fica em Wander)
  inst.hasWanderDest = false;
  inst.nextWanderAt = {};
  return true;
}

NpcSpawnPayload NpcManager::toSpawnPayload(const NpcRuntimeInstance& inst) const {
  NpcSpawnPayload p;
  p.npcId = inst.npcInstanceId;
  p.templateId = inst.templateId;
  p.x = inst.x;
  p.y = inst.y;
  p.z = inst.z;
  p.yaw = inst.yaw;
  p.currentHealth = inst.currentHealth;
  p.maxHealth = inst.maxHealth;
  p.level = inst.level;
  p.npcName = inst.npcName;
  p.skeletalMeshPath = inst.skeletalMeshPath;
  p.animBlueprintPath = inst.animBlueprintPath;
  p.flags = 0;
  if (inst.isAttackable) p.flags |= 0x01;
  if (inst.hasVendor) p.flags |= 0x02;
  if (inst.hasQuestDialog) p.flags |= 0x04;
  p.interactionRadius = inst.interactionRadius;
  p.vendorId = inst.vendorId;
  p.meshScale = inst.meshScale;
  return p;
}

NpcStatePayload NpcManager::toStatePayload(const NpcRuntimeInstance& inst) const {
  NpcStatePayload p;
  p.npcId = inst.npcInstanceId;
  p.currentHealth = inst.currentHealth;
  p.maxHealth = inst.maxHealth;
  p.x = inst.x;
  p.y = inst.y;
  p.z = inst.z;
  p.yaw = inst.yaw;
  return p;
}

}  // namespace Zone
}  // namespace Umbra
