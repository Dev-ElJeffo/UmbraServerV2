-- Script para criar sistema de pontos de atributos
-- Execute este script no MySQL Workbench ou phpMyAdmin

USE umbra_eternum;

-- ============================================================================
-- TABELA: player_stat_points
-- Armazena pontos não distribuídos e atributos adicionais do player
-- ============================================================================
CREATE TABLE IF NOT EXISTS player_stat_points (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    player_id BIGINT UNSIGNED NOT NULL,
    unspent_points INT UNSIGNED DEFAULT 0 COMMENT 'Pontos não distribuídos',
    strength_points INT UNSIGNED DEFAULT 0 COMMENT 'Pontos adicionais em Strength (além do base da classe)',
    dexterity_points INT UNSIGNED DEFAULT 0 COMMENT 'Pontos adicionais em Dexterity',
    intelligence_points INT UNSIGNED DEFAULT 0 COMMENT 'Pontos adicionais em Intelligence',
    vitality_points INT UNSIGNED DEFAULT 0 COMMENT 'Pontos adicionais em Vitality',
    luck_points INT UNSIGNED DEFAULT 0 COMMENT 'Pontos adicionais em Luck',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY unique_player_stat_points (player_id),
    FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE,
    INDEX idx_player_id (player_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- Inicializar player_stat_points para players existentes
-- ============================================================================
INSERT INTO player_stat_points (player_id, unspent_points, strength_points, dexterity_points, intelligence_points, vitality_points, luck_points)
SELECT 
    id as player_id,
    0 as unspent_points,  -- Começa com 0 pontos não distribuídos
    0 as strength_points,
    0 as dexterity_points,
    0 as intelligence_points,
    0 as vitality_points,
    0 as luck_points
FROM players
WHERE id NOT IN (SELECT player_id FROM player_stat_points)
ON DUPLICATE KEY UPDATE updated_at = CURRENT_TIMESTAMP;

-- ============================================================================
-- Verificar estrutura criada
-- ============================================================================
SELECT 
    TABLE_NAME,
    COLUMN_NAME,
    DATA_TYPE,
    COLUMN_TYPE,
    COLUMN_COMMENT
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = 'umbra_eternum'
  AND TABLE_NAME = 'player_stat_points'
ORDER BY ORDINAL_POSITION;

