#pragma once

#include "zone/MovementProtocol.hpp"
#include "database/MySQLConnector.hpp"
#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace Umbra {
namespace Zone {

enum class NpcAiState : uint8_t {
  Idle = 0,
  Wander = 1,
  Combat = 2,
  Chase = 3,
  Return = 4
};

struct NpcRuntimeInstance {
  uint32_t npcInstanceId = 0;
  uint32_t templateId = 0;
  uint32_t zoneId = 0;
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
  float yaw = 0.f;
  float homeX = 0.f;
  float homeY = 0.f;
  float homeZ = 0.f;
  int32_t currentHealth = 0;
  int32_t maxHealth = 0;
  int32_t currentMana = 0;
  int32_t maxMana = 0;
  uint32_t level = 1;
  int32_t physicalAttack = 0;
  int32_t magicAttack = 0;
  int32_t physicalDefense = 0;
  int32_t magicDefense = 0;
  int32_t accuracy = 0;
  int32_t dodge = 0;
  int32_t critical = 0;
  int32_t criticalResistance = 0;
  int32_t doubleAttackRate = 0;
  int32_t doubleAttackResistance = 0;
  std::string npcName;
  std::string skeletalMeshPath;
  std::string animBlueprintPath;
  float meshScale = 1.f;
  bool isAttackable = true;
  bool isHostile = true;
  float interactionRadius = 300.f;
  float aggroRadius = 0.f;
  float leashRadius = 0.f;
  float attackRange = 150.f;
  uint32_t attackCooldownMs = 1500;
  float moveSpeed = 200.f;
  float roamRadius = 0.f;
  bool hasVendor = false;
  bool hasQuestDialog = false;
  uint32_t vendorId = 0;
  bool isDead = false;
  uint32_t respawnSeconds = 30;
  std::chrono::system_clock::time_point respawnAt{};

  // AI runtime (não persistido a cada tick)
  NpcAiState aiState = NpcAiState::Idle;
  uint32_t targetPlayerId = 0;
  float wanderDestX = 0.f;
  float wanderDestY = 0.f;
  float wanderDestZ = 0.f;
  bool hasWanderDest = false;
  std::chrono::steady_clock::time_point nextWanderAt{};
  std::chrono::steady_clock::time_point lastAttackAt{};
  std::chrono::steady_clock::time_point lastBroadcastAt{};
  float lastBroadcastX = 0.f;
  float lastBroadcastY = 0.f;
  float lastBroadcastYaw = 0.f;

  float effectiveLeashRadius() const {
    if (leashRadius > 0.f) return leashRadius;
    if (aggroRadius > 0.f) return aggroRadius * 2.f;
    // Aggro só por dano: leash curto para não arrastar o mob para longe do spawn.
    if (roamRadius > 0.f) return std::max(roamRadius * 2.f, 1200.f);
    return 1200.f;
  }

  /** Distância 2D além da qual o mob desiste do alvo e volta ao home. */
  float effectiveDeaggroRadius() const {
    // Deve cobrir o leash — senão o mob “roama” de novo a poucos metros do player.
    const float leash = effectiveLeashRadius();
    if (aggroRadius > 0.f) return std::max(aggroRadius * 2.5f, leash);
    return std::max(std::max(attackRange * 3.f, 600.f), leash);
  }
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

  /** Mutação segura sob lock (AI tick). */
  void forEachAlive(const std::function<void(NpcRuntimeInstance&)>& fn);
  bool mutateInstance(uint32_t npcInstanceId, const std::function<void(NpcRuntimeInstance&)>& fn);

  /** Aplica dano/cura; retorna delta aplicado. outNpcDied=true quando HP chegou a 0. */
  int32_t applyDamage(uint32_t npcInstanceId, int32_t delta, bool& outIsCrit, bool* outNpcDied = nullptr);

  /** Escrita assíncrona de SQL (write-behind). Se não definido, usa DB síncrono. */
  void setAsyncDbWrite(std::function<void(std::string)> fn) { asyncDbWrite_ = std::move(fn); }

  /** Processa respawns pendentes; retorna IDs que voltaram a ficar vivos. */
  std::vector<uint32_t> tickRespawns(float deltaSeconds);
  bool removeInstance(uint32_t npcInstanceId);
  /**
   * Atualiza transform em memória (e opcionalmente persiste no MySQL).
   * Se updateHome=true, redefine home_* (admin move / nova área de roam).
   */
  bool setInstanceTransform(uint32_t npcInstanceId, float x, float y, float z, float yaw,
                            bool persistToDb = false, bool updateHome = false);

  /** Aggro por dano: fixa alvo e entra em combate se hostil. */
  bool setAggroTarget(uint32_t npcInstanceId, uint32_t playerId);

  NpcSpawnPayload toSpawnPayload(const NpcRuntimeInstance& inst) const;
  NpcStatePayload toStatePayload(const NpcRuntimeInstance& inst) const;

private:
  static const char* kInstanceSelectSql;
  static std::string zoneWhereClause();
  void loadInstanceFromRow(const std::vector<std::string>& row);
  bool respawnInstance(NpcRuntimeInstance& inst);
  void resetAiState(NpcRuntimeInstance& inst);
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
