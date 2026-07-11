-- Validação rápida do banco umbra_eternum (rodar após import do dump)
-- Uso: mariadb -u root -p < validate_database.sql

SELECT '=== SERVIDOR ===' AS info;
SELECT VERSION() AS db_version, DATABASE() AS current_db;

SELECT '=== SCHEMA ===' AS info;
SELECT COUNT(*) AS total_tabelas
FROM information_schema.tables
WHERE table_schema = 'umbra_eternum';

SELECT * FROM umbra_eternum.schema_version;

SELECT '=== CONTAS / PERSONAGENS ===' AS info;
SELECT COUNT(*) AS accounts FROM umbra_eternum.accounts;
SELECT COUNT(*) AS players FROM umbra_eternum.players;

SELECT '=== COMBATE / NPCs ===' AS info;
SELECT COUNT(*) AS skills FROM umbra_eternum.skills;
SELECT COUNT(*) AS basic_attacks FROM umbra_eternum.basic_attacks;
SELECT COUNT(*) AS npc_templates FROM umbra_eternum.npc_templates;
SELECT COUNT(*) AS npc_instances FROM umbra_eternum.npc_instances;

SELECT '=== ITENS / INVENTARIO ===' AS info;
SELECT COUNT(*) AS item_templates FROM umbra_eternum.item_templates;
SELECT COUNT(*) AS player_inventory FROM umbra_eternum.player_inventory;

SELECT '=== QUESTS / MUNDO ===' AS info;
SELECT COUNT(*) AS quest_templates FROM umbra_eternum.quest_templates;
SELECT COUNT(*) AS exp_zones FROM umbra_eternum.exp_zones;

SELECT '=== TABELAS ESPERADAS (amostra) ===' AS info;
SELECT table_name
FROM information_schema.tables
WHERE table_schema = 'umbra_eternum'
  AND table_name IN (
    'accounts', 'players', 'schema_version',
    'skills', 'basic_attacks', 'npc_templates', 'npc_instances',
    'item_templates', 'player_inventory', 'quest_templates', 'exp_zones',
    'player_skills', 'spawn_points', 'guilds', 'auction_listings'
  )
ORDER BY table_name;
