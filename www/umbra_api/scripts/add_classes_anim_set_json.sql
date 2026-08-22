-- Animation set humanoide por classe (mesmo JSON de npc_templates.anim_states_json).
-- Idempotente.

SET @col := (
  SELECT COUNT(*) FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'anim_set_json'
);
SET @sql := IF(@col = 0,
  'ALTER TABLE classes ADD COLUMN anim_set_json JSON NULL COMMENT ''Humanoid anim set: idle/walk/run dirs + attacks/casts/buffs'' AFTER class_description',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
