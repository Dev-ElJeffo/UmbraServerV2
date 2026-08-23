-- ============================================================================
-- UMBRA ETERNUM - NPCs interativos (diálogo, vendedor, não atacáveis)
-- Compatibilidade: MySQL 5.7+ / 8.0
-- ============================================================================
SET NAMES utf8mb4;

DELIMITER //

DROP PROCEDURE IF EXISTS ApplyNpcInteractiveColumns //
CREATE PROCEDURE ApplyNpcInteractiveColumns()
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'is_attackable'
    ) THEN
        ALTER TABLE npc_templates
            ADD COLUMN is_attackable TINYINT(1) NOT NULL DEFAULT 1 AFTER is_editable;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'interaction_radius'
    ) THEN
        ALTER TABLE npc_templates
            ADD COLUMN interaction_radius FLOAT NOT NULL DEFAULT 300.0 AFTER is_attackable;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'collision_radius'
    ) THEN
        ALTER TABLE npc_templates
            ADD COLUMN collision_radius FLOAT NOT NULL DEFAULT 45.0 AFTER interaction_radius;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'nameplate_radius'
    ) THEN
        ALTER TABLE npc_templates
            ADD COLUMN nameplate_radius FLOAT NOT NULL DEFAULT 2000.0 AFTER collision_radius;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'has_vendor'
    ) THEN
        ALTER TABLE npc_templates
            ADD COLUMN has_vendor TINYINT(1) NOT NULL DEFAULT 0 AFTER nameplate_radius;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'has_quest_dialog'
    ) THEN
        ALTER TABLE npc_templates
            ADD COLUMN has_quest_dialog TINYINT(1) NOT NULL DEFAULT 0 AFTER has_vendor;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'dialog_title'
    ) THEN
        ALTER TABLE npc_templates
            ADD COLUMN dialog_title VARCHAR(120) NULL AFTER has_quest_dialog;
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'dialog_text'
    ) THEN
        ALTER TABLE npc_templates
            ADD COLUMN dialog_text TEXT NULL AFTER dialog_title;
    END IF;
END //

DELIMITER ;

CALL ApplyNpcInteractiveColumns();
DROP PROCEDURE IF EXISTS ApplyNpcInteractiveColumns;

CREATE TABLE IF NOT EXISTS npc_vendors (
  vendor_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  npc_template_id BIGINT UNSIGNED NOT NULL,
  vendor_display_name VARCHAR(100) NULL,
  sell_rate_percent TINYINT UNSIGNED NOT NULL DEFAULT 50,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (vendor_id),
  UNIQUE KEY uk_npc_vendor_template (npc_template_id),
  CONSTRAINT fk_npc_vendors_template FOREIGN KEY (npc_template_id)
    REFERENCES npc_templates(npc_template_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS npc_vendor_stock (
  stock_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  vendor_id BIGINT UNSIGNED NOT NULL,
  item_template_id INT NOT NULL,
  buy_price_gold INT UNSIGNED NOT NULL,
  stock_qty INT NOT NULL DEFAULT -1,
  max_buy_per_tx INT UNSIGNED NOT NULL DEFAULT 99,
  sort_order INT NOT NULL DEFAULT 0,
  is_active TINYINT(1) NOT NULL DEFAULT 1,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (stock_id),
  UNIQUE KEY uk_vendor_item (vendor_id, item_template_id),
  KEY idx_vendor_stock_vendor (vendor_id),
  CONSTRAINT fk_vendor_stock_vendor FOREIGN KEY (vendor_id)
    REFERENCES npc_vendors(vendor_id) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT fk_vendor_stock_item FOREIGN KEY (item_template_id)
    REFERENCES item_templates(item_id) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------------------------
-- Seed: vendedor de exemplo (npc_merchant_01) — idempotente por npc_name
-- --------------------------------------------------------------------------
INSERT INTO npc_templates (
  npc_name, level, max_health, max_mana,
  strength, dexterity, vitality, intelligence, luck,
  physical_attack, magic_attack, physical_defense, magic_defense,
  accuracy, dodge, critical, critical_resistance,
  double_attack_rate, double_attack_resistance,
  skeletal_mesh_path, anim_blueprint_path, is_editable,
  is_attackable, interaction_radius, has_vendor, has_quest_dialog,
  dialog_title, dialog_text
)
SELECT
  'npc_merchant_01', 1, 1000, 100,
  5, 5, 5, 5, 5,
  0, 0, 10, 10,
  0, 0, 0, 0,
  0, 0,
  NULL, NULL, 1,
  0, 300.0, 1, 1,
  'Mercador do Vilarejo',
  'Bem-vindo, aventureiro! Posso vender suprimentos ou ouvir o que você precisa. (Sistema de quests em breve.)'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM npc_templates WHERE npc_name = 'npc_merchant_01' LIMIT 1);

INSERT INTO npc_vendors (npc_template_id, vendor_display_name, sell_rate_percent)
SELECT nt.npc_template_id, 'Mercador do Vilarejo', 50
FROM npc_templates nt
WHERE nt.npc_name = 'npc_merchant_01'
ON DUPLICATE KEY UPDATE
  vendor_display_name = VALUES(vendor_display_name),
  sell_rate_percent = VALUES(sell_rate_percent);

INSERT INTO npc_instances (
  npc_template_id, zone_id, pos_x, pos_y, pos_z, yaw,
  current_health, current_mana, is_dead
)
SELECT nt.npc_template_id, 1, 500.0, 0.0, 200.0, 90.0,
       nt.max_health, nt.max_mana, 0
FROM npc_templates nt
WHERE nt.npc_name = 'npc_merchant_01'
  AND NOT EXISTS (
    SELECT 1 FROM npc_instances ni
    INNER JOIN npc_templates nt2 ON nt2.npc_template_id = ni.npc_template_id
    WHERE nt2.npc_name = 'npc_merchant_01' AND ni.zone_id = 1 AND ni.is_dead = 0
  )
LIMIT 1;

INSERT INTO npc_vendor_stock (vendor_id, item_template_id, buy_price_gold, stock_qty, max_buy_per_tx, sort_order, is_active)
SELECT nv.vendor_id, it.item_id,
       GREATEST(10, COALESCE(it.value, 10)),
       CASE WHEN it.item_type = 'consumable' THEN 99 ELSE 10 END,
       99,
       (@rownum := @rownum + 1) - 1,
       1
FROM npc_vendors nv
INNER JOIN npc_templates nt ON nt.npc_template_id = nv.npc_template_id
CROSS JOIN (
  SELECT item_id, value, item_type
  FROM item_templates
  WHERE tradeable = 1
  ORDER BY item_id ASC
  LIMIT 5
) it
CROSS JOIN (SELECT @rownum := 0) r
WHERE nt.npc_name = 'npc_merchant_01'
ON DUPLICATE KEY UPDATE
  buy_price_gold = VALUES(buy_price_gold),
  stock_qty = VALUES(stock_qty),
  is_active = VALUES(is_active);
