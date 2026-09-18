-- VFX de refino por slot (+7..+12). Idempotente MySQL 5.7+.
-- Ex.: {"main_hand":"/Game/...System","off_hand":"...","chest":"..."}

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'refinement_config' AND COLUMN_NAME = 'slot_vfx_json'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE refinement_config ADD COLUMN slot_vfx_json JSON NULL COMMENT ''Niagara por slot visual no nivel de refine (main_hand, off_hand, head, chest, hands, legs, feet)'' AFTER stat_bonus_multiplier',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
