using UmbraManager.Models;

namespace UmbraManager.Services;

public static class GmCommandCatalogService
{
    public static IReadOnlyList<GmCommandDefinition> BuildDefaultCatalog() =>
    [
        new() { Scope = "common", Name = "ping", Description = "Testa conectividade e versão do serviço.", Example = "ping" },
        new() { Scope = "common", Name = "stats", Description = "Retorna métricas de CPU, RAM, uptime e versão.", Example = "stats" },
        new() { Scope = "common", Name = "set_log_level", Description = "Ajusta o nível de log em runtime.", ArgsHint = "level=DEBUG|INFO|WARN|ERROR|CRITICAL", Example = "set_log_level level=DEBUG" },
        new() { Scope = "common", Name = "reload_config", Description = "Recarrega o arquivo de configuração do serviço.", Example = "reload_config" },
        new() { Scope = "common", Name = "shutdown", Description = "Agenda shutdown gracioso do serviço.", ArgsHint = "grace_sec=3", Example = "shutdown grace_sec=5", IsDestructive = true },

        new() { Scope = "auth", Name = "sessions_count", Description = "Conta sessões ativas e devolve stats do Auth.", Example = "sessions_count" },

        new() { Scope = "gateway", Name = "clients", Description = "Lista clientes conectados no gateway.", Example = "clients" },
        new() { Scope = "gateway", Name = "kick_client", Description = "Desconecta um client_id do gateway.", ArgsHint = "client_id=42", Example = "kick_client client_id=42", IsDestructive = true },
        new() { Scope = "gateway", Name = "auth_pool_status", Description = "Mostra estado do pool de autenticação.", Example = "auth_pool_status" },

        new() { Scope = "world", Name = "time_info", Description = "Mostra hora do mundo e delta time atual.", Example = "time_info" },
        new() { Scope = "world", Name = "events", Description = "Retorna o estado operacional do world.", Example = "events" },

        new() { Scope = "chat", Name = "channels", Description = "Retorna quantidade de canais e porta do chat.", Example = "channels" },
        new() { Scope = "chat", Name = "recent_messages", Description = "Consulta histórico recente quando houver persistência.", Example = "recent_messages" },

        new() { Scope = "zone", Name = "zone_info", Description = "Retorna metadados da zone em execução.", Example = "zone_info" },
        new() { Scope = "zone", Name = "players", Description = "Lista jogadores ativos na zone.", Example = "players" },
        new() { Scope = "zone", Name = "kick_player", Description = "Expulsa um jogador da zone.", ArgsHint = "player_id=42", Example = "kick_player player_id=42", IsDestructive = true },
        new() { Scope = "zone", Name = "teleport", Description = "Teleporta um jogador para coordenadas informadas.", ArgsHint = "player_id=42 x=1000 y=2000 z=100", Example = "teleport player_id=42 x=1000 y=2000 z=100" },
        new() { Scope = "zone", Name = "broadcast", Description = "Envia mensagem administrativa para todos da zone (chat 70 + toast 73).", ArgsHint = "message=\"Servidor reinicia em 5 min\"", Example = "broadcast message=\"Servidor reinicia em 5 min\"" },
        new() { Scope = "zone", Name = "notify_mail", Description = "Notifica player online sobre nova carta (opcode 74).", ArgsHint = "player_id=1 mail_id=10 from_name=\"Sistema\" subject=\"Presente\"", Example = "notify_mail player_id=1 mail_id=10 from_name=\"Sistema\" subject=\"Presente\"" },
        new() { Scope = "zone", Name = "flush_mail_notify_queue", Description = "Consome fila mail_notify_queue e envia opcode 74 aos online.", ArgsHint = "", Example = "flush_mail_notify_queue" },
        new() { Scope = "zone", Name = "force_save_positions", Description = "Persiste posições dos jogadores imediatamente.", Example = "force_save_positions" },
        new() { Scope = "zone", Name = "spawn_npc_instance", Description = "Hot spawn de instância já gravada no banco.", ArgsHint = "npc_instance_id=123", Example = "spawn_npc_instance npc_instance_id=123" },
        new() { Scope = "zone", Name = "reload_npc_instances", Description = "Carrega instâncias ausentes do banco em runtime.", Example = "reload_npc_instances" },
        new() { Scope = "zone", Name = "list_npcs", Description = "Lista NPCs vivos atualmente carregados em memória.", Example = "list_npcs" },
        new() { Scope = "zone", Name = "despawn_npc_instance", Description = "Remove uma instância NPC do runtime e envia opcode 101.", ArgsHint = "npc_instance_id=123", Example = "despawn_npc_instance npc_instance_id=123", IsDestructive = true },
        new() { Scope = "zone", Name = "move_npc_instance", Description = "Move NPC em runtime e rebroadcast opcode 100 (não grava MySQL).", ArgsHint = "npc_instance_id=16 pos_x=100 pos_y=200 pos_z=90 yaw=0", Example = "move_npc_instance npc_instance_id=16 pos_x=100 pos_y=200 pos_z=90" },
        new() { Scope = "zone", Name = "reload_game_rates", Description = "Recarrega multiplicadores globais EXP/drop da tabela game_rates.", Example = "reload_game_rates" },
    ];
}
