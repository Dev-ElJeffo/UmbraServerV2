-- Quest demo: escolha multi-grupo (armas por classe + armaduras por classe).
-- Requer: seed_class_test_weapons.sql + seed_class_test_armor.sql (e coluna allowed_class_ids).
-- Idempotente por quest_key = quest_class_gear_choice.
SET NAMES utf8mb4;

-- Armaduras de peito por classe (complementa seed_class_test_armor.sql)
INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Chest (Templar)',
    'Armadura de teste: peitoral (só Templar).',
    'armor', 'chest', CAST('[2]' AS JSON),
    1, 'chest', 1,
    CAST('{"physical_defense":14,"vitality":2}' AS JSON),
    'common', 10, 3.2, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Chest (Templar)');

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Chest (Dark Mage)',
    'Armadura de teste: peitoral (só Dark Mage).',
    'armor', 'chest', CAST('[3]' AS JSON),
    1, 'chest', 1,
    CAST('{"magic_defense":12,"intelligence":1}' AS JSON),
    'common', 10, 2.8, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Chest (Dark Mage)');

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Chest (Alchemist)',
    'Armadura de teste: peitoral (só Alchemist).',
    'armor', 'chest', CAST('[4]' AS JSON),
    1, 'chest', 1,
    CAST('{"magic_defense":11,"intelligence":1}' AS JSON),
    'common', 10, 2.6, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Chest (Alchemist)');

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Chest (Assassin)',
    'Armadura de teste: peitoral (só Assassin).',
    'armor', 'chest', CAST('[5]' AS JSON),
    1, 'chest', 1,
    CAST('{"physical_defense":10,"dexterity":2}' AS JSON),
    'common', 10, 2.4, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Chest (Assassin)');

INSERT INTO item_templates (
    item_name, item_description, item_type, item_subtype, allowed_class_ids,
    max_stack_size, equipment_slot, required_level, stats_json,
    rarity, value, weight, can_be_refined, tradeable, item_category
)
SELECT
    'Test Chest (Martial)',
    'Armadura de teste: peitoral (só Martial).',
    'armor', 'chest', CAST('[6]' AS JSON),
    1, 'chest', 1,
    CAST('{"physical_defense":11,"dexterity":1}' AS JSON),
    'common', 10, 2.5, 0, 1, 'equipment'
FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM item_templates WHERE item_name = 'Test Chest (Martial)');

-- Quest + offer no mercador
INSERT INTO quests (
    quest_key, title, description, offer_text, turn_in_text,
    min_level, prerequisite_quest_id, repeatable, turn_in_npc_template_id, is_active
)
SELECT
    'quest_class_gear_choice',
    'Equipamento de Classe',
    'Escolha uma arma e uma armadura entre as opções de todas as classes.',
    'Prove-se e escolha seu equipamento.',
    'Escolha uma arma (grupo 1) e uma armadura (grupo 2).',
    1, NULL, 1,
    nt.npc_template_id, 1
FROM npc_templates nt
WHERE nt.npc_name = 'npc_merchant_01'
  AND NOT EXISTS (SELECT 1 FROM quests WHERE quest_key = 'quest_class_gear_choice')
LIMIT 1;

UPDATE quests q
INNER JOIN npc_templates nt ON nt.npc_name = 'npc_merchant_01'
SET q.turn_in_npc_template_id = nt.npc_template_id
WHERE q.quest_key = 'quest_class_gear_choice'
  AND (q.turn_in_npc_template_id IS NULL OR q.turn_in_npc_template_id = 0);

INSERT INTO quest_objectives (quest_id, sort_order, objective_type, description, params_json)
SELECT q.quest_id, 0, 'talk', 'Fale com o mercador para concluir o treino de equipamento.', '{}'
FROM quests q
WHERE q.quest_key = 'quest_class_gear_choice'
  AND NOT EXISTS (
    SELECT 1 FROM quest_objectives o WHERE o.quest_id = q.quest_id AND o.objective_type = 'talk'
  );

INSERT INTO quest_rewards (quest_id, reward_type, amount, item_template_id, quantity, choice_group_id, sort_order)
SELECT q.quest_id, 'gold', 50, NULL, 1, NULL, 0
FROM quests q
WHERE q.quest_key = 'quest_class_gear_choice'
  AND NOT EXISTS (
    SELECT 1 FROM quest_rewards r WHERE r.quest_id = q.quest_id AND r.reward_type = 'gold' AND r.amount = 50
  );

INSERT INTO quest_rewards (quest_id, reward_type, amount, item_template_id, quantity, choice_group_id, sort_order)
SELECT q.quest_id, 'experience', 75, NULL, 1, NULL, 1
FROM quests q
WHERE q.quest_key = 'quest_class_gear_choice'
  AND NOT EXISTS (
    SELECT 1 FROM quest_rewards r WHERE r.quest_id = q.quest_id AND r.reward_type = 'experience' AND r.amount = 75
  );

-- Grupo 1: armas (todas as classes)
INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 1, it.item_name, 'item', 0, it.item_id, 1, 0
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Axe (Barbarian)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 1 AND c.item_template_id = it.item_id
  );

INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 1, it.item_name, 'item', 0, it.item_id, 1, 1
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Sword (Templar)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 1 AND c.item_template_id = it.item_id
  );

INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 1, it.item_name, 'item', 0, it.item_id, 1, 2
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Scythe (Dark Mage)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 1 AND c.item_template_id = it.item_id
  );

INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 1, it.item_name, 'item', 0, it.item_id, 1, 3
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Staff (Alchemist)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 1 AND c.item_template_id = it.item_id
  );

INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 1, it.item_name, 'item', 0, it.item_id, 1, 4
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Daggers (Assassin)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 1 AND c.item_template_id = it.item_id
  );

INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 1, it.item_name, 'item', 0, it.item_id, 1, 5
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Cestus (Martial)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 1 AND c.item_template_id = it.item_id
  );

-- Grupo 2: armaduras (todas as classes)
INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 2, it.item_name, 'item', 0, it.item_id, 1, 0
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Chest (Barbarian)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 2 AND c.item_template_id = it.item_id
  );

INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 2, it.item_name, 'item', 0, it.item_id, 1, 1
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Chest (Templar)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 2 AND c.item_template_id = it.item_id
  );

INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 2, it.item_name, 'item', 0, it.item_id, 1, 2
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Chest (Dark Mage)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 2 AND c.item_template_id = it.item_id
  );

INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 2, it.item_name, 'item', 0, it.item_id, 1, 3
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Chest (Alchemist)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 2 AND c.item_template_id = it.item_id
  );

INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 2, it.item_name, 'item', 0, it.item_id, 1, 4
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Chest (Assassin)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 2 AND c.item_template_id = it.item_id
  );

INSERT INTO quest_reward_choices (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
SELECT q.quest_id, 2, it.item_name, 'item', 0, it.item_id, 1, 5
FROM quests q
CROSS JOIN item_templates it
WHERE q.quest_key = 'quest_class_gear_choice'
  AND it.item_name = 'Test Chest (Martial)'
  AND NOT EXISTS (
    SELECT 1 FROM quest_reward_choices c
    WHERE c.quest_id = q.quest_id AND c.choice_group_id = 2 AND c.item_template_id = it.item_id
  );

INSERT INTO npc_quest_offers (npc_template_id, quest_id, sort_order, is_quest_giver)
SELECT nt.npc_template_id, q.quest_id, 10, 1
FROM npc_templates nt
INNER JOIN quests q ON q.quest_key = 'quest_class_gear_choice'
WHERE nt.npc_name = 'npc_merchant_01'
  AND NOT EXISTS (
    SELECT 1 FROM npc_quest_offers o
    WHERE o.npc_template_id = nt.npc_template_id AND o.quest_id = q.quest_id
  );

UPDATE npc_templates SET has_quest_dialog = 1 WHERE npc_name = 'npc_merchant_01';
