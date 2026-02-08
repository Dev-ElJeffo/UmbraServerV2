-- Adiciona colunas de gold na tabela trade_sessions
-- Para troca de gold entre jogadores durante a sessão de trade
-- Compatível com MySQL 5.7+

DELIMITER //

DROP PROCEDURE IF EXISTS AddTradeGoldColumns //

CREATE PROCEDURE AddTradeGoldColumns()
BEGIN
    IF NOT EXISTS (
        SELECT * FROM INFORMATION_SCHEMA.COLUMNS 
        WHERE TABLE_SCHEMA = DATABASE() 
        AND TABLE_NAME = 'trade_sessions' 
        AND COLUMN_NAME = 'player1_gold_offer'
    ) THEN
        ALTER TABLE trade_sessions ADD COLUMN player1_gold_offer BIGINT UNSIGNED NOT NULL DEFAULT 0 
        COMMENT 'Gold que player1 oferece na troca';
        SELECT 'Coluna player1_gold_offer criada!' AS resultado;
    ELSE
        SELECT 'Coluna player1_gold_offer já existe.' AS resultado;
    END IF;

    IF NOT EXISTS (
        SELECT * FROM INFORMATION_SCHEMA.COLUMNS 
        WHERE TABLE_SCHEMA = DATABASE() 
        AND TABLE_NAME = 'trade_sessions' 
        AND COLUMN_NAME = 'player2_gold_offer'
    ) THEN
        ALTER TABLE trade_sessions ADD COLUMN player2_gold_offer BIGINT UNSIGNED NOT NULL DEFAULT 0 
        COMMENT 'Gold que player2 oferece na troca';
        SELECT 'Coluna player2_gold_offer criada!' AS resultado;
    ELSE
        SELECT 'Coluna player2_gold_offer já existe.' AS resultado;
    END IF;
END //

DELIMITER ;

CALL AddTradeGoldColumns();
DROP PROCEDURE IF EXISTS AddTradeGoldColumns;
