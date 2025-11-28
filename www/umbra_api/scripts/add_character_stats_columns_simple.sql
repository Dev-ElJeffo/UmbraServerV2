-- Script SIMPLES para adicionar campos de stats faltantes na tabela players
-- Execute este script no MySQL Workbench ou phpMyAdmin
-- 
-- NOTA: Se alguma coluna já existir, você receberá um erro, mas pode ignorar

USE umbra_eternum;

-- Adicionar coluna agility
ALTER TABLE players
ADD COLUMN agility INT UNSIGNED DEFAULT 10 AFTER vitality;

-- Adicionar coluna constitution
ALTER TABLE players
ADD COLUMN constitution INT UNSIGNED DEFAULT 10 AFTER agility;

-- Adicionar coluna luck
ALTER TABLE players
ADD COLUMN luck INT UNSIGNED DEFAULT 10 AFTER constitution;

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

