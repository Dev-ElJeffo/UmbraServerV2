#include "zone/NpcManager.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <ctime>

namespace Umbra {
namespace Zone {

namespace {
constexpr size_t kMinRowFields = 24;

std::chrono::system_clock::time_point respawnTimeFromUnix(uint64_t unixTs) {
  if (unixTs == 0) {
    return std::chrono::system_clock::now();
  }
  return std::chrono::system_clock::from_time_t(static_cast<std::time_t>(unixTs));
}
}  // namespace

const char* NpcManager::kInstanceSelectSql =
    "SELECT ni.npc_instance_id, ni.npc_template_id, ni.zone_id, ni.pos_x, ni.pos_y, ni.pos_z, ni.yaw, "
    "ni.current_health, ni.current_mana, ni.is_dead, "
    "COALESCE(UNIX_TIMESTAMP(ni.respawn_at), 0) AS respawn_at_unix, "
    "nt.npc_name, nt.level, nt.max_health, nt.max_mana, nt.physical_defense, "
    "nt.skeletal_mesh_path, nt.anim_blueprint_path, "
    "nt.is_attackable, nt.interaction_radius, nt.has_vendor, nt.has_quest_dialog, "
    "COALESCE(nv.vendor_id, 0) AS vendor_id, "
    "COALESCE(nt.respawn_seconds, 30) AS respawn_seconds "
    "FROM npc_instances ni "
    "JOIN npc_templates nt ON nt.npc_template_id = ni.npc_template_id "
    "LEFT JOIN npc_vendors nv ON nv.npc_template_id = nt.npc_template_id ";

std::string NpcManager::zoneWhereClause() {
  return "WHERE (ni.zone_id = ? OR ni.zone_id = 0) ";
}

NpcManager::NpcManager(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId)
    : db_(std::move(db)), zoneId_(zoneId) {}

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
    inst.physicalDefense = std::stoi(row[15]);
    inst.skeletalMeshPath = row[16];
    inst.animBlueprintPath = row[17];
    inst.isAttackable = (std::stoi(row[18]) != 0);
    inst.interactionRadius = std::stof(row[19]);
    inst.hasVendor = (std::stoi(row[20]) != 0);
    inst.hasQuestDialog = (std::stoi(row[21]) != 0);
    inst.vendorId = static_cast<uint32_t>(std::stoul(row[22]));
    inst.respawnSeconds = static_cast<uint32_t>(std::stoul(row[23]));
    if (inst.respawnSeconds == 0) {
      inst.respawnSeconds = kDefaultRespawnSeconds;
    }
    if (inst.isDead) {
      inst.respawnAt = respawnTimeFromUnix(respawnUnix);
    }
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

bool NpcManager::respawnInstance(NpcRuntimeInstance& inst) {
  inst.isDead = false;
  inst.currentHealth = inst.maxHealth;
  inst.currentMana = inst.maxMana;
  inst.respawnAt = {};

  if (db_ && db_->isConnected()) {
    const std::string idStr = std::to_string(inst.npcInstanceId);
    db_->executePreparedInsert(
        "UPDATE npc_instances SET current_health = ?, current_mana = ?, is_dead = 0, respawn_at = NULL "
        "WHERE npc_instance_id = ?",
        {std::to_string(inst.currentHealth), std::to_string(inst.currentMana), idStr});
  }
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
    const uint32_t respawnSec = std::max(1u, inst.respawnSeconds);
    inst.respawnAt = std::chrono::system_clock::now() + std::chrono::seconds(respawnSec);
    if (outNpcDied) *outNpcDied = true;

    if (db_ && db_->isConnected()) {
      const std::string idStr = std::to_string(npcInstanceId);
      db_->executePreparedInsert(
          "UPDATE npc_instances SET current_health = 0, is_dead = 1, "
          "respawn_at = DATE_ADD(NOW(), INTERVAL " +
              std::to_string(respawnSec) +
              " SECOND), last_combat_at = CURRENT_TIMESTAMP WHERE npc_instance_id = ?",
          {idStr});
    }
    return applied;
  }

  if (db_ && db_->isConnected()) {
    const std::string idStr = std::to_string(npcInstanceId);
    db_->executePreparedInsert(
        "UPDATE npc_instances SET current_health = ?, is_dead = 0, last_combat_at = CURRENT_TIMESTAMP "
        "WHERE npc_instance_id = ?",
        {std::to_string(inst.currentHealth), idStr});
  }

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
