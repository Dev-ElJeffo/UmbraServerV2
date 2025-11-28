-- Script para adicionar coluna next_level_exp na tabela players
-- Execute este script no MySQL Workbench

USE umbra_eternum;

-- Verificar se a coluna já existe
SET @col_exists = (
    SELECT COUNT(*) 
    FROM INFORMATION_SCHEMA.COLUMNS 
    WHERE TABLE_SCHEMA = 'umbra_eternum' 
    AND TABLE_NAME = 'players' 
    AND COLUMN_NAME = 'next_level_exp'
);

-- Se não existir, adicionar coluna
SET @sql = IF(@col_exists = 0,
    'ALTER TABLE players ADD COLUMN next_level_exp INT UNSIGNED DEFAULT 0 COMMENT ''EXP necessário para próximo nível (cache)'' AFTER experience',
    'SELECT ''Coluna next_level_exp já existe'' AS message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Atualizar next_level_exp para todos os jogadores baseado em seus níveis
-- Usar WHERE para evitar erro de safe update mode
UPDATE players p
INNER JOIN player_levels pl ON p.level = pl.level_number
SET p.next_level_exp = pl.exp_for_next_level
WHERE p.level IS NOT NULL AND p.level > 0;

-- Verificar estrutura
DESCRIBE players;

-- Verificar alguns registros
SELECT 
    id,
    character_name,
    level,
    experience,
    next_level_exp
FROM players
LIMIT 10;

SELECT '✅ Coluna next_level_exp adicionada e atualizada com sucesso!' AS message;

