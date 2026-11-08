-- Tempo de respawn por template de NPC (segundos). Idempotente MySQL 5.7+.
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'npc_templates'
    AND COLUMN_NAME = 'respawn_seconds'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN respawn_seconds INT UNSIGNED NOT NULL DEFAULT 30 COMMENT ''Segundos até respawn após morte''',
  'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Dummy / treino: respawn em 15s (ajuste npc_template_id conforme seu banco)
UPDATE npc_templates
SET respawn_seconds = 15
WHERE npc_name IN ('dummy_treino', 'npc_combat_training', 'npc_training_dummy')
   OR npc_template_id = 6;
