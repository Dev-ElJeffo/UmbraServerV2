-- Admin audit central (UmbraManager + PHP admin)
CREATE TABLE IF NOT EXISTS admin_audit (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  operator_account_id BIGINT UNSIGNED NULL,
  operator_name VARCHAR(64) NOT NULL DEFAULT '',
  action VARCHAR(64) NOT NULL,
  target_type VARCHAR(32) NULL,
  target_id BIGINT NULL,
  details TEXT NULL,
  ip_address VARCHAR(45) NULL,
  payload_json JSON NULL,
  PRIMARY KEY (id),
  INDEX idx_admin_audit_created (created_at),
  INDEX idx_admin_audit_action (action),
  INDEX idx_admin_audit_operator (operator_name),
  INDEX idx_admin_audit_target (target_type, target_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
