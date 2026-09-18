-- Expand damage_type ENUM to include TRUE on classes / npc_templates / npc_skills.
-- skills.damage_type already allows TRUE via combat_v2.sql.

-- classes.damage_type
SET @sql = (
  SELECT IF(
    EXISTS (
      SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'damage_type'
    ),
    'ALTER TABLE classes MODIFY COLUMN damage_type ENUM(''PHYSICAL'',''MAGIC'',''TRUE'') NOT NULL DEFAULT ''PHYSICAL''',
    'SELECT 1'
  )
);
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- npc_templates.damage_type
SET @sql = (
  SELECT IF(
    EXISTS (
      SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'damage_type'
    ),
    'ALTER TABLE npc_templates MODIFY COLUMN damage_type ENUM(''PHYSICAL'',''MAGIC'',''TRUE'') NOT NULL DEFAULT ''PHYSICAL''',
    'SELECT 1'
  )
);
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- npc_skills.damage_type
SET @sql = (
  SELECT IF(
    EXISTS (
      SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_skills' AND COLUMN_NAME = 'damage_type'
    ),
    'ALTER TABLE npc_skills MODIFY COLUMN damage_type ENUM(''PHYSICAL'',''MAGIC'',''TRUE'') NOT NULL DEFAULT ''PHYSICAL''',
    'SELECT 1'
  )
);
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- basic_attacks.damage_type (se existir)
SET @sql = (
  SELECT IF(
    EXISTS (
      SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'basic_attacks' AND COLUMN_NAME = 'damage_type'
    ),
    'ALTER TABLE basic_attacks MODIFY COLUMN damage_type ENUM(''PHYSICAL'',''MAGIC'',''TRUE'') NOT NULL DEFAULT ''PHYSICAL''',
    'SELECT 1'
  )
);
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
