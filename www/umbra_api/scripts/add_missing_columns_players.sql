-- Script para adicionar todas as colunas necessárias na tabela players
-- Execute este script no MySQL Workbench ou phpMyAdmin

USE umbra_eternum;

-- Adicionar class_id se não existir
SET @column_exists = (
    SELECT COUNT(*) 
    FROM INFORMATION_SCHEMA.COLUMNS 
    WHERE TABLE_SCHEMA = 'umbra_eternum' 
    AND TABLE_NAME = 'players' 
    AND COLUMN_NAME = 'class_id'
);

SET @sql = IF(@column_exists = 0,
    'ALTER TABLE players ADD COLUMN class_id BIGINT UNSIGNED DEFAULT NULL AFTER vitality',
    'SELECT "Coluna class_id já existe" AS message'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Adicionar hair se não existir
SET @column_exists = (
    SELECT COUNT(*) 
    FROM INFORMATION_SCHEMA.COLUMNS 
    WHERE TABLE_SCHEMA = 'umbra_eternum' 
    AND TABLE_NAME = 'players' 
    AND COLUMN_NAME = 'hair'
);

SET @sql = IF(@column_exists = 0,
    'ALTER TABLE players ADD COLUMN hair INT UNSIGNED DEFAULT 0 AFTER class_id',
    'SELECT "Coluna hair já existe" AS message'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Adicionar head se não existir
SET @column_exists = (
    SELECT COUNT(*) 
    FROM INFORMATION_SCHEMA.COLUMNS 
    WHERE TABLE_SCHEMA = 'umbra_eternum' 
    AND TABLE_NAME = 'players' 
    AND COLUMN_NAME = 'head'
);

SET @sql = IF(@column_exists = 0,
    'ALTER TABLE players ADD COLUMN head INT UNSIGNED DEFAULT 0 AFTER hair',
    'SELECT "Coluna head já existe" AS message'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Adicionar luck se não existir
SET @column_exists = (
    SELECT COUNT(*) 
    FROM INFORMATION_SCHEMA.COLUMNS 
    WHERE TABLE_SCHEMA = 'umbra_eternum' 
    AND TABLE_NAME = 'players' 
    AND COLUMN_NAME = 'luck'
);

SET @sql = IF(@column_exists = 0,
    'ALTER TABLE players ADD COLUMN luck INT UNSIGNED DEFAULT 10 AFTER vitality',
    'SELECT "Coluna luck já existe" AS message'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Atualizar valores existentes
UPDATE players SET hair = 0 WHERE hair IS NULL;
UPDATE players SET head = 0 WHERE head IS NULL;
UPDATE players SET luck = 10 WHERE luck IS NULL OR luck = 0;

-- Verificar colunas adicionadas
SELECT 
    COLUMN_NAME, 
    DATA_TYPE, 
    COLUMN_DEFAULT, 
    IS_NULLABLE
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = 'umbra_eternum'
  AND TABLE_NAME = 'players'
  AND COLUMN_NAME IN ('class_id', 'hair', 'head', 'luck')
ORDER BY ORDINAL_POSITION;

