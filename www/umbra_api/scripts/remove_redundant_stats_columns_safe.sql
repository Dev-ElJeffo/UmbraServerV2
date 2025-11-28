-- Script SEGURO para remover colunas de stats redundantes da tabela players
-- Remove: agility (redundante com dexterity) e constitution (redundante com vitality)
-- Este script verifica se as colunas existem antes de removê-las
-- Execute este script no MySQL Workbench ou phpMyAdmin

USE umbra_eternum;

-- Verificar e remover coluna agility (se existir)
SET @dbname = DATABASE();
SET @tablename = 'players';
SET @columnname = 'agility';
SET @preparedStatement = (SELECT IF(
    (
        SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
        AND TABLE_NAME = @tablename
        AND COLUMN_NAME = @columnname
    ) > 0,
    CONCAT('ALTER TABLE ', @tablename, ' DROP COLUMN ', @columnname, ';'),
    'SELECT "Coluna agility não existe, ignorando..." AS message;'
));
PREPARE alterIfExists FROM @preparedStatement;
EXECUTE alterIfExists;
DEALLOCATE PREPARE alterIfExists;

-- Verificar e remover coluna constitution (se existir)
SET @columnname = 'constitution';
SET @preparedStatement = (SELECT IF(
    (
        SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
        AND TABLE_NAME = @tablename
        AND COLUMN_NAME = @columnname
    ) > 0,
    CONCAT('ALTER TABLE ', @tablename, ' DROP COLUMN ', @columnname, ';'),
    'SELECT "Coluna constitution não existe, ignorando..." AS message;'
));
PREPARE alterIfExists FROM @preparedStatement;
EXECUTE alterIfExists;
DEALLOCATE PREPARE alterIfExists;

-- Verificar se as colunas foram removidas
SELECT 
    COLUMN_NAME, 
    DATA_TYPE, 
    COLUMN_DEFAULT
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = 'umbra_eternum'
  AND TABLE_NAME = 'players'
  AND COLUMN_NAME IN ('agility', 'constitution');

-- Deve retornar 0 linhas (colunas não existem mais)

-- Verificar stats finais
SELECT 
    COLUMN_NAME
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = 'umbra_eternum'
  AND TABLE_NAME = 'players'
  AND COLUMN_NAME IN ('strength', 'dexterity', 'intelligence', 'vitality', 'luck')
ORDER BY ORDINAL_POSITION;

-- Deve retornar 5 linhas:
-- strength
-- dexterity
-- intelligence
-- vitality
-- luck

