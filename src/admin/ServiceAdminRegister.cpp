#include "ServiceAdminRegister.hpp"
#include "auth/AuthServer.hpp"
#include "gateway/GatewayServer.hpp"
#include "zone/ZoneServer.hpp"
#include "zone/MovementServer.hpp"
#include "zone/CombatCoreEngine.hpp"
#include "world/WorldServer.hpp"
#include "world/TimeManager.hpp"
#include "world/EventManager.hpp"
#include "chat/ChatServer.hpp"
#include "chat/ChannelManager.hpp"
#include "network/SocketServer.hpp"
#include "database/MySQLConnector.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace Umbra {
namespace Admin {

void registerAuthCommands(CommandRegistry& registry, Auth::AuthServer& server) {
  registry.registerCommand("sessions_count", [&server](const nlohmann::json&) {
    nlohmann::json d;
    d["count"] = server.getActiveSessionCount();
    d["stats"] = nlohmann::json::parse(server.getStats());
    return d;
  });
}

void registerGatewayCommands(CommandRegistry& registry, Gateway::GatewayServer& server) {
  registry.registerCommand("clients", [&server](const nlohmann::json&) {
    return nlohmann::json::parse(server.getAdminClientsJson());
  });

  registry.registerCommand("kick_client", [&server](const nlohmann::json& args) {
    const uint32_t clientId = args.value("client_id", 0u);
    nlohmann::json d;
    d["kicked"] = server.kickClient(clientId);
    d["client_id"] = clientId;
    return d;
  });

  registry.registerCommand("auth_pool_status", [&server](const nlohmann::json&) {
    nlohmann::json d;
    d["stats"] = nlohmann::json::parse(server.getAuthStatsJson());
    return d;
  });
}

void registerZoneCommands(CommandRegistry& registry, Zone::ZoneServer& server) {
  registry.registerCommand("zone_info", [&server](const nlohmann::json&) {
    const auto& cfg = server.getConfig();
    nlohmann::json d;
    d["zone_id"] = cfg.zoneId;
    d["zone_name"] = cfg.zoneName;
    d["port"] = cfg.port;
    d["max_players"] = cfg.maxPlayers;
    d["tick_rate"] = cfg.tickRate;
    d["running"] = server.isRunning();
    if (auto* mov = server.getMovementServer()) {
      d["players_online"] = mov->getOnlinePlayerCount();
    }
    return d;
  });

  registry.registerCommand("players", [&server](const nlohmann::json&) {
    nlohmann::json d;
    d["players"] = nlohmann::json::array();
    auto* mov = server.getMovementServer();
    if (!mov) return d;

    const auto states = mov->getPlayerStates();
    for (const auto& [pid, st] : states) {
      nlohmann::json p;
      p["player_id"] = pid;
      p["name"] = st.characterName;
      p["title"] = st.characterTitle;
      p["guild"] = st.guildName;
      p["x"] = st.x;
      p["y"] = st.y;
      p["z"] = st.z;
      p["yaw"] = st.yaw;
      p["speed"] = st.speed;
      p["is_dead"] = st.isDead;
      p["is_in_air"] = st.isInAir;
      p["ts_ms"] = st.tsMs;
      d["players"].push_back(p);
    }
    d["count"] = states.size();
    return d;
  });

  registry.registerCommand("kick_player", [&server](const nlohmann::json& args) {
    const uint32_t playerId = args.value("player_id", 0u);
    nlohmann::json d;
    auto* mov = server.getMovementServer();
    d["kicked"] = mov ? mov->kickPlayer(playerId) : false;
    d["player_id"] = playerId;
    return d;
  });

  registry.registerCommand("teleport", [&server](const nlohmann::json& args) {
    const uint32_t playerId = args.value("player_id", 0u);
    const float x = args.value("x", 0.0f);
    const float y = args.value("y", 0.0f);
    const float z = args.value("z", 0.0f);
    nlohmann::json d;
    auto* mov = server.getMovementServer();
    d["ok"] = mov ? mov->teleportPlayer(playerId, x, y, z) : false;
    return d;
  });

  registry.registerCommand("broadcast", [&server](const nlohmann::json& args) {
    const std::string msg = args.value("message", "");
    nlohmann::json d;
    auto* mov = server.getMovementServer();
    d["sent"] = mov ? mov->broadcastAdminMessage(msg) : false;
    return d;
  });

  registry.registerCommand("notify_mail", [&server](const nlohmann::json& args) {
    nlohmann::json d;
    const uint32_t playerId = args.value("player_id", 0u);
    const uint32_t mailId = args.value("mail_id", 0u);
    const std::string fromName = args.value("from_name", "Sistema");
    const std::string subject = args.value("subject", "");
    auto* mov = server.getMovementServer();
    const bool sent = mov && playerId > 0 && mailId > 0 &&
                      mov->notifyMailToPlayer(playerId, mailId, fromName, subject);
    d["sent"] = sent;
    d["player_id"] = playerId;
    d["mail_id"] = mailId;
    return d;
  });

  registry.registerCommand("flush_mail_notify_queue", [&server](const nlohmann::json& args) {
    (void)args;
    nlohmann::json d;
    d["notified"] = 0;
    d["consumed"] = 0;
    auto* mov = server.getMovementServer();
    auto db = server.getConfig().dbConnector;
    if (!mov || !db) {
      d["ok"] = false;
      d["message"] = "MovementServer ou MySQL indisponível";
      return d;
    }
    db->execute(
        "CREATE TABLE IF NOT EXISTS mail_notify_queue ("
        "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
        "recipient_player_id BIGINT UNSIGNED NOT NULL,"
        "mail_id BIGINT UNSIGNED NOT NULL,"
        "from_name VARCHAR(64) NOT NULL DEFAULT '',"
        "subject VARCHAR(128) NOT NULL DEFAULT '',"
        "created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "consumed TINYINT(1) NOT NULL DEFAULT 0,"
        "PRIMARY KEY (id),"
        "INDEX idx_mail_notify_pending (consumed, recipient_player_id)"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
    auto rows = db->executeQuery(
        "SELECT id, recipient_player_id, mail_id, from_name, subject "
        "FROM mail_notify_queue WHERE consumed = 0 ORDER BY id ASC LIMIT 200");
    int notified = 0;
    int consumed = 0;
    for (const auto& row : rows) {
      if (row.size() < 5) continue;
      const uint64_t qid = std::stoull(row[0]);
      const uint32_t playerId = static_cast<uint32_t>(std::stoul(row[1]));
      const uint32_t mailId = static_cast<uint32_t>(std::stoul(row[2]));
      const std::string fromName = row[3];
      const std::string subject = row[4];
      if (mov->notifyMailToPlayer(playerId, mailId, fromName, subject)) {
        ++notified;
      }
      db->execute("UPDATE mail_notify_queue SET consumed = 1 WHERE id = " + std::to_string(qid));
      ++consumed;
    }
    d["ok"] = true;
    d["notified"] = notified;
    d["consumed"] = consumed;
    return d;
  });

  registry.registerCommand("force_save_positions", [&server](const nlohmann::json&) {
    server.forceSavePositions();
    nlohmann::json d;
    d["saved"] = true;
    return d;
  });

  registry.registerCommand("spawn_npc_instance", [&server](const nlohmann::json& args) {
    nlohmann::json d;
    const uint32_t instanceId = args.value("npc_instance_id", 0u);
    d["npc_instance_id"] = instanceId;
    auto* engine = server.getCombatCoreEngine();
    if (!engine) {
      d["spawned"] = false;
      d["message"] = "CombatCoreEngine não inicializado (MySQL?)";
      return d;
    }
    const bool ok = engine->spawnNpcInstance(instanceId);
    d["spawned"] = ok;
    d["clients_notified"] = ok;
    d["server_zone_id"] = server.getConfig().zoneId;
    if (!ok) {
      d["message"] = "Instância não encontrada nesta zone (is_dead=1, zone_id diferente, ou MySQL sem linha). "
                     "server_zone_id=" + std::to_string(server.getConfig().zoneId);
    }
    return d;
  });

  registry.registerCommand("reload_npc_instances", [&server](const nlohmann::json& args) {
    nlohmann::json d;
    auto* engine = server.getCombatCoreEngine();
    if (!engine) {
      d["loaded"] = 0;
      d["message"] = "CombatCoreEngine não inicializado";
      return d;
    }
    // default: full reload (pos/home/roam). args.reload_missing_only=true → comportamento antigo.
    const bool missingOnly = args.value("reload_missing_only", false);
    if (missingOnly) {
      const size_t loaded = engine->reloadMissingInstancesFromDatabase();
      d["loaded"] = loaded;
      d["mode"] = "missing_only";
      d["clients_notified"] = loaded > 0;
      return d;
    }
    const size_t spawned = engine->reloadAllNpcInstancesFromDatabase();
    d["loaded"] = spawned;
    d["mode"] = "full_reload";
    d["clients_notified"] = spawned > 0;
    return d;
  });

  registry.registerCommand("despawn_npc_instance", [&server](const nlohmann::json& args) {
    nlohmann::json d;
    const uint32_t instanceId = args.value("npc_instance_id", 0u);
    d["npc_instance_id"] = instanceId;
    auto* engine = server.getCombatCoreEngine();
    if (!engine) {
      d["despawned"] = false;
      d["message"] = "CombatCoreEngine não inicializado";
      return d;
    }
    const bool ok = engine->despawnNpcInstance(instanceId, 2);
    d["despawned"] = ok;
    d["server_zone_id"] = server.getConfig().zoneId;
    if (!ok) {
      d["message"] = "Instância não encontrada em memória nesta zone.";
    }
    return d;
  });

  registry.registerCommand("move_npc_instance", [&server](const nlohmann::json& args) {
    nlohmann::json d;
    const uint32_t instanceId = args.value("npc_instance_id", 0u);
    const float x = args.value("x", args.value("pos_x", 0.0f));
    const float y = args.value("y", args.value("pos_y", 0.0f));
    const float z = args.value("z", args.value("pos_z", 0.0f));
    const float yaw = args.value("yaw", 0.0f);
    d["npc_instance_id"] = instanceId;
    d["x"] = x;
    d["y"] = y;
    d["z"] = z;
    d["yaw"] = yaw;
    auto* engine = server.getCombatCoreEngine();
    if (!engine) {
      d["moved"] = false;
      d["message"] = "CombatCoreEngine não inicializado";
      return d;
    }
    const bool ok = engine->moveNpcInstance(instanceId, x, y, z, yaw);
    d["moved"] = ok;
    d["clients_notified"] = ok;
    d["server_zone_id"] = server.getConfig().zoneId;
    if (!ok) {
      d["message"] = "Instância não encontrada nesta zone (zone_id diferente ou MySQL sem linha).";
    }
    return d;
  });

  registry.registerCommand("list_npcs", [&server](const nlohmann::json&) {
    nlohmann::json d;
    d["npcs"] = nlohmann::json::array();
    auto* engine = server.getCombatCoreEngine();
    if (!engine || !engine->getNpcManager()) {
      d["count"] = 0;
      return d;
    }
    for (const auto& inst : engine->getNpcManager()->getAllInstances()) {
      if (inst.isDead) continue;
      nlohmann::json n;
      n["npc_instance_id"] = inst.npcInstanceId;
      n["npc_template_id"] = inst.templateId;
      n["npc_name"] = inst.npcName;
      n["zone_id"] = inst.zoneId;
      n["x"] = inst.x;
      n["y"] = inst.y;
      n["z"] = inst.z;
      n["yaw"] = inst.yaw;
      n["current_health"] = inst.currentHealth;
      n["max_health"] = inst.maxHealth;
      d["npcs"].push_back(n);
    }
    d["count"] = d["npcs"].size();
    return d;
  });

  registry.registerCommand("reload_game_rates", [&server](const nlohmann::json&) {
    server.reloadGameRates();
    nlohmann::json d;
    d["reloaded"] = true;
    double expM = 1.0;
    double dropM = 1.0;
    if (auto* exp = server.getExperienceService()) expM = exp->getExpMultiplier();
    if (auto* loot = server.getLootService()) dropM = loot->getDropMultiplier();
    d["exp_multiplier"] = expM;
    d["drop_multiplier"] = dropM;
    d["zone_id"] = server.getConfig().zoneId;
    return d;
  });

  registry.registerCommand("reload_skills", [&server](const nlohmann::json&) {
    nlohmann::json d;
    auto* combat = server.getCombatCoreEngine();
    const bool ok = combat && combat->reloadSkills();
    d["ok"] = ok;
    d["reloaded"] = ok;
    d["zone_id"] = server.getConfig().zoneId;
    if (!ok) d["message"] = "CombatCoreEngine/SkillService indisponível";
    return d;
  });
}

void registerWorldCommands(CommandRegistry& registry, World::WorldServer& server) {
  registry.registerCommand("time_info", [&server](const nlohmann::json&) {
    nlohmann::json d;
    d["game_time"] = server.getTimeManager().getGameTime();
    d["delta_time"] = server.getTimeManager().getDeltaTime();
    return d;
  });

  registry.registerCommand("events", [&server](const nlohmann::json&) {
    nlohmann::json d;
    d["running"] = server.isRunning();
    d["tick_rate"] = server.getConfig().tickRate;
    return d;
  });
}

void registerChatCommands(CommandRegistry& registry, Chat::ChatServer& server) {
  registry.registerCommand("channels", [&server](const nlohmann::json&) {
    nlohmann::json d;
    d["channel_count"] = server.getChannelManager().getChannelCount();
    d["port"] = server.getConfig().port;
    return d;
  });

  registry.registerCommand("recent_messages", [&server](const nlohmann::json& args) {
    (void)args;
    (void)server;
    nlohmann::json d;
    d["messages"] = nlohmann::json::array();
    d["note"] = "message history not persisted yet";
    return d;
  });
}

}  // namespace Admin
}  // namespace Umbra
