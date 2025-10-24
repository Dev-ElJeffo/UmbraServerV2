-- Verificar se coluna isadmin existe
USE umbra_eternum;

-- Mostrar estrutura da tabela accounts
DESCRIBE accounts;

-- Se a coluna não existir, adicionar
-- (Descomente as linhas abaixo se necessário)

-- ALTER TABLE accounts 
-- ADD COLUMN isadmin TINYINT(1) NOT NULL DEFAULT 0 
-- AFTER banned;

-- CREATE INDEX idx_isadmin ON accounts(isadmin);

-- UPDATE accounts SET isadmin = 1 WHERE id = 1;

-- Verificar dados
SELECT id, username, email, banned, isadmin FROM accounts;

