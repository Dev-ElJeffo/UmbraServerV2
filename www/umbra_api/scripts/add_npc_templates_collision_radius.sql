-- Raio de corpo do NPC (uu), separado de interaction_radius (fala/loja). Idempotente.
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'npc_templates'
    AND COLUMN_NAME = 'collision_radius'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN collision_radius FLOAT NOT NULL DEFAULT 45.0 COMMENT ''Raio de corpo XY (uu); nao reutilizar interaction_radius'' AFTER interaction_radius',
  'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
