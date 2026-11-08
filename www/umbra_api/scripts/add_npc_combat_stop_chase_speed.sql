-- Stop range de combate + multiplicador de velocidade em chase. Idempotente MySQL 5.7+.

-- npc_templates ---------------------------------------------------------------
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'combat_stop_range'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN combat_stop_range FLOAT NOT NULL DEFAULT 0 COMMENT ''Distância 2D para parar de chase e atacar; 0 = usa attack_range'' AFTER attack_range',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'chase_speed_mult'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN chase_speed_mult FLOAT NOT NULL DEFAULT 1.5 COMMENT ''Multiplicador de move_speed no Chase (aggro)'' AFTER move_speed',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- npc_instances (override; NULL = herda template) -----------------------------
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'combat_stop_range'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN combat_stop_range FLOAT NULL DEFAULT NULL COMMENT ''Override; NULL = herda template'' AFTER move_speed',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'chase_speed_mult'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN chase_speed_mult FLOAT NULL DEFAULT NULL COMMENT ''Override; NULL = herda template'' AFTER combat_stop_range',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
