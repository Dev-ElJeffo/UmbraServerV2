-- cast_anim_path em npc_skills (montage UE), separado de Niagara vfx_path. Idempotente.

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_skills' AND COLUMN_NAME = 'cast_anim_path'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_skills ADD COLUMN cast_anim_path VARCHAR(512) NULL DEFAULT NULL COMMENT ''Montage/Sequence UE no cast (como skills.cast_anim_path)'' AFTER icon_path',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
