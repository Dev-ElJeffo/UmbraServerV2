// BossController.cpp — chefes com fases (etapa 5b). Ver BossController.hpp.
#include "zone/BossController.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "core/Logger.hpp"
#include "database/MySQLConnector.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "zone/MovementProtocol.hpp"
#include "zone/MovementServer.hpp"

namespace Umbra {
namespace Zone {

namespace {
uint32_t toU32(const std::string& s, uint32_t def) {
  if (s.empty()) return def;
  try {
    return static_cast<uint32_t>(std::stoul(s));
  } catch (...) {
    return def;
  }
}
float toF(const std::string& s, float def) {
  if (s.empty()) return def;
  try {
    return std::stof(s);
  } catch (...) {
    return def;
  }
}
int64_t msSince(std::chrono::steady_clock::time_point t) {
  if (t.time_since_epoch().count() == 0) return 0;
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t).count();
}
}  // namespace

// ---------------- Funções puras ----------------

size_t bossPhaseIndexForHealth(const std::vector<BossPhaseDef>& phases, int32_t currentHealth, int32_t maxHealth) {
  if (phases.size() < 2 || maxHealth <= 0) return 0;
  const int64_t hp100 = static_cast<int64_t>(std::max(0, currentHealth)) * 100;
  size_t idx = 0;
  for (size_t i = 1; i < phases.size(); ++i) {
    if (hp100 <= static_cast<int64_t>(phases[i].hpThresholdPct) * maxHealth) idx = i;
  }
  return idx;
}

bool bossEnrageDue(uint32_t enrageSeconds, int64_t combatElapsedMs) {
  return enrageSeconds > 0 && combatElapsedMs >= static_cast<int64_t>(enrageSeconds) * 1000;
}

// ---------------- BossController ----------------

BossController::BossController(std::shared_ptr<Database::MySQLConnector> db, NpcManager* npcs,
                               MovementServer* movement, CombatCoreEngine* combat)
    : db_(std::move(db)), npcs_(npcs), movement_(movement), combat_(combat) {}

size_t BossController::reload() {
  if (!db_ || !db_->isConnected()) return 0;
  if (db_->executeQuery("SHOW TABLES LIKE 'npc_boss_configs'").empty()) {
    Core::Logger::getInstance().warn(
        "[BossController] tabelas npc_boss_* ausentes: rode www/umbra_api/scripts/add_boss_phases.sql");
    return 0;
  }
  std::unordered_map<uint32_t, BossConfigDef> loaded;
  for (const auto& r : db_->executeQuery(
           "SELECT npc_template_id, enrage_seconds, enrage_damage_mult, enrage_text, reset_on_leash, show_boss_bar "
           "FROM npc_boss_configs")) {
    if (r.size() < 6) continue;
    BossConfigDef c;
    c.templateId = toU32(r[0], 0);
    if (c.templateId == 0) continue;
    c.enrageSeconds = toU32(r[1], 0);
    c.enrageDamageMult = std::clamp(toF(r[2], 2.f), 1.f, 100.f);
    c.enrageText = r[3];
    c.resetOnLeash = toU32(r[4], 1) != 0;
    c.showBar = toU32(r[5], 1) != 0;
    loaded[c.templateId] = std::move(c);
  }
  std::unordered_map<uint32_t, std::pair<uint32_t, size_t>> phaseIndex;  // phase_id -> (template, índice)
  for (const auto& r : db_->executeQuery(
           "SELECT phase_id, npc_template_id, phase_order, name, hp_threshold_pct, damage_mult, speed_mult, "
           "attack_cooldown_mult, invulnerable_ms, announce_text, transition_anim_path, transition_vfx_path "
           "FROM npc_boss_phases ORDER BY npc_template_id, phase_order")) {
    if (r.size() < 12) continue;
    auto it = loaded.find(toU32(r[1], 0));
    if (it == loaded.end()) continue;  // fase sem config: ignorada
    BossPhaseDef p;
    p.order = static_cast<uint8_t>(std::clamp<uint32_t>(toU32(r[2], 1), 1, 255));
    p.name = r[3];
    p.hpThresholdPct = static_cast<uint8_t>(std::clamp<uint32_t>(toU32(r[4], 100), 1, 100));
    p.damageMult = std::clamp(toF(r[5], 1.f), 0.1f, 100.f);
    p.speedMult = std::clamp(toF(r[6], 1.f), 0.2f, 5.f);
    p.attackCooldownMult = std::clamp(toF(r[7], 1.f), 0.2f, 5.f);
    p.invulnerableMs = std::min<uint32_t>(toU32(r[8], 0), 30000);
    p.announce = r[9];
    p.animPath = r[10];
    p.vfxPath = r[11];
    phaseIndex[toU32(r[0], 0)] = {it->first, it->second.phases.size()};
    it->second.phases.push_back(std::move(p));
  }
  for (const auto& r : db_->executeQuery(
           "SELECT ps.phase_id, ps.npc_skill_id, ps.skill_rank, ps.weight, ps.cooldown_override_ms, ns.cooldown_ms, "
           "ns.range_max FROM npc_boss_phase_skills ps JOIN npc_skills ns ON ns.npc_skill_id = ps.npc_skill_id "
           "WHERE ns.is_enabled = 1")) {
    if (r.size() < 7) continue;
    auto pi = phaseIndex.find(toU32(r[0], 0));
    if (pi == phaseIndex.end()) continue;
    NpcRuntimeInstance::NpcBoundSkill b;
    b.npcSkillId = toU32(r[1], 0);
    b.rank = static_cast<uint8_t>(std::clamp<uint32_t>(toU32(r[2], 1), 1, 255));
    b.weight = static_cast<int32_t>(toU32(r[3], 100));
    const uint32_t ov = toU32(r[4], 0);
    b.cooldownMs = ov > 0 ? ov : toU32(r[5], 4000);
    b.rangeMax = static_cast<uint16_t>(std::min<uint32_t>(toU32(r[6], 200), 65535));
    if (b.npcSkillId == 0) continue;
    loaded[pi->second.first].phases[pi->second.second].skills.push_back(b);
  }
  for (const auto& r : db_->executeQuery("SELECT phase_id, npc_template_id, count, radius FROM npc_boss_phase_adds")) {
    if (r.size() < 4) continue;
    auto pi = phaseIndex.find(toU32(r[0], 0));
    if (pi == phaseIndex.end()) continue;
    BossPhaseDef::Add a;
    a.templateId = toU32(r[1], 0);
    a.count = std::clamp<uint32_t>(toU32(r[2], 1), 1, 20);
    a.radius = std::clamp(toF(r[3], 400.f), 50.f, 3000.f);
    if (a.templateId == 0) continue;
    loaded[pi->second.first].phases[pi->second.second].adds.push_back(a);
  }
  size_t count = 0;
  {
    std::lock_guard<std::mutex> lock(mu_);
    configs_ = std::move(loaded);
    count = configs_.size();
  }
  Core::Logger::getInstance().info("[BossController] {} chefe(s) configurado(s)", count);
  return count;
}

bool BossController::isBossTemplate(uint32_t templateId) const {
  std::lock_guard<std::mutex> lock(mu_);
  return configs_.count(templateId) > 0;
}

bool BossController::getConfig(uint32_t templateId, BossConfigDef& out) const {
  std::lock_guard<std::mutex> lock(mu_);
  auto it = configs_.find(templateId);
  if (it == configs_.end()) return false;
  out = it->second;
  return true;
}

BossStatePayload BossController::buildState(const NpcRuntimeInstance& inst, const BossConfigDef& cfg) const {
  BossStatePayload p;
  p.npcId = inst.npcInstanceId;
  const size_t phaseCount = std::max<size_t>(1, cfg.phases.size());
  p.phase = static_cast<uint8_t>(std::min<size_t>(inst.bossPhase + 1, phaseCount));
  p.phaseCount = static_cast<uint8_t>(std::min<size_t>(phaseCount, 255));
  if (inst.bossPhase < cfg.phases.size()) p.phaseName = cfg.phases[inst.bossPhase].name;
  if (inst.bossEnraged) p.flags |= kBossFlagEnraged;
  if (inst.bossInCombat) p.flags |= kBossFlagInCombat;
  if (cfg.showBar) p.flags |= kBossFlagShowBar;
  const auto now = std::chrono::steady_clock::now();
  if (now < inst.invulnerableUntil) {
    p.flags |= kBossFlagInvulnerable;
    p.invulnerableMs = static_cast<uint16_t>(std::min<int64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(inst.invulnerableUntil - now).count(), 0xFFFF));
  }
  if (inst.bossInCombat && !inst.bossEnraged && cfg.enrageSeconds > 0) {
    const int64_t left = static_cast<int64_t>(cfg.enrageSeconds) - msSince(inst.bossCombatStartedAt) / 1000;
    p.enrageInSeconds = static_cast<uint16_t>(std::clamp<int64_t>(left, 0, 0xFFFF));
  }
  for (size_t i = 1; i < cfg.phases.size(); ++i) p.thresholds.push_back(cfg.phases[i].hpThresholdPct);
  return p;
}

void BossController::broadcastState(uint32_t layer, const BossStatePayload& p) {
  if (movement_) movement_->broadcastToLayer(layer, encodeBossState(p));
}

void BossController::onNpcDamaged(uint32_t npcInstanceId, uint32_t attackerPlayerId) {
  if (!combat_ || !npcs_) return;
  {
    // Filtro barato antes de enfileirar: só templates de chefe.
    const NpcRuntimeInstance* inst = npcs_->findInstance(npcInstanceId);
    if (!inst || !isBossTemplate(inst->templateId)) return;
  }
  combat_->enqueueWorldOp([this, npcInstanceId, attackerPlayerId]() { processDamage(npcInstanceId, attackerPlayerId); });
}

void BossController::onNpcDied(uint32_t npcInstanceId) {
  if (!combat_ || !npcs_) return;
  {
    const NpcRuntimeInstance* inst = npcs_->findInstance(npcInstanceId);
    if (!inst || !isBossTemplate(inst->templateId)) return;
  }
  combat_->enqueueWorldOp([this, npcInstanceId]() { processDeath(npcInstanceId); });
}

void BossController::tick() {
  if (!combat_) return;
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (configs_.empty()) return;
  }
  combat_->enqueueWorldOp([this]() { processScan(); });
}

void BossController::processDamage(uint32_t npcInstanceId, uint32_t attackerPlayerId) {
  uint32_t templateId = 0;
  {
    const NpcRuntimeInstance* inst = npcs_->findInstance(npcInstanceId);
    if (!inst || inst->isDead) return;
    templateId = inst->templateId;
  }
  BossConfigDef cfg;
  if (!getConfig(templateId, cfg)) return;

  bool startedCombat = false;
  bool changedPhase = false;
  BossStatePayload state;
  std::string announce, anim, vfx;
  std::vector<BossPhaseDef::Add> adds;
  uint32_t layer = 0;
  float bx = 0.f, by = 0.f, bz = 0.f;
  npcs_->mutateInstance(npcInstanceId, [&](NpcRuntimeInstance& inst) {
    if (inst.isDead) return;
    if (!inst.bossInitialized) {
      inst.baseBoundSkills = inst.boundSkills;
      inst.bossInitialized = true;
    }
    if (!inst.bossInCombat) {
      inst.bossInCombat = true;
      inst.bossCombatStartedAt = std::chrono::steady_clock::now();
      startedCombat = true;
    }
    const size_t target = bossPhaseIndexForHealth(cfg.phases, inst.currentHealth, inst.maxHealth);
    if (target > inst.bossPhase && target < cfg.phases.size()) {
      // Fases puladas por dano alto: aplica todas em sequência; anuncia só a última (D4).
      for (size_t i = inst.bossPhase + 1; i <= target; ++i) {
        const BossPhaseDef& ph = cfg.phases[i];
        adds.insert(adds.end(), ph.adds.begin(), ph.adds.end());
        if (!ph.announce.empty()) announce = ph.announce;
        if (!ph.animPath.empty()) anim = ph.animPath;
        if (!ph.vfxPath.empty()) vfx = ph.vfxPath;
      }
      const BossPhaseDef& last = cfg.phases[target];
      inst.bossPhase = static_cast<uint8_t>(target);
      inst.bossDamageMult = last.damageMult * (inst.bossEnraged ? cfg.enrageDamageMult : 1.f);
      inst.bossSpeedMult = last.speedMult;
      inst.bossAttackCooldownMult = last.attackCooldownMult;
      inst.boundSkills = last.skills.empty() ? inst.baseBoundSkills : last.skills;
      if (last.invulnerableMs > 0) {
        inst.invulnerableUntil = std::chrono::steady_clock::now() + std::chrono::milliseconds(last.invulnerableMs);
      }
      changedPhase = true;
    } else if (startedCombat && !cfg.phases.empty()) {
      // Entrou em combate na fase 1: aplica os multiplicadores dela.
      const BossPhaseDef& first = cfg.phases[0];
      inst.bossDamageMult = first.damageMult;
      inst.bossSpeedMult = first.speedMult;
      inst.bossAttackCooldownMult = first.attackCooldownMult;
      if (!first.skills.empty()) inst.boundSkills = first.skills;
    }
    layer = inst.instanceId;
    bx = inst.x;
    by = inst.y;
    bz = inst.z;
    state = buildState(inst, cfg);
  });
  if (!startedCombat && !changedPhase) return;

  // Adds (só dentro de instância: limpeza garantida ao fechar a cópia).
  if (changedPhase && layer != 0) {
    for (const auto& a : adds) {
      const auto ids = npcs_->spawnTemplateClones(a.templateId, layer, bx, by, bz, a.count, a.radius, npcInstanceId,
                                                  attackerPlayerId);
      for (uint32_t id : ids) {
        if (const NpcRuntimeInstance* add = npcs_->findInstance(id)) combat_->broadcastNpcSpawnToAll(*add);
      }
    }
  }
  state.announce = announce;
  state.animPath = anim;
  state.vfxPath = vfx;
  broadcastState(layer, state);
  if (changedPhase) {
    Core::Logger::getInstance().info("[BossController] chefe {} (tpl {}) entrou na fase {}/{} ({} add(s))",
                                     npcInstanceId, templateId, state.phase, state.phaseCount, adds.size());
  }
}

void BossController::processDeath(uint32_t npcInstanceId) {
  uint32_t templateId = 0;
  uint32_t layer = 0;
  {
    const NpcRuntimeInstance* inst = npcs_->findInstance(npcInstanceId);
    if (!inst) return;
    templateId = inst->templateId;
    layer = inst->instanceId;
  }
  BossConfigDef cfg;
  if (!getConfig(templateId, cfg)) return;
  BossStatePayload state;
  npcs_->mutateInstance(npcInstanceId, [&](NpcRuntimeInstance& inst) {
    inst.bossInCombat = false;
    state = buildState(inst, cfg);
  });
  broadcastState(layer, state);  // sem flag de combate: a barra some
  for (uint32_t id : npcs_->removeBossAdds(npcInstanceId)) {
    NpcDespawnPayload d;
    d.npcId = id;
    d.reason = 1;
    if (movement_) movement_->broadcastToLayer(layer, encodeNpcDespawnNotify(d));
  }
}

void BossController::resetBoss(uint32_t npcInstanceId, const BossConfigDef& cfg) {
  BossStatePayload state;
  NpcStatePayload npcState;
  uint32_t layer = 0;
  bool done = false;
  npcs_->mutateInstance(npcInstanceId, [&](NpcRuntimeInstance& inst) {
    if (inst.isDead) return;
    inst.bossPhase = 0;
    inst.bossInCombat = false;
    inst.bossEnraged = false;
    inst.bossCombatStartedAt = {};
    inst.invulnerableUntil = {};
    inst.bossDamageMult = 1.f;
    inst.bossSpeedMult = 1.f;
    inst.bossAttackCooldownMult = 1.f;
    if (inst.bossInitialized) inst.boundSkills = inst.baseBoundSkills;
    if (cfg.resetOnLeash) inst.currentHealth = inst.maxHealth;
    layer = inst.instanceId;
    state = buildState(inst, cfg);
    npcState = npcs_->toStatePayload(inst);
    done = true;
  });
  if (!done) return;
  // Sem isso os DOTs dos jogadores mortos continuam batendo e o chefe volta ao combate a cada scan.
  if (combat_) combat_->clearNpcHostileEffects(npcInstanceId);
  for (uint32_t id : npcs_->removeBossAdds(npcInstanceId)) {
    NpcDespawnPayload d;
    d.npcId = id;
    d.reason = 0;
    if (movement_) movement_->broadcastToLayer(layer, encodeNpcDespawnNotify(d));
  }
  if (combat_) combat_->broadcastNpcStatePublic(npcState, layer);  // vida cheia no cliente
  broadcastState(layer, state);
  Core::Logger::getInstance().info("[BossController] chefe {} reiniciado (wipe/leash)", npcInstanceId);
}

void BossController::processScan() {
  std::vector<ScanEntry> entries;
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (configs_.empty()) return;
  }
  npcs_->forEachAlive([&](NpcRuntimeInstance& inst) {
    if (!inst.bossInitialized && !isBossTemplate(inst.templateId)) return;
    ScanEntry e;
    e.npcId = inst.npcInstanceId;
    e.templateId = inst.templateId;
    e.layer = inst.instanceId;
    e.chasing = inst.aiState == NpcAiState::Chase || inst.aiState == NpcAiState::Combat;
    e.inCombat = inst.bossInCombat;
    e.enraged = inst.bossEnraged;
    e.combatMs = msSince(inst.bossCombatStartedAt);
    entries.push_back(e);
  });
  for (const auto& e : entries) {
    BossConfigDef cfg;
    if (!getConfig(e.templateId, cfg)) continue;
    if (!e.inCombat && e.chasing) {
      processDamage(e.npcId, 0);  // aggro por proximidade: entra em combate e mostra a barra
      continue;
    }
    if (e.inCombat && !e.chasing) {
      resetBoss(e.npcId, cfg);  // perdeu o alvo e voltou ao home (wipe/fuga)
      continue;
    }
    if (e.inCombat && !e.enraged && bossEnrageDue(cfg.enrageSeconds, e.combatMs)) {
      BossStatePayload state;
      uint32_t layer = 0;
      std::string bossName;
      npcs_->mutateInstance(e.npcId, [&](NpcRuntimeInstance& inst) {
        inst.bossEnraged = true;
        inst.bossDamageMult *= cfg.enrageDamageMult;
        layer = inst.instanceId;
        bossName = inst.npcName;
        state = buildState(inst, cfg);
      });
      state.announce = cfg.enrageText.empty() ? bossName + " se enfurece!" : cfg.enrageText;
      broadcastState(layer, state);
      Core::Logger::getInstance().info("[BossController] chefe {} enfureceu", e.npcId);
    }
  }
}

void BossController::sendSnapshotToClient(uint32_t clientId, uint32_t layer) {
  if (!movement_ || !npcs_) return;
  for (const auto& inst : npcs_->getAllInstances()) {
    if (inst.instanceId != layer || inst.isDead || !inst.bossInCombat) continue;
    BossConfigDef cfg;
    if (!getConfig(inst.templateId, cfg)) continue;
    movement_->sendBinaryToClient(clientId, encodeBossState(buildState(inst, cfg)));
  }
}

}  // namespace Zone
}  // namespace Umbra
