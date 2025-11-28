-- Script para remover colunas de stats redundantes da tabela players
-- Remove: agility (redundante com dexterity) e constitution (redundante com vitality)
-- Execute este script no MySQL Workbench ou phpMyAdmin
-- 
-- NOTA: Se alguma coluna não existir, você receberá um erro, mas pode ignorar

USE umbra_eternum;

-- Remover coluna agility (redundante com dexterity)
ALTER TABLE players
DROP COLUMN agility;

-- Remover coluna constitution (redundante com vitality)
ALTER TABLE players
DROP COLUMN constitution;

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

