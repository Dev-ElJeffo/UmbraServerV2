-- Niagara do ataque básico de NPC (opcode 99). Idempotente.
SET NAMES utf8mb4;

SET @sql := (
  SELECT IF(
    EXISTS(
      SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'basic_vfx_path'
    ),
    'SELECT 1',
    'ALTER TABLE npc_templates ADD COLUMN basic_vfx_path VARCHAR(512) NULL DEFAULT NULL COMMENT ''Niagara caster do basic attack'' AFTER chase_speed_mult'
  )
);
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @sql := (
  SELECT IF(
    EXISTS(
      SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'basic_hit_vfx_path'
    ),
    'SELECT 1',
    'ALTER TABLE npc_templates ADD COLUMN basic_hit_vfx_path VARCHAR(512) NULL DEFAULT NULL COMMENT ''Niagara hit do basic attack'' AFTER basic_vfx_path'
  )
);
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
