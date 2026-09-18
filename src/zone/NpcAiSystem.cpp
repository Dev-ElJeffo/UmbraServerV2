#include "zone/NpcAiSystem.hpp"
#include "zone/NpcManager.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "zone/CombatRange.hpp"
#include "zone/MovementServer.hpp"
#include "core/Logger.hpp"

#include <chrono>
#include <cmath>
#include <random>

namespace Umbra {
namespace Zone {

namespace {
constexpr float kStateBroadcastHz = 8.f;
constexpr float kWanderPauseMinSec = 1.5f;
constexpr float kWanderPauseMaxSec = 4.0f;
constexpr float kArriveEpsilon = 40.f;
constexpr float kHomeArriveEpsilon = 60.f;

thread_local std::mt19937 gNpcAiRng{std::random_device{}()};

float randFloat(float lo, float hi) {
  std::uniform_real_distribution<float> dist(lo, hi);
  return dist(gNpcAiRng);
}

float yawFromDir(float dx, float dy) {
  return std::atan2(dy, dx) * (180.f / 3.14159265358979323846f);
}

float dist2dSq(float x1, float y1, float x2, float y2) {
  const float dx = x2 - x1;
  const float dy = y2 - y1;
  return dx * dx + dy * dy;
}

/** Ponto do anel holdRadius na linha NPC→player (mais próximo) — sem órbita por hash. */
void chaseAimXY(float npcX, float npcY, float targetX, float targetY, float holdRadius,
                float& outX, float& outY) {
  const float r = std::max(40.f, holdRadius);
  const float dx = npcX - targetX;
  const float dy = npcY - targetY;
  const float len = std::sqrt(dx * dx + dy * dy);
  if (len > 0.001f) {
    outX = targetX + (dx / len) * r;
    outY = targetY + (dy / len) * r;
  } else {
    outX = targetX + r;
    outY = targetY;
  }
}

void forceBroadcast(CombatCoreEngine* combat, NpcManager* npcManager, NpcRuntimeInstance& inst,
                    const std::chrono::steady_clock::time_point& now) {
  if (!combat || !npcManager) return;
  combat->broadcastNpcStatePublic(npcManager->toStatePayload(inst));
  inst.lastBroadcastX = inst.x;
  inst.lastBroadcastY = inst.y;
  inst.lastBroadcastYaw = inst.yaw;
  inst.lastBroadcastAt = now;
}

void enterReturn(NpcRuntimeInstance& inst) {
  inst.targetPlayerId = 0;
  inst.aiState = NpcAiState::Return;
  inst.hasWanderDest = false;
}

struct PendingNpcAttack {
  uint32_t npcInstanceId = 0;
  uint32_t npcSkillId = 0;  // 0 = basic attack
};

bool boundSkillReady(const NpcRuntimeInstance::NpcBoundSkill& b,
                     const std::chrono::steady_clock::time_point& now) {
  if (b.weight <= 0) return false;
  if (b.lastUsedAt.time_since_epoch().count() == 0) return true;
  return now >= b.lastUsedAt + std::chrono::milliseconds(b.cooldownMs);
}

uint32_t pickBoundSkillId(const NpcRuntimeInstance& inst, float distToTarget, bool includeBasic,
                           const std::chrono::steady_clock::time_point& now) {
  struct Cand {
    uint32_t id = 0;
    int32_t w = 0;
  };
  std::vector<Cand> cands;
  int32_t total = 0;
  for (const auto& b : inst.boundSkills) {
    if (!boundSkillReady(b, now)) continue;
    const float range = std::max(50.f, static_cast<float>(b.rangeMax));
    if (distToTarget > range * 1.05f) continue;
    const int32_t w = std::max(0, b.weight);
    if (w <= 0) continue;
    cands.push_back({b.npcSkillId, w});
    total += w;
  }
  if (includeBasic) {
    cands.push_back({0, 100});
    total += 100;
  }
  if (cands.empty() || total <= 0) return 0;
  std::uniform_int_distribution<int32_t> dist(1, total);
  int32_t roll = dist(gNpcAiRng);
  for (const auto& c : cands) {
    roll -= c.w;
    if (roll <= 0) return c.id;
  }
  return cands.back().id;
}
}  // namespace

NpcAiSystem::NpcAiSystem(NpcManager* npcManager, MovementServer* movementServer,
                           CombatCoreEngine* combat)
    : npcManager_(npcManager), movementServer_(movementServer), combat_(combat) {}

void NpcAiSystem::tick(float deltaSeconds) {
  if (!npcManager_ || !movementServer_ || deltaSeconds <= 0.f) return;

  const auto players = movementServer_->getPlayerStates();
  const auto now = std::chrono::steady_clock::now();

  std::vector<PendingNpcAttack> attackQueue;
  attackQueue.reserve(8);

  npcManager_->forEachAlive([&](NpcRuntimeInstance& inst) {
    // Usar overloads com `inst` — NÃO chamar findInstance aqui (forEachAlive já segura mu_).
    const bool npcCanMove = !combat_ || combat_->canNpcMove(inst);
    const bool npcCanAct = !combat_ || combat_->canNpcAct(inst, false);
    const float speedMultiplier =
        combat_ ? combat_->getNpcMovementSpeedPercent(inst) / 100.f : 1.f;
    // --- Aggro por proximidade ---
    if (inst.isHostile && inst.aggroRadius > 0.f &&
        (inst.aiState == NpcAiState::Idle || inst.aiState == NpcAiState::Wander)) {
      const float aggroSq = inst.aggroRadius * inst.aggroRadius;
      uint32_t bestId = 0;
      float bestDist = aggroSq;
      for (const auto& kv : players) {
        const PlayerStateNet& p = kv.second;
        if (p.isDead || p.playerId == 0) continue;
        const float d = dist2dSq(inst.x, inst.y, p.x, p.y);
        if (d <= bestDist) {
          bestDist = d;
          bestId = p.playerId;
        }
      }
      if (bestId > 0) {
        inst.targetPlayerId = bestId;
        inst.aiState = NpcAiState::Chase;
        inst.hasWanderDest = false;
        inst.nextWanderAt = {};
      }
    }

    // --- Resolve alvo / leash / deaggro ---
    if (inst.aiState == NpcAiState::Combat || inst.aiState == NpcAiState::Chase) {
      const auto it = players.find(inst.targetPlayerId);
      bool lost = (inst.targetPlayerId == 0 || it == players.end() || it->second.isDead);
      float tx = 0.f, ty = 0.f;
      if (!lost) {
        tx = it->second.x;
        ty = it->second.y;
        const float leash = inst.effectiveLeashRadius();
        const float fromHome = std::sqrt(dist2dSq(inst.homeX, inst.homeY, inst.x, inst.y));
        if (fromHome > leash) {
          lost = true;
        } else {
          // Saiu do raio de interesse → volta ao spawn (não fica seguindo o player pelo mapa).
          const float deaggro = inst.effectiveDeaggroRadius();
          const float toPlayer = std::sqrt(dist2dSq(inst.x, inst.y, tx, ty));
          if (toPlayer > deaggro) {
            lost = true;
          }
        }
      }
      if (lost) {
        enterReturn(inst);
      } else {
        // Para de chase em combat_stop_range (capado ao melee reach) e ataca sem orbitar.
        const float stopR = inst.effectiveCombatStopRange();
        const float meleeReach = inst.effectiveNpcMeleeReach2D();
        float aimX = tx, aimY = ty;
        chaseAimXY(inst.x, inst.y, tx, ty, stopR, aimX, aimY);
        const float toPlayer2d = std::sqrt(dist2dSq(inst.x, inst.y, tx, ty));
        if (toPlayer2d <= stopR) {
          inst.aiState = NpcAiState::Combat;
          inst.z = inst.homeZ;
          // Mantém distância de stop: se o player aproximou, o NPC recua até ~stopR.
          const float holdMin = stopR * 0.92f;
          if (npcCanMove && toPlayer2d < holdMin && toPlayer2d > 0.001f) {
            const float awayX = inst.x - tx;
            const float awayY = inst.y - ty;
            const float awayLen = std::sqrt(awayX * awayX + awayY * awayY);
            if (awayLen > 0.001f) {
              const float kiteMult = inst.effectiveKiteSpeedMult();
              const float step = inst.moveSpeed * kiteMult * speedMultiplier * deltaSeconds;
              const float need = stopR - toPlayer2d;
              const float move = std::min(need, step);
              inst.x += (awayX / awayLen) * move;
              inst.y += (awayY / awayLen) * move;
              inst.z = inst.homeZ;
            }
          }
          // Stop-and-attack: não orbita; face o player.
          const float dx = tx - inst.x;
          const float dy = ty - inst.y;
          if (dx * dx + dy * dy > 1.f) {
            inst.yaw = yawFromDir(dx, dy);
          }
          const auto readyAt =
              inst.lastAttackAt + std::chrono::milliseconds(inst.attackCooldownMs);
          // Só enfileira basic/skills e consome CD se já estiver no reach de hit.
          if (npcCanAct && now >= readyAt && toPlayer2d <= meleeReach) {
            const uint32_t skillId = pickBoundSkillId(inst, toPlayer2d, true, now);
            attackQueue.push_back({inst.npcInstanceId, skillId});
            inst.lastAttackAt = now;
          }
        } else {
          inst.aiState = NpcAiState::Chase;
          const float chaseMult = inst.effectiveChaseSpeedMult();
          const float step = inst.moveSpeed * chaseMult * speedMultiplier * deltaSeconds;
          float dx = aimX - inst.x;
          float dy = aimY - inst.y;
          float dist = std::sqrt(dx * dx + dy * dy);
          // Dead zone do anel: se já no aim mas ainda fora do stop, fecha direto no player.
          if (dist <= 1.f && toPlayer2d > stopR) {
            dx = tx - inst.x;
            dy = ty - inst.y;
            dist = std::sqrt(dx * dx + dy * dy);
          }
          if (npcCanMove && dist > 1.f && step > 0.f) {
            const float t = std::min(1.f, step / dist);
            inst.x += dx * t;
            inst.y += dy * t;
            // Nunca copiar Z do player — evita flutuação / spawn deslocado.
            inst.z = inst.homeZ;
            inst.yaw = yawFromDir(tx - inst.x, ty - inst.y);
          }
          if (npcCanAct &&
              now >= inst.lastAttackAt + std::chrono::milliseconds(inst.attackCooldownMs)) {
            const uint32_t skillId = pickBoundSkillId(inst, toPlayer2d, false, now);
            if (skillId > 0) {
              attackQueue.push_back({inst.npcInstanceId, skillId});
              inst.lastAttackAt = now;
            }
          }
          // Chase: leash um pouco mais folgado que o idle (senão kita e o mob “desiste”).
          const float leash = std::max(inst.effectiveLeashRadius(), inst.effectiveDeaggroRadius());
          const float fromHome = std::sqrt(dist2dSq(inst.homeX, inst.homeY, inst.x, inst.y));
          if (fromHome > leash && fromHome > 0.001f) {
            enterReturn(inst);
          }
        }
      }
    }

    // --- Return ao home ---
    if (inst.aiState == NpcAiState::Return) {
      const float dx = inst.homeX - inst.x;
      const float dy = inst.homeY - inst.y;
      const float dist = std::sqrt(dx * dx + dy * dy);
      if (dist <= kHomeArriveEpsilon) {
        inst.x = inst.homeX;
        inst.y = inst.homeY;
        inst.z = inst.homeZ;
        inst.aiState = NpcAiState::Idle;
        inst.targetPlayerId = 0;
        inst.hasWanderDest = false;
        inst.nextWanderAt = now + std::chrono::milliseconds(
                                      static_cast<int>(randFloat(kWanderPauseMinSec, kWanderPauseMaxSec) * 1000.f));
        forceBroadcast(combat_, npcManager_, inst, now);
      } else {
        const float step =
            inst.moveSpeed * speedMultiplier * deltaSeconds * 1.15f;  // volta um pouco mais rápido
        if (npcCanMove && dist > 1.f && step > 0.f) {
          const float t = std::min(1.f, step / dist);
          inst.x += dx * t;
          inst.y += dy * t;
          inst.z = inst.homeZ;
          inst.yaw = yawFromDir(dx, dy);
        }
      }
    }

    // --- Wander ---
    if (inst.aiState == NpcAiState::Idle || inst.aiState == NpcAiState::Wander) {
      if (inst.roamRadius <= 0.f) {
        inst.aiState = NpcAiState::Idle;
        inst.z = inst.homeZ;
      } else {
        if (!inst.hasWanderDest) {
          if (inst.nextWanderAt.time_since_epoch().count() == 0 || now >= inst.nextWanderAt) {
            const float angle = randFloat(0.f, 6.28318530718f);
            const float r = randFloat(inst.roamRadius * 0.25f, inst.roamRadius);
            inst.wanderDestX = inst.homeX + std::cos(angle) * r;
            inst.wanderDestY = inst.homeY + std::sin(angle) * r;
            inst.wanderDestZ = inst.homeZ;
            inst.hasWanderDest = true;
            inst.aiState = NpcAiState::Wander;
          }
        } else {
          const float dx = inst.wanderDestX - inst.x;
          const float dy = inst.wanderDestY - inst.y;
          const float dist = std::sqrt(dx * dx + dy * dy);
          if (dist <= kArriveEpsilon) {
            inst.hasWanderDest = false;
            inst.aiState = NpcAiState::Idle;
            inst.z = inst.homeZ;
            inst.nextWanderAt = now + std::chrono::milliseconds(
                                          static_cast<int>(randFloat(kWanderPauseMinSec, kWanderPauseMaxSec) * 1000.f));
          } else {
            const float step = inst.moveSpeed * speedMultiplier * deltaSeconds;
            if (npcCanMove && dist > 1.f && step > 0.f) {
              const float t = std::min(1.f, step / dist);
              inst.x += dx * t;
              inst.y += dy * t;
              inst.z = inst.homeZ;
              inst.yaw = yawFromDir(dx, dy);
            }
            const float fromHome = std::sqrt(dist2dSq(inst.homeX, inst.homeY, inst.x, inst.y));
            if (fromHome > inst.roamRadius && fromHome > 0.001f) {
              const float s = inst.roamRadius / fromHome;
              inst.x = inst.homeX + (inst.x - inst.homeX) * s;
              inst.y = inst.homeY + (inst.y - inst.homeY) * s;
              inst.z = inst.homeZ;
            }
            inst.aiState = NpcAiState::Wander;
          }
        }
      }
    }

    if (npcCanMove && inst.aiState != NpcAiState::Dying) {
      for (const auto& kv : players) {
        const PlayerStateNet& p = kv.second;
        if (p.isDead || p.playerId == 0) continue;
        // Separação física só por body: o anel de stop fica no recuo limitado por kite_speed_mult.
        // Usar combat_stop_range aqui teleportava o ranged e impedia melee de fechar.
        const float minDist = inst.bodyMinDist();
        const float dx = inst.x - p.x;
        const float dy = inst.y - p.y;
        const float d = std::sqrt(dx * dx + dy * dy);
        if (d < 0.001f) {
          inst.x = p.x + minDist;
          inst.y = p.y;
          inst.z = inst.homeZ;
        } else if (d < minDist) {
          const float s = minDist / d;
          inst.x = p.x + dx * s;
          inst.y = p.y + dy * s;
          inst.z = inst.homeZ;
        }
      }
    }

    // --- Broadcast 102 throttle (Chase/Return: mais frequente para o cliente acompanhar) ---
    if (combat_) {
      const float dx = inst.x - inst.lastBroadcastX;
      const float dy = inst.y - inst.lastBroadcastY;
      float dyaw = inst.yaw - inst.lastBroadcastYaw;
      if (dyaw > 180.f) dyaw -= 360.f;
      if (dyaw < -180.f) dyaw += 360.f;
      const bool chasing = (inst.aiState == NpcAiState::Chase || inst.aiState == NpcAiState::Return);
      const float moveThreshSq = chasing ? 25.f : 100.f;  // ~5uu vs ~10uu
      const bool moved = (dx * dx + dy * dy) > moveThreshSq || std::fabs(dyaw) > 5.f;
      const float hz = chasing ? (kStateBroadcastHz * 2.f) : kStateBroadcastHz;
      const auto minInterval = std::chrono::milliseconds(static_cast<int>(1000.f / hz));
      if (moved && (inst.lastBroadcastAt.time_since_epoch().count() == 0 ||
                    now - inst.lastBroadcastAt >= minInterval)) {
        forceBroadcast(combat_, npcManager_, inst, now);
      }
    }
  });

  for (const PendingNpcAttack& atk : attackQueue) {
    if (!combat_) break;
    uint32_t targetId = 0;
    npcManager_->mutateInstance(atk.npcInstanceId, [&](NpcRuntimeInstance& inst) {
      targetId = inst.targetPlayerId;
    });
    if (targetId == 0) continue;
    if (atk.npcSkillId > 0) {
      combat_->processNpcSkillCast(atk.npcInstanceId, targetId, atk.npcSkillId);
    } else {
      combat_->processNpcBasicAttack(atk.npcInstanceId, targetId);
    }
  }
}

}  // namespace Zone
}  // namespace Umbra
