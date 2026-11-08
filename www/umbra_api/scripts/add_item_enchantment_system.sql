-- Encantamento de itens (até 3 slots por instância de equipamento)
-- Idempotente: colunas/tabelas/sementes só são criadas se faltarem.

USE umbra_eternum;

-- player_inventory.enchantments_json
SET @col_exists := (
  SELECT COUNT(*) FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'player_inventory'
    AND COLUMN_NAME = 'enchantments_json'
);
SET @sql := IF(@col_exists = 0,
  'ALTER TABLE player_inventory ADD COLUMN enchantments_json JSON NULL COMMENT ''Afixos da instância: [{slot,stat_key,value}]''',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- mail_attachments.enchantments_json
SET @col_exists := (
  SELECT COUNT(*) FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'mail_attachments'
    AND COLUMN_NAME = 'enchantments_json'
);
SET @sql := IF(@col_exists = 0,
  'ALTER TABLE mail_attachments ADD COLUMN enchantments_json JSON NULL COMMENT ''Cópia dos afixos do item anexado''',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

CREATE TABLE IF NOT EXISTS enchant_stat_weights (
  stat_key VARCHAR(64) NOT NULL,
  display_name VARCHAR(80) NOT NULL DEFAULT '',
  weight INT NOT NULL DEFAULT 1,
  value_min INT NOT NULL DEFAULT 1,
  value_max INT NOT NULL DEFAULT 1,
  tier TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT '1 atributos, 2 atk/def, 3 secundarios, 4 CC',
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (stat_key)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS enchant_slot_chances (
  slot_count TINYINT UNSIGNED NOT NULL COMMENT '0..3 afixos no spawn',
  weight INT NOT NULL DEFAULT 1,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (slot_count)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO enchant_slot_chances (slot_count, weight) VALUES
  (0, 40), (1, 35), (2, 18), (3, 7)
ON DUPLICATE KEY UPDATE slot_count = slot_count;

INSERT INTO enchant_stat_weights (stat_key, display_name, weight, value_min, value_max, tier) VALUES
  ('strength', 'Força', 100, 1, 8, 1),
  ('dexterity', 'Destreza', 100, 1, 8, 1),
  ('vitality', 'Vitalidade', 100, 1, 8, 1),
  ('intelligence', 'Inteligência', 100, 1, 8, 1),
  ('luck', 'Sorte', 100, 1, 8, 1),
  ('attack', 'Ataque físico', 50, 1, 12, 2),
  ('magic_attack', 'Ataque mágico', 50, 1, 12, 2),
  ('defense', 'Defesa física', 50, 1, 12, 2),
  ('magic_defense', 'Defesa mágica', 50, 1, 12, 2),
  ('critical', 'Crítico', 25, 1, 5, 3),
  ('double_attack_rate', 'Ataque duplo', 25, 1, 5, 3),
  ('accuracy', 'Precisão', 25, 1, 5, 3),
  ('dodge', 'Esquiva', 25, 1, 5, 3),
  ('movement', 'Movimento', 25, 1, 5, 3),
  ('stun_chance', 'Chance de atordoar', 10, 1, 5, 4),
  ('silence_chance', 'Chance de silenciar', 10, 1, 5, 4),
  ('root_chance', 'Chance de enraizar', 10, 1, 5, 4),
  ('slow_chance', 'Chance de lentidão', 10, 1, 5, 4),
  ('stun_resist', 'Resist. atordoar', 10, 1, 5, 4),
  ('silence_resist', 'Resist. silenciar', 10, 1, 5, 4),
  ('root_resist', 'Resist. enraizar', 10, 1, 5, 4),
  ('slow_resist', 'Resist. lentidão', 10, 1, 5, 4)
ON DUPLICATE KEY UPDATE stat_key = stat_key;

INSERT INTO item_templates (
  item_name, item_description, item_type, item_subtype, equipment_slot,
  max_stack_size, icon_path, rarity, value, weight, item_category, tradeable, can_be_refined
)
SELECT
  'Cristal de Encantamento',
  'Preenche um slot vazio de encanto no equipamento. Sempre aplica um afixo aleatório.',
  'material', 'enchant_crystal', 'none',
  999, '/Game/UI/Icons/Items/ICO_EnchantCrystal', 'uncommon', 150, 0.1, 'upgrade', TRUE, FALSE
WHERE NOT EXISTS (
  SELECT 1 FROM item_templates WHERE item_name = 'Cristal de Encantamento' LIMIT 1
);

INSERT INTO item_templates (
  item_name, item_description, item_type, item_subtype, equipment_slot,
  max_stack_size, icon_path, rarity, value, weight, item_category, tradeable, can_be_refined
)
SELECT
  'Extrator de Encanto',
  'Remove um afixo escolhido do equipamento, liberando o slot para um novo cristal.',
  'material', 'enchant_extractor', 'none',
  999, '/Game/UI/Icons/Items/ICO_EnchantExtractor', 'rare', 200, 0.1, 'upgrade', TRUE, FALSE
WHERE NOT EXISTS (
  SELECT 1 FROM item_templates WHERE item_name = 'Extrator de Encanto' LIMIT 1
);
