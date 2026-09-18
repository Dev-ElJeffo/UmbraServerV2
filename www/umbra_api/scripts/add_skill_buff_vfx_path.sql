-- Niagara de loop para buffs de skill. Idempotente MySQL 5.7+.

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'buff_vfx_path'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE skills ADD COLUMN buff_vfx_path VARCHAR(512) NULL DEFAULT NULL COMMENT ''Niagara loop no corpo enquanto o buff estiver ativo'' AFTER hit_vfx_path',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
