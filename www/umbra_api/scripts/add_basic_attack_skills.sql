-- ============================================================================
-- UMBRA ETERNUM - Skills dedicadas de ataque básico (skill_id 91-96)
-- Espelha basic_attacks; separado da árvore (skill_order 1-15, ex. Golpe da Ruína)
-- Idempotente: coluna is_basic_attack + INSERT por skill_key
-- ============================================================================
SET NAMES utf8mb4;

DELIMITER //

DROP PROCEDURE IF EXISTS ApplyBasicAttackSkillsSchema //
CREATE PROCEDURE ApplyBasicAttackSkillsSchema()
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

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'is_basic_attack'
    ) THEN
        SET @sql = 'ALTER TABLE skills ADD COLUMN is_basic_attack TINYINT(1) NOT NULL DEFAULT 0 AFTER is_enabled';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND INDEX_NAME = 'idx_class_basic'
    ) THEN
        SET @sql = 'ALTER TABLE skills ADD KEY idx_class_basic (class_id, is_basic_attack)';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;
END //

DELIMITER ;

CALL ApplyBasicAttackSkillsSchema();
DROP PROCEDURE IF EXISTS ApplyBasicAttackSkillsSchema;

-- skill_id fixos 91-96 (após as 90 skills da árvore)
INSERT INTO skills (
    skill_id, skill_key, skill_name, class_id, skill_order, required_level, skill_cost, max_rank,
    type_id, target_id, element_id, scaling_stat_id,
    str_scaling, dex_scaling, vit_scaling, int_scaling, lck_scaling,
    power_coef, secondary_coef,
    resource_type, resource_cost, resource_cost_percent,
    cooldown_ms, cast_time_ms, duration_ms,
    range_min, range_max, area_radius,
    is_stackable, max_stacks, can_crit, ignores_defense, is_interrupt, requires_target, can_move_while_casting,
    threat_modifier, pvp_modifier,
    icon_path, vfx_key, sfx_key,
    cast_anim_path, vfx_path, sfx_path, hit_window_ms, damage_type,
    description, tooltip_template,
    server_tags, effects_json,
    is_enabled, is_basic_attack
) VALUES
(91, 'BARB_BASIC_ATTACK', 'Ataque Básico', 1, 0, 1, 0, 1,
 1, 2, 1, 1,
 40, 0, 0, 0, 0,
 85, 0,
 'NONE', 0, 0,
 800, 0, 0,
 0, 250, 0,
 0, 1, 1, 0, 0, 1, 0,
 100, 100,
 'Skills/Barbarian/T_Basic_Attack', 'VFX_BasicHit_Physical', 'SFX_BasicHit_Physical',
 '/Game/Animations/Combat/AM_Barb_BasicAttack', '/Game/VFX/Combat/VFX_BasicHit_Physical', '/Game/SFX/Combat/SFX_BasicHit_Physical', 300, 'PHYSICAL',
 'Ataque básico corpo a corpo da Barbarian. Sempre disponível.',
 'Causa {damage} de dano físico.',
 '["melee", "basic_attack", "single_target"]',
 '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 85}]',
 1, 1),

(92, 'TEMP_BASIC_ATTACK', 'Ataque Básico', 2, 0, 1, 0, 1,
 1, 2, 4, 1,
 35, 0, 0, 35, 0,
 82, 0,
 'NONE', 0, 0,
 850, 0, 0,
 0, 250, 0,
 0, 1, 1, 0, 0, 1, 0,
 100, 100,
 'Skills/Templar/T_Basic_Attack', 'VFX_BasicHit_Holy', 'SFX_BasicHit_Holy',
 '/Game/Animations/Combat/AM_Temp_BasicAttack', '/Game/VFX/Combat/VFX_BasicHit_Holy', '/Game/SFX/Combat/SFX_BasicHit_Holy', 320, 'PHYSICAL',
 'Ataque básico consagrado do Templar. Sempre disponível.',
 'Causa {damage} de dano físico sagrado.',
 '["melee", "basic_attack", "single_target"]',
 '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 82}]',
 1, 1),

(93, 'DMAGE_BASIC_ATTACK', 'Ataque Básico', 3, 0, 1, 0, 1,
 1, 2, 2, 2,
 0, 0, 0, 50, 0,
 78, 0,
 'NONE', 0, 0,
 900, 0, 0,
 0, 300, 0,
 0, 1, 1, 0, 0, 1, 0,
 100, 100,
 'Skills/DarkMage/T_Basic_Attack', 'VFX_BasicHit_Shadow', 'SFX_BasicHit_Shadow',
 '/Game/Animations/Combat/AM_DMage_BasicAttack', '/Game/VFX/Combat/VFX_BasicHit_Shadow', '/Game/SFX/Combat/SFX_BasicHit_Shadow', 350, 'MAGIC',
 'Ataque básico sombrio da Dark Mage. Sempre disponível.',
 'Causa {damage} de dano mágico.',
 '["ranged", "basic_attack", "single_target"]',
 '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 78}]',
 1, 1),

(94, 'CLERIC_BASIC_ATTACK', 'Ataque Básico', 4, 0, 1, 0, 1,
 1, 2, 4, 2,
 0, 0, 0, 45, 0,
 80, 0,
 'NONE', 0, 0,
 900, 0, 0,
 0, 280, 0,
 0, 1, 1, 0, 0, 1, 0,
 100, 100,
 'Skills/Cleric/T_Basic_Attack', 'VFX_BasicHit_Holy', 'SFX_BasicHit_Holy',
 '/Game/Animations/Combat/AM_Cleric_BasicAttack', '/Game/VFX/Combat/VFX_BasicHit_Holy', '/Game/SFX/Combat/SFX_BasicHit_Holy', 340, 'MAGIC',
 'Ataque básico do Cleric. Sempre disponível.',
 'Causa {damage} de dano mágico.',
 '["ranged", "basic_attack", "single_target"]',
 '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 80}]',
 1, 1),

(95, 'ASSN_BASIC_ATTACK', 'Ataque Básico', 5, 0, 1, 0, 1,
 1, 2, 1, 1,
 30, 40, 0, 0, 0,
 88, 0,
 'NONE', 0, 0,
 700, 0, 0,
 0, 220, 0,
 0, 1, 1, 0, 0, 1, 0,
 100, 100,
 'Skills/Assassin/T_Basic_Attack', 'VFX_BasicHit_Crit', 'SFX_BasicHit_Crit',
 '/Game/Animations/Combat/AM_Assn_BasicAttack', '/Game/VFX/Combat/VFX_BasicHit_Crit', '/Game/SFX/Combat/SFX_BasicHit_Crit', 250, 'PHYSICAL',
 'Ataque básico rápido do Assassin. Sempre disponível.',
 'Causa {damage} de dano físico.',
 '["melee", "basic_attack", "single_target"]',
 '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 88}]',
 1, 1),

(96, 'MONK_BASIC_ATTACK', 'Ataque Básico', 6, 0, 1, 0, 1,
 1, 2, 1, 1,
 35, 25, 0, 0, 0,
 84, 0,
 'NONE', 0, 0,
 750, 0, 0,
 0, 240, 0,
 0, 1, 1, 0, 0, 1, 0,
 100, 100,
 'Skills/Monk/T_Basic_Attack', 'VFX_BasicHit_Wind', 'SFX_BasicHit_Wind',
 '/Game/Animations/Combat/AM_Monk_BasicAttack', '/Game/VFX/Combat/VFX_BasicHit_Wind', '/Game/SFX/Combat/SFX_BasicHit_Wind', 280, 'PHYSICAL',
 'Ataque básico marcial do Monk. Sempre disponível.',
 'Causa {damage} de dano físico.',
 '["melee", "basic_attack", "single_target"]',
 '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 84}]',
 1, 1)
ON DUPLICATE KEY UPDATE
    skill_name = VALUES(skill_name),
    class_id = VALUES(class_id),
    skill_order = VALUES(skill_order),
    required_level = VALUES(required_level),
    skill_cost = VALUES(skill_cost),
    max_rank = VALUES(max_rank),
    type_id = VALUES(type_id),
    target_id = VALUES(target_id),
    element_id = VALUES(element_id),
    scaling_stat_id = VALUES(scaling_stat_id),
    str_scaling = VALUES(str_scaling),
    dex_scaling = VALUES(dex_scaling),
    vit_scaling = VALUES(vit_scaling),
    int_scaling = VALUES(int_scaling),
    lck_scaling = VALUES(lck_scaling),
    power_coef = VALUES(power_coef),
    secondary_coef = VALUES(secondary_coef),
    resource_type = VALUES(resource_type),
    resource_cost = VALUES(resource_cost),
    resource_cost_percent = VALUES(resource_cost_percent),
    cooldown_ms = VALUES(cooldown_ms),
    cast_time_ms = VALUES(cast_time_ms),
    duration_ms = VALUES(duration_ms),
    range_min = VALUES(range_min),
    range_max = VALUES(range_max),
    area_radius = VALUES(area_radius),
    icon_path = VALUES(icon_path),
    vfx_key = VALUES(vfx_key),
    sfx_key = VALUES(sfx_key),
    cast_anim_path = VALUES(cast_anim_path),
    vfx_path = VALUES(vfx_path),
    sfx_path = VALUES(sfx_path),
    hit_window_ms = VALUES(hit_window_ms),
    damage_type = VALUES(damage_type),
    description = VALUES(description),
    tooltip_template = VALUES(tooltip_template),
    server_tags = VALUES(server_tags),
    effects_json = VALUES(effects_json),
    is_enabled = VALUES(is_enabled),
    is_basic_attack = VALUES(is_basic_attack);

-- Garantir AUTO_INCREMENT acima de 96 para futuras skills da árvore
ALTER TABLE skills AUTO_INCREMENT = 97;
