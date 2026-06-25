-- ============================================================================
-- UMBRA ETERNUM - COMBAT V2
-- Sistema autoritativo de combate + basic attacks + NPCs de teste
-- Compatibilidade: MySQL 5.7+ / 8.0 (sem ADD COLUMN IF NOT EXISTS)
-- ============================================================================
SET NAMES utf8mb4;

-- --------------------------------------------------------------------------
-- Skills: caminhos de animação/VFX/SFX e metadados de hit
-- --------------------------------------------------------------------------
DELIMITER //

DROP PROCEDURE IF EXISTS ApplyCombatV2SkillsColumns //
CREATE PROCEDURE ApplyCombatV2SkillsColumns()
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'cast_anim_path'
    ) THEN
        SET @sql = 'ALTER TABLE skills ADD COLUMN cast_anim_path VARCHAR(255) NULL AFTER sfx_key';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'vfx_path'
    ) THEN
        SET @sql = 'ALTER TABLE skills ADD COLUMN vfx_path VARCHAR(255) NULL AFTER cast_anim_path';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'sfx_path'
    ) THEN
        SET @sql = 'ALTER TABLE skills ADD COLUMN sfx_path VARCHAR(255) NULL AFTER vfx_path';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'hit_window_ms'
    ) THEN
        SET @sql = 'ALTER TABLE skills ADD COLUMN hit_window_ms INT UNSIGNED NOT NULL DEFAULT 300 AFTER sfx_path';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'damage_type'
    ) THEN
        SET @sql = 'ALTER TABLE skills ADD COLUMN damage_type ENUM(''PHYSICAL'', ''MAGIC'', ''TRUE'') NOT NULL DEFAULT ''PHYSICAL'' AFTER hit_window_ms';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;
END //

DELIMITER ;

CALL ApplyCombatV2SkillsColumns();
DROP PROCEDURE IF EXISTS ApplyCombatV2SkillsColumns;
-- --------------------------------------------------------------------------
-- Basic attack por classe (sempre disponível)
-- --------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS basic_attacks (
  class_id BIGINT UNSIGNED NOT NULL,
  attack_name VARCHAR(100) NOT NULL,
  power_coef SMALLINT UNSIGNED NOT NULL DEFAULT 80,
  damage_type ENUM('PHYSICAL', 'MAGIC', 'TRUE') NOT NULL DEFAULT 'PHYSICAL',
  element_id TINYINT UNSIGNED NOT NULL DEFAULT 1,
  cooldown_ms INT UNSIGNED NOT NULL DEFAULT 800,
  range_max SMALLINT UNSIGNED NOT NULL DEFAULT 250,
  cast_anim_path VARCHAR(255) NULL,
  vfx_path VARCHAR(255) NULL,
  sfx_path VARCHAR(255) NULL,
  hit_window_ms INT UNSIGNED NOT NULL DEFAULT 300,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (class_id),
  CONSTRAINT fk_basic_attacks_class FOREIGN KEY (class_id) REFERENCES classes(class_id) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT fk_basic_attacks_element FOREIGN KEY (element_id) REFERENCES skill_elements(element_id) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO basic_attacks (
  class_id, attack_name, power_coef, damage_type, element_id, cooldown_ms, range_max,
  cast_anim_path, vfx_path, sfx_path, hit_window_ms
) VALUES
  (1, 'Barbarian Basic Attack', 85, 'PHYSICAL', 1, 800, 250, '/Game/Animations/Combat/AM_Barb_BasicAttack', '/Game/VFX/Combat/VFX_BasicHit_Physical', '/Game/SFX/Combat/SFX_BasicHit_Physical', 300),
  (2, 'Templar Basic Attack',   82, 'PHYSICAL', 4, 850, 250, '/Game/Animations/Combat/AM_Temp_BasicAttack', '/Game/VFX/Combat/VFX_BasicHit_Holy',    '/Game/SFX/Combat/SFX_BasicHit_Holy',    320),
  (3, 'DarkMage Basic Attack',  78, 'MAGIC',    2, 900, 300, '/Game/Animations/Combat/AM_DMage_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Shadow',  '/Game/SFX/Combat/SFX_BasicHit_Shadow',  350),
  (4, 'Cleric Basic Attack',    80, 'MAGIC',    4, 900, 280, '/Game/Animations/Combat/AM_Cleric_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Holy',   '/Game/SFX/Combat/SFX_BasicHit_Holy',    340),
  (5, 'Assassin Basic Attack',  88, 'PHYSICAL', 1, 700, 220, '/Game/Animations/Combat/AM_Assn_BasicAttack', '/Game/VFX/Combat/VFX_BasicHit_Crit',    '/Game/SFX/Combat/SFX_BasicHit_Crit',    250),
  (6, 'Monk Basic Attack',      84, 'PHYSICAL', 1, 750, 240, '/Game/Animations/Combat/AM_Monk_BasicAttack', '/Game/VFX/Combat/VFX_BasicHit_Wind',    '/Game/SFX/Combat/SFX_BasicHit_Wind',    280)
ON DUPLICATE KEY UPDATE
attack_name = VALUES(attack_name),
  power_coef = VALUES(power_coef),
  damage_type = VALUES(damage_type),
  element_id = VALUES(element_id),
  cooldown_ms = VALUES(cooldown_ms),
  range_max = VALUES(range_max),
  cast_anim_path = VALUES(cast_anim_path),
  vfx_path = VALUES(vfx_path),
  sfx_path = VALUES(sfx_path),
  hit_window_ms = VALUES(hit_window_ms);
-- --------------------------------------------------------------------------
-- NPC templates: espelho de atributos de jogador para testes de combate
-- --------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS npc_templates (
  npc_template_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  npc_name VARCHAR(100) NOT NULL,
  level INT UNSIGNED NOT NULL DEFAULT 1,
  max_health INT UNSIGNED NOT NULL DEFAULT 100,
  max_mana INT UNSIGNED NOT NULL DEFAULT 50,
  strength INT UNSIGNED NOT NULL DEFAULT 10,
  dexterity INT UNSIGNED NOT NULL DEFAULT 10,
  vitality INT UNSIGNED NOT NULL DEFAULT 10,
  intelligence INT UNSIGNED NOT NULL DEFAULT 10,
  luck INT UNSIGNED NOT NULL DEFAULT 10,
  physical_attack INT NOT NULL DEFAULT 0,
  magic_attack INT NOT NULL DEFAULT 0,
  physical_defense INT NOT NULL DEFAULT 0,
  magic_defense INT NOT NULL DEFAULT 0,
  accuracy INT NOT NULL DEFAULT 0,
  dodge INT NOT NULL DEFAULT 0,
  critical INT NOT NULL DEFAULT 0,
  critical_resistance INT NOT NULL DEFAULT 0,
  double_attack_rate INT NOT NULL DEFAULT 0,
  double_attack_resistance INT NOT NULL DEFAULT 0,
  skeletal_mesh_path VARCHAR(255) NULL,
  anim_blueprint_path VARCHAR(255) NULL,
  is_editable TINYINT(1) NOT NULL DEFAULT 1,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (npc_template_id),
  KEY idx_npc_templates_name (npc_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
CREATE TABLE IF NOT EXISTS npc_instances (
  npc_instance_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  npc_template_id BIGINT UNSIGNED NOT NULL,
  zone_id INT UNSIGNED NOT NULL DEFAULT 1,
  pos_x FLOAT NOT NULL DEFAULT 0,
  pos_y FLOAT NOT NULL DEFAULT 0,
  pos_z FLOAT NOT NULL DEFAULT 200,
  yaw FLOAT NOT NULL DEFAULT 0,
  current_health INT NOT NULL DEFAULT 100,
  current_mana INT NOT NULL DEFAULT 50,
  is_dead TINYINT(1) NOT NULL DEFAULT 0,
  respawn_at TIMESTAMP NULL DEFAULT NULL,
  last_combat_at TIMESTAMP NULL DEFAULT NULL,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (npc_instance_id),
  KEY idx_npc_instances_zone (zone_id),
  KEY idx_npc_instances_template (npc_template_id),
  CONSTRAINT fk_npc_instances_template FOREIGN KEY (npc_template_id) REFERENCES npc_templates(npc_template_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
INSERT INTO npc_templates (
  npc_name, level, max_health, max_mana, strength, dexterity, vitality, intelligence, luck,
  physical_attack, magic_attack, physical_defense, magic_defense, accuracy, dodge,
  critical, critical_resistance, double_attack_rate, double_attack_resistance,
  skeletal_mesh_path, anim_blueprint_path, is_editable
) VALUES (
  'dummy_treino', 1, 5000, 1000, 20, 20, 20, 20, 20,
  120, 120, 80, 80, 100, 20,
  5, 5, 5, 5,
  NULL,
  NULL,
  1
)
ON DUPLICATE KEY UPDATE
  level = VALUES(level),
  max_health = VALUES(max_health),
  max_mana = VALUES(max_mana),
  strength = VALUES(strength),
  dexterity = VALUES(dexterity),
  vitality = VALUES(vitality),
  intelligence = VALUES(intelligence),
  luck = VALUES(luck),
  physical_attack = VALUES(physical_attack),
  magic_attack = VALUES(magic_attack),
  physical_defense = VALUES(physical_defense),
  magic_defense = VALUES(magic_defense),
  accuracy = VALUES(accuracy),
  dodge = VALUES(dodge),
  critical = VALUES(critical),
  critical_resistance = VALUES(critical_resistance),
  double_attack_rate = VALUES(double_attack_rate),
  double_attack_resistance = VALUES(double_attack_resistance),
  skeletal_mesh_path = VALUES(skeletal_mesh_path),
  anim_blueprint_path = VALUES(anim_blueprint_path),
  is_editable = VALUES(is_editable);
-- --------------------------------------------------------------------------
-- Trigger: garantir ataque básico (is_basic_attack) no player
-- --------------------------------------------------------------------------
DELIMITER //
DROP TRIGGER IF EXISTS trg_player_skills_grant_basic_attack//
CREATE TRIGGER trg_player_skills_grant_basic_attack
AFTER INSERT ON players
FOR EACH ROW
BEGIN
  DECLARE v_skill_id INT UNSIGNED DEFAULT NULL;
  SELECT s.skill_id INTO v_skill_id
    FROM skills s
   WHERE s.class_id = NEW.class_id
     AND s.is_basic_attack = 1
     AND s.is_enabled = 1
   ORDER BY s.skill_id ASC
   LIMIT 1;

  IF v_skill_id IS NOT NULL THEN
    INSERT IGNORE INTO player_skills (player_id, skill_id, current_rank)
    VALUES (NEW.id, v_skill_id, 1);
  END IF;
END//
DELIMITER ;
