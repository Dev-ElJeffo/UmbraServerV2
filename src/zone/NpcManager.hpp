#pragma once

#include "zone/MovementProtocol.hpp"
#include "database/MySQLConnector.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace Umbra {
namespace Zone {

struct NpcRuntimeInstance {
  uint32_t npcInstanceId = 0;
  uint32_t templateId = 0;
  uint32_t zoneId = 0;
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
  float yaw = 0.f;
  int32_t currentHealth = 0;
  int32_t maxHealth = 0;
  int32_t currentMana = 0;
  int32_t maxMana = 0;
  uint32_t level = 1;
  int32_t physicalDefense = 0;
  std::string npcName;
  std::string skeletalMeshPath;
  std::string animBlueprintPath;
  bool isAttackable = true;
  float interactionRadius = 300.f;
  bool hasVendor = false;
  bool hasQuestDialog = false;
  uint32_t vendorId = 0;
  bool isDead = false;
  uint32_t respawnSeconds = 30;
  std::chrono::system_clock::time_point respawnAt{};
};

class NpcManager {
public:
  static constexpr uint32_t kDefaultRespawnSeconds = 30;

  explicit NpcManager(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId);

  bool reloadFromDatabase();
  /** Carrega instância do DB (viva ou morta aguardando respawn). */
  bool loadInstanceById(uint32_t npcInstanceId);
  /** Carrega instâncias do DB ausentes na memória. Retorna quantas foram adicionadas. */
  size_t reloadMissingInstancesFromDatabase();

  const std::vector<NpcRuntimeInstance>& getAllInstances() const { return instances_; }
  const NpcRuntimeInstance* findInstance(uint32_t npcInstanceId) const;

  /** Aplica dano/cura; retorna delta aplicado. outNpcDied=true quando HP chegou a 0. */
  int32_t applyDamage(uint32_t npcInstanceId, int32_t delta, bool& outIsCrit, bool* outNpcDied = nullptr);

  /** Escrita assíncrona de SQL (write-behind). Se não definido, usa DB síncrono. */
  void setAsyncDbWrite(std::function<void(std::string)> fn) { asyncDbWrite_ = std::move(fn); }

  /** Processa respawns pendentes; retorna IDs que voltaram a ficar vivos. */
  std::vector<uint32_t> tickRespawns(float deltaSeconds);

  NpcSpawnPayload toSpawnPayload(const NpcRuntimeInstance& inst) const;
  NpcStatePayload toStatePayload(const NpcRuntimeInstance& inst) const;

private:
  static const char* kInstanceSelectSql;
  static std::string zoneWhereClause();
  void loadInstanceFromRow(const std::vector<std::string>& row);
  bool respawnInstance(NpcRuntimeInstance& inst);
  void persistNpcSql(const std::string& sql);

  std::shared_ptr<Database::MySQLConnector> db_;
  std::function<void(std::string)> asyncDbWrite_;
  uint32_t zoneId_;
  mutable std::mutex mu_;
  std::vector<NpcRuntimeInstance> instances_;
  std::unordered_map<uint32_t, size_t> indexById_;
};

}  // namespace Zone
}  // namespace Umbra
