-- Role do UmbraManager: super | ops | content
ALTER TABLE accounts
  ADD COLUMN admin_role VARCHAR(16) NOT NULL DEFAULT 'super'
  COMMENT 'UmbraManager role: super|ops|content';

UPDATE accounts SET admin_role = 'super' WHERE isadmin = 1 AND (admin_role IS NULL OR admin_role = '');
