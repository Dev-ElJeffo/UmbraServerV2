-- Colunas de morte/respawn em players (idempotente)
SET NAMES utf8mb4;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'players' AND COLUMN_NAME = 'is_dead'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE `players` ADD COLUMN `is_dead` TINYINT(1) NOT NULL DEFAULT 0 AFTER `mana`',
  'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @col_exists2 = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'players' AND COLUMN_NAME = 'last_death_at'
);
SET @sql2 = IF(@col_exists2 = 0,
  'ALTER TABLE `players` ADD COLUMN `last_death_at` TIMESTAMP NULL DEFAULT NULL AFTER `is_dead`',
  'SELECT 1');
PREPARE stmt2 FROM @sql2;
EXECUTE stmt2;
DEALLOCATE PREPARE stmt2;
