-- Script para adicionar coluna gold na tabela players
-- Gold é a moeda do jogo que pode ser transferida entre inventário e armazém
-- Compatível com MySQL 5.7+

-- Procedimento seguro para adicionar coluna se não existir
DELIMITER //

DROP PROCEDURE IF EXISTS AddGoldColumns //

CREATE PROCEDURE AddGoldColumns()
BEGIN
    -- Verificar e adicionar coluna gold em players
    IF NOT EXISTS (
        SELECT * FROM INFORMATION_SCHEMA.COLUMNS 
        WHERE TABLE_SCHEMA = DATABASE() 
        AND TABLE_NAME = 'players' 
        AND COLUMN_NAME = 'gold'
    ) THEN
        ALTER TABLE players ADD COLUMN gold BIGINT UNSIGNED NOT NULL DEFAULT 0 
        COMMENT 'Quantidade de gold que o jogador possui';
        SELECT 'Coluna players.gold criada com sucesso!' AS resultado;
    ELSE
        SELECT 'Coluna players.gold já existe.' AS resultado;
    END IF;

    -- Verificar e adicionar coluna stored_gold em accounts
    IF NOT EXISTS (
        SELECT * FROM INFORMATION_SCHEMA.COLUMNS 
        WHERE TABLE_SCHEMA = DATABASE() 
        AND TABLE_NAME = 'accounts' 
        AND COLUMN_NAME = 'stored_gold'
    ) THEN
        ALTER TABLE accounts ADD COLUMN stored_gold BIGINT UNSIGNED NOT NULL DEFAULT 0 
        COMMENT 'Quantidade de gold armazenada no banco (compartilhado entre personagens da conta)';
        SELECT 'Coluna accounts.stored_gold criada com sucesso!' AS resultado;
    ELSE
        SELECT 'Coluna accounts.stored_gold já existe.' AS resultado;
    END IF;
END //

DELIMITER ;

-- Executar o procedimento
CALL AddGoldColumns();

-- Limpar (opcional)
DROP PROCEDURE IF EXISTS AddGoldColumns;

-- Verificar se as colunas foram criadas
SELECT 
    'players.gold' as coluna,
    COLUMN_TYPE as tipo,
    COLUMN_DEFAULT as padrao,
    IS_NULLABLE as nullable
FROM INFORMATION_SCHEMA.COLUMNS 
WHERE TABLE_SCHEMA = DATABASE() 
AND TABLE_NAME = 'players' 
AND COLUMN_NAME = 'gold'
UNION ALL
SELECT 
    'accounts.stored_gold' as coluna,
    COLUMN_TYPE as tipo,
    COLUMN_DEFAULT as padrao,
    IS_NULLABLE as nullable
FROM INFORMATION_SCHEMA.COLUMNS 
WHERE TABLE_SCHEMA = DATABASE() 
AND TABLE_NAME = 'accounts' 
AND COLUMN_NAME = 'stored_gold';
