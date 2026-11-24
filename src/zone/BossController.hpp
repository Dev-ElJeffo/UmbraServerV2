#pragma once
// BossController.hpp — chefes com fases (etapa 5b, opcode 140).
//
// Config por TEMPLATE de NPC (npc_boss_configs/phases/phase_skills/phase_adds). Estado de cada chefe
// fica no próprio NpcRuntimeInstance (bossPhase, multiplicadores, enrage...). Tudo que muda a lista de
// NPCs (adds) ou depende da ordem do dano roda no combat worker (CombatCoreEngine::enqueueWorldOp),
// serializado com o processamento de combate.

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "zone/NpcManager.hpp"

namespace Umbra {
namespace Database {
class MySQLConnector;
}
namespace Zone {

class MovementServer;
class CombatCoreEngine;

struct BossPhaseDef {
  uint8_t order = 1;
  std::string name;
  /** Entra quando a vida <= X% (a fase 1 ignora). */
  uint8_t hpThresholdPct = 100;
  float damageMult = 1.f;
  float speedMult = 1.f;
  float attackCooldownMult = 1.f;
  uint32_t invulnerableMs = 0;
  std::string announce;
  std::string animPath;
  std::string vfxPath;
  /** Vazio = skills do template. */
  std::vector<NpcRuntimeInstance::NpcBoundSkill> skills;
  struct Add {
    uint32_t templateId = 0;
    uint32_t count = 1;
    float radius = 400.f;
  };
  std::vector<Add> adds;
};

struct BossConfigDef {
  uint32_t templateId = 0;
  uint32_t enrageSeconds = 0;
  float enrageDamageMult = 2.f;
  std::string enrageText;
  bool resetOnLeash = true;
  bool showBar = true;
  /** Ordenadas por phase_order; [0] = fase 1. Vazio = só barra/enrage. */
  std::vector<BossPhaseDef> phases;
};

// ---- Funções puras (tests/test_boss_phases.cpp) ----

/** Índice da fase mais profunda alcançada com essa vida (0 = fase 1). Fases só avançam: o chamador
 *  aplica do índice atual + 1 até o retornado. */
size_t bossPhaseIndexForHealth(const std::vector<BossPhaseDef>& phases, int32_t currentHealth, int32_t maxHealth);

/** Enrage vence? (enrageSeconds 0 = nunca). */
bool bossEnrageDue(uint32_t enrageSeconds, int64_t combatElapsedMs);

class BossController {
 public:
  BossController(std::shared_ptr<Database::MySQLConnector> db, NpcManager* npcs, MovementServer* movement,
                 CombatCoreEngine* combat);

  /** npc_boss_* → memória. Retorna quantos templates são chefes. */
  size_t reload();
  bool isBossTemplate(uint32_t templateId) const;

  /** NPC tomou dano (qualquer thread): enfileira a checagem de fase no combat worker. */
  void onNpcDamaged(uint32_t npcInstanceId, uint32_t attackerPlayerId);
  /** NPC morreu (qualquer thread): esconde a barra e remove os adds do chefe (no combat worker). */
  void onNpcDied(uint32_t npcInstanceId);
  /** 1 s (tick da zone): enfileira a varredura de entrada em combate, enrage e reset. */
  void tick();

  /** 140 dos chefes em combate na camada, só para o cliente (quem entra/troca de camada). */
  void sendSnapshotToClient(uint32_t clientId, uint32_t layer);

 private:
  struct ScanEntry {
    uint32_t npcId = 0;
    uint32_t templateId = 0;
    uint32_t layer = 0;
    bool chasing = false;
    bool inCombat = false;
    bool enraged = false;
    int64_t combatMs = 0;
  };

  bool getConfig(uint32_t templateId, BossConfigDef& out) const;
  void processDamage(uint32_t npcInstanceId, uint32_t attackerPlayerId);
  void processDeath(uint32_t npcInstanceId);
  void processScan();
  void resetBoss(uint32_t npcInstanceId, const BossConfigDef& cfg);
  BossStatePayload buildState(const NpcRuntimeInstance& inst, const BossConfigDef& cfg) const;
  void broadcastState(uint32_t layer, const BossStatePayload& p);

  std::shared_ptr<Database::MySQLConnector> db_;
  NpcManager* npcs_ = nullptr;
  MovementServer* movement_ = nullptr;
  CombatCoreEngine* combat_ = nullptr;

  mutable std::mutex mu_;
  std::unordered_map<uint32_t, BossConfigDef> configs_;
};

}  // namespace Zone
}  // namespace Umbra
