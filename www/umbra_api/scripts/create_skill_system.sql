-- ============================================================================
-- UMBRA ETERNUM - SISTEMA DE SKILLS COMPLETO
-- Schema SQL para MySQL 8+
-- Server-Side Authoritative Combat System
-- ============================================================================
-- Versão: 1.0.0
-- Data: 2026-02-21
-- ============================================================================

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ============================================================================
-- TABELA: skill_elements
-- Elementos de dano/resistência
-- ============================================================================
DROP TABLE IF EXISTS `skill_elements`;
CREATE TABLE `skill_elements` (
    `element_id` TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `element_name` VARCHAR(30) NOT NULL,
    `element_key` VARCHAR(20) NOT NULL,
    `color_hex` VARCHAR(7) DEFAULT '#FFFFFF',
    `description` TEXT,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`element_id`),
    UNIQUE KEY `uk_element_key` (`element_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `skill_elements` (`element_name`, `element_key`, `color_hex`, `description`) VALUES
('Physical', 'PHYSICAL', '#C0C0C0', 'Dano físico bruto'),
('Shadow', 'SHADOW', '#4B0082', 'Energia dimensional da Neblina Vermelha'),
('Fire', 'FIRE', '#FF4500', 'Chamas e calor intenso'),
('Holy', 'HOLY', '#FFD700', 'Energia sagrada purificadora'),
('Poison', 'POISON', '#32CD32', 'Toxinas e venenos'),
('Ice', 'ICE', '#00BFFF', 'Gelo e frio congelante'),
('Lightning', 'LIGHTNING', '#FFFF00', 'Eletricidade e raios'),
('Arcane', 'ARCANE', '#9400D3', 'Magia pura dimensional');

-- ============================================================================
-- TABELA: skill_types
-- Tipos de habilidades
-- ============================================================================
DROP TABLE IF EXISTS `skill_types`;
CREATE TABLE `skill_types` (
    `type_id` TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `type_name` VARCHAR(30) NOT NULL,
    `type_key` VARCHAR(20) NOT NULL,
    `description` TEXT,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`type_id`),
    UNIQUE KEY `uk_type_key` (`type_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `skill_types` (`type_name`, `type_key`, `description`) VALUES
('Active', 'ACTIVE', 'Habilidade ativa que requer ativação manual'),
('Passive', 'PASSIVE', 'Habilidade passiva sempre ativa'),
('Buff', 'BUFF', 'Buff que melhora stats próprios ou aliados'),
('Debuff', 'DEBUFF', 'Debuff que reduz stats inimigos'),
('Aura', 'AURA', 'Aura que afeta área ao redor continuamente'),
('Ultimate', 'ULTIMATE', 'Habilidade ultimate poderosa'),
('Reaction', 'REACTION', 'Habilidade reativa a eventos'),
('DOT', 'DOT', 'Damage Over Time - dano contínuo'),
('HOT', 'HOT', 'Heal Over Time - cura contínua');

-- ============================================================================
-- TABELA: skill_targets
-- Alvos possíveis das habilidades
-- ============================================================================
DROP TABLE IF EXISTS `skill_targets`;
CREATE TABLE `skill_targets` (
    `target_id` TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `target_name` VARCHAR(30) NOT NULL,
    `target_key` VARCHAR(20) NOT NULL,
    `description` TEXT,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`target_id`),
    UNIQUE KEY `uk_target_key` (`target_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `skill_targets` (`target_name`, `target_key`, `description`) VALUES
('Self', 'SELF', 'Afeta apenas o próprio usuário'),
('Enemy', 'ENEMY', 'Afeta um inimigo único'),
('Ally', 'ALLY', 'Afeta um aliado único'),
('Area', 'AREA', 'Afeta área (inimigos)'),
('Party', 'PARTY', 'Afeta todo o grupo'),
('Area Ally', 'AREA_ALLY', 'Afeta área (aliados)');

-- ============================================================================
-- TABELA: skill_scaling_stats
-- Stats de scaling principal
-- ============================================================================
DROP TABLE IF EXISTS `skill_scaling_stats`;
CREATE TABLE `skill_scaling_stats` (
    `scaling_id` TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `scaling_name` VARCHAR(30) NOT NULL,
    `scaling_key` VARCHAR(20) NOT NULL,
    `description` TEXT,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`scaling_id`),
    UNIQUE KEY `uk_scaling_key` (`scaling_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `skill_scaling_stats` (`scaling_name`, `scaling_key`, `description`) VALUES
('Physical Attack', 'PHYS_ATK', 'Escala com ataque físico'),
('Magic Attack', 'MAG_ATK', 'Escala com ataque mágico'),
('Health', 'HEALTH', 'Escala com vida máxima'),
('Defense', 'DEFENSE', 'Escala com defesa'),
('None', 'NONE', 'Sem scaling de stat');

-- ============================================================================
-- TABELA: skills
-- Definição principal de todas as habilidades
-- ============================================================================
DROP TABLE IF EXISTS `skills`;
CREATE TABLE `skills` (
    `skill_id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `skill_key` VARCHAR(50) NOT NULL,
    `skill_name` VARCHAR(100) NOT NULL,
    `class_id` BIGINT UNSIGNED NOT NULL,
    `skill_order` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `required_level` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `skill_cost` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `max_rank` TINYINT UNSIGNED NOT NULL DEFAULT 5,
    
    -- Tipo e Target
    `type_id` TINYINT UNSIGNED NOT NULL,
    `target_id` TINYINT UNSIGNED NOT NULL,
    `element_id` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    
    -- Scaling principal
    `scaling_stat_id` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    
    -- Scaling por atributo (percentuais 0-100 representando 0%-100%)
    `str_scaling` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `dex_scaling` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `vit_scaling` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `int_scaling` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `lck_scaling` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    
    -- Coeficientes de poder (multiplicados por 100 para precisão, ex: 150 = 1.5x)
    `power_coef` SMALLINT UNSIGNED NOT NULL DEFAULT 100,
    `secondary_coef` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    
    -- Recursos
    `resource_type` ENUM('MANA', 'HEALTH', 'STAMINA', 'NONE') NOT NULL DEFAULT 'MANA',
    `resource_cost` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    `resource_cost_percent` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    
    -- Timing (em milissegundos)
    `cooldown_ms` INT UNSIGNED NOT NULL DEFAULT 0,
    `cast_time_ms` INT UNSIGNED NOT NULL DEFAULT 0,
    `duration_ms` INT UNSIGNED NOT NULL DEFAULT 0,
    
    -- Range (em unidades de jogo)
    `range_min` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    `range_max` SMALLINT UNSIGNED NOT NULL DEFAULT 100,
    `area_radius` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    
    -- Flags
    `is_stackable` TINYINT(1) NOT NULL DEFAULT 0,
    `max_stacks` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `can_crit` TINYINT(1) NOT NULL DEFAULT 1,
    `ignores_defense` TINYINT(1) NOT NULL DEFAULT 0,
    `is_interrupt` TINYINT(1) NOT NULL DEFAULT 0,
    `requires_target` TINYINT(1) NOT NULL DEFAULT 1,
    `can_move_while_casting` TINYINT(1) NOT NULL DEFAULT 0,
    
    -- Threat
    `threat_modifier` SMALLINT NOT NULL DEFAULT 100,
    
    -- PvP
    `pvp_modifier` TINYINT UNSIGNED NOT NULL DEFAULT 100,
    
    -- Ícone e visual
    `icon_path` VARCHAR(255) DEFAULT NULL,
    `vfx_key` VARCHAR(100) DEFAULT NULL,
    `sfx_key` VARCHAR(100) DEFAULT NULL,
    
    -- Descrição
    `description` TEXT,
    `tooltip_template` TEXT,
    
    -- Tags para filtros e lógica especial (JSON array)
    `server_tags` JSON DEFAULT NULL,
    
    -- Efeitos complexos (JSON)
    `effects_json` JSON DEFAULT NULL,
    
    -- Metadados
    `is_enabled` TINYINT(1) NOT NULL DEFAULT 1,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    PRIMARY KEY (`skill_id`),
    UNIQUE KEY `uk_skill_key` (`skill_key`),
    KEY `idx_class_id` (`class_id`),
    KEY `idx_type_id` (`type_id`),
    KEY `idx_required_level` (`required_level`),
    KEY `idx_class_order` (`class_id`, `skill_order`),
    
    CONSTRAINT `fk_skills_class` FOREIGN KEY (`class_id`) REFERENCES `classes` (`class_id`) ON DELETE RESTRICT ON UPDATE CASCADE,
    CONSTRAINT `fk_skills_type` FOREIGN KEY (`type_id`) REFERENCES `skill_types` (`type_id`) ON DELETE RESTRICT ON UPDATE CASCADE,
    CONSTRAINT `fk_skills_target` FOREIGN KEY (`target_id`) REFERENCES `skill_targets` (`target_id`) ON DELETE RESTRICT ON UPDATE CASCADE,
    CONSTRAINT `fk_skills_element` FOREIGN KEY (`element_id`) REFERENCES `skill_elements` (`element_id`) ON DELETE RESTRICT ON UPDATE CASCADE,
    CONSTRAINT `fk_skills_scaling` FOREIGN KEY (`scaling_stat_id`) REFERENCES `skill_scaling_stats` (`scaling_id`) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: skill_effects
-- Efeitos normalizados das habilidades
-- ============================================================================
DROP TABLE IF EXISTS `skill_effects`;
CREATE TABLE `skill_effects` (
    `effect_id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `skill_id` INT UNSIGNED NOT NULL,
    `effect_order` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `effect_type` ENUM(
        'DAMAGE', 'HEAL', 'SHIELD',
        'BUFF_STAT', 'DEBUFF_STAT',
        'DOT', 'HOT',
        'CLEANSE', 'DISPEL',
        'STUN', 'SILENCE', 'SLOW', 'ROOT', 'KNOCKBACK',
        'TAUNT', 'STEALTH', 'INVULNERABLE',
        'LIFESTEAL', 'MANASTEAL',
        'SUMMON', 'TELEPORT',
        'EXECUTE', 'REFLECT',
        'COOLDOWN_RESET', 'RESOURCE_RESTORE'
    ) NOT NULL,
    `target_stat` VARCHAR(50) DEFAULT NULL,
    `value_flat` INT NOT NULL DEFAULT 0,
    `value_percent` SMALLINT NOT NULL DEFAULT 0,
    `duration_ms` INT UNSIGNED NOT NULL DEFAULT 0,
    `tick_interval_ms` INT UNSIGNED NOT NULL DEFAULT 1000,
    `chance_percent` TINYINT UNSIGNED NOT NULL DEFAULT 100,
    `conditions_json` JSON DEFAULT NULL,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    PRIMARY KEY (`effect_id`),
    KEY `idx_skill_effects` (`skill_id`, `effect_order`),
    
    CONSTRAINT `fk_effects_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: skill_rank_scaling
-- Scaling por rank da skill (opcional, para ajuste fino)
-- ============================================================================
DROP TABLE IF EXISTS `skill_rank_scaling`;
CREATE TABLE `skill_rank_scaling` (
    `scaling_id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `skill_id` INT UNSIGNED NOT NULL,
    `rank` TINYINT UNSIGNED NOT NULL,
    `power_coef_bonus` SMALLINT NOT NULL DEFAULT 0,
    `resource_cost_bonus` SMALLINT NOT NULL DEFAULT 0,
    `cooldown_reduction_ms` INT NOT NULL DEFAULT 0,
    `duration_bonus_ms` INT NOT NULL DEFAULT 0,
    `extra_effects_json` JSON DEFAULT NULL,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    PRIMARY KEY (`scaling_id`),
    UNIQUE KEY `uk_skill_rank` (`skill_id`, `rank`),
    
    CONSTRAINT `fk_rank_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: player_skills
-- Skills aprendidas pelos jogadores
-- ============================================================================
DROP TABLE IF EXISTS `player_skills`;
CREATE TABLE `player_skills` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `player_id` BIGINT UNSIGNED NOT NULL,
    `skill_id` INT UNSIGNED NOT NULL,
    `current_rank` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `learned_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    `last_used_at` TIMESTAMP NULL DEFAULT NULL,
    `total_uses` INT UNSIGNED NOT NULL DEFAULT 0,
    
    PRIMARY KEY (`id`),
    UNIQUE KEY `uk_player_skill` (`player_id`, `skill_id`),
    KEY `idx_player_skills` (`player_id`),
    
    CONSTRAINT `fk_pskills_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT `fk_pskills_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: player_skill_points
-- Pontos de skill do jogador
-- ============================================================================
DROP TABLE IF EXISTS `player_skill_points`;
CREATE TABLE `player_skill_points` (
    `player_id` BIGINT UNSIGNED NOT NULL,
    `total_points_earned` INT UNSIGNED NOT NULL DEFAULT 0,
    `points_spent` INT UNSIGNED NOT NULL DEFAULT 0,
    `points_available` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    PRIMARY KEY (`player_id`),
    
    CONSTRAINT `fk_skillpoints_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: player_skillbar
-- Configuração da barra de skills do jogador
-- ============================================================================
DROP TABLE IF EXISTS `player_skillbar`;
CREATE TABLE `player_skillbar` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `player_id` BIGINT UNSIGNED NOT NULL,
    `slot_index` TINYINT UNSIGNED NOT NULL,
    `skill_id` INT UNSIGNED DEFAULT NULL,
    `keybind` VARCHAR(20) DEFAULT NULL,
    `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    PRIMARY KEY (`id`),
    UNIQUE KEY `uk_player_slot` (`player_id`, `slot_index`),
    KEY `idx_player_skillbar` (`player_id`),
    
    CONSTRAINT `fk_skillbar_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT `fk_skillbar_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE SET NULL ON UPDATE CASCADE,
    CONSTRAINT `chk_slot_index` CHECK (`slot_index` >= 0 AND `slot_index` < 20)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: player_cooldowns
-- Cooldowns ativos (runtime, pode usar Redis também)
-- ============================================================================
DROP TABLE IF EXISTS `player_cooldowns`;
CREATE TABLE `player_cooldowns` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `player_id` BIGINT UNSIGNED NOT NULL,
    `skill_id` INT UNSIGNED NOT NULL,
    `started_at` TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    `expires_at` TIMESTAMP(3) NOT NULL,
    
    PRIMARY KEY (`id`),
    UNIQUE KEY `uk_player_cooldown` (`player_id`, `skill_id`),
    KEY `idx_expires` (`expires_at`),
    
    CONSTRAINT `fk_cooldown_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT `fk_cooldown_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: active_buffs
-- Buffs ativos em tempo real
-- ============================================================================
DROP TABLE IF EXISTS `active_buffs`;
CREATE TABLE `active_buffs` (
    `buff_id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `target_player_id` BIGINT UNSIGNED NOT NULL,
    `source_player_id` BIGINT UNSIGNED DEFAULT NULL,
    `skill_id` INT UNSIGNED NOT NULL,
    `effect_id` INT UNSIGNED DEFAULT NULL,
    `buff_type` ENUM('BUFF', 'DEBUFF', 'AURA', 'DOT', 'HOT', 'SHIELD') NOT NULL,
    `current_stacks` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `value_snapshot` INT NOT NULL DEFAULT 0,
    `started_at` TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    `expires_at` TIMESTAMP(3) NOT NULL,
    `last_tick_at` TIMESTAMP(3) DEFAULT NULL,
    `is_permanent` TINYINT(1) NOT NULL DEFAULT 0,
    `snapshot_json` JSON DEFAULT NULL,
    
    PRIMARY KEY (`buff_id`),
    KEY `idx_target_buffs` (`target_player_id`, `expires_at`),
    KEY `idx_source_buffs` (`source_player_id`),
    KEY `idx_skill_buffs` (`skill_id`),
    KEY `idx_expires` (`expires_at`),
    
    CONSTRAINT `fk_buff_target` FOREIGN KEY (`target_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT `fk_buff_source` FOREIGN KEY (`source_player_id`) REFERENCES `players` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
    CONSTRAINT `fk_buff_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: active_dots
-- DOTs ativos (damage/heal over time)
-- ============================================================================
DROP TABLE IF EXISTS `active_dots`;
CREATE TABLE `active_dots` (
    `dot_id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `target_player_id` BIGINT UNSIGNED NOT NULL,
    `source_player_id` BIGINT UNSIGNED DEFAULT NULL,
    `skill_id` INT UNSIGNED NOT NULL,
    `effect_id` INT UNSIGNED DEFAULT NULL,
    `dot_type` ENUM('DAMAGE', 'HEAL', 'MANA') NOT NULL,
    `element_id` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `tick_value` INT NOT NULL DEFAULT 0,
    `tick_interval_ms` INT UNSIGNED NOT NULL DEFAULT 1000,
    `ticks_remaining` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `started_at` TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    `next_tick_at` TIMESTAMP(3) NOT NULL,
    `expires_at` TIMESTAMP(3) NOT NULL,
    `snapshot_json` JSON DEFAULT NULL,
    
    PRIMARY KEY (`dot_id`),
    KEY `idx_target_dots` (`target_player_id`, `next_tick_at`),
    KEY `idx_next_tick` (`next_tick_at`),
    
    CONSTRAINT `fk_dot_target` FOREIGN KEY (`target_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT `fk_dot_source` FOREIGN KEY (`source_player_id`) REFERENCES `players` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
    CONSTRAINT `fk_dot_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: combat_log
-- Log de combate para análise e replay
-- ============================================================================
DROP TABLE IF EXISTS `combat_log`;
CREATE TABLE `combat_log` (
    `log_id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `timestamp` TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    `source_player_id` BIGINT UNSIGNED DEFAULT NULL,
    `target_player_id` BIGINT UNSIGNED DEFAULT NULL,
    `skill_id` INT UNSIGNED DEFAULT NULL,
    `action_type` ENUM('DAMAGE', 'HEAL', 'BUFF', 'DEBUFF', 'DEATH', 'RESURRECT', 'MISS', 'DODGE', 'BLOCK', 'CRIT') NOT NULL,
    `value` INT NOT NULL DEFAULT 0,
    `is_critical` TINYINT(1) NOT NULL DEFAULT 0,
    `overkill` INT NOT NULL DEFAULT 0,
    `zone_id` VARCHAR(50) DEFAULT NULL,
    `details_json` JSON DEFAULT NULL,
    
    PRIMARY KEY (`log_id`),
    KEY `idx_timestamp` (`timestamp`),
    KEY `idx_source` (`source_player_id`, `timestamp`),
    KEY `idx_target` (`target_player_id`, `timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: player_resistances
-- Resistências elementais do jogador
-- ============================================================================
DROP TABLE IF EXISTS `player_resistances`;
CREATE TABLE `player_resistances` (
    `player_id` BIGINT UNSIGNED NOT NULL,
    `physical_res` SMALLINT NOT NULL DEFAULT 0,
    `shadow_res` SMALLINT NOT NULL DEFAULT 0,
    `fire_res` SMALLINT NOT NULL DEFAULT 0,
    `holy_res` SMALLINT NOT NULL DEFAULT 0,
    `poison_res` SMALLINT NOT NULL DEFAULT 0,
    `ice_res` SMALLINT NOT NULL DEFAULT 0,
    `lightning_res` SMALLINT NOT NULL DEFAULT 0,
    `arcane_res` SMALLINT NOT NULL DEFAULT 0,
    `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    PRIMARY KEY (`player_id`),
    
    CONSTRAINT `fk_resistances_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: player_threat
-- Sistema de ameaça (aggro)
-- ============================================================================
DROP TABLE IF EXISTS `player_threat`;
CREATE TABLE `player_threat` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `mob_instance_id` VARCHAR(100) NOT NULL,
    `player_id` BIGINT UNSIGNED NOT NULL,
    `threat_value` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` TIMESTAMP(3) DEFAULT CURRENT_TIMESTAMP(3) ON UPDATE CURRENT_TIMESTAMP(3),
    
    PRIMARY KEY (`id`),
    UNIQUE KEY `uk_mob_player` (`mob_instance_id`, `player_id`),
    KEY `idx_mob_threat` (`mob_instance_id`, `threat_value` DESC),
    
    CONSTRAINT `fk_threat_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: skill_points_per_level
-- Configuração de pontos ganhos por nível
-- ============================================================================
DROP TABLE IF EXISTS `skill_points_per_level`;
CREATE TABLE `skill_points_per_level` (
    `level` TINYINT UNSIGNED NOT NULL,
    `points_granted` TINYINT UNSIGNED NOT NULL DEFAULT 3,
    `cumulative_points` INT UNSIGNED NOT NULL DEFAULT 0,
    
    PRIMARY KEY (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Inserir pontos por nível (3 pontos/nível, níveis 1-50)
INSERT INTO `skill_points_per_level` (`level`, `points_granted`, `cumulative_points`)
SELECT 
    level_number as `level`,
    3 as `points_granted`,
    (level_number * 3) as `cumulative_points`
FROM `player_levels`
WHERE level_number <= 50
ON DUPLICATE KEY UPDATE 
    points_granted = 3,
    cumulative_points = VALUES(cumulative_points);

-- ============================================================================
-- PROCEDURES: Utilidades
-- ============================================================================

DELIMITER //

-- Procedure para limpar cooldowns expirados
DROP PROCEDURE IF EXISTS `cleanup_expired_cooldowns`//
CREATE PROCEDURE `cleanup_expired_cooldowns`()
BEGIN
    DELETE FROM `player_cooldowns` WHERE `expires_at` < NOW(3);
END//

-- Procedure para limpar buffs expirados
DROP PROCEDURE IF EXISTS `cleanup_expired_buffs`//
CREATE PROCEDURE `cleanup_expired_buffs`()
BEGIN
    DELETE FROM `active_buffs` WHERE `expires_at` < NOW(3) AND `is_permanent` = 0;
END//

-- Procedure para limpar DOTs expirados
DROP PROCEDURE IF EXISTS `cleanup_expired_dots`//
CREATE PROCEDURE `cleanup_expired_dots`()
BEGIN
    DELETE FROM `active_dots` WHERE `expires_at` < NOW(3);
END//

-- Procedure para calcular pontos de skill disponíveis
DROP PROCEDURE IF EXISTS `calculate_skill_points`//
CREATE PROCEDURE `calculate_skill_points`(IN p_player_id BIGINT UNSIGNED)
BEGIN
    DECLARE v_level INT;
    DECLARE v_total_points INT;
    DECLARE v_spent_points INT;
    
    -- Obter nível do jogador
    SELECT `level` INTO v_level FROM `players` WHERE `id` = p_player_id;
    
    -- Calcular pontos totais baseado no nível
    SELECT COALESCE(cumulative_points, 0) INTO v_total_points 
    FROM `skill_points_per_level` 
    WHERE `level` = v_level;
    
    -- Calcular pontos gastos
    SELECT COALESCE(SUM(s.skill_cost * ps.current_rank), 0) INTO v_spent_points
    FROM `player_skills` ps
    JOIN `skills` s ON ps.skill_id = s.skill_id
    WHERE ps.player_id = p_player_id;
    
    -- Atualizar ou inserir registro
    INSERT INTO `player_skill_points` (`player_id`, `total_points_earned`, `points_spent`, `points_available`)
    VALUES (p_player_id, v_total_points, v_spent_points, v_total_points - v_spent_points)
    ON DUPLICATE KEY UPDATE
        total_points_earned = v_total_points,
        points_spent = v_spent_points,
        points_available = v_total_points - v_spent_points;
END//

-- Procedure para aprender skill
DROP PROCEDURE IF EXISTS `learn_skill`//
CREATE PROCEDURE `learn_skill`(
    IN p_player_id BIGINT UNSIGNED,
    IN p_skill_id INT UNSIGNED,
    OUT p_result VARCHAR(100),
    OUT p_success TINYINT
)
BEGIN
    DECLARE v_player_level INT;
    DECLARE v_player_class BIGINT UNSIGNED;
    DECLARE v_skill_class BIGINT UNSIGNED;
    DECLARE v_skill_level INT;
    DECLARE v_skill_cost INT;
    DECLARE v_points_available INT;
    DECLARE v_already_learned INT;
    
    SET p_success = 0;
    
    -- Verificar se jogador existe e obter dados
    SELECT `level`, `class_id` INTO v_player_level, v_player_class
    FROM `players` WHERE `id` = p_player_id;
    
    IF v_player_level IS NULL THEN
        SET p_result = 'PLAYER_NOT_FOUND';
        RETURN;
    END IF;
    
    -- Verificar se skill existe e obter dados
    SELECT `class_id`, `required_level`, `skill_cost` 
    INTO v_skill_class, v_skill_level, v_skill_cost
    FROM `skills` WHERE `skill_id` = p_skill_id AND `is_enabled` = 1;
    
    IF v_skill_class IS NULL THEN
        SET p_result = 'SKILL_NOT_FOUND';
        RETURN;
    END IF;
    
    -- Verificar classe
    IF v_skill_class != v_player_class THEN
        SET p_result = 'WRONG_CLASS';
        RETURN;
    END IF;
    
    -- Verificar nível
    IF v_player_level < v_skill_level THEN
        SET p_result = 'LEVEL_TOO_LOW';
        RETURN;
    END IF;
    
    -- Verificar se já aprendeu
    SELECT COUNT(*) INTO v_already_learned
    FROM `player_skills` WHERE `player_id` = p_player_id AND `skill_id` = p_skill_id;
    
    IF v_already_learned > 0 THEN
        SET p_result = 'ALREADY_LEARNED';
        RETURN;
    END IF;
    
    -- Calcular pontos disponíveis
    CALL calculate_skill_points(p_player_id);
    
    SELECT `points_available` INTO v_points_available
    FROM `player_skill_points` WHERE `player_id` = p_player_id;
    
    IF v_points_available < v_skill_cost THEN
        SET p_result = 'NOT_ENOUGH_POINTS';
        RETURN;
    END IF;
    
    -- Aprender skill
    INSERT INTO `player_skills` (`player_id`, `skill_id`, `current_rank`)
    VALUES (p_player_id, p_skill_id, 1);
    
    -- Atualizar pontos
    CALL calculate_skill_points(p_player_id);
    
    SET p_result = 'SUCCESS';
    SET p_success = 1;
END//

DELIMITER ;

-- ============================================================================
-- EVENT: Limpeza automática
-- ============================================================================
SET GLOBAL event_scheduler = ON;

DROP EVENT IF EXISTS `evt_cleanup_combat_data`;
CREATE EVENT `evt_cleanup_combat_data`
ON SCHEDULE EVERY 1 MINUTE
DO
BEGIN
    CALL cleanup_expired_cooldowns();
    CALL cleanup_expired_buffs();
    CALL cleanup_expired_dots();
END;

-- Limpar combat_log antigo (manter 7 dias)
DROP EVENT IF EXISTS `evt_cleanup_combat_log`;
CREATE EVENT `evt_cleanup_combat_log`
ON SCHEDULE EVERY 1 DAY
STARTS CURRENT_DATE + INTERVAL 3 HOUR
DO
BEGIN
    DELETE FROM `combat_log` WHERE `timestamp` < DATE_SUB(NOW(), INTERVAL 7 DAY);
END;

SET FOREIGN_KEY_CHECKS = 1;

-- ============================================================================
-- FIM DO SCHEMA BASE
-- ============================================================================
