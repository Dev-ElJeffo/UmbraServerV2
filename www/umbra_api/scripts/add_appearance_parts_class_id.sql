-- Appearance parts por classe. Idempotente.
SET NAMES utf8mb4;

SET @sql := (
  SELECT IF(
    EXISTS(
      SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'player_appearance_parts' AND COLUMN_NAME = 'class_id'
    ),
    'SELECT 1',
    'ALTER TABLE player_appearance_parts ADD COLUMN class_id INT UNSIGNED NOT NULL DEFAULT 0 COMMENT ''0 = todas as classes; >0 = só essa class_id'' AFTER part_id'
  )
);
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- Troca unique (part_type, part_id) -> (part_type, part_id, class_id)
SET @hasOld := (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.STATISTICS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'player_appearance_parts' AND INDEX_NAME = 'uq_part_type_id'
);
SET @sql := IF(@hasOld > 0, 'ALTER TABLE player_appearance_parts DROP INDEX uq_part_type_id', 'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @hasNew := (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.STATISTICS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'player_appearance_parts' AND INDEX_NAME = 'uq_part_type_id_class'
);
SET @sql := IF(@hasNew = 0,
  'ALTER TABLE player_appearance_parts ADD UNIQUE KEY uq_part_type_id_class (part_type, part_id, class_id)',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @hasIdx := (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.STATISTICS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'player_appearance_parts' AND INDEX_NAME = 'idx_class_type_enabled'
);
SET @sql := IF(@hasIdx = 0,
  'ALTER TABLE player_appearance_parts ADD INDEX idx_class_type_enabled (class_id, part_type, is_enabled)',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
