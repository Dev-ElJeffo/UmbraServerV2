#include "zone/ReactionEngine.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "zone/MovementProtocol.hpp"
#include "core/Logger.hpp"

#include <algorithm>
#include <chrono>

namespace Umbra {
namespace Zone {

namespace {

ReactionTrigger triggerFromString(const std::string& s) {
  if (s == "on_crit_received") return ReactionTrigger::OnCritReceived;
  if (s == "on_dodge") return ReactionTrigger::OnDodge;
  if (s == "on_attack_received") return ReactionTrigger::OnAttackReceived;
  if (s == "ally_damaged") return ReactionTrigger::AllyDamaged;
  return ReactionTrigger::None;
}

int64_t jsonIntLocal(const nlohmann::json& j, const char* key, int64_t fallback = 0) {
  auto it = j.find(key);
  if (it == j.end() || it->is_null()) return fallback;
  try {
    if (it->is_number()) return static_cast<int64_t>(it->get<double>());
    if (it->is_string()) return std::stoll(it->get<std::string>());
  } catch (...) {
  }
  return fallback;
}

}  // namespace

ReactionTrigger ReactionEngine::parseTrigger(const nlohmann::json& conditions) {
  if (!conditions.is_object()) return ReactionTrigger::None;
  if (conditions.contains("trigger") && conditions["trigger"].is_string()) {
    return triggerFromString(conditions["trigger"].get<std::string>());
  }
  return ReactionTrigger::None;
}

std::vector<ArmedReaction>* ReactionEngine::findArmedList(uint32_t playerId) {
  auto it = armedByPlayer_.find(playerId);
  return it != armedByPlayer_.end() ? &it->second : nullptr;
}

void ReactionEngine::reloadArmedForPlayer(uint32_t playerId) {
  if (!db_ || !db_->isConnected()) return;
  const std::string pid = std::to_string(playerId);
  auto rows = db_->executePreparedQuery(
      "SELECT ab.buff_id, ab.skill_id, ab.snapshot_json, s.area_radius "
      "FROM active_buffs ab "
      "INNER JOIN skills s ON ab.skill_id = s.skill_id "
      "WHERE ab.target_player_id = ? AND ab.expires_at > NOW(3) "
      "AND ab.snapshot_json LIKE '%reaction_armed%'",
      {pid});

  std::lock_guard<std::mutex> lock(mu_);
  armedByPlayer_.erase(playerId);

  for (const auto& row : rows) {
    if (row.size() < 4) continue;
    nlohmann::json snap = nlohmann::json::parse(row[2], nullptr, false);
    if (!snap.is_object() || !snap.value("reaction_armed", false)) continue;

    ArmedReaction ar;
    ar.buffId = std::stoull(row[0]);
    ar.skillId = static_cast<uint32_t>(std::stoul(row[1]));
    ar.ownerPlayerId = playerId;
    ar.trigger = parseTrigger(snap);
    ar.usesRemaining = static_cast<int32_t>(jsonIntLocal(snap, "uses_remaining", 1));
    ar.redirectToSelf = snap.value("redirect_to_self", false);
    try {
      ar.areaRadius = static_cast<uint32_t>(std::stoul(row[3]));
    } catch (...) {
      ar.areaRadius = 0;
    }

    ar.effect.targetStat = snap.value("target_stat", std::string{});
    ar.effect.valueFlat = static_cast<int32_t>(jsonIntLocal(snap, "value_flat"));
    ar.effect.valuePercent = static_cast<int16_t>(jsonIntLocal(snap, "value_percent"));
    ar.effect.durationMs = static_cast<uint32_t>(jsonIntLocal(snap, "duration_ms"));

    const std::string effectTypeStr = snap.value("effect_type", std::string{});
    if (effectTypeStr == "DAMAGE") ar.effect.effectType = Combat::EffectType::DAMAGE;
    else if (effectTypeStr == "BUFF_STAT") ar.effect.effectType = Combat::EffectType::BUFF_STAT;
    else if (effectTypeStr == "REFLECT") ar.effect.effectType = Combat::EffectType::REFLECT;
    else ar.effect.effectType = Combat::EffectType::DAMAGE;

    armedByPlayer_[playerId].push_back(std::move(ar));
  }
}

uint64_t ReactionEngine::armReaction(uint32_t ownerPlayerId, uint32_t sourcePlayerId, uint32_t skillId,
                                     const Combat::SkillData& skill, const Combat::SkillEffect& effect) {
  if (!db_ || !db_->isConnected() || ownerPlayerId == 0) return 0;

  const ReactionTrigger trigger = parseTrigger(effect.conditions);
  if (trigger == ReactionTrigger::None) return 0;

  uint32_t durationMs = effect.durationMs > 0 ? effect.durationMs : skill.durationMs;
  if (durationMs == 0) durationMs = 30000;

  nlohmann::json snap;
  snap["reaction_armed"] = true;
  snap["target_stat"] = effect.targetStat;
  snap["value_flat"] = effect.valueFlat;
  snap["value_percent"] = effect.valuePercent;
  snap["effect_type"] = (effect.effectType == Combat::EffectType::REFLECT) ? "REFLECT" :
                        (effect.effectType == Combat::EffectType::BUFF_STAT) ? "BUFF_STAT" : "DAMAGE";
  snap["duration_ms"] = effect.durationMs;
  if (effect.conditions.is_object()) {
    if (effect.conditions.contains("trigger")) snap["trigger"] = effect.conditions["trigger"];
    if (effect.conditions.contains("max_uses")) {
      snap["uses_remaining"] = effect.conditions["max_uses"];
    } else {
      snap["uses_remaining"] = 1;
    }
    if (effect.conditions.contains("redirect_to_self")) {
      snap["redirect_to_self"] = effect.conditions["redirect_to_self"];
    }
  }
  const std::string snapshotStr = snap.dump();
  const std::string durUsStr = std::to_string(static_cast<uint64_t>(durationMs) * 1000ULL);

  if (!db_->executePreparedInsert(
          "INSERT INTO active_buffs (target_player_id, source_player_id, skill_id, buff_type, "
          "current_stacks, value_snapshot, expires_at, snapshot_json) VALUES "
          "(?, ?, ?, 'AURA', 1, ?, DATE_ADD(NOW(3), INTERVAL ? MICROSECOND), ?)",
          {std::to_string(ownerPlayerId), std::to_string(sourcePlayerId), std::to_string(skillId),
           std::to_string(effect.valuePercent != 0 ? effect.valuePercent : effect.valueFlat), durUsStr,
           snapshotStr})) {
    return 0;
  }

  const uint64_t buffId = db_->getLastInsertId();

  ArmedReaction ar;
  ar.buffId = buffId;
  ar.ownerPlayerId = ownerPlayerId;
  ar.skillId = skillId;
  ar.effect = effect;
  ar.trigger = trigger;
  ar.usesRemaining = static_cast<int32_t>(jsonIntLocal(snap, "uses_remaining", 1));
  ar.redirectToSelf = snap.value("redirect_to_self", false);
  ar.areaRadius = skill.areaRadius;

  {
    std::lock_guard<std::mutex> lock(mu_);
    armedByPlayer_[ownerPlayerId].push_back(std::move(ar));
  }

  Core::Logger::getInstance().info(
      "[ReactionEngine] reacao armada player={} skill={} buff_id={} trigger={}", ownerPlayerId,
      skillId, buffId, static_cast<int>(trigger));
  return buffId;
}

void ReactionEngine::broadcastDisarm(const ArmedReaction& reaction) {
  if (!combatEngine_) return;
  SkillBuffSyncPayload sync;
  sync.action = 1;
  sync.targetPlayerId = reaction.ownerPlayerId;
  sync.buffId = reaction.buffId;
  sync.skillId = reaction.skillId;
  sync.targetStat = reaction.effect.targetStat;
  combatEngine_->enrichSkillBuffSyncPayload(sync);
  combatEngine_->broadcastSkillBuffSyncPublic(sync);
}

void ReactionEngine::disarm(uint32_t ownerPlayerId, ArmedReaction& reaction, bool consumeUse) {
  if (consumeUse && reaction.usesRemaining > 0) {
    --reaction.usesRemaining;
    if (reaction.usesRemaining > 0) return;
  }

  if (db_ && db_->isConnected() && reaction.buffId > 0) {
    db_->executePreparedInsert("DELETE FROM active_buffs WHERE buff_id = ? AND target_player_id = ?",
                               {std::to_string(reaction.buffId), std::to_string(ownerPlayerId)});
  }
  broadcastDisarm(reaction);
  reaction.buffId = 0;
}

void ReactionEngine::executeReaction(uint32_t ownerPlayerId, uint32_t triggerSourceId,
                                     ArmedReaction& reaction) {
  if (!combatEngine_) return;

  const auto& eff = reaction.effect;
  if (eff.effectType == Combat::EffectType::BUFF_STAT) {
    combatEngine_->applyReactionBuff(ownerPlayerId, ownerPlayerId, reaction.skillId, eff);
    Core::Logger::getInstance().info("[ReactionEngine] buff reacao player={} skill={}", ownerPlayerId,
                                     reaction.skillId);
  } else if (eff.effectType == Combat::EffectType::DAMAGE ||
             eff.effectType == Combat::EffectType::REFLECT) {
    if (triggerSourceId == 0 || triggerSourceId == ownerPlayerId) {
      disarm(ownerPlayerId, reaction, true);
      return;
    }
    combatEngine_->applyReactionCounterDamage(ownerPlayerId, triggerSourceId, reaction.skillId, eff);
    Core::Logger::getInstance().info(
        "[ReactionEngine] contra-dano player={} -> source={} skill={}", ownerPlayerId, triggerSourceId,
        reaction.skillId);
  }

  disarm(ownerPlayerId, reaction, true);
}

bool ReactionEngine::onPlayerHitReceived(uint32_t targetPlayerId, uint32_t sourcePlayerId) {
  std::lock_guard<std::mutex> lock(mu_);
  auto* list = findArmedList(targetPlayerId);
  if (!list) return false;

  for (auto& ar : *list) {
    if (ar.buffId == 0 || ar.trigger != ReactionTrigger::OnAttackReceived) continue;
    executeReaction(targetPlayerId, sourcePlayerId, ar);
    return true;
  }
  return false;
}

void ReactionEngine::onPlayerDamaged(uint32_t targetPlayerId, uint32_t sourcePlayerId, int32_t delta,
                                     bool isCrit) {
  if (delta >= 0) return;

  std::vector<ArmedReaction> toFire;
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (isCrit) {
      if (auto* list = findArmedList(targetPlayerId)) {
        for (auto& ar : *list) {
          if (ar.buffId != 0 && ar.trigger == ReactionTrigger::OnCritReceived) {
            toFire.push_back(ar);
          }
        }
      }
    }
  }

  for (auto& ar : toFire) {
    std::lock_guard<std::mutex> lock(mu_);
    auto* list = findArmedList(targetPlayerId);
    if (!list) continue;
    for (auto& live : *list) {
      if (live.buffId == ar.buffId) {
        executeReaction(targetPlayerId, sourcePlayerId, live);
        break;
      }
    }
  }
}

void ReactionEngine::onPlayerDodge(uint32_t targetPlayerId, uint32_t sourcePlayerId) {
  std::vector<ArmedReaction> toFire;
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (auto* list = findArmedList(targetPlayerId)) {
      for (auto& ar : *list) {
        if (ar.buffId != 0 && ar.trigger == ReactionTrigger::OnDodge) {
          toFire.push_back(ar);
        }
      }
    }
  }

  for (auto& ar : toFire) {
    std::lock_guard<std::mutex> lock(mu_);
    auto* list = findArmedList(targetPlayerId);
    if (!list) continue;
    for (auto& live : *list) {
      if (live.buffId == ar.buffId) {
        executeReaction(targetPlayerId, sourcePlayerId, live);
        break;
      }
    }
  }
}

void ReactionEngine::onAllyDamaged(uint32_t allyPlayerId, uint32_t sourcePlayerId, int32_t& delta) {
  if (delta >= 0) return;

  struct Redirect {
    uint32_t protectorId;
    ArmedReaction reaction;
  };
  std::vector<Redirect> redirects;

  {
    std::lock_guard<std::mutex> lock(mu_);
    for (auto& [protectorId, list] : armedByPlayer_) {
      if (protectorId == allyPlayerId) continue;
      for (auto& ar : list) {
        if (ar.buffId != 0 && ar.trigger == ReactionTrigger::AllyDamaged) {
          redirects.push_back({protectorId, ar});
        }
      }
    }
  }

  for (auto& rd : redirects) {
    std::lock_guard<std::mutex> lock(mu_);
    auto* list = findArmedList(rd.protectorId);
    if (!list) continue;
    for (auto& live : *list) {
      if (live.buffId != rd.reaction.buffId) continue;
      const int32_t pct = live.effect.valuePercent > 0 ? live.effect.valuePercent : 50;
      const int32_t redirectAmount = std::max(1, std::abs(delta) * pct / 100);
      if (combatEngine_) {
        combatEngine_->applyDirectPlayerDamage(sourcePlayerId, live.ownerPlayerId, redirectAmount,
                                               static_cast<uint8_t>(CombatReason::Skill));
      }
      delta += redirectAmount;
      disarm(live.ownerPlayerId, live, true);
      break;
    }
  }
}

}  // namespace Zone
}  // namespace Umbra
