#include "zone/GuildMissionService.hpp"
#include "zone/MovementProtocol.hpp"
#include "zone/MovementServer.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <chrono>

namespace Umbra {
namespace Zone {

namespace {

constexpr int64_t kPlayerGuildTtlMs = 60000;
/** Missões e membros da guilda: recarrega (e sorteia o período novo) depois disso. */
constexpr int64_t kGuildCacheTtlMs = 30000;
/** 133 por missão no máximo a cada 2 s (a conclusão sai sempre). */
constexpr int64_t kNotifyThrottleMs = 2000;

long long toLL(const std::string& s, long long def = 0) {
  try {
    return s.empty() ? def : std::stoll(s);
  } catch (...) {
    return def;
  }
}

}  // namespace

GuildMissionService::GuildMissionService(std::shared_ptr<Database::MySQLConnector> db, MovementServer* movementServer)
    : db_(std::move(db)), movementServer_(movementServer) {}

int64_t GuildMissionService::nowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

uint32_t GuildMissionService::resolveGuild(uint32_t playerId, int64_t now) {
  PlayerGuild& pg = playerGuild_[playerId];
  if (pg.loadedAtMs == 0 || now - pg.loadedAtMs > kPlayerGuildTtlMs) {
    pg.loadedAtMs = now;
    const auto g = db_->executePreparedScalar("SELECT guild_id FROM guild_members WHERE player_id = ? LIMIT 1",
                                              {std::to_string(playerId)});
    pg.guildId = g ? static_cast<uint32_t>(toLL(*g)) : 0;
  }
  return pg.guildId;
}

GuildMissionService::GuildCache& GuildMissionService::ensureGuild(uint32_t guildId, int64_t now) {
  GuildCache& c = guilds_[guildId];
  if (c.loadedAtMs != 0 && now - c.loadedAtMs <= kGuildCacheTtlMs) return c;
  c.loadedAtMs = now;
  c.missions.clear();
  c.members.clear();
  // Sorteio do período (mesma procedure do PHP; idempotente). guildId é numérico.
  db_->execute("CALL sp_guild_missions_ensure(" + std::to_string(guildId) + ")");
  // Ordem posicional: conferir os índices abaixo ao mudar o SELECT.
  const auto rows = db_->executePreparedQuery(
      "SELECT gm.guild_mission_id, t.objective_type, COALESCE(t.params_json, '{}'), gm.progress, gm.required_total, "
      "gm.status, t.title, t.reward_guild_xp "
      "FROM guild_missions gm INNER JOIN guild_mission_templates t ON t.mission_id = gm.mission_id "
      "WHERE gm.guild_id = ? AND ((gm.period = 'daily' AND gm.period_key = CURDATE()) "
      "  OR (gm.period = 'weekly' AND gm.period_key = DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY)))",
      {std::to_string(guildId)});
  for (const auto& r : rows) {
    if (r.size() < 8) continue;
    Mission m;
    m.guildMissionId = static_cast<uint64_t>(toLL(r[0]));
    m.kind = questKindFromString(r[1]);
    m.params = parseQuestObjectiveParams(m.kind, r[2]);
    m.progress = static_cast<int32_t>(toLL(r[3]));
    m.required = static_cast<int32_t>(std::max<long long>(1, toLL(r[4], 1)));
    m.completed = r[5] == "completed";
    m.title = r[6];
    m.rewardGuildXp = static_cast<int32_t>(toLL(r[7]));
    c.missions.push_back(std::move(m));
  }
  for (const auto& r : db_->executePreparedQuery("SELECT player_id FROM guild_members WHERE guild_id = ?",
                                                 {std::to_string(guildId)})) {
    if (!r.empty()) c.members.push_back(static_cast<uint32_t>(toLL(r[0])));
  }
  return c;
}

void GuildMissionService::onEvent(uint32_t playerId, const QuestEvent& ev) {
  if (!db_ || !db_->isConnected() || playerId == 0) return;
  if (invalidate_.exchange(false)) {
    playerGuild_.clear();
    guilds_.clear();
  }
  const int64_t now = nowMs();
  const uint32_t guildId = resolveGuild(playerId, now);
  if (guildId == 0) return;
  GuildCache& cache = ensureGuild(guildId, now);
  for (Mission& m : cache.missions) {
    if (m.completed || m.kind == QuestObjectiveKind::Other) continue;
    if (!questEventMatches(m.kind, m.params, ev, 0)) continue;
    const int32_t amount = m.kind == QuestObjectiveKind::EarnGold ? std::max(0, ev.amount) : 1;
    if (amount > 0) credit(guildId, playerId, m, amount, cache, now);
  }
}

void GuildMissionService::credit(uint32_t guildId, uint32_t playerId, Mission& m, int32_t amount, GuildCache& cache,
                                 int64_t now) {
  const std::string gmId = std::to_string(m.guildMissionId);
  db_->executePreparedInsert(
      "UPDATE guild_missions SET progress = LEAST(required_total, progress + ?) "
      "WHERE guild_mission_id = ? AND status = 'active'",
      {std::to_string(amount), gmId});
  db_->executePreparedInsert(
      "INSERT INTO guild_mission_contributions (guild_mission_id, player_id, amount) VALUES (?, ?, ?) "
      "ON DUPLICATE KEY UPDATE amount = amount + VALUES(amount)",
      {gmId, std::to_string(playerId), std::to_string(amount)});
  // Valor real do DB: membros em outras zones também somam.
  const auto row = db_->executePreparedQuery(
      "SELECT progress, required_total, status FROM guild_missions WHERE guild_mission_id = ?", {gmId});
  if (!row.empty() && row[0].size() >= 3) {
    m.progress = static_cast<int32_t>(toLL(row[0][0]));
    m.required = static_cast<int32_t>(std::max<long long>(1, toLL(row[0][1], 1)));
    m.completed = row[0][2] == "completed";
  }
  if (!m.completed && m.progress >= m.required) {
    completeMission(guildId, m, cache);
    return;
  }
  if (now - m.lastNotifyMs >= kNotifyThrottleMs) {
    m.lastNotifyMs = now;
    GuildMissionProgressPayload p;
    p.guildMissionId = static_cast<uint32_t>(m.guildMissionId);
    p.progress = static_cast<uint32_t>(m.progress);
    p.required = static_cast<uint32_t>(m.required);
    p.status = m.completed ? 1 : 0;
    notifyMembers(cache, encodeGuildMissionProgress(p));
  }
}

void GuildMissionService::completeMission(uint32_t guildId, Mission& m, GuildCache& cache) {
  const std::string gmId = std::to_string(m.guildMissionId);
  const std::string gid = std::to_string(guildId);
  db_->executePreparedInsert(
      "UPDATE guild_missions SET status = 'completed', completed_at = NOW() "
      "WHERE guild_mission_id = ? AND status = 'active' AND progress >= required_total",
      {gmId});
  // EXP uma vez só (xp_awarded), mesmo com várias zones concluindo ao mesmo tempo.
  db_->executePreparedInsert(
      "UPDATE guilds g INNER JOIN guild_missions gm ON gm.guild_id = g.guild_id "
      "INNER JOIN guild_mission_templates t ON t.mission_id = gm.mission_id "
      "SET g.guild_xp = g.guild_xp + t.reward_guild_xp, gm.xp_awarded = 1 "
      "WHERE gm.guild_mission_id = ? AND gm.xp_awarded = 0 AND gm.status = 'completed'",
      {gmId});
  // Mesma curva do PHP (guild_calculate_level_from_xp): floor(sqrt(xp / 1000)) + 1.
  db_->executePreparedInsert(
      "UPDATE guilds SET guild_level = GREATEST(1, FLOOR(SQRT(guild_xp / 1000)) + 1), ranking_score = guild_xp "
      "WHERE guild_id = ?",
      {gid});
  m.completed = true;

  GuildMissionProgressPayload p;
  p.guildMissionId = static_cast<uint32_t>(m.guildMissionId);
  p.progress = static_cast<uint32_t>(m.required);
  p.required = static_cast<uint32_t>(m.required);
  p.status = 1;
  notifyMembers(cache, encodeGuildMissionProgress(p));
  const std::string text = "Missão de guilda concluída: " + m.title + " (+" + std::to_string(m.rewardGuildXp) +
                           " EXP de guilda). Resgate sua recompensa na janela de missões.";
  notifyMembers(cache, encodeSystemBroadcast(1, 6000, "Guilda", text));
  Core::Logger::getInstance().info("[GuildMissionService] guilda {} concluiu a missão {} ({})", guildId,
                                   m.guildMissionId, m.title);
}

void GuildMissionService::notifyMembers(const GuildCache& cache, const std::vector<uint8_t>& msg) {
  if (!movementServer_) return;
  for (uint32_t pid : cache.members) {
    if (movementServer_->isPlayerConnected(pid)) movementServer_->sendToPlayer(pid, msg);
  }
}

}  // namespace Zone
}  // namespace Umbra
