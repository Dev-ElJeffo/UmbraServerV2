-- Raio XY (uu) em que o nameplate do NPC aparece para o player. Idempotente.
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'npc_templates'
    AND COLUMN_NAME = 'nameplate_radius'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN nameplate_radius FLOAT NOT NULL DEFAULT 2000.0 COMMENT ''Raio XY (uu) do nameplate; nao reutilizar interaction_radius'' AFTER collision_radius',
  'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
