#include "ServiceAdminRegister.hpp"
#include "auth/AuthServer.hpp"
#include "gateway/GatewayServer.hpp"
#include "zone/ZoneServer.hpp"
#include "zone/MovementServer.hpp"
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
