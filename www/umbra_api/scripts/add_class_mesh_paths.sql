-- Mesh base e AnimBP por classe (player). Idempotente.
SET NAMES utf8mb4;

DELIMITER //

DROP PROCEDURE IF EXISTS ApplyClassMeshPathsSchema //
CREATE PROCEDURE ApplyClassMeshPathsSchema()
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'skeletal_mesh_path'
    ) THEN
        SET @sql = 'ALTER TABLE classes ADD COLUMN skeletal_mesh_path VARCHAR(512) NULL';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    ELSE
        SET @sql = 'ALTER TABLE classes MODIFY skeletal_mesh_path VARCHAR(512) NULL';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'anim_blueprint_path'
    ) THEN
        SET @sql = 'ALTER TABLE classes ADD COLUMN anim_blueprint_path VARCHAR(512) NULL AFTER skeletal_mesh_path';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    ELSE
        SET @sql = 'ALTER TABLE classes MODIFY anim_blueprint_path VARCHAR(512) NULL';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;
END //

DELIMITER ;

CALL ApplyClassMeshPathsSchema();
DROP PROCEDURE IF EXISTS ApplyClassMeshPathsSchema;

-- Fallback mannequin UE5 (ajuste paths por classe no Manager quando assets estiverem prontos)
UPDATE classes SET
  skeletal_mesh_path = COALESCE(NULLIF(skeletal_mesh_path, ''), '/Game/Characters/Mannequins/Meshes/SKM_Manny'),
  anim_blueprint_path = COALESCE(NULLIF(anim_blueprint_path, ''), '/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed')
WHERE class_id BETWEEN 1 AND 6;
