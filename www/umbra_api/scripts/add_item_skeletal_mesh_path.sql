-- Path SKM/SM visual por item (equipamento). Idempotente.
SET NAMES utf8mb4;

DELIMITER //

DROP PROCEDURE IF EXISTS ApplyItemSkeletalMeshPathSchema //
CREATE PROCEDURE ApplyItemSkeletalMeshPathSchema()
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM information_schema.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'item_templates'
          AND COLUMN_NAME = 'skeletal_mesh_path'
    ) THEN
        ALTER TABLE item_templates
            ADD COLUMN skeletal_mesh_path VARCHAR(512) NULL DEFAULT NULL
            AFTER icon_path;
    END IF;
END //

DELIMITER ;

CALL ApplyItemSkeletalMeshPathSchema();
DROP PROCEDURE IF EXISTS ApplyItemSkeletalMeshPathSchema;
