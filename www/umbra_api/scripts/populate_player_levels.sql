-- Script para POPULAR a tabela player_levels
-- Execute este script APÓS criar a tabela
-- Use este script se a tabela já existe mas está vazia

USE umbra_eternum;

-- Limpar dados existentes (se houver)
DELETE FROM player_levels;

-- ============================================================================
-- INSERIR NÍVEIS 1-50
-- Fórmula: EXP crescente exponencial (15% por nível)
-- Level 1->2: 1000 EXP
-- Level N->N+1: EXP anterior * 1.15
-- ============================================================================

-- Level 1
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (1, 0, 1000, 10, 20, 20, 5, 5, 3, 3);

-- Level 2
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (2, 1000, 1150, 10, 20, 20, 5, 5, 3, 3);

-- Level 3
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (3, 2150, 1323, 10, 20, 20, 5, 5, 3, 3);

-- Level 4
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (4, 3473, 1521, 10, 20, 20, 5, 5, 3, 3);

-- Level 5
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (5, 4994, 1750, 10, 20, 20, 5, 5, 3, 3);

-- Level 6
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (6, 6744, 2013, 10, 20, 20, 5, 5, 3, 3);

-- Level 7
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (7, 8757, 2315, 10, 20, 20, 5, 5, 3, 3);

-- Level 8
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (8, 11072, 2662, 10, 20, 20, 5, 5, 3, 3);

-- Level 9
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (9, 13734, 3061, 10, 20, 20, 5, 5, 3, 3);

-- Level 10
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (10, 16795, 3520, 10, 20, 20, 5, 5, 3, 3);

-- Level 11
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (11, 20315, 4048, 10, 20, 20, 5, 5, 3, 3);

-- Level 12
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (12, 24363, 4655, 10, 20, 20, 5, 5, 3, 3);

-- Level 13
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (13, 29018, 5353, 10, 20, 20, 5, 5, 3, 3);

-- Level 14
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (14, 34371, 6156, 10, 20, 20, 5, 5, 3, 3);

-- Level 15
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (15, 40527, 7080, 10, 20, 20, 5, 5, 3, 3);

-- Level 16
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (16, 47607, 8142, 10, 20, 20, 5, 5, 3, 3);

-- Level 17
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (17, 55749, 9363, 10, 20, 20, 5, 5, 3, 3);

-- Level 18
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (18, 65112, 10777, 10, 20, 20, 5, 5, 3, 3);

-- Level 19
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (19, 75889, 12394, 10, 20, 20, 5, 5, 3, 3);

-- Level 20
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (20, 88283, 14253, 10, 20, 20, 5, 5, 3, 3);

-- Level 21
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (21, 102536, 16391, 10, 20, 20, 5, 5, 3, 3);

-- Level 22
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (22, 118927, 18850, 10, 20, 20, 5, 5, 3, 3);

-- Level 23
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (23, 137777, 21677, 10, 20, 20, 5, 5, 3, 3);

-- Level 24
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (24, 159454, 24929, 10, 20, 20, 5, 5, 3, 3);

-- Level 25
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (25, 184383, 28669, 10, 20, 20, 5, 5, 3, 3);

-- Level 26
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (26, 213052, 32969, 10, 20, 20, 5, 5, 3, 3);

-- Level 27
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (27, 246021, 37914, 10, 20, 20, 5, 5, 3, 3);

-- Level 28
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (28, 283935, 43601, 10, 20, 20, 5, 5, 3, 3);

-- Level 29
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (29, 327536, 50141, 10, 20, 20, 5, 5, 3, 3);

-- Level 30
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (30, 377677, 57662, 10, 20, 20, 5, 5, 3, 3);

-- Level 31
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (31, 435339, 66311, 10, 20, 20, 5, 5, 3, 3);

-- Level 32
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (32, 501650, 76258, 10, 20, 20, 5, 5, 3, 3);

-- Level 33
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (33, 577908, 87697, 10, 20, 20, 5, 5, 3, 3);

-- Level 34
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (34, 665605, 100851, 10, 20, 20, 5, 5, 3, 3);

-- Level 35
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (35, 766456, 115979, 10, 20, 20, 5, 5, 3, 3);

-- Level 36
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (36, 882435, 133376, 10, 20, 20, 5, 5, 3, 3);

-- Level 37
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (37, 1015811, 153382, 10, 20, 20, 5, 5, 3, 3);

-- Level 38
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (38, 1169193, 176390, 10, 20, 20, 5, 5, 3, 3);

-- Level 39
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (39, 1345583, 202848, 10, 20, 20, 5, 5, 3, 3);

-- Level 40
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (40, 1548431, 233276, 10, 20, 20, 5, 5, 3, 3);

-- Level 41
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (41, 1781707, 268267, 10, 20, 20, 5, 5, 3, 3);

-- Level 42
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (42, 2049974, 308507, 10, 20, 20, 5, 5, 3, 3);

-- Level 43
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (43, 2358481, 354793, 10, 20, 20, 5, 5, 3, 3);

-- Level 44
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (44, 2713274, 408012, 10, 20, 20, 5, 5, 3, 3);

-- Level 45
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (45, 3121286, 469214, 10, 20, 20, 5, 5, 3, 3);

-- Level 46
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (46, 3590500, 539596, 10, 20, 20, 5, 5, 3, 3);

-- Level 47
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (47, 4130096, 620535, 10, 20, 20, 5, 5, 3, 3);

-- Level 48
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (48, 4750631, 713615, 10, 20, 20, 5, 5, 3, 3);

-- Level 49
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (49, 5464246, 820658, 10, 20, 20, 5, 5, 3, 3);

-- Level 50
INSERT INTO player_levels (level_number, exp_required, exp_for_next_level, stat_points_gained, hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
VALUES (50, 6284904, 0, 10, 20, 20, 5, 5, 3, 3);

-- ============================================================================
-- VERIFICAR DADOS INSERIDOS
-- ============================================================================
SELECT 
    level_number as 'Nível',
    exp_required as 'EXP Total',
    exp_for_next_level as 'EXP Próximo',
    stat_points_gained as 'Pontos',
    hp_gain as 'HP',
    mp_gain as 'MP'
FROM player_levels
ORDER BY level_number;

-- Verificar alguns níveis específicos
SELECT 
    level_number,
    exp_required,
    exp_for_next_level
FROM player_levels
WHERE level_number IN (1, 2, 5, 10, 20, 30, 40, 50)
ORDER BY level_number;

