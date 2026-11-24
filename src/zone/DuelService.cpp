// DuelService.cpp — duelo 1x1 na zone (etapa 6, opcodes 50-54).
#include "zone/DuelService.hpp"

#include <algorithm>
#include <cmath>
#include <string>

#include "core/Logger.hpp"
#include "database/MySQLConnector.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "zone/MovementProtocol.hpp"
#include "zone/MovementServer.hpp"

namespace Umbra {
namespace Zone {

namespace {
uint32_t toU32(const std::string& s, uint32_t def) {
  try {
    return s.empty() ? def : static_cast<uint32_t>(std::stoul(s));
  } catch (...) {
    return def;
  }
}

float toF(const std::string& s, float def) {
  try {
    return s.empty() ? def : std::stof(s);
  } catch (...) {
    return def;
  }
}

using Clock = std::chrono::steady_clock;
}  // namespace

DuelService::DuelService(std::shared_ptr<Database::MySQLConnector> db, uint32_t zoneId, MovementServer* movement,
                         CombatCoreEngine* combat)
    : db_(std::move(db)), zoneId_(zoneId), movement_(movement), combat_(combat) {}

bool DuelService::reload() {
  PvpSettings s;
  bool ok = false;
  if (db_ && db_->isConnected() && !db_->executeQuery("SHOW TABLES LIKE 'pvp_settings'").empty()) {
    const auto rows = db_->executeQuery(
        "SELECT open_world_pvp, duel_request_distance, duel_request_timeout_sec, duel_radius, duel_countdown_sec, "
        "duel_max_duration_sec, duel_out_of_bounds_sec, duel_restore_on_end, duel_flag_mesh_path, "
        "duel_flag_vfx_path FROM pvp_settings WHERE id = 1");
    if (!rows.empty() && rows[0].size() >= 10) {
      const auto& r = rows[0];
      s.openWorldPvp = toU32(r[0], 0) != 0;
      s.requestDistance = std::clamp(toF(r[1], 1500.f), 100.f, 20000.f);
      s.requestTimeoutSec = std::clamp<uint32_t>(toU32(r[2], 30), 5, 120);
      s.radius = std::clamp(toF(r[3], 3000.f), 500.f, 20000.f);
      s.countdownSec = std::min<uint32_t>(toU32(r[4], 3), 10);
      s.maxDurationSec = std::clamp<uint32_t>(toU32(r[5], 180), 30, 3600);
      s.outOfBoundsSec = std::clamp<uint32_t>(toU32(r[6], 5), 1, 30);
      s.restoreOnEnd = toU32(r[7], 0) != 0;
      s.flagMeshPath = r[8];
      s.flagVfxPath = r[9];
      ok = true;
    }
  } else {
    Core::Logger::getInstance().warn(
        "[DuelService] tabela pvp_settings ausente: rode www/umbra_api/scripts/add_pvp_duel.sql (usando padrões)");
  }
  {
    std::lock_guard<std::mutex> lock(mu_);
    settings_ = s;
  }
  Core::Logger::getInstance().info("[DuelService] PvP livre no mundo={} raio={} contagem={}s duração={}s",
                                   s.openWorldPvp ? 1 : 0, s.radius, s.countdownSec, s.maxDurationSec);
  return ok;
}

PvpSettings DuelService::settings() const {
  std::lock_guard<std::mutex> lock(mu_);
  return settings_;
}

bool DuelService::openWorldPvp() const {
  std::lock_guard<std::mutex> lock(mu_);
  return settings_.openWorldPvp;
}

const DuelService::ActiveDuel* DuelService::duelOfLocked(uint32_t playerId) const {
  auto it = duelByPlayer_.find(playerId);
  if (it == duelByPlayer_.end()) return nullptr;
  auto d = duels_.find(it->second);
  return d == duels_.end() ? nullptr : &d->second;
}

bool DuelService::busyLocked(uint32_t playerId) const {
  if (duelByPlayer_.count(playerId) > 0) return true;
  // Desafio pendente enviado ou recebido também ocupa (um por vez).
  return std::any_of(pending_.begin(), pending_.end(), [playerId](const PendingRequest& p) {
    return p.fromId == playerId || p.targetId == playerId;
  });
}

void DuelService::sendNotice(uint32_t playerId, uint32_t otherId, uint8_t reason) {
  if (!movement_ || playerId == 0) return;
  DuelEndedPayload p;
  p.duelId = 0;
  p.winnerId = otherId;  // resposta ao desafio: o outro jogador envolvido
  p.loserId = playerId;
  p.reason = reason;
  movement_->sendToPlayer(playerId, encodeDuelEnded(p));
}

void DuelService::handleRequest(uint32_t fromPlayerId, uint32_t targetPlayerId) {
  if (!movement_ || fromPlayerId == 0) return;
  auto reply = [&](uint8_t reason) { sendNotice(fromPlayerId, targetPlayerId, reason); };
  if (targetPlayerId == 0 || targetPlayerId == fromPlayerId) {
    reply(static_cast<uint8_t>(DuelEndReason::RequestInvalid));
    return;
  }
  PlayerBrief a, b;
  if (!movement_->tryGetPlayerBrief(fromPlayerId, a) || !movement_->tryGetPlayerBrief(targetPlayerId, b) ||
      a.isDead || b.isDead) {
    reply(static_cast<uint8_t>(DuelEndReason::RequestInvalid));
    return;
  }
  // D4: só no mundo aberto (instâncias e dungeons são cooperativas).
  if (a.layer != 0 || b.layer != 0) {
    reply(static_cast<uint8_t>(DuelEndReason::RequestNotHere));
    return;
  }
  uint32_t timeoutSec = 30;
  {
    std::lock_guard<std::mutex> lock(mu_);
    const float maxD = settings_.requestDistance;
    const float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    if (dx * dx + dy * dy + dz * dz > maxD * maxD) {
      // fora do lock abaixo
      timeoutSec = 0;
    } else if (busyLocked(fromPlayerId) || busyLocked(targetPlayerId)) {
      timeoutSec = 1;
    } else {
      timeoutSec = settings_.requestTimeoutSec;
      PendingRequest p;
      p.fromId = fromPlayerId;
      p.targetId = targetPlayerId;
      p.expiresAt = Clock::now() + std::chrono::seconds(timeoutSec);
      pending_.push_back(p);
    }
  }
  if (timeoutSec == 0) {
    reply(static_cast<uint8_t>(DuelEndReason::RequestTooFar));
    return;
  }
  if (timeoutSec == 1) {
    reply(static_cast<uint8_t>(DuelEndReason::RequestBusy));
    return;
  }
  DuelRequestReceivedPayload r;
  r.fromPlayerId = fromPlayerId;
  r.expiresSec = static_cast<uint8_t>(std::min<uint32_t>(timeoutSec, 255));
  r.fromName = a.name;
  movement_->sendToPlayer(targetPlayerId, encodeDuelRequestReceived(r));
  reply(static_cast<uint8_t>(DuelEndReason::RequestSent));
  Core::Logger::getInstance().info("[DuelService] {} desafiou {}", fromPlayerId, targetPlayerId);
}

void DuelService::handleResponse(uint32_t playerId, uint8_t action, uint32_t otherPlayerId) {
  if (!movement_ || playerId == 0) return;
  const auto act = static_cast<DuelResponseAction>(action);

  if (act == DuelResponseAction::Forfeit) {
    EndAction end;
    {
      std::lock_guard<std::mutex> lock(mu_);
      const ActiveDuel* d = duelOfLocked(playerId);
      if (!d) return;
      const uint32_t winner = d->p1 == playerId ? d->p2 : d->p1;
      end = takeDuelLocked(d->duelId, winner, playerId, static_cast<uint8_t>(DuelEndReason::Forfeit));
    }
    finish(end);
    return;
  }

  if (act == DuelResponseAction::CancelRequest) {
    uint32_t target = 0;
    {
      std::lock_guard<std::mutex> lock(mu_);
      auto it = std::find_if(pending_.begin(), pending_.end(),
                             [playerId](const PendingRequest& p) { return p.fromId == playerId; });
      if (it == pending_.end()) return;
      target = it->targetId;
      pending_.erase(it);
    }
    sendNotice(target, playerId, static_cast<uint8_t>(DuelEndReason::RequestCanceled));
    sendNotice(playerId, target, static_cast<uint8_t>(DuelEndReason::RequestCanceled));
    return;
  }

  // Aceitar/recusar: o desafio precisa ser de otherPlayerId para playerId.
  bool found = false;
  {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = std::find_if(pending_.begin(), pending_.end(), [&](const PendingRequest& p) {
      return p.fromId == otherPlayerId && p.targetId == playerId;
    });
    if (it != pending_.end()) {
      pending_.erase(it);
      found = true;
    }
  }
  if (!found) {
    sendNotice(playerId, otherPlayerId, static_cast<uint8_t>(DuelEndReason::RequestExpired));
    return;
  }
  if (act == DuelResponseAction::Decline) {
    sendNotice(otherPlayerId, playerId, static_cast<uint8_t>(DuelEndReason::RequestDeclined));
    return;
  }

  // Aceite: revalida (os dois podem ter se afastado, morrido ou entrado em instância).
  PlayerBrief a, b;
  if (!movement_->tryGetPlayerBrief(otherPlayerId, a) || !movement_->tryGetPlayerBrief(playerId, b) || a.isDead ||
      b.isDead || a.layer != 0 || b.layer != 0) {
    sendNotice(playerId, otherPlayerId, static_cast<uint8_t>(DuelEndReason::RequestInvalid));
    sendNotice(otherPlayerId, playerId, static_cast<uint8_t>(DuelEndReason::RequestInvalid));
    return;
  }
  DuelStartedPayload start;
  {
    std::lock_guard<std::mutex> lock(mu_);
    const float maxD = settings_.requestDistance * 1.5f;  // folga: andaram durante o popup
    const float dx = a.x - b.x, dy = a.y - b.y;
    if (busyLocked(playerId) || busyLocked(otherPlayerId) || dx * dx + dy * dy > maxD * maxD) {
      start.duelId = 0;
    } else {
      ActiveDuel d;
      d.duelId = nextDuelId_++;
      if (nextDuelId_ == 0) nextDuelId_ = 1;
      d.p1 = otherPlayerId;
      d.p2 = playerId;
      d.layer = 0;
      d.flagX = (a.x + b.x) * 0.5f;
      d.flagY = (a.y + b.y) * 0.5f;
      d.flagZ = std::min(a.z, b.z);
      d.startedAt = Clock::now();
      d.fightAt = d.startedAt + std::chrono::seconds(settings_.countdownSec);
      duels_[d.duelId] = d;
      duelByPlayer_[d.p1] = d.duelId;
      duelByPlayer_[d.p2] = d.duelId;
      start.duelId = d.duelId;
      start.player1Id = d.p1;
      start.player2Id = d.p2;
      start.flagX = d.flagX;
      start.flagY = d.flagY;
      start.flagZ = d.flagZ;
      start.radius = settings_.radius;
      start.countdownMs = static_cast<uint16_t>(settings_.countdownSec * 1000);
      start.maxDurationSec = static_cast<uint16_t>(std::min<uint32_t>(settings_.maxDurationSec, 65535));
      start.outOfBoundsSec = static_cast<uint8_t>(settings_.outOfBoundsSec);
      start.flagMeshPath = settings_.flagMeshPath;
      start.flagVfxPath = settings_.flagVfxPath;
    }
  }
  if (start.duelId == 0) {
    sendNotice(playerId, otherPlayerId, static_cast<uint8_t>(DuelEndReason::RequestBusy));
    sendNotice(otherPlayerId, playerId, static_cast<uint8_t>(DuelEndReason::RequestBusy));
    return;
  }
  start.opponentName = b.name;
  movement_->sendToPlayer(otherPlayerId, encodeDuelStarted(start));
  start.opponentName = a.name;
  movement_->sendToPlayer(playerId, encodeDuelStarted(start));
  Core::Logger::getInstance().info("[DuelService] duelo {} iniciado: {} x {}", start.duelId, otherPlayerId,
                                   playerId);
}

DuelService::EndAction DuelService::takeDuelLocked(uint32_t duelId, uint32_t winnerId, uint32_t loserId,
                                                   uint8_t reason) {
  EndAction end;
  auto it = duels_.find(duelId);
  if (it == duels_.end()) return end;
  end.duel = it->second;
  end.winnerId = winnerId;
  end.loserId = loserId;
  end.reason = reason;
  end.restore = settings_.restoreOnEnd;
  recentlyEnded_.push_back({it->second.p1, it->second.p2,
                            Clock::now() + std::chrono::milliseconds(kPostDuelGraceMs)});
  duelByPlayer_.erase(it->second.p1);
  duelByPlayer_.erase(it->second.p2);
  duels_.erase(it);
  return end;
}

void DuelService::finish(const EndAction& end) {
  if (end.duel.duelId == 0) return;
  const ActiveDuel& d = end.duel;
  DuelEndedPayload p;
  p.duelId = d.duelId;
  p.winnerId = end.winnerId;
  p.loserId = end.loserId;
  p.reason = end.reason;
  const auto msg = encodeDuelEnded(p);
  if (movement_) {
    movement_->sendToPlayer(d.p1, msg);
    movement_->sendToPlayer(d.p2, msg);
  }
  const auto durationSec = static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - d.startedAt).count());
  if (combat_) {
    combat_->enqueueDuelCleanup(d.p1, d.p2, end.restore);
    // D5: histórico (base do ranking da arena).
    combat_->enqueueDbWritePublic("INSERT INTO duel_results (zone_id, player1_id, player2_id, winner_id, reason, "
                            "duration_sec) VALUES (" +
                            std::to_string(zoneId_) + ", " + std::to_string(d.p1) + ", " + std::to_string(d.p2) +
                            ", " + std::to_string(end.winnerId) + ", " + std::to_string(end.reason) + ", " +
                            std::to_string(durationSec) + ")");
  }
  Core::Logger::getInstance().info("[DuelService] duelo {} encerrado: vencedor={} perdedor={} motivo={} ({}s)",
                                   d.duelId, end.winnerId, end.loserId, end.reason, durationSec);
}

void DuelService::tick() {
  if (!movement_) return;
  struct Snapshot {
    uint32_t duelId;
    uint32_t p1, p2;
  };
  std::vector<PendingRequest> expired;
  std::vector<Snapshot> snaps;
  {
    std::lock_guard<std::mutex> lock(mu_);
    const auto now = Clock::now();
    recentlyEnded_.erase(std::remove_if(recentlyEnded_.begin(), recentlyEnded_.end(),
                                        [now](const EndedPair& p) { return now >= p.until; }),
                         recentlyEnded_.end());
    for (auto it = pending_.begin(); it != pending_.end();) {
      if (now >= it->expiresAt) {
        expired.push_back(*it);
        it = pending_.erase(it);
      } else {
        ++it;
      }
    }
    snaps.reserve(duels_.size());
    for (const auto& [id, d] : duels_) snaps.push_back({id, d.p1, d.p2});
  }
  for (const auto& e : expired) {
    sendNotice(e.fromId, e.targetId, static_cast<uint8_t>(DuelEndReason::RequestExpired));
    sendNotice(e.targetId, e.fromId, static_cast<uint8_t>(DuelEndReason::RequestExpired));
  }
  if (snaps.empty()) return;

  std::vector<EndAction> ends;
  for (const auto& s : snaps) {
    PlayerBrief a, b;
    const bool haveA = movement_->tryGetPlayerBrief(s.p1, a);
    const bool haveB = movement_->tryGetPlayerBrief(s.p2, b);
    std::lock_guard<std::mutex> lock(mu_);
    auto it = duels_.find(s.duelId);
    if (it == duels_.end()) continue;  // encerrado no meio (golpe letal)
    ActiveDuel& d = it->second;
    const auto now = Clock::now();
    // Desconexão (sem reconexão rápida: o player sai do mapa).
    if (!haveA || !haveB) {
      const uint32_t loser = !haveA ? d.p1 : d.p2;
      const uint32_t winner = !haveA ? d.p2 : d.p1;
      ends.push_back(takeDuelLocked(d.duelId, winner, loser, static_cast<uint8_t>(DuelEndReason::Disconnect)));
      continue;
    }
    // Morte por outra fonte, ou entrou em instância: interrompido, sem vencedor.
    if (a.isDead || b.isDead || a.layer != d.layer || b.layer != d.layer) {
      ends.push_back(takeDuelLocked(d.duelId, 0, 0, static_cast<uint8_t>(DuelEndReason::Interrupted)));
      continue;
    }
    if (now - d.startedAt >= std::chrono::seconds(settings_.countdownSec + settings_.maxDurationSec)) {
      ends.push_back(takeDuelLocked(d.duelId, 0, 0, static_cast<uint8_t>(DuelEndReason::Timeout)));
      continue;
    }
    // Fora da área por mais que a tolerância: perde por fuga.
    const auto grace = std::chrono::seconds(settings_.outOfBoundsSec);
    const bool out1 = DuelRules::isOutOfBounds(d.flagX, d.flagY, a.x, a.y, settings_.radius);
    const bool out2 = DuelRules::isOutOfBounds(d.flagX, d.flagY, b.x, b.y, settings_.radius);
    if (out1 && !d.oob1) d.oobSince1 = now;
    if (out2 && !d.oob2) d.oobSince2 = now;
    d.oob1 = out1;
    d.oob2 = out2;
    if (out1 && now - d.oobSince1 >= grace) {
      ends.push_back(takeDuelLocked(d.duelId, d.p2, d.p1, static_cast<uint8_t>(DuelEndReason::Fled)));
      continue;
    }
    if (out2 && now - d.oobSince2 >= grace) {
      ends.push_back(takeDuelLocked(d.duelId, d.p1, d.p2, static_cast<uint8_t>(DuelEndReason::Fled)));
      continue;
    }
  }
  for (const auto& e : ends) finish(e);
}

DuelRelation DuelService::relation(uint32_t a, uint32_t b) const {
  std::lock_guard<std::mutex> lock(mu_);
  if (duelByPlayer_.empty()) return DuelRelation::None;
  const ActiveDuel* da = duelOfLocked(a);
  const ActiveDuel* db = duelOfLocked(b);
  if (!da && !db) return DuelRelation::None;
  if (da && db && da->duelId == db->duelId) {
    return Clock::now() >= da->fightAt ? DuelRelation::Fighting : DuelRelation::Countdown;
  }
  return DuelRelation::Interfering;
}

bool DuelService::isRecentlyEndedPair(uint32_t a, uint32_t b) const {
  std::lock_guard<std::mutex> lock(mu_);
  if (recentlyEnded_.empty()) return false;
  const auto now = Clock::now();
  for (const auto& p : recentlyEnded_) {
    if (now < p.until && ((p.a == a && p.b == b) || (p.a == b && p.b == a))) return true;
  }
  return false;
}

void DuelService::onLethalHit(uint32_t loserId, uint32_t winnerId) {
  EndAction end;
  {
    std::lock_guard<std::mutex> lock(mu_);
    const ActiveDuel* d = duelOfLocked(loserId);
    if (!d || (d->p1 != winnerId && d->p2 != winnerId)) return;
    end = takeDuelLocked(d->duelId, winnerId, loserId, static_cast<uint8_t>(DuelEndReason::Defeat));
  }
  finish(end);
}

void DuelService::onPlayerGone(uint32_t playerId) {
  EndAction end;
  std::vector<PendingRequest> dropped;
  {
    std::lock_guard<std::mutex> lock(mu_);
    for (auto it = pending_.begin(); it != pending_.end();) {
      if (it->fromId == playerId || it->targetId == playerId) {
        dropped.push_back(*it);
        it = pending_.erase(it);
      } else {
        ++it;
      }
    }
    if (const ActiveDuel* d = duelOfLocked(playerId)) {
      const uint32_t winner = d->p1 == playerId ? d->p2 : d->p1;
      end = takeDuelLocked(d->duelId, winner, playerId, static_cast<uint8_t>(DuelEndReason::Disconnect));
    }
  }
  for (const auto& p : dropped) {
    const uint32_t other = p.fromId == playerId ? p.targetId : p.fromId;
    sendNotice(other, playerId, static_cast<uint8_t>(DuelEndReason::RequestCanceled));
  }
  finish(end);
}

}  // namespace Zone
}  // namespace Umbra
