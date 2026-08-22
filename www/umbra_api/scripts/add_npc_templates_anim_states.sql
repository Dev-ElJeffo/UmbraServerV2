-- Paths de montage/sequence por estado do NPC (attacks[], hits[], death, skill, idle, walk, death_ms).
-- Idempotente.

SET @col := (
  SELECT COUNT(*) FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'anim_states_json'
);
SET @sql := IF(@col = 0,
  'ALTER TABLE npc_templates ADD COLUMN anim_states_json JSON NULL COMMENT ''NPC anim states: attacks/hits arrays + death/skill/idle/walk/death_ms'' AFTER left_hand_rel_scale',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
