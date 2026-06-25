#pragma once

#include "AdminServer.hpp"
#include <nlohmann/json.hpp>

namespace Umbra { namespace Auth { class AuthServer; } }
namespace Umbra { namespace Gateway { class GatewayServer; } }
namespace Umbra { namespace Zone { class ZoneServer; } }
namespace Umbra { namespace World { class WorldServer; } }
namespace Umbra { namespace Chat { class ChatServer; } }

namespace Umbra {
namespace Admin {

void registerAuthCommands(CommandRegistry& registry, Auth::AuthServer& server);
void registerGatewayCommands(CommandRegistry& registry, Gateway::GatewayServer& server);
void registerZoneCommands(CommandRegistry& registry, Zone::ZoneServer& server);
void registerWorldCommands(CommandRegistry& registry, World::WorldServer& server);
void registerChatCommands(CommandRegistry& registry, Chat::ChatServer& server);

}  // namespace Admin
}  // namespace Umbra
