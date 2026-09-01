-- Visual meshes JSON por item (multi-slot + override por classe). Idempotente.
SET NAMES utf8mb4;

DELIMITER //

DROP PROCEDURE IF EXISTS ApplyItemVisualMeshesJsonSchema //
CREATE PROCEDURE ApplyItemVisualMeshesJsonSchema()
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM information_schema.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'item_templates'
          AND COLUMN_NAME = 'visual_meshes_json'
    ) THEN
        ALTER TABLE item_templates
            ADD COLUMN visual_meshes_json JSON NULL DEFAULT NULL
            AFTER skeletal_mesh_path;
    END IF;
END //

DELIMITER ;

CALL ApplyItemVisualMeshesJsonSchema();
DROP PROCEDURE IF EXISTS ApplyItemVisualMeshesJsonSchema;

-- Migrar skeletal_mesh_path legado -> visual_meshes_json.default[]
UPDATE item_templates
SET visual_meshes_json = JSON_OBJECT(
    'default', JSON_ARRAY(
        JSON_OBJECT(
            'slot', equipment_slot,
            'path', skeletal_mesh_path
        )
    ),
    'by_class', JSON_OBJECT()
)
WHERE (visual_meshes_json IS NULL OR JSON_LENGTH(visual_meshes_json) = 0)
  AND skeletal_mesh_path IS NOT NULL
  AND TRIM(skeletal_mesh_path) != ''
  AND equipment_slot IS NOT NULL
  AND equipment_slot != 'none';
