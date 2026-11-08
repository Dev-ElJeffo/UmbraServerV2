-- Offsets RelLoc/RelRot/RelScale por mão (colar dos Parâmetros de Soquete do SK editor).
-- Idempotente.

-- RIGHT
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'right_hand_rel_x');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN right_hand_rel_x FLOAT NOT NULL DEFAULT 0 AFTER left_hand_mesh_path', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'right_hand_rel_y');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN right_hand_rel_y FLOAT NOT NULL DEFAULT 0 AFTER right_hand_rel_x', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'right_hand_rel_z');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN right_hand_rel_z FLOAT NOT NULL DEFAULT 0 AFTER right_hand_rel_y', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'right_hand_rel_pitch');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN right_hand_rel_pitch FLOAT NOT NULL DEFAULT 0 AFTER right_hand_rel_z', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'right_hand_rel_yaw');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN right_hand_rel_yaw FLOAT NOT NULL DEFAULT 0 AFTER right_hand_rel_pitch', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'right_hand_rel_roll');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN right_hand_rel_roll FLOAT NOT NULL DEFAULT 0 AFTER right_hand_rel_yaw', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'right_hand_rel_scale');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN right_hand_rel_scale FLOAT NOT NULL DEFAULT 1 AFTER right_hand_rel_roll', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;

-- LEFT
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'left_hand_rel_x');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN left_hand_rel_x FLOAT NOT NULL DEFAULT 0 AFTER right_hand_rel_scale', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'left_hand_rel_y');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN left_hand_rel_y FLOAT NOT NULL DEFAULT 0 AFTER left_hand_rel_x', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'left_hand_rel_z');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN left_hand_rel_z FLOAT NOT NULL DEFAULT 0 AFTER left_hand_rel_y', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'left_hand_rel_pitch');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN left_hand_rel_pitch FLOAT NOT NULL DEFAULT 0 AFTER left_hand_rel_z', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'left_hand_rel_yaw');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN left_hand_rel_yaw FLOAT NOT NULL DEFAULT 0 AFTER left_hand_rel_pitch', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'left_hand_rel_roll');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN left_hand_rel_roll FLOAT NOT NULL DEFAULT 0 AFTER left_hand_rel_yaw', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
SET @c := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'left_hand_rel_scale');
SET @sql := IF(@c = 0, 'ALTER TABLE npc_templates ADD COLUMN left_hand_rel_scale FLOAT NOT NULL DEFAULT 1 AFTER left_hand_rel_roll', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
