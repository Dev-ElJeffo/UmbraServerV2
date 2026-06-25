-- Sistema de uso de itens consumíveis (cooldown por subtype + skillbar com item_template_id)
-- Idempotente: pode rodar mais de uma vez.

USE umbra_eternum;

-- Cooldown configurável por template (agrupado por item_subtype na lógica PHP)
SET @col_exists = (
    SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'item_templates' AND COLUMN_NAME = 'use_cooldown_ms'
);
SET @sql = IF(@col_exists = 0,
    'ALTER TABLE item_templates ADD COLUMN use_cooldown_ms INT UNSIGNED NOT NULL DEFAULT 5000 COMMENT ''Cooldown em ms ao usar (por subtype)'' AFTER stats_json',
    'SELECT ''use_cooldown_ms já existe'' AS info');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Cooldown ativo por jogador + subtype
CREATE TABLE IF NOT EXISTS player_consumable_cooldowns (
    player_id BIGINT UNSIGNED NOT NULL,
    item_subtype VARCHAR(64) NOT NULL,
    expires_at_ms BIGINT UNSIGNED NOT NULL,
    updated_at TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (player_id, item_subtype),
    CONSTRAINT fk_pcc_player FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Skillbar: slot pode ter skill OU consumível (item_template_id)
SET @col_exists = (
    SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'player_skillbar' AND COLUMN_NAME = 'item_template_id'
);
SET @sql = IF(@col_exists = 0,
    'ALTER TABLE player_skillbar ADD COLUMN item_template_id INT NULL DEFAULT NULL COMMENT ''Consumível na barra (mutuamente exclusivo com skill_id)'' AFTER skill_id',
    'SELECT ''item_template_id já existe'' AS info');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- item_templates.item_id é INT (signed); INT UNSIGNED quebra FK (erro 3780)
SET @col_exists = (
    SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'player_skillbar' AND COLUMN_NAME = 'item_template_id'
);
SET @sql = IF(@col_exists > 0,
    'ALTER TABLE player_skillbar MODIFY COLUMN item_template_id INT NULL DEFAULT NULL COMMENT ''Consumível na barra (mutuamente exclusivo com skill_id)''',
    'SELECT ''item_template_id ausente'' AS info');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @fk_exists = (
    SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLE_CONSTRAINTS
    WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'player_skillbar' AND CONSTRAINT_NAME = 'fk_skillbar_item_template'
);
SET @sql = IF(@fk_exists = 0,
    'ALTER TABLE player_skillbar ADD CONSTRAINT fk_skillbar_item_template FOREIGN KEY (item_template_id) REFERENCES item_templates(item_id) ON DELETE SET NULL ON UPDATE CASCADE',
    'SELECT ''fk_skillbar_item_template já existe'' AS info');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Poção de Vida Menor (id 7) — cooldown 5s por subtype health_potion
UPDATE item_templates SET use_cooldown_ms = 5000 WHERE item_id = 7 AND item_subtype = 'health_potion';
