-- Unifica left/right hand em arms_mesh_path (modular Polyart SK_Male_Arms). Idempotente.
SET NAMES utf8mb4;

DELIMITER //

DROP PROCEDURE IF EXISTS ApplyClassArmsMeshPathSchema //
CREATE PROCEDURE ApplyClassArmsMeshPathSchema()
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'arms_mesh_path'
    ) THEN
        SET @sql = 'ALTER TABLE classes ADD COLUMN arms_mesh_path VARCHAR(512) NULL COMMENT ''Modular arms SK (ex. SK_Male_Arms)''';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    ELSE
        SET @sql = 'ALTER TABLE classes MODIFY arms_mesh_path VARCHAR(512) NULL';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'left_hand_mesh_path'
    ) THEN
        UPDATE classes SET arms_mesh_path = COALESCE(NULLIF(arms_mesh_path, ''), NULLIF(left_hand_mesh_path, ''), NULLIF(right_hand_mesh_path, ''))
        WHERE (arms_mesh_path IS NULL OR arms_mesh_path = '')
          AND ((left_hand_mesh_path IS NOT NULL AND left_hand_mesh_path != '')
            OR (right_hand_mesh_path IS NOT NULL AND right_hand_mesh_path != ''));

        SET @sql = 'ALTER TABLE classes DROP COLUMN left_hand_mesh_path';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'right_hand_mesh_path'
    ) THEN
        SET @sql = 'ALTER TABLE classes DROP COLUMN right_hand_mesh_path';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;
END //

DELIMITER ;

CALL ApplyClassArmsMeshPathSchema();
DROP PROCEDURE IF EXISTS ApplyClassArmsMeshPathSchema;
