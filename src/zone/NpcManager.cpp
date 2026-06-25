#include "zone/NpcManager.hpp"
#include "core/Logger.hpp"
#include <algorithm>

namespace Umbra {
namespace Zone {

const char* NpcManager::kInstanceSelectSql =
    "SELECT ni.npc_instance_id, ni.npc_template_id, ni.zone_id, ni.pos_x, ni.pos_y, ni.pos_z, ni.yaw, "
    "ni.current_health, ni.current_mana, ni.is_dead, "
    "nt.npc_name, nt.level, nt.max_health, nt.max_mana, nt.physical_defense, "
    "nt.skeletal_mesh_path, nt.anim_blueprint_path "
    "FROM npc_instances ni "
    "JOIN npc_templates nt ON nt.npc_template_id = ni.npc_template_id ";

NpcManager::NpcManager(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId)
    : db_(std::move(db)), zoneId_(zoneId) {}

bool NpcManager::reloadFromDatabase() {
  if (!db_ || !db_->isConnected()) return false;

  std::lock_guard<std::mutex> lock(mu_);
  instances_.clear();
  indexById_.clear();

  const std::string zoneStr = std::to_string(zoneId_);
  auto rows = db_->executePreparedQuery(
      std::string(kInstanceSelectSql) + "WHERE ni.zone_id = ? AND ni.is_dead = 0",
      {zoneStr});

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
      std::string(kInstanceSelectSql) +
          "WHERE ni.npc_instance_id = ? AND ni.zone_id = ? AND ni.is_dead = 0 LIMIT 1",
      {idStr, zoneStr});

  if (rows.empty()) return false;

  std::lock_guard<std::mutex> lock(mu_);
  if (indexById_.find(npcInstanceId) != indexById_.end()) return true;
  loadInstanceFromRow(rows[0]);
  return indexById_.find(npcInstanceId) != indexById_.end();
}

size_t NpcManager::reloadMissingInstancesFromDatabase() {
  if (!db_ || !db_->isConnected()) return 0;

  const std::string zoneStr = std::to_string(zoneId_);
  auto rows = db_->executePreparedQuery(
      std::string(kInstanceSelectSql) + "WHERE ni.zone_id = ? AND ni.is_dead = 0",
      {zoneStr});

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
  if (row.size() < 17) return;

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
    inst.npcName = row[10];
    inst.level = static_cast<uint32_t>(std::stoul(row[11]));
    inst.maxHealth = std::stoi(row[12]);
    inst.maxMana = std::stoi(row[13]);
    inst.physicalDefense = std::stoi(row[14]);
    inst.skeletalMeshPath = row[15];
    inst.animBlueprintPath = row[16];
  } catch (...) {
    return;
  }

  if (inst.isDead) return;

  indexById_[inst.npcInstanceId] = instances_.size();
  instances_.push_back(std::move(inst));
}

const NpcRuntimeInstance* NpcManager::findInstance(uint32_t npcInstanceId) const {
  std::lock_guard<std::mutex> lock(mu_);
  auto it = indexById_.find(npcInstanceId);
  if (it == indexById_.end()) return nullptr;
  return &instances_[it->second];
}

int32_t NpcManager::applyDamage(uint32_t npcInstanceId, int32_t delta, bool& outIsCrit, bool* outNpcDied) {
  outIsCrit = false;
  if (outNpcDied) *outNpcDied = false;
  if (delta == 0) return 0;

  std::lock_guard<std::mutex> lock(mu_);
  auto it = indexById_.find(npcInstanceId);
  if (it == indexById_.end()) return 0;

  NpcRuntimeInstance& inst = instances_[it->second];
  if (inst.isDead) return 0;

  const int32_t before = inst.currentHealth;
  inst.currentHealth = std::max(0, std::min(inst.maxHealth, inst.currentHealth + delta));
  const int32_t applied = inst.currentHealth - before;

  if (inst.currentHealth <= 0) {
    inst.isDead = true;
    inst.respawnAt = std::chrono::steady_clock::now() +
                     std::chrono::seconds(kDefaultRespawnSeconds);
    if (outNpcDied) *outNpcDied = true;
  }

  if (db_ && db_->isConnected()) {
    const std::string idStr = std::to_string(npcInstanceId);
    if (inst.isDead) {
      db_->executePreparedInsert(
          "UPDATE npc_instances SET current_health = ?, is_dead = 1, "
          "respawn_at = DATE_ADD(NOW(), INTERVAL " +
              std::to_string(kDefaultRespawnSeconds) +
              " SECOND), last_combat_at = CURRENT_TIMESTAMP WHERE npc_instance_id = ?",
          {std::to_string(inst.currentHealth), idStr});
    } else {
      db_->executePreparedInsert(
          "UPDATE npc_instances SET current_health = ?, is_dead = 0, last_combat_at = CURRENT_TIMESTAMP "
          "WHERE npc_instance_id = ?",
          {std::to_string(inst.currentHealth), idStr});
    }
  }

  return applied;
}

std::vector<uint32_t> NpcManager::tickRespawns(float deltaSeconds) {
  (void)deltaSeconds;
  std::vector<uint32_t> respawned;
  if (!db_ || !db_->isConnected()) return respawned;

  const auto now = std::chrono::steady_clock::now();

  {
    std::lock_guard<std::mutex> lock(mu_);
    for (auto& inst : instances_) {
      if (!inst.isDead || inst.respawnAt.time_since_epoch().count() == 0) continue;
      if (now < inst.respawnAt) continue;

      inst.isDead = false;
      inst.currentHealth = inst.maxHealth;
      inst.currentMana = inst.maxMana;
      inst.respawnAt = {};
      respawned.push_back(inst.npcInstanceId);

      const std::string idStr = std::to_string(inst.npcInstanceId);
      db_->executePreparedInsert(
          "UPDATE npc_instances SET current_health = ?, current_mana = ?, is_dead = 0, respawn_at = NULL "
          "WHERE npc_instance_id = ?",
          {std::to_string(inst.currentHealth), std::to_string(inst.currentMana), idStr});
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
