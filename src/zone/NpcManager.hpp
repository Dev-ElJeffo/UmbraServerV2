#pragma once

#include "zone/MovementProtocol.hpp"
#include "database/MySQLConnector.hpp"
#include <chrono>
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
  bool isDead = false;
  std::chrono::steady_clock::time_point respawnAt{};
};

class NpcManager {
public:
  static constexpr int kDefaultRespawnSeconds = 10;

  explicit NpcManager(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId);

  bool reloadFromDatabase();
  /** Carrega uma instância viva do DB; retorna true se já estava carregada ou foi adicionada. */
  bool loadInstanceById(uint32_t npcInstanceId);
  /** Carrega instâncias vivas presentes no DB mas ausentes na memória. Retorna quantas foram adicionadas. */
  size_t reloadMissingInstancesFromDatabase();

  const std::vector<NpcRuntimeInstance>& getAllInstances() const { return instances_; }
  const NpcRuntimeInstance* findInstance(uint32_t npcInstanceId) const;

  /** Aplica dano/cura; retorna delta aplicado. outNpcDied=true quando HP chegou a 0. */
  int32_t applyDamage(uint32_t npcInstanceId, int32_t delta, bool& outIsCrit, bool* outNpcDied = nullptr);
  /** Processa respawns pendentes; retorna IDs que voltaram a ficar vivos. */
  std::vector<uint32_t> tickRespawns(float deltaSeconds);

  NpcSpawnPayload toSpawnPayload(const NpcRuntimeInstance& inst) const;
  NpcStatePayload toStatePayload(const NpcRuntimeInstance& inst) const;

private:
  static const char* kInstanceSelectSql;
  void loadInstanceFromRow(const std::vector<std::string>& row);

  std::shared_ptr<Database::MySQLConnector> db_;
  uint32_t zoneId_;
  mutable std::mutex mu_;
  std::vector<NpcRuntimeInstance> instances_;
  std::unordered_map<uint32_t, size_t> indexById_;
};

}  // namespace Zone
}  // namespace Umbra
