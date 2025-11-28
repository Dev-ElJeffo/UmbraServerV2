-- Script para adicionar colunas de EXP na tabela players (se necessário)
-- Execute este script no MySQL Workbench ou phpMyAdmin

USE umbra_eternum;

-- Verificar se a coluna 'experience' já existe
-- Se não existir, adicionar
SET @col_exists = (
    SELECT COUNT(*) 
    FROM INFORMATION_SCHEMA.COLUMNS 
    WHERE TABLE_SCHEMA = 'umbra_eternum' 
    AND TABLE_NAME = 'players' 
    AND COLUMN_NAME = 'experience'
);

-- Se não existir, adicionar coluna 'experience' (EXP atual do jogador)
SET @sql = IF(@col_exists = 0,
    'ALTER TABLE players ADD COLUMN experience INT UNSIGNED DEFAULT 0 COMMENT ''EXP atual do jogador'' AFTER level',
    'SELECT ''Coluna experience já existe'' AS message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Adicionar coluna 'next_level_exp' (EXP necessário para próximo nível - calculado dinamicamente)
-- Esta coluna pode ser calculada via JOIN com player_levels, mas adicionamos para cache
SET @col_exists2 = (
    SELECT COUNT(*) 
    FROM INFORMATION_SCHEMA.COLUMNS 
    WHERE TABLE_SCHEMA = 'umbra_eternum' 
    AND TABLE_NAME = 'players' 
    AND COLUMN_NAME = 'next_level_exp'
);

SET @sql2 = IF(@col_exists2 = 0,
    'ALTER TABLE players ADD COLUMN next_level_exp INT UNSIGNED DEFAULT 0 COMMENT ''EXP necessário para próximo nível (cache)'' AFTER experience',
    'SELECT ''Coluna next_level_exp já existe'' AS message'
);
PREPARE stmt2 FROM @sql2;
EXECUTE stmt2;
DEALLOCATE PREPARE stmt2;

-- Verificar estrutura final
DESCRIBE players;

-- Atualizar next_level_exp para todos os jogadores baseado em seus níveis
UPDATE players p
INNER JOIN player_levels pl ON p.level = pl.level_number
SET p.next_level_exp = pl.exp_for_next_level;

SELECT '✅ Colunas de EXP adicionadas/atualizadas com sucesso!' AS message;

