-- Script para adicionar coluna isadmin na tabela accounts
-- Execute no MySQL Workbench ou via terminal

USE umbra_eternum;

-- Adicionar coluna isadmin (0 = usuário normal, 1 = admin)
ALTER TABLE accounts 
ADD COLUMN isadmin TINYINT(1) NOT NULL DEFAULT 0 
AFTER banned;

-- Criar índice para consultas rápidas
CREATE INDEX idx_isadmin ON accounts(isadmin);

-- Tornar primeira conta admin (para teste)
UPDATE accounts 
SET isadmin = 1 
WHERE id = 1;

-- Ver resultado
SELECT id, username, email, banned, isadmin 
FROM accounts 
ORDER BY id;

-- Informações
SELECT 
    COUNT(*) as total_accounts,
    SUM(CASE WHEN isadmin = 1 THEN 1 ELSE 0 END) as admin_accounts,
    SUM(CASE WHEN isadmin = 0 THEN 1 ELSE 0 END) as regular_accounts,
    SUM(CASE WHEN banned = 1 THEN 1 ELSE 0 END) as banned_accounts
FROM accounts;

