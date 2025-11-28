-- Script para adicionar campos de stats faltantes na tabela players
-- Execute este script no MySQL Workbench ou phpMyAdmin
-- 
-- IMPORTANTE: Execute cada ALTER TABLE separadamente para evitar erros se a coluna já existir

USE umbra_eternum;

-- Verificar se as colunas já existem antes de adicionar
-- Se alguma coluna já existir, você receberá um erro, mas pode ignorar

-- Adicionar coluna agility (se não existir)
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
    'SELECT "Coluna agility já existe" AS message;',
    CONCAT('ALTER TABLE ', @tablename, ' ADD COLUMN ', @columnname, ' INT UNSIGNED DEFAULT 10 AFTER vitality;')
));
PREPARE alterIfNotExists FROM @preparedStatement;
EXECUTE alterIfNotExists;
DEALLOCATE PREPARE alterIfNotExists;

-- Adicionar coluna constitution (se não existir)
SET @columnname = 'constitution';
SET @preparedStatement = (SELECT IF(
    (
        SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
        AND TABLE_NAME = @tablename
        AND COLUMN_NAME = @columnname
    ) > 0,
    'SELECT "Coluna constitution já existe" AS message;',
    CONCAT('ALTER TABLE ', @tablename, ' ADD COLUMN ', @columnname, ' INT UNSIGNED DEFAULT 10 AFTER agility;')
));
PREPARE alterIfNotExists FROM @preparedStatement;
EXECUTE alterIfNotExists;
DEALLOCATE PREPARE alterIfNotExists;

-- Adicionar coluna luck (se não existir)
SET @columnname = 'luck';
SET @preparedStatement = (SELECT IF(
    (
        SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = @dbname
        AND TABLE_NAME = @tablename
        AND COLUMN_NAME = @columnname
    ) > 0,
    'SELECT "Coluna luck já existe" AS message;',
    CONCAT('ALTER TABLE ', @tablename, ' ADD COLUMN ', @columnname, ' INT UNSIGNED DEFAULT 10 AFTER constitution;')
));
PREPARE alterIfNotExists FROM @preparedStatement;
EXECUTE alterIfNotExists;
DEALLOCATE PREPARE alterIfNotExists;

-- Atualizar valores existentes para o padrão (10) se forem NULL
UPDATE players 
SET agility = 10 
WHERE agility IS NULL;

UPDATE players 
SET constitution = 10 
WHERE constitution IS NULL;

UPDATE players 
SET luck = 10 
WHERE luck IS NULL;

-- Verificar se as colunas foram adicionadas
SELECT 
    COLUMN_NAME, 
    DATA_TYPE, 
    COLUMN_DEFAULT, 
    IS_NULLABLE
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = 'umbra_eternum'
  AND TABLE_NAME = 'players'
  AND COLUMN_NAME IN ('agility', 'constitution', 'luck')
ORDER BY ORDINAL_POSITION;

