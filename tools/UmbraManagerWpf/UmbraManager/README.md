# UmbraManager WPF

Standalone desktop admin application for UmbraEternum.

## Current structure

- `MainWindow.xaml`: main shell and tab navigation.
- `ViewModels/MainViewModel.cs`: legacy core orchestration for services, polling and existing tabs.
- `ViewModels/MainViewModel.AdminHub.cs`: new admin surfaces, project overview, player inspector, loot, EXP zones and refinement.
- `Services/AdminChannelHub.cs`: TCP admin channel to C++ services.
- `Services/PhpAdminClient.cs`: PHP admin API client for accounts, items, skills, NPCs and extended admin surfaces.
- `Services/GmCommandCatalogService.cs`: source of truth for GM/admin command catalog shown in UI.
- `Services/GmCommandLineParser.cs`: robust parser for GM command input with quotes and JSON payload fragments.
- `Models/`: UI-facing rows and DTO-like classes used by tabs and forms.

## UI areas

- `Dashboard`: service health, CPU chart and project-wide coverage summary.
- `Servers / Zones / Players / Logs`: live operations for the running stack.
- `Accounts / Items / Skills / NPCs`: core game data and runtime actions.
- `GM Console`: command catalog plus structured command execution.
- `Admin Hub`:
  - `Player Inspector`
  - `EXP Zones`
  - `Refinement`
  - `Guilds` / `Auction` / `Mail` (até 5 anexos inventory-slot + envio em massa)
- `Audit`: MySQL `admin_audit` (fallback SQLite local)
- `Rates`: multiplicadores globais EXP/Drop

## External integrations

- Admin TCP: commands handled in `src/admin/` (incl. `notify_mail`, `flush_mail_notify_queue`, broadcast 70+73).
- PHP Admin API: endpoints under `www/umbra_api/api/admin/`.
- Mail player API: `www/umbra_api/api/mail/`.
- Runtime NPC operations: `src/zone/CombatCoreEngine.*`, `src/zone/NpcManager.*`.

## Notable admin endpoints added/expanded

- `api/admin/update_item.php`
- `api/admin/list_exp_zones.php`
- `api/admin/list_skills.php` / `get_skill.php` / CRUD + `upsert_skill_rank_scaling.php`
- Zone admin TCP: `reload_skills`
- `api/admin/upsert_exp_zone.php`
- `api/admin/player_inspector.php`
- `api/admin/project_state_summary.php`
- `api/admin/list_admin_audit.php` / `log_admin_audit.php`
- `api/admin/list_guilds.php` / `get_guild.php` / `kick_guild_member.php` / `disband_guild_admin.php` / `transfer_owner_admin.php`
- `api/admin/list_auctions_admin.php` / `force_cancel_auction.php` / `expire_stale.php`
- `api/admin/admin_send_mail.php` / `admin_send_mail_all.php` / `list_mail_admin.php`
- `api/admin/get_refinement_config.php`

## Validation notes

- WPF project validated with `dotnet build`.
- Updated C++ admin/zone libraries validated with `cmake --build ... --target umbra_zone umbra_admin`.
- Full `zone_server.exe` relink may fail if the binary is running and locked by Windows.
