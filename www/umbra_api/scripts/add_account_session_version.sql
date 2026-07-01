-- Sessão única por conta: incrementado a cada login HTTP.
-- JWT carrega session_version; zone valida contra accounts.session_version.
--
-- Compatível com MySQL 5.7+ (sem ADD COLUMN IF NOT EXISTS, suportado só em 8.0.29+).

SET @dbname = DATABASE();
SET @preparedStatement = (
  SELECT IF(
    EXISTS(
      SELECT 1
      FROM INFORMATION_SCHEMA.COLUMNS
      WHERE TABLE_SCHEMA = @dbname
        AND TABLE_NAME = 'accounts'
        AND COLUMN_NAME = 'session_version'
    ),
    'SELECT ''session_version já existe em accounts'' AS info',
    'ALTER TABLE accounts ADD COLUMN session_version INT UNSIGNED NOT NULL DEFAULT 0'
  )
);
PREPARE stmt FROM @preparedStatement;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
