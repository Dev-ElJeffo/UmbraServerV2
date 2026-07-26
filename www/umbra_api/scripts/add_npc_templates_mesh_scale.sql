-- Escala uniforme do actor NPC no cliente (1.0 = tamanho padrao). Idempotente MySQL 5.7+.
SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'npc_templates'
    AND COLUMN_NAME = 'mesh_scale'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN mesh_scale FLOAT NOT NULL DEFAULT 1.0 COMMENT ''Escala uniforme do actor no cliente (1.0 = tamanho padrao)'' AFTER anim_blueprint_path',
  'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Goblin menor que o player
UPDATE npc_templates SET mesh_scale = 0.70 WHERE npc_template_id = 10;
