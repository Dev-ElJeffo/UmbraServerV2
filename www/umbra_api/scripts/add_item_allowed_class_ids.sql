-- Coluna allowed_class_ids em item_templates (null/[] = todas as classes). Idempotente.
SET NAMES utf8mb4;

DELIMITER //

DROP PROCEDURE IF EXISTS ApplyItemAllowedClassIdsSchema //
CREATE PROCEDURE ApplyItemAllowedClassIdsSchema()
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM information_schema.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'item_templates'
          AND COLUMN_NAME = 'allowed_class_ids'
    ) THEN
        ALTER TABLE item_templates
            ADD COLUMN allowed_class_ids JSON NULL DEFAULT NULL
            COMMENT 'null/[] = todas as classes; [1,2] = só essas class_id'
            AFTER item_subtype;
    END IF;
END //

DELIMITER ;

CALL ApplyItemAllowedClassIdsSchema();
DROP PROCEDURE IF EXISTS ApplyItemAllowedClassIdsSchema;
