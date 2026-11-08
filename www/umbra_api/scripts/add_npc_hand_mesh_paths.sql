-- Paths de StaticMesh anexados em HandGrip_R / HandGrip_L (runtime; não é preview do SK editor).
-- Idempotente.

SET @col := (
  SELECT COUNT(*) FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'right_hand_mesh_path'
);
SET @sql := IF(@col = 0,
  'ALTER TABLE npc_templates ADD COLUMN right_hand_mesh_path VARCHAR(512) NULL COMMENT ''StaticMesh path HandGrip_R'' AFTER anim_blueprint_path',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col := (
  SELECT COUNT(*) FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'left_hand_mesh_path'
);
SET @sql := IF(@col = 0,
  'ALTER TABLE npc_templates ADD COLUMN left_hand_mesh_path VARCHAR(512) NULL COMMENT ''StaticMesh path HandGrip_L'' AFTER right_hand_mesh_path',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- Guardas Charles (12) e Rick (13): espada na mão direita
UPDATE npc_templates
SET right_hand_mesh_path = '/Game/Models/weapons/sword1'
WHERE npc_template_id IN (12, 13)
  AND (right_hand_mesh_path IS NULL OR right_hand_mesh_path = '');
