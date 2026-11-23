#pragma once
// GuildMissionService.hpp — missões de guilda (etapa 3b): progresso somado pelos membros a partir
// dos mesmos eventos das quests (kill_any, earn_gold, use_skill, use_consumable...).
// Roda na thread do QuestProgressService (que repassa os eventos): sem locks próprios no cache.
// craft_item/refine_item são creditados pelo PHP.

#include "zone/QuestProgressService.hpp"
#include "database/MySQLConnector.hpp"
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Umbra {
namespace Zone {

class MovementServer;

class GuildMissionService {
public:
  GuildMissionService(std::shared_ptr<Database::MySQLConnector> db, MovementServer* movementServer);

  /** Evento de jogo de um jogador (thread do QuestProgressService). */
  void onEvent(uint32_t playerId, const QuestEvent& ev);

  /** Admin reload_guild_missions / missão editada no Manager: descarta os caches. */
  void invalidate() { invalidate_.store(true); }

private:
  struct Mission {
    uint64_t guildMissionId = 0;
    QuestObjectiveKind kind = QuestObjectiveKind::Other;
    QuestObjectiveParams params;
    int32_t progress = 0;
    int32_t required = 1;
    bool completed = false;
    std::string title;
    int32_t rewardGuildXp = 0;
    int64_t lastNotifyMs = 0;
  };
  struct GuildCache {
    int64_t loadedAtMs = 0;
    std::vector<Mission> missions;
    std::vector<uint32_t> members;
  };
  struct PlayerGuild {
    uint32_t guildId = 0;
    int64_t loadedAtMs = 0;
  };

  uint32_t resolveGuild(uint32_t playerId, int64_t now);
  GuildCache& ensureGuild(uint32_t guildId, int64_t now);
  void credit(uint32_t guildId, uint32_t playerId, Mission& m, int32_t amount, GuildCache& cache, int64_t now);
  void completeMission(uint32_t guildId, Mission& m, GuildCache& cache);
  void notifyMembers(const GuildCache& cache, const std::vector<uint8_t>& msg);
  static int64_t nowMs();

  std::shared_ptr<Database::MySQLConnector> db_;
  MovementServer* movementServer_ = nullptr;
  std::atomic<bool> invalidate_{false};
  std::unordered_map<uint32_t, PlayerGuild> playerGuild_;
  std::unordered_map<uint32_t, GuildCache> guilds_;
};

}  // namespace Zone
}  // namespace Umbra
