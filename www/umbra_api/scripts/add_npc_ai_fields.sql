-- Campos de AI / roam / aggro para NPCs. Idempotente MySQL 5.7+.

-- npc_templates ---------------------------------------------------------------
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'aggro_radius'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN aggro_radius FLOAT NOT NULL DEFAULT 0 COMMENT ''Raio de detecção; 0 = não inicia combate por proximidade'' AFTER kill_exp',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'leash_radius'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN leash_radius FLOAT NOT NULL DEFAULT 0 COMMENT ''Distância máxima do home; 0 = aggro_radius*2'' AFTER aggro_radius',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'attack_range'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN attack_range FLOAT NOT NULL DEFAULT 150 COMMENT ''Alcance do basic attack do mob'' AFTER leash_radius',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'attack_cooldown_ms'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN attack_cooldown_ms INT UNSIGNED NOT NULL DEFAULT 1500 COMMENT ''Cooldownown do basic attack do mob (ms)'' AFTER attack_range',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'move_speed'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN move_speed FLOAT NOT NULL DEFAULT 200 COMMENT ''Velocidade de wander/chase (uu/s)'' AFTER attack_cooldown_ms',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'roam_radius'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN roam_radius FLOAT NOT NULL DEFAULT 0 COMMENT ''Raio de wander em torno do home; 0 = parado'' AFTER move_speed',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'is_hostile'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN is_hostile TINYINT(1) NOT NULL DEFAULT 1 COMMENT ''1 = pode aggro/atacar players; separado de is_attackable'' AFTER roam_radius',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- npc_instances ---------------------------------------------------------------
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'home_x'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN home_x FLOAT NULL DEFAULT NULL COMMENT ''Centro da área de roam; NULL = pos_x'' AFTER yaw',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'home_y'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN home_y FLOAT NULL DEFAULT NULL AFTER home_x',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'home_z'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN home_z FLOAT NULL DEFAULT NULL AFTER home_y',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'roam_radius'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN roam_radius FLOAT NULL DEFAULT NULL COMMENT ''Override; NULL = herda template'' AFTER home_z',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'aggro_radius'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN aggro_radius FLOAT NULL DEFAULT NULL AFTER roam_radius',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'leash_radius'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN leash_radius FLOAT NULL DEFAULT NULL AFTER aggro_radius',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'move_speed'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_instances ADD COLUMN move_speed FLOAT NULL DEFAULT NULL AFTER leash_radius',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- Backfill home a partir da posição atual
UPDATE npc_instances
SET home_x = pos_x, home_y = pos_y, home_z = pos_z
WHERE home_x IS NULL OR home_y IS NULL OR home_z IS NULL;
