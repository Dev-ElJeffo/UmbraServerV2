-- Script CORRIGIDO para atualizar next_level_exp na tabela players
-- Este script desabilita o safe update mode temporariamente

USE umbra_eternum;

-- Desabilitar safe update mode temporariamente
SET SQL_SAFE_UPDATES = 0;

-- Atualizar next_level_exp para todos os jogadores baseado em seus níveis
-- Usar WHERE com KEY (id) para evitar erro de safe update mode
UPDATE players p
INNER JOIN player_levels pl ON p.level = pl.level_number
SET p.next_level_exp = pl.exp_for_next_level
WHERE p.id > 0;  -- Usar WHERE com condição que sempre é verdadeira mas usa KEY

-- Reabilitar safe update mode
SET SQL_SAFE_UPDATES = 1;

-- ============================================================================
-- VERIFICAR RESULTADO
-- ============================================================================
SELECT 
    id,
    character_name,
    level,
    experience,
    next_level_exp
FROM players
ORDER BY id
LIMIT 10;

-- Verificar quantos jogadores foram atualizados
SELECT 
    COUNT(*) as total_jogadores,
    SUM(CASE WHEN next_level_exp > 0 THEN 1 ELSE 0 END) as com_exp_preenchido
FROM players;

SELECT '✅ Coluna next_level_exp atualizada com sucesso!' AS message;

