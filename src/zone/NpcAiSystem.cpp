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

/** Slot 2D estável em torno do alvo — evita vários NPCs no mesmo XY no chase. */
constexpr float kChaseSeparationRadius = 100.f;
void chaseAimXY(uint32_t npcInstanceId, float targetX, float targetY, float& outX, float& outY) {
  const float angle =
      (static_cast<float>((npcInstanceId * 2654435761u) & 0xFFFFu) / 65535.f) * 6.28318530718f;
  outX = targetX + std::cos(angle) * kChaseSeparationRadius;
  outY = targetY + std::sin(angle) * kChaseSeparationRadius;
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
}  // namespace

NpcAiSystem::NpcAiSystem(NpcManager* npcManager, MovementServer* movementServer,
                           CombatCoreEngine* combat)
    : npcManager_(npcManager), movementServer_(movementServer), combat_(combat) {}

void NpcAiSystem::tick(float deltaSeconds) {
  if (!npcManager_ || !movementServer_ || deltaSeconds <= 0.f) return;

  const auto players = movementServer_->getPlayerStates();
  const auto now = std::chrono::steady_clock::now();

  std::vector<uint32_t> attackQueue;
  attackQueue.reserve(8);

  npcManager_->forEachAlive([&](NpcRuntimeInstance& inst) {
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
        // Melee: só ataca bem perto; fora disso persegue (evita “bater de longe”).
        const float minDist = inst.bodyMinDist();
        const float attackR = std::max(50.f, inst.attackRange);
        const float meleeReach = std::max(std::max(40.f, attackR * 0.75f), minDist);
        float aimX = tx, aimY = ty;
        chaseAimXY(inst.npcInstanceId, tx, ty, aimX, aimY);
        const float toPlayer2d = std::sqrt(dist2dSq(inst.x, inst.y, tx, ty));
        if (toPlayer2d <= meleeReach) {
          inst.aiState = NpcAiState::Combat;
          inst.z = inst.homeZ;
          // Mantém slot de separation (não empilha no mesmo ponto do player).
          {
            const float sdx = aimX - inst.x;
            const float sdy = aimY - inst.y;
            const float sdist = std::sqrt(sdx * sdx + sdy * sdy);
            const float step = inst.moveSpeed * deltaSeconds * 0.6f;
            if (sdist > 25.f && step > 0.f) {
              const float t = std::min(1.f, step / sdist);
              inst.x += sdx * t;
              inst.y += sdy * t;
            }
          }
          const float dx = tx - inst.x;
          const float dy = ty - inst.y;
          if (dx * dx + dy * dy > 1.f) {
            inst.yaw = yawFromDir(dx, dy);
          }
          const auto readyAt =
              inst.lastAttackAt + std::chrono::milliseconds(inst.attackCooldownMs);
          if (now >= readyAt) {
            attackQueue.push_back(inst.npcInstanceId);
            inst.lastAttackAt = now;
          }
        } else {
          inst.aiState = NpcAiState::Chase;
          const float step = inst.moveSpeed * deltaSeconds;
          const float dx = aimX - inst.x;
          const float dy = aimY - inst.y;
          const float dist = std::sqrt(dx * dx + dy * dy);
          if (dist > 1.f && step > 0.f) {
            const float t = std::min(1.f, step / dist);
            inst.x += dx * t;
            inst.y += dy * t;
            // Nunca copiar Z do player — evita flutuação / spawn deslocado.
            inst.z = inst.homeZ;
            inst.yaw = yawFromDir(tx - inst.x, ty - inst.y);
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
        const float step = inst.moveSpeed * deltaSeconds * 1.15f;  // volta um pouco mais rápido
        if (dist > 1.f && step > 0.f) {
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
            const float step = inst.moveSpeed * deltaSeconds;
            if (dist > 1.f && step > 0.f) {
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

    if (inst.aiState != NpcAiState::Dying) {
      const float minDist = inst.bodyMinDist();
      for (const auto& kv : players) {
        const PlayerStateNet& p = kv.second;
        if (p.isDead || p.playerId == 0) continue;
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

  for (const uint32_t npcId : attackQueue) {
    if (!combat_) break;
    uint32_t targetId = 0;
    npcManager_->mutateInstance(npcId, [&](NpcRuntimeInstance& inst) {
      targetId = inst.targetPlayerId;
    });
    if (targetId > 0) {
      combat_->processNpcBasicAttack(npcId, targetId);
    }
  }
}

}  // namespace Zone
}  // namespace Umbra
