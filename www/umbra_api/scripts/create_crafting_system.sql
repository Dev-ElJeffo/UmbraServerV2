-- ============================================
-- UMBRA ETERNUM - SISTEMA DE CRAFTING
-- ============================================
-- Execute após setup_inventory_system.sql (item_templates / player_inventory).
-- Categorias: material | consumable | equipment
-- - material/consumable + recipe_item: aprender 1x (unlock permanente)
-- - equipment + recipe_item: consome 1 scroll por craft (sem unlock)
-- - recipe_item NULL: receita padrão (sempre listada)

USE umbra_eternum;

CREATE TABLE IF NOT EXISTS craft_recipes (
    recipe_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    recipe_key VARCHAR(64) NOT NULL,
    display_name VARCHAR(128) NOT NULL,
    craft_category ENUM('material', 'consumable', 'equipment') NOT NULL,
    result_item_template_id INT NOT NULL,
    result_quantity INT NOT NULL DEFAULT 1,
    gold_cost INT NOT NULL DEFAULT 0,
    min_level INT NOT NULL DEFAULT 1,
    recipe_item_template_id INT NULL COMMENT 'NULL = padrão; setado = exige item-receita',
    allowed_class_ids JSON NULL COMMENT 'NULL/[] = todas as classes',
    is_active TINYINT(1) NOT NULL DEFAULT 1,
    sort_order INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (recipe_id),
    UNIQUE KEY uq_craft_recipe_key (recipe_key),
    KEY idx_craft_category (craft_category),
    KEY idx_craft_result (result_item_template_id),
    KEY idx_craft_recipe_item (recipe_item_template_id),
    CONSTRAINT fk_craft_result_item
        FOREIGN KEY (result_item_template_id) REFERENCES item_templates (item_id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    CONSTRAINT fk_craft_recipe_item
        FOREIGN KEY (recipe_item_template_id) REFERENCES item_templates (item_id)
        ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS craft_recipe_ingredients (
    ingredient_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    recipe_id INT UNSIGNED NOT NULL,
    slot_index TINYINT UNSIGNED NOT NULL COMMENT '0-4 (até 5 slots no widget)',
    item_template_id INT NOT NULL,
    quantity INT NOT NULL DEFAULT 1,
    PRIMARY KEY (ingredient_id),
    UNIQUE KEY uq_craft_recipe_slot (recipe_id, slot_index),
    KEY idx_craft_ing_item (item_template_id),
    CONSTRAINT fk_craft_ing_recipe
        FOREIGN KEY (recipe_id) REFERENCES craft_recipes (recipe_id)
        ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT fk_craft_ing_item
        FOREIGN KEY (item_template_id) REFERENCES item_templates (item_id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    CONSTRAINT chk_craft_slot CHECK (slot_index <= 4),
    CONSTRAINT chk_craft_ing_qty CHECK (quantity > 0)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS player_known_craft_recipes (
    player_id BIGINT UNSIGNED NOT NULL,
    recipe_id INT UNSIGNED NOT NULL,
    learned_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (player_id, recipe_id),
    KEY idx_known_recipe (recipe_id),
    CONSTRAINT fk_known_craft_recipe
        FOREIGN KEY (recipe_id) REFERENCES craft_recipes (recipe_id)
        ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================
-- SEED: templates mínimos (por nome, idempotente)
-- ============================================

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Folha Umbra', 'Material básico de crafting.', 'material', 'herb', 99, 'none', 1, '{}', 'common', 2, 0.1
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Folha Umbra');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Pó de Folha', 'Material refinado (craft padrão).', 'material', 'herb', 99, 'none', 1, '{}', 'common', 5, 0.1
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Pó de Folha');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Essência de Folha', 'Material avançado (receita aprendível).', 'material', 'herb', 99, 'none', 1, '{}', 'uncommon', 15, 0.1
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Essência de Folha');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Receita: Essência de Folha', 'Aprenda a craftar Essência de Folha.', 'misc', 'craft_recipe', 20, 'none', 1, '{}', 'uncommon', 50, 0.05
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Receita: Essência de Folha');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Frasco Vazio', 'Frasco para poções.', 'material', 'vial', 99, 'none', 1, '{}', 'common', 3, 0.2
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Frasco Vazio');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Poção Menor de Vida (Craft)', 'Restaura vida (craft padrão).', 'consumable', 'health_potion', 20, 'none', 1,
       JSON_OBJECT('health_restore', 40), 'common', 25, 0.3
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Poção Menor de Vida (Craft)');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Poção Maior de Vida (Craft)', 'Restaura mais vida (receita aprendível).', 'consumable', 'health_potion', 20, 'none', 1,
       JSON_OBJECT('health_restore', 120), 'uncommon', 80, 0.3
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Poção Maior de Vida (Craft)');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Receita: Poção Maior de Vida', 'Aprenda a craftar Poção Maior de Vida.', 'misc', 'craft_recipe', 20, 'none', 1, '{}', 'uncommon', 100, 0.05
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Receita: Poção Maior de Vida');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Lingote Umbra', 'Metal para forja.', 'material', 'ore', 99, 'none', 1, '{}', 'common', 8, 0.5
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Lingote Umbra');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Espada de Aprendiz (Craft)', 'Arma simples (craft padrão).', 'weapon', 'sword', 1, 'main_hand', 1,
       JSON_OBJECT('damage', 8), 'common', 40, 2.0
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Espada de Aprendiz (Craft)');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Lâmina Temperada (Craft)', 'Arma melhor (consome receita a cada craft).', 'weapon', 'sword', 1, 'main_hand', 5,
       JSON_OBJECT('damage', 18), 'uncommon', 120, 2.5
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Lâmina Temperada (Craft)');

INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, max_stack_size, equipment_slot, required_level, stats_json, rarity, value, weight)
SELECT 'Receita: Lâmina Temperada', 'Necessária a cada craft da Lâmina Temperada.', 'misc', 'craft_recipe', 20, 'none', 1, '{}', 'rare', 200, 0.05
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Receita: Lâmina Temperada');

-- Helper IDs via variáveis de sessão
SET @folha := (SELECT item_id FROM item_templates WHERE item_name = 'Folha Umbra' LIMIT 1);
SET @po := (SELECT item_id FROM item_templates WHERE item_name = 'Pó de Folha' LIMIT 1);
SET @essencia := (SELECT item_id FROM item_templates WHERE item_name = 'Essência de Folha' LIMIT 1);
SET @rec_essencia := (SELECT item_id FROM item_templates WHERE item_name = 'Receita: Essência de Folha' LIMIT 1);
SET @frasco := (SELECT item_id FROM item_templates WHERE item_name = 'Frasco Vazio' LIMIT 1);
SET @pocao_menor := (SELECT item_id FROM item_templates WHERE item_name = 'Poção Menor de Vida (Craft)' LIMIT 1);
SET @pocao_maior := (SELECT item_id FROM item_templates WHERE item_name = 'Poção Maior de Vida (Craft)' LIMIT 1);
SET @rec_pocao := (SELECT item_id FROM item_templates WHERE item_name = 'Receita: Poção Maior de Vida' LIMIT 1);
SET @lingote := (SELECT item_id FROM item_templates WHERE item_name = 'Lingote Umbra' LIMIT 1);
SET @espada := (SELECT item_id FROM item_templates WHERE item_name = 'Espada de Aprendiz (Craft)' LIMIT 1);
SET @lamina := (SELECT item_id FROM item_templates WHERE item_name = 'Lâmina Temperada (Craft)' LIMIT 1);
SET @rec_lamina := (SELECT item_id FROM item_templates WHERE item_name = 'Receita: Lâmina Temperada' LIMIT 1);

-- Material padrão: 2 Folha -> 1 Pó
INSERT INTO craft_recipes (recipe_key, display_name, craft_category, result_item_template_id, result_quantity, gold_cost, min_level, recipe_item_template_id, is_active, sort_order)
SELECT 'mat_po_folha', 'Pó de Folha', 'material', @po, 1, 5, 1, NULL, 1, 10
FROM DUAL WHERE @folha IS NOT NULL AND @po IS NOT NULL
  AND NOT EXISTS (SELECT 1 FROM craft_recipes WHERE recipe_key = 'mat_po_folha');

INSERT INTO craft_recipe_ingredients (recipe_id, slot_index, item_template_id, quantity)
SELECT r.recipe_id, 0, @folha, 2
FROM craft_recipes r
WHERE r.recipe_key = 'mat_po_folha'
  AND NOT EXISTS (SELECT 1 FROM craft_recipe_ingredients i WHERE i.recipe_id = r.recipe_id AND i.slot_index = 0);

-- Material aprendível: 3 Folha + 1 Pó -> Essência (precisa aprender)
INSERT INTO craft_recipes (recipe_key, display_name, craft_category, result_item_template_id, result_quantity, gold_cost, min_level, recipe_item_template_id, is_active, sort_order)
SELECT 'mat_essencia_folha', 'Essência de Folha', 'material', @essencia, 1, 20, 1, @rec_essencia, 1, 20
FROM DUAL WHERE @folha IS NOT NULL AND @po IS NOT NULL AND @essencia IS NOT NULL AND @rec_essencia IS NOT NULL
  AND NOT EXISTS (SELECT 1 FROM craft_recipes WHERE recipe_key = 'mat_essencia_folha');

INSERT INTO craft_recipe_ingredients (recipe_id, slot_index, item_template_id, quantity)
SELECT r.recipe_id, 0, @folha, 3 FROM craft_recipes r
WHERE r.recipe_key = 'mat_essencia_folha'
  AND NOT EXISTS (SELECT 1 FROM craft_recipe_ingredients i WHERE i.recipe_id = r.recipe_id AND i.slot_index = 0);

INSERT INTO craft_recipe_ingredients (recipe_id, slot_index, item_template_id, quantity)
SELECT r.recipe_id, 1, @po, 1 FROM craft_recipes r
WHERE r.recipe_key = 'mat_essencia_folha'
  AND NOT EXISTS (SELECT 1 FROM craft_recipe_ingredients i WHERE i.recipe_id = r.recipe_id AND i.slot_index = 1);

-- Consumível padrão: 1 Frasco + 2 Folha -> Poção Menor
INSERT INTO craft_recipes (recipe_key, display_name, craft_category, result_item_template_id, result_quantity, gold_cost, min_level, recipe_item_template_id, is_active, sort_order)
SELECT 'con_pocao_menor', 'Poção Menor de Vida', 'consumable', @pocao_menor, 1, 10, 1, NULL, 1, 10
FROM DUAL WHERE @frasco IS NOT NULL AND @folha IS NOT NULL AND @pocao_menor IS NOT NULL
  AND NOT EXISTS (SELECT 1 FROM craft_recipes WHERE recipe_key = 'con_pocao_menor');

INSERT INTO craft_recipe_ingredients (recipe_id, slot_index, item_template_id, quantity)
SELECT r.recipe_id, 0, @frasco, 1 FROM craft_recipes r
WHERE r.recipe_key = 'con_pocao_menor'
  AND NOT EXISTS (SELECT 1 FROM craft_recipe_ingredients i WHERE i.recipe_id = r.recipe_id AND i.slot_index = 0);

INSERT INTO craft_recipe_ingredients (recipe_id, slot_index, item_template_id, quantity)
SELECT r.recipe_id, 1, @folha, 2 FROM craft_recipes r
WHERE r.recipe_key = 'con_pocao_menor'
  AND NOT EXISTS (SELECT 1 FROM craft_recipe_ingredients i WHERE i.recipe_id = r.recipe_id AND i.slot_index = 1);

-- Consumível aprendível
INSERT INTO craft_recipes (recipe_key, display_name, craft_category, result_item_template_id, result_quantity, gold_cost, min_level, recipe_item_template_id, is_active, sort_order)
SELECT 'con_pocao_maior', 'Poção Maior de Vida', 'consumable', @pocao_maior, 1, 35, 1, @rec_pocao, 1, 20
FROM DUAL WHERE @frasco IS NOT NULL AND @essencia IS NOT NULL AND @pocao_maior IS NOT NULL AND @rec_pocao IS NOT NULL
  AND NOT EXISTS (SELECT 1 FROM craft_recipes WHERE recipe_key = 'con_pocao_maior');

INSERT INTO craft_recipe_ingredients (recipe_id, slot_index, item_template_id, quantity)
SELECT r.recipe_id, 0, @frasco, 1 FROM craft_recipes r
WHERE r.recipe_key = 'con_pocao_maior'
  AND NOT EXISTS (SELECT 1 FROM craft_recipe_ingredients i WHERE i.recipe_id = r.recipe_id AND i.slot_index = 0);

INSERT INTO craft_recipe_ingredients (recipe_id, slot_index, item_template_id, quantity)
SELECT r.recipe_id, 1, @essencia, 1 FROM craft_recipes r
WHERE r.recipe_key = 'con_pocao_maior'
  AND NOT EXISTS (SELECT 1 FROM craft_recipe_ingredients i WHERE i.recipe_id = r.recipe_id AND i.slot_index = 1);

-- Equipamento padrão: 3 Lingote -> Espada Aprendiz
INSERT INTO craft_recipes (recipe_key, display_name, craft_category, result_item_template_id, result_quantity, gold_cost, min_level, recipe_item_template_id, is_active, sort_order)
SELECT 'eq_espada_aprendiz', 'Espada de Aprendiz', 'equipment', @espada, 1, 25, 1, NULL, 1, 10
FROM DUAL WHERE @lingote IS NOT NULL AND @espada IS NOT NULL
  AND NOT EXISTS (SELECT 1 FROM craft_recipes WHERE recipe_key = 'eq_espada_aprendiz');

INSERT INTO craft_recipe_ingredients (recipe_id, slot_index, item_template_id, quantity)
SELECT r.recipe_id, 0, @lingote, 3 FROM craft_recipes r
WHERE r.recipe_key = 'eq_espada_aprendiz'
  AND NOT EXISTS (SELECT 1 FROM craft_recipe_ingredients i WHERE i.recipe_id = r.recipe_id AND i.slot_index = 0);

-- Equipamento com receita consumida por craft
INSERT INTO craft_recipes (recipe_key, display_name, craft_category, result_item_template_id, result_quantity, gold_cost, min_level, recipe_item_template_id, is_active, sort_order)
SELECT 'eq_lamina_temperada', 'Lâmina Temperada', 'equipment', @lamina, 1, 75, 5, @rec_lamina, 1, 20
FROM DUAL WHERE @lingote IS NOT NULL AND @lamina IS NOT NULL AND @rec_lamina IS NOT NULL
  AND NOT EXISTS (SELECT 1 FROM craft_recipes WHERE recipe_key = 'eq_lamina_temperada');

INSERT INTO craft_recipe_ingredients (recipe_id, slot_index, item_template_id, quantity)
SELECT r.recipe_id, 0, @lingote, 5 FROM craft_recipes r
WHERE r.recipe_key = 'eq_lamina_temperada'
  AND NOT EXISTS (SELECT 1 FROM craft_recipe_ingredients i WHERE i.recipe_id = r.recipe_id AND i.slot_index = 0);

INSERT INTO craft_recipe_ingredients (recipe_id, slot_index, item_template_id, quantity)
SELECT r.recipe_id, 1, @po, 2 FROM craft_recipes r
WHERE r.recipe_key = 'eq_lamina_temperada' AND @po IS NOT NULL
  AND NOT EXISTS (SELECT 1 FROM craft_recipe_ingredients i WHERE i.recipe_id = r.recipe_id AND i.slot_index = 1);
