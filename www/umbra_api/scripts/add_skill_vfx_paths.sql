-- Paths Niagara de skill (caster + hit). Idempotente.
SET NAMES utf8mb4;

DELIMITER //

DROP PROCEDURE IF EXISTS ApplySkillVfxPathsSchema //
CREATE PROCEDURE ApplySkillVfxPathsSchema()
BEGIN
    IF EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'vfx_path'
    ) THEN
        SET @sql = 'ALTER TABLE skills MODIFY vfx_path VARCHAR(512) NULL';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    ELSE
        SET @sql = 'ALTER TABLE skills ADD COLUMN vfx_path VARCHAR(512) NULL AFTER cast_anim_path';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'hit_vfx_path'
    ) THEN
        SET @sql = 'ALTER TABLE skills ADD COLUMN hit_vfx_path VARCHAR(512) NULL AFTER vfx_path';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_skills' AND COLUMN_NAME = 'vfx_path'
    ) THEN
        SET @sql = 'ALTER TABLE npc_skills ADD COLUMN vfx_path VARCHAR(512) NULL AFTER vfx_key';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_skills' AND COLUMN_NAME = 'hit_vfx_path'
    ) THEN
        SET @sql = 'ALTER TABLE npc_skills ADD COLUMN hit_vfx_path VARCHAR(512) NULL AFTER vfx_path';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;
END //

DELIMITER ;

CALL ApplySkillVfxPathsSchema();
DROP PROCEDURE IF EXISTS ApplySkillVfxPathsSchema;
