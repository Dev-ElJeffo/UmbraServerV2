-- Paths modulares por classe (torso, mãos, pernas, pés, armas). Idempotente.
SET NAMES utf8mb4;

DELIMITER //

DROP PROCEDURE IF EXISTS ApplyClassModularMeshPathsSchema //
CREATE PROCEDURE ApplyClassModularMeshPathsSchema()
BEGIN
    DECLARE col_name VARCHAR(64);
    DECLARE done INT DEFAULT 0;
    DECLARE col_cursor CURSOR FOR
        SELECT column_name FROM (
            SELECT 'torso_mesh_path' AS column_name UNION ALL
            SELECT 'arms_mesh_path' UNION ALL
            SELECT 'legs_mesh_path' UNION ALL
            SELECT 'feet_mesh_path' UNION ALL
            SELECT 'main_hand_mesh_path' UNION ALL
            SELECT 'off_hand_mesh_path'
        ) cols;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;

    OPEN col_cursor;
    read_loop: LOOP
        FETCH col_cursor INTO col_name;
        IF done THEN
            LEAVE read_loop;
        END IF;

        IF NOT EXISTS (
            SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = DATABASE()
              AND TABLE_NAME = 'classes'
              AND COLUMN_NAME = col_name
        ) THEN
            SET @sql = CONCAT(
                'ALTER TABLE classes ADD COLUMN ', col_name,
                ' VARCHAR(512) NULL COMMENT ''Modular mesh path UE'''
            );
            PREPARE stmt FROM @sql;
            EXECUTE stmt;
            DEALLOCATE PREPARE stmt;
        ELSE
            SET @sql = CONCAT(
                'ALTER TABLE classes MODIFY ', col_name, ' VARCHAR(512) NULL'
            );
            PREPARE stmt FROM @sql;
            EXECUTE stmt;
            DEALLOCATE PREPARE stmt;
        END IF;
    END LOOP;
    CLOSE col_cursor;
END //

DELIMITER ;

CALL ApplyClassModularMeshPathsSchema();
DROP PROCEDURE IF EXISTS ApplyClassModularMeshPathsSchema;

-- Migra instalações antigas: left/right_hand_mesh_path -> arms_mesh_path
DELIMITER //

DROP PROCEDURE IF EXISTS MigrateClassHandsToArms //
CREATE PROCEDURE MigrateClassHandsToArms()
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'arms_mesh_path'
    ) THEN
        SET @sql = 'ALTER TABLE classes ADD COLUMN arms_mesh_path VARCHAR(512) NULL';
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

CALL MigrateClassHandsToArms();
DROP PROCEDURE IF EXISTS MigrateClassHandsToArms;
