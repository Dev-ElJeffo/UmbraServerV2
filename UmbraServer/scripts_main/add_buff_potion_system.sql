-- Sistema de poções de buff (buff_potion)
-- Tabela de buffs temporários vindos de itens (separada de active_buffs de skills)

CREATE TABLE IF NOT EXISTS player_item_buffs (
    player_id BIGINT UNSIGNED NOT NULL,
    buff_key VARCHAR(64) NOT NULL COMMENT 'Ex: strength_buff, attack_buff',
    item_template_id INT NOT NULL,
    item_subtype VARCHAR(64) NOT NULL DEFAULT 'buff_potion',
    bonus_value INT NOT NULL DEFAULT 0,
    duration_ms INT UNSIGNED NOT NULL,
    started_at_ms BIGINT UNSIGNED NOT NULL,
    expires_at_ms BIGINT UNSIGNED NOT NULL,
    updated_at TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (player_id, buff_key),
    CONSTRAINT fk_pib_player FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT fk_pib_item_template FOREIGN KEY (item_template_id) REFERENCES item_templates(item_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

SET @idx_exists = (
    SELECT COUNT(*) FROM INFORMATION_SCHEMA.STATISTICS
    WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'player_item_buffs' AND INDEX_NAME = 'idx_pib_expires'
);
SET @sql = IF(@idx_exists = 0,
    'CREATE INDEX idx_pib_expires ON player_item_buffs (expires_at_ms)',
    'SELECT ''idx_pib_expires já existe'' AS info');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
