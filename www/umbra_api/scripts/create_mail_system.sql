-- Sistema de correio in-game (até 5 anexos estilo inventory slot)
CREATE TABLE IF NOT EXISTS mail_messages (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  sender_player_id BIGINT UNSIGNED NULL COMMENT 'NULL = sistema/admin',
  recipient_player_id BIGINT UNSIGNED NOT NULL,
  subject VARCHAR(128) NOT NULL DEFAULT '',
  body TEXT NOT NULL,
  is_read TINYINT(1) NOT NULL DEFAULT 0,
  attachment_count TINYINT UNSIGNED NOT NULL DEFAULT 0,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  expires_at TIMESTAMP NULL DEFAULT NULL,
  deleted_by_recipient TINYINT(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (id),
  INDEX idx_mail_recipient (recipient_player_id, deleted_by_recipient, created_at),
  INDEX idx_mail_sender (sender_player_id),
  CONSTRAINT fk_mail_recipient FOREIGN KEY (recipient_player_id) REFERENCES players(id) ON DELETE CASCADE,
  CONSTRAINT fk_mail_sender FOREIGN KEY (sender_player_id) REFERENCES players(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS mail_attachments (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  mail_id BIGINT UNSIGNED NOT NULL,
  slot_index TINYINT UNSIGNED NOT NULL COMMENT '0-4',
  item_template_id INT NOT NULL,
  quantity INT NOT NULL DEFAULT 1,
  refinement_level TINYINT NOT NULL DEFAULT 0,
  durability FLOAT NOT NULL DEFAULT 100.0,
  bonus_stats_json JSON NULL,
  claimed TINYINT(1) NOT NULL DEFAULT 0,
  claimed_at TIMESTAMP NULL DEFAULT NULL,
  PRIMARY KEY (id),
  UNIQUE KEY uq_mail_slot (mail_id, slot_index),
  INDEX idx_mail_attach_mail (mail_id, claimed),
  CONSTRAINT fk_mail_attach_mail FOREIGN KEY (mail_id) REFERENCES mail_messages(id) ON DELETE CASCADE,
  CONSTRAINT chk_mail_slot CHECK (slot_index <= 4),
  CONSTRAINT chk_mail_qty CHECK (quantity >= 1)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
