-- Armaduras de teste por classe (chest) + uma universal. Idempotente por item_name.
SET NAMES utf8mb4;

-- Requer: add_item_allowed_class_ids.sql

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Chest (Barbarian)',
    'Armadura de teste: peitoral (só Barbarian).',
    'armor', 'chest', CAST('[1]' AS JSON),
    1, 'chest', 1,
    CAST('{"physical_defense":12,"vitality":2}' AS JSON),
    'common', 10, 3.0, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Chest (Barbarian)');

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Universal Chest',
    'Armadura de teste: peitoral equipável por todas as classes (allowed_class_ids vazio).',
    'armor', 'chest', CAST('[]' AS JSON),
    1, 'chest', 1,
    CAST('{"physical_defense":8}' AS JSON),
    'common', 5, 2.5, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Universal Chest');
