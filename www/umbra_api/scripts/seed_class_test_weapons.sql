-- Armas de teste por classe (main_hand) + uma universal. Idempotente por item_name.
SET NAMES utf8mb4;

-- Requer: add_item_allowed_class_ids.sql

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Axe (Barbarian)',
    'Arma de teste: machado (só Barbarian).',
    'weapon', 'axe', CAST('[1]' AS JSON),
    1, 'main_hand', 1,
    CAST('{"physical_attack":15,"strength":2}' AS JSON),
    'common', 10, 2.5, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Axe (Barbarian)');

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Sword (Templar)',
    'Arma de teste: espada (só Templar). Escudo off-hand em sequência.',
    'weapon', 'sword', CAST('[2]' AS JSON),
    1, 'main_hand', 1,
    CAST('{"physical_attack":14,"vitality":1}' AS JSON),
    'common', 10, 2.0, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Sword (Templar)');

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Scythe (Dark Mage)',
    'Arma de teste: foice (só Dark Mage).',
    'weapon', 'scythe', CAST('[3]' AS JSON),
    1, 'main_hand', 1,
    CAST('{"magic_attack":16,"intelligence":2}' AS JSON),
    'common', 10, 2.2, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Scythe (Dark Mage)');

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Staff (Alchemist)',
    'Arma de teste: cajado (só Alchemist/Cleric).',
    'weapon', 'staff', CAST('[4]' AS JSON),
    1, 'main_hand', 1,
    CAST('{"magic_attack":15,"intelligence":2}' AS JSON),
    'common', 10, 1.8, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Staff (Alchemist)');

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Daggers (Assassin)',
    'Arma de teste: adagas (só Assassin).',
    'weapon', 'dagger', CAST('[5]' AS JSON),
    1, 'main_hand', 1,
    CAST('{"physical_attack":12,"dexterity":3,"critical":2}' AS JSON),
    'common', 10, 1.2, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Daggers (Assassin)');

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Cestus (Martial)',
    'Arma de teste: cestus (só Martial/Monk).',
    'weapon', 'cestus', CAST('[6]' AS JSON),
    1, 'main_hand', 1,
    CAST('{"physical_attack":13,"dexterity":2}' AS JSON),
    'common', 10, 1.0, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Cestus (Martial)');

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Universal Blade',
    'Arma de teste: equipável por todas as classes (allowed_class_ids vazio).',
    'weapon', 'sword', CAST('[]' AS JSON),
    1, 'main_hand', 1,
    CAST('{"physical_attack":10}' AS JSON),
    'common', 5, 1.5, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Universal Blade');
