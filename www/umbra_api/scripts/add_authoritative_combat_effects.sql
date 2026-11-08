-- Runtime autoritativo de efeitos. Compatível com MySQL 5.7+/8.0.
SET @dbname = DATABASE();

SET @sql = (SELECT IF(EXISTS(
  SELECT 1 FROM information_schema.columns
  WHERE table_schema=@dbname AND table_name='skills' AND column_name='include_caster'),
  'SELECT 1', 'ALTER TABLE skills ADD COLUMN include_caster TINYINT(1) NOT NULL DEFAULT 0 AFTER can_move_while_casting'));
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @sql = (SELECT IF(EXISTS(
  SELECT 1 FROM information_schema.columns
  WHERE table_schema=@dbname AND table_name='npc_templates' AND column_name='stun_resist'),
  'SELECT 1', 'ALTER TABLE npc_templates ADD COLUMN stun_resist TINYINT UNSIGNED NOT NULL DEFAULT 0'));
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @sql = (SELECT IF(EXISTS(
  SELECT 1 FROM information_schema.columns
  WHERE table_schema=@dbname AND table_name='npc_templates' AND column_name='silence_resist'),
  'SELECT 1', 'ALTER TABLE npc_templates ADD COLUMN silence_resist TINYINT UNSIGNED NOT NULL DEFAULT 0'));
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @sql = (SELECT IF(EXISTS(
  SELECT 1 FROM information_schema.columns
  WHERE table_schema=@dbname AND table_name='npc_templates' AND column_name='root_resist'),
  'SELECT 1', 'ALTER TABLE npc_templates ADD COLUMN root_resist TINYINT UNSIGNED NOT NULL DEFAULT 0'));
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @sql = (SELECT IF(EXISTS(
  SELECT 1 FROM information_schema.columns
  WHERE table_schema=@dbname AND table_name='npc_templates' AND column_name='slow_resist'),
  'SELECT 1', 'ALTER TABLE npc_templates ADD COLUMN slow_resist TINYINT UNSIGNED NOT NULL DEFAULT 0'));
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @sql = (SELECT IF(EXISTS(
  SELECT 1 FROM information_schema.columns
  WHERE table_schema=@dbname AND table_name='active_dots' AND column_name='target_resource'),
  'SELECT 1', 'ALTER TABLE active_dots ADD COLUMN target_resource VARCHAR(16) NOT NULL DEFAULT ''health'' AFTER dot_type'));
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
