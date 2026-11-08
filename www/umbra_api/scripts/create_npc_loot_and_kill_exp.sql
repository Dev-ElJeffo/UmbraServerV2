-- Loot droplist por monstro + EXP única no kill + corpses temporários
-- Idempotente o quanto possível (ADD COLUMN / CREATE IF NOT EXISTS).

-- A) EXP por mob
SET @col_exists := (
  SELECT COUNT(*) FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'npc_templates'
    AND COLUMN_NAME = 'kill_exp'
);
SET @sql := IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN kill_exp BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT ''EXP unica concedida ao killer na morte''',
  'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

UPDATE npc_templates SET kill_exp = 50 WHERE npc_template_id = 10 AND kill_exp = 0;

-- B) Droplist editável
-- npc_template_id deve ser BIGINT UNSIGNED (mesmo tipo de npc_templates.npc_template_id)
CREATE TABLE IF NOT EXISTS npc_loot_entries (
  loot_entry_id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  npc_template_id BIGINT UNSIGNED NOT NULL,
  entry_kind TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=item 1=gold',
  item_template_id INT UNSIGNED NULL,
  drop_chance DECIMAL(8,4) NOT NULL,
  min_qty INT UNSIGNED NOT NULL DEFAULT 1,
  max_qty INT UNSIGNED NOT NULL DEFAULT 1,
  enabled TINYINT(1) NOT NULL DEFAULT 1,
  sort_order INT NOT NULL DEFAULT 0,
  INDEX idx_loot_template (npc_template_id),
  CONSTRAINT fk_loot_npc FOREIGN KEY (npc_template_id)
    REFERENCES npc_templates(npc_template_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS loot_corpses (
  corpse_id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  zone_id INT UNSIGNED NOT NULL,
  npc_instance_id INT UNSIGNED NOT NULL,
  npc_template_id BIGINT UNSIGNED NOT NULL,
  killer_player_id INT UNSIGNED NOT NULL,
  pos_x FLOAT NULL,
  pos_y FLOAT NULL,
  pos_z FLOAT NULL,
  created_at DATETIME NOT NULL,
  expires_at DATETIME NOT NULL,
  closed TINYINT(1) NOT NULL DEFAULT 0,
  INDEX idx_corpse_killer (killer_player_id),
  INDEX idx_corpse_expires (expires_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS loot_corpse_items (
  corpse_item_id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  corpse_id BIGINT UNSIGNED NOT NULL,
  slot_index TINYINT UNSIGNED NOT NULL,
  entry_kind TINYINT UNSIGNED NOT NULL DEFAULT 0,
  item_template_id INT UNSIGNED NULL,
  quantity INT UNSIGNED NOT NULL,
  taken TINYINT(1) NOT NULL DEFAULT 0,
  UNIQUE KEY uq_corpse_slot (corpse_id, slot_index),
  CONSTRAINT fk_corpse_items FOREIGN KEY (corpse_id)
    REFERENCES loot_corpses(corpse_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Seed Goblin (template 10): drops garantidos para teste de UI (chance 100%)
DELETE FROM npc_loot_entries WHERE npc_template_id = 10;

INSERT INTO npc_loot_entries
  (npc_template_id, entry_kind, item_template_id, drop_chance, min_qty, max_qty, enabled, sort_order)
VALUES
  (10, 1, NULL, 1.0000, 10, 25, 1, 0);

SET @potion_id := (
  SELECT item_id FROM item_templates
  WHERE item_name = 'Poção de Vida Menor' LIMIT 1
);
SET @mana_id := (
  SELECT item_id FROM item_templates
  WHERE item_name = 'Poção de Mana' LIMIT 1
);
SET @sword_id := (
  SELECT item_id FROM item_templates
  WHERE item_name = 'Espada de Ferro' LIMIT 1
);
SET @gem_id := (
  SELECT item_id FROM item_templates
  WHERE item_name = 'Gema Brilhante' LIMIT 1
);

INSERT INTO npc_loot_entries
  (npc_template_id, entry_kind, item_template_id, drop_chance, min_qty, max_qty, enabled, sort_order)
SELECT 10, 0, @potion_id, 1.0000, 1, 2, 1, 10
WHERE @potion_id IS NOT NULL;

INSERT INTO npc_loot_entries
  (npc_template_id, entry_kind, item_template_id, drop_chance, min_qty, max_qty, enabled, sort_order)
SELECT 10, 0, @mana_id, 1.0000, 1, 1, 1, 20
WHERE @mana_id IS NOT NULL;

INSERT INTO npc_loot_entries
  (npc_template_id, entry_kind, item_template_id, drop_chance, min_qty, max_qty, enabled, sort_order)
SELECT 10, 0, @sword_id, 1.0000, 1, 1, 1, 30
WHERE @sword_id IS NOT NULL;

INSERT INTO npc_loot_entries
  (npc_template_id, entry_kind, item_template_id, drop_chance, min_qty, max_qty, enabled, sort_order)
SELECT 10, 0, @gem_id, 1.0000, 1, 3, 1, 40
WHERE @gem_id IS NOT NULL;
