-- Multiplicador de velocidade ao se distanciar (kite/recuo). Idempotente MySQL 5.7+.

-- npc_templates ---------------------------------------------------------------
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'kite_speed_mult'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN kite_speed_mult FLOAT NOT NULL DEFAULT 1.0 COMMENT ''Multiplicador de move_speed ao afastar do player (recuo); 1.0 = roam'' AFTER chase_speed_mult',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- npc_instances (override; NULL = herda template) -----------------------------
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'kite_speed_mult'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN kite_speed_mult FLOAT NULL DEFAULT NULL COMMENT ''Override; NULL = herda template'' AFTER chase_speed_mult',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
