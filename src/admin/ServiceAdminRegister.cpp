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
#include <nlohmann/json.hpp>

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

  registry.registerCommand("reload_npc_instances", [&server](const nlohmann::json&) {
    nlohmann::json d;
    auto* engine = server.getCombatCoreEngine();
    if (!engine) {
      d["loaded"] = 0;
      d["message"] = "CombatCoreEngine não inicializado";
      return d;
    }
    const size_t loaded = engine->reloadMissingInstancesFromDatabase();
    d["loaded"] = loaded;
    d["clients_notified"] = loaded > 0;
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
