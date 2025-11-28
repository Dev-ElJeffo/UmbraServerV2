-- Script para criar tabela de níveis do personagem
-- Execute este script no MySQL Workbench ou phpMyAdmin

USE umbra_eternum;

-- ============================================================================
-- TABELA: player_levels
-- Armazena informações de cada nível (EXP necessário, bônus, etc.)
-- ============================================================================
CREATE TABLE IF NOT EXISTS player_levels (
    level_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    level_number INT UNSIGNED NOT NULL UNIQUE COMMENT 'Número do nível (1-50)',
    exp_required INT UNSIGNED NOT NULL COMMENT 'EXP necessário para alcançar este nível (EXP total acumulado)',
    exp_for_next_level INT UNSIGNED NOT NULL COMMENT 'EXP necessário para passar deste nível para o próximo',
    stat_points_gained INT UNSIGNED DEFAULT 10 COMMENT 'Pontos de atributos ganhos neste nível',
    hp_gain INT UNSIGNED DEFAULT 20 COMMENT 'HP máximo ganho neste nível',
    mp_gain INT UNSIGNED DEFAULT 20 COMMENT 'MP máximo ganho neste nível',
    phys_atk_gain INT UNSIGNED DEFAULT 5 COMMENT 'Ataque físico ganho neste nível',
    mag_atk_gain INT UNSIGNED DEFAULT 5 COMMENT 'Ataque mágico ganho neste nível',
    phys_def_gain INT UNSIGNED DEFAULT 3 COMMENT 'Defesa física ganha neste nível',
    mag_def_gain INT UNSIGNED DEFAULT 3 COMMENT 'Defesa mágica ganha neste nível',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (level_id),
    UNIQUE KEY unique_level_number (level_number),
    INDEX idx_level_number (level_number)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- POPULAR TABELA: Níveis 1 a 50
-- Fórmula: EXP crescente exponencial
-- Level 1->2: 1000 EXP
-- Level N->N+1: EXP anterior * 1.15 (aumento de 15% por nível)
-- ============================================================================

-- Limpar dados existentes (se houver)
DELETE FROM player_levels;

-- Inserir níveis 1 a 50
-- Level 1: EXP total = 0 (já está no nível 1)
-- Level 2: EXP total = 1000 (precisa de 1000 para chegar ao nível 2)
-- Level 3: EXP total = 1000 + 1150 = 2150
-- Level 4: EXP total = 2150 + 1322.5 = 3472.5
-- etc.

SET @current_exp = 0;
SET @exp_for_current_level = 1000;
SET @level = 1;

-- Level 1 (já está no nível 1, não precisa de EXP)
INSERT INTO player_levels (
    level_number, 
    exp_required, 
    exp_for_next_level,
    stat_points_gained,
    hp_gain,
    mp_gain,
    phys_atk_gain,
    mag_atk_gain,
    phys_def_gain,
    mag_def_gain
) VALUES (
    1,
    0,
    1000,
    10,
    20,
    20,
    5,
    5,
    3,
    3
);

-- Níveis 2 a 50
SET @level = 2;
SET @exp_for_current_level = 1000;
SET @current_exp = 1000;

WHILE @level <= 50 DO
    -- Calcular EXP necessário para este nível (EXP total acumulado)
    -- EXP para próximo nível = EXP anterior * 1.15 (arredondado)
    SET @exp_for_next = ROUND(@exp_for_current_level * 1.15);
    
    INSERT INTO player_levels (
        level_number,
        exp_required,
        exp_for_next_level,
        stat_points_gained,
        hp_gain,
        mp_gain,
        phys_atk_gain,
        mag_atk_gain,
        phys_def_gain,
        mag_def_gain
    ) VALUES (
        @level,
        @current_exp,
        @exp_for_next,
        10,
        20,
        20,
        5,
        5,
        3,
        3
    );
    
    -- Atualizar para próximo nível
    SET @current_exp = @current_exp + @exp_for_next;
    SET @exp_for_current_level = @exp_for_next;
    SET @level = @level + 1;
END WHILE;

-- ============================================================================
-- VERIFICAR DADOS INSERIDOS
-- ============================================================================
SELECT 
    level_number,
    exp_required as 'EXP Total Acumulado',
    exp_for_next_level as 'EXP para Próximo Nível',
    stat_points_gained,
    hp_gain,
    mp_gain
FROM player_levels
ORDER BY level_number
LIMIT 10;

-- Verificar alguns níveis específicos
SELECT 
    level_number,
    exp_required,
    exp_for_next_level
FROM player_levels
WHERE level_number IN (1, 2, 5, 10, 20, 30, 40, 50)
ORDER BY level_number;

