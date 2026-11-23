-- =============================================================================
-- Companheiros etapa 2: elemental que usa consumíveis por regras do jogador.
--
-- companion_templates.max_rules      quantas regras o elemental aceita (1-4)
-- companion_templates.auto_check_ms  intervalo de checagem das regras na zone (ms)
-- player_companions.auto_rules_json  regras do jogador: [{"trigger":"hp_below","threshold":40,"item_template_id":7}, ...]
--                                    trigger: hp_below | mp_below | buff_missing (threshold ignorado)
-- Seed: elemental "Espírito da Fonte" (ELEM_SPRING, 2 regras, 500 ms) + selo.
--
-- Idempotente (INFORMATION_SCHEMA + PREPARE/EXECUTE; seeds com WHERE NOT EXISTS).
-- Depois: Manager > Companheiros > Recarregar no Zone (ou reiniciar a zone).
-- =============================================================================

SET NAMES utf8mb4;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'companion_templates' AND COLUMN_NAME = 'max_rules'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE companion_templates ADD COLUMN max_rules TINYINT UNSIGNED NOT NULL DEFAULT 2 COMMENT ''Elemental: regras aceitas (1-4)'' AFTER loot_delay_ms',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'companion_templates' AND COLUMN_NAME = 'auto_check_ms'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE companion_templates ADD COLUMN auto_check_ms INT UNSIGNED NOT NULL DEFAULT 500 COMMENT ''Elemental: intervalo de checagem das regras (ms, 250-5000)'' AFTER max_rules',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'player_companions' AND COLUMN_NAME = 'auto_rules_json'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE player_companions ADD COLUMN auto_rules_json JSON NULL COMMENT ''Elemental: regras de uso automático (gravadas pela zone, opcode 129)'' AFTER loot_gold',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- Seed: elemental de teste (sem mesh = esfera; trocar os paths no Manager).
INSERT INTO companion_templates (companion_key, name, description, companion_type, hover_height, scale,
                                 follow_distance, max_rules, auto_check_ms)
SELECT 'ELEM_SPRING', 'Espírito da Fonte', 'Usa suas poções automaticamente seguindo as regras que você definir.',
       'elemental', 120, 0.8, 120, 2, 500
WHERE NOT EXISTS (SELECT 1 FROM companion_templates WHERE companion_key = 'ELEM_SPRING');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, item_category,
                            max_stack_size, required_level, stats_json, use_cooldown_ms, rarity, value, tradeable)
SELECT 'Selo do Espírito da Fonte', 'Use para obter o elemental Espírito da Fonte.', 'consumable', 'companion',
       'consumable', 1, 1, JSON_OBJECT('companion_template_id', ct.companion_id), 0, 'rare', 800, 1
  FROM companion_templates ct
 WHERE ct.companion_key = 'ELEM_SPRING'
   AND NOT EXISTS (SELECT 1 FROM item_templates WHERE item_subtype = 'companion' AND item_name = 'Selo do Espírito da Fonte');

-- Conferência
SELECT companion_id, companion_key, companion_type, max_rules, auto_check_ms FROM companion_templates;
SELECT item_id, item_name, stats_json FROM item_templates WHERE item_subtype = 'companion';
