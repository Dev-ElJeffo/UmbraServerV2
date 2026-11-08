-- damage_type PHYSICAL/MAGIC para classes, npc_templates, npc_skills + basic_power_coef do mob.
-- Idempotente.

-- classes.damage_type
SET @sql := (
  SELECT IF(
    EXISTS(
      SELECT 1 FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'damage_type'
    ),
    'SELECT 1',
    'ALTER TABLE classes ADD COLUMN damage_type ENUM(''PHYSICAL'',''MAGIC'') NOT NULL DEFAULT ''PHYSICAL'' COMMENT ''Base ATK do basic/dano da classe'' AFTER base_magic_attack'
  )
);
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- Backfill classes a partir de basic_attacks
UPDATE classes c
INNER JOIN basic_attacks ba ON ba.class_id = c.class_id
SET c.damage_type = IF(ba.damage_type = 'MAGIC', 'MAGIC', 'PHYSICAL')
WHERE EXISTS (
  SELECT 1 FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'classes' AND COLUMN_NAME = 'damage_type'
);

-- npc_templates.damage_type
SET @sql := (
  SELECT IF(
    EXISTS(
      SELECT 1 FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'damage_type'
    ),
    'SELECT 1',
    'ALTER TABLE npc_templates ADD COLUMN damage_type ENUM(''PHYSICAL'',''MAGIC'') NOT NULL DEFAULT ''PHYSICAL'' COMMENT ''School do basic attack do mob'' AFTER magic_attack'
  )
);
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- npc_templates.basic_power_coef
SET @sql := (
  SELECT IF(
    EXISTS(
      SELECT 1 FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'basic_power_coef'
    ),
    'SELECT 1',
    'ALTER TABLE npc_templates ADD COLUMN basic_power_coef SMALLINT UNSIGNED NOT NULL DEFAULT 100 COMMENT ''power_coef do auto-attack'' AFTER damage_type'
  )
);
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- npc_skills.damage_type
SET @sql := (
  SELECT IF(
    EXISTS(
      SELECT 1 FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_skills' AND COLUMN_NAME = 'damage_type'
    ),
    'SELECT 1',
    'ALTER TABLE npc_skills ADD COLUMN damage_type ENUM(''PHYSICAL'',''MAGIC'') NOT NULL DEFAULT ''PHYSICAL'' COMMENT ''ATK base do dano'' AFTER scaling_stat_id'
  )
);
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- Backfill npc_skills: element_id=1 (PHYSICAL) → PHYSICAL; senão MAGIC
UPDATE npc_skills
SET damage_type = IF(element_id = 1, 'PHYSICAL', 'MAGIC')
WHERE damage_type = 'PHYSICAL' OR damage_type = 'MAGIC';
