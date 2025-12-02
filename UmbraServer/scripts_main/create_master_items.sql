-- ============================================
-- CRIAR 4 ITENS DO MESTRE + ADICIONAR AO INVENTÁRIO
-- ============================================

-- 1. Anel do Mestre (Ring)
INSERT INTO item_templates (
    item_name,
    item_description,
    item_type,
    item_subtype,
    icon_path,
    max_stack_size,
    equipment_slot,
    required_level,
    stats_json,
    rarity,
    value,
    weight
) VALUES (
    'Anel do Mestre',
    'Um anel único pertencente somente ao mestre do universo. Concede poderes imensuráveis.',
    'armor',
    'ring',
    '/Game/UI/Icons/Items/ICO_MasterRing',
    1,
    'ring',
    1,
    '{
        "strength": 50,
        "dexterity": 50,
        "intelligence": 50,
        "vitality": 50,
        "luck": 50,
        "health_bonus": 500,
        "mana_bonus": 500,
        "defense": 100,
        "magic_defense": 100,
        "attack": 100,
        "magic_attack": 100,
        "accuracy": 50,
        "dodge": 50,
        "critical": 50,
        "movement": 25,
        "resistance": 50,
        "double_attack_rate": 50
    }',
    'legendary',
    999999,
    0.0
);

-- 2. Amuleto do Mestre (Amulet)
INSERT INTO item_templates (
    item_name,
    item_description,
    item_type,
    item_subtype,
    icon_path,
    max_stack_size,
    equipment_slot,
    required_level,
    stats_json,
    rarity,
    value,
    weight
) VALUES (
    'Amuleto do Mestre',
    'Um amuleto único pertencente somente ao mestre do universo. Concede poderes imensuráveis.',
    'armor',
    'amulet',
    '/Game/UI/Icons/Items/ICO_MasterAmulet',
    1,
    'amulet',
    1,
    '{
        "strength": 50,
        "dexterity": 50,
        "intelligence": 50,
        "vitality": 50,
        "luck": 50,
        "health_bonus": 500,
        "mana_bonus": 500,
        "defense": 100,
        "magic_defense": 100,
        "attack": 100,
        "magic_attack": 100,
        "accuracy": 50,
        "dodge": 50,
        "critical": 50,
        "movement": 25,
        "resistance": 50,
        "double_attack_rate": 50
    }',
    'legendary',
    999999,
    0.0
);

-- 3. Colar do Mestre (Amulet - mas vamos usar como amulet também, ou criar como necklace se existir)
-- Vou criar como "necklace" no item_subtype, mas equipment_slot como "amulet"
INSERT INTO item_templates (
    item_name,
    item_description,
    item_type,
    item_subtype,
    icon_path,
    max_stack_size,
    equipment_slot,
    required_level,
    stats_json,
    rarity,
    value,
    weight
) VALUES (
    'Colar do Mestre',
    'Um colar único pertencente somente ao mestre do universo. Concede poderes imensuráveis.',
    'armor',
    'necklace',
    '/Game/UI/Icons/Items/ICO_MasterNecklace',
    1,
    'amulet',
    1,
    '{
        "strength": 50,
        "dexterity": 50,
        "intelligence": 50,
        "vitality": 50,
        "luck": 50,
        "health_bonus": 500,
        "mana_bonus": 500,
        "defense": 100,
        "magic_defense": 100,
        "attack": 100,
        "magic_attack": 100,
        "accuracy": 50,
        "dodge": 50,
        "critical": 50,
        "movement": 25,
        "resistance": 50,
        "double_attack_rate": 50
    }',
    'legendary',
    999999,
    0.0
);

-- 4. Brincos do Mestre (Earring)
INSERT INTO item_templates (
    item_name,
    item_description,
    item_type,
    item_subtype,
    icon_path,
    max_stack_size,
    equipment_slot,
    required_level,
    stats_json,
    rarity,
    value,
    weight
) VALUES (
    'Brincos do Mestre',
    'Brincos únicos pertencentes somente ao mestre do universo. Concedem poderes imensuráveis.',
    'armor',
    'earring',
    '/Game/UI/Icons/Items/ICO_MasterEarring',
    1,
    'earring',
    1,
    '{
        "strength": 50,
        "dexterity": 50,
        "intelligence": 50,
        "vitality": 50,
        "luck": 50,
        "health_bonus": 500,
        "mana_bonus": 500,
        "defense": 100,
        "magic_defense": 100,
        "attack": 100,
        "magic_attack": 100,
        "accuracy": 50,
        "dodge": 50,
        "critical": 50,
        "movement": 25,
        "resistance": 50,
        "double_attack_rate": 50
    }',
    'legendary',
    999999,
    0.0
);

-- ============================================
-- ADICIONAR ITENS AO INVENTÁRIO DO PLAYER_ID = 1
-- ============================================

-- Encontrar os IDs dos itens recém-criados
SET @ring_id = (SELECT item_id FROM item_templates WHERE item_name = 'Anel do Mestre' ORDER BY item_id DESC LIMIT 1);
SET @amulet_id = (SELECT item_id FROM item_templates WHERE item_name = 'Amuleto do Mestre' ORDER BY item_id DESC LIMIT 1);
SET @necklace_id = (SELECT item_id FROM item_templates WHERE item_name = 'Colar do Mestre' ORDER BY item_id DESC LIMIT 1);
SET @earring_id = (SELECT item_id FROM item_templates WHERE item_name = 'Brincos do Mestre' ORDER BY item_id DESC LIMIT 1);

-- Encontrar slots vazios no inventário (0-49)
SET @slot_ring = (
    SELECT MIN(t.slot_index)
    FROM (
        SELECT 0 as slot_index UNION ALL SELECT 1 UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT 4
        UNION ALL SELECT 5 UNION ALL SELECT 6 UNION ALL SELECT 7 UNION ALL SELECT 8 UNION ALL SELECT 9
        UNION ALL SELECT 10 UNION ALL SELECT 11 UNION ALL SELECT 12 UNION ALL SELECT 13 UNION ALL SELECT 14
        UNION ALL SELECT 15 UNION ALL SELECT 16 UNION ALL SELECT 17 UNION ALL SELECT 18 UNION ALL SELECT 19
        UNION ALL SELECT 20 UNION ALL SELECT 21 UNION ALL SELECT 22 UNION ALL SELECT 23 UNION ALL SELECT 24
        UNION ALL SELECT 25 UNION ALL SELECT 26 UNION ALL SELECT 27 UNION ALL SELECT 28 UNION ALL SELECT 29
        UNION ALL SELECT 30 UNION ALL SELECT 31 UNION ALL SELECT 32 UNION ALL SELECT 33 UNION ALL SELECT 34
        UNION ALL SELECT 35 UNION ALL SELECT 36 UNION ALL SELECT 37 UNION ALL SELECT 38 UNION ALL SELECT 39
        UNION ALL SELECT 40 UNION ALL SELECT 41 UNION ALL SELECT 42 UNION ALL SELECT 43 UNION ALL SELECT 44
        UNION ALL SELECT 45 UNION ALL SELECT 46 UNION ALL SELECT 47 UNION ALL SELECT 48 UNION ALL SELECT 49
    ) t
    LEFT JOIN player_inventory pi ON pi.slot_index = t.slot_index 
        AND pi.player_id = 1 
        AND pi.is_equipped = FALSE
    WHERE pi.inventory_id IS NULL
    LIMIT 1
);

SET @slot_amulet = (
    SELECT MIN(t.slot_index)
    FROM (
        SELECT 0 as slot_index UNION ALL SELECT 1 UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT 4
        UNION ALL SELECT 5 UNION ALL SELECT 6 UNION ALL SELECT 7 UNION ALL SELECT 8 UNION ALL SELECT 9
        UNION ALL SELECT 10 UNION ALL SELECT 11 UNION ALL SELECT 12 UNION ALL SELECT 13 UNION ALL SELECT 14
        UNION ALL SELECT 15 UNION ALL SELECT 16 UNION ALL SELECT 17 UNION ALL SELECT 18 UNION ALL SELECT 19
        UNION ALL SELECT 20 UNION ALL SELECT 21 UNION ALL SELECT 22 UNION ALL SELECT 23 UNION ALL SELECT 24
        UNION ALL SELECT 25 UNION ALL SELECT 26 UNION ALL SELECT 27 UNION ALL SELECT 28 UNION ALL SELECT 29
        UNION ALL SELECT 30 UNION ALL SELECT 31 UNION ALL SELECT 32 UNION ALL SELECT 33 UNION ALL SELECT 34
        UNION ALL SELECT 35 UNION ALL SELECT 36 UNION ALL SELECT 37 UNION ALL SELECT 38 UNION ALL SELECT 39
        UNION ALL SELECT 40 UNION ALL SELECT 41 UNION ALL SELECT 42 UNION ALL SELECT 43 UNION ALL SELECT 44
        UNION ALL SELECT 45 UNION ALL SELECT 46 UNION ALL SELECT 47 UNION ALL SELECT 48 UNION ALL SELECT 49
    ) t
    LEFT JOIN player_inventory pi ON pi.slot_index = t.slot_index 
        AND pi.player_id = 1 
        AND pi.is_equipped = FALSE
    WHERE pi.inventory_id IS NULL
    LIMIT 1
);

SET @slot_necklace = (
    SELECT MIN(t.slot_index)
    FROM (
        SELECT 0 as slot_index UNION ALL SELECT 1 UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT 4
        UNION ALL SELECT 5 UNION ALL SELECT 6 UNION ALL SELECT 7 UNION ALL SELECT 8 UNION ALL SELECT 9
        UNION ALL SELECT 10 UNION ALL SELECT 11 UNION ALL SELECT 12 UNION ALL SELECT 13 UNION ALL SELECT 14
        UNION ALL SELECT 15 UNION ALL SELECT 16 UNION ALL SELECT 17 UNION ALL SELECT 18 UNION ALL SELECT 19
        UNION ALL SELECT 20 UNION ALL SELECT 21 UNION ALL SELECT 22 UNION ALL SELECT 23 UNION ALL SELECT 24
        UNION ALL SELECT 25 UNION ALL SELECT 26 UNION ALL SELECT 27 UNION ALL SELECT 28 UNION ALL SELECT 29
        UNION ALL SELECT 30 UNION ALL SELECT 31 UNION ALL SELECT 32 UNION ALL SELECT 33 UNION ALL SELECT 34
        UNION ALL SELECT 35 UNION ALL SELECT 36 UNION ALL SELECT 37 UNION ALL SELECT 38 UNION ALL SELECT 39
        UNION ALL SELECT 40 UNION ALL SELECT 41 UNION ALL SELECT 42 UNION ALL SELECT 43 UNION ALL SELECT 44
        UNION ALL SELECT 45 UNION ALL SELECT 46 UNION ALL SELECT 47 UNION ALL SELECT 48 UNION ALL SELECT 49
    ) t
    LEFT JOIN player_inventory pi ON pi.slot_index = t.slot_index 
        AND pi.player_id = 1 
        AND pi.is_equipped = FALSE
    WHERE pi.inventory_id IS NULL
    LIMIT 1
);

SET @slot_earring = (
    SELECT MIN(t.slot_index)
    FROM (
        SELECT 0 as slot_index UNION ALL SELECT 1 UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT 4
        UNION ALL SELECT 5 UNION ALL SELECT 6 UNION ALL SELECT 7 UNION ALL SELECT 8 UNION ALL SELECT 9
        UNION ALL SELECT 10 UNION ALL SELECT 11 UNION ALL SELECT 12 UNION ALL SELECT 13 UNION ALL SELECT 14
        UNION ALL SELECT 15 UNION ALL SELECT 16 UNION ALL SELECT 17 UNION ALL SELECT 18 UNION ALL SELECT 19
        UNION ALL SELECT 20 UNION ALL SELECT 21 UNION ALL SELECT 22 UNION ALL SELECT 23 UNION ALL SELECT 24
        UNION ALL SELECT 25 UNION ALL SELECT 26 UNION ALL SELECT 27 UNION ALL SELECT 28 UNION ALL SELECT 29
        UNION ALL SELECT 30 UNION ALL SELECT 31 UNION ALL SELECT 32 UNION ALL SELECT 33 UNION ALL SELECT 34
        UNION ALL SELECT 35 UNION ALL SELECT 36 UNION ALL SELECT 37 UNION ALL SELECT 38 UNION ALL SELECT 39
        UNION ALL SELECT 40 UNION ALL SELECT 41 UNION ALL SELECT 42 UNION ALL SELECT 43 UNION ALL SELECT 44
        UNION ALL SELECT 45 UNION ALL SELECT 46 UNION ALL SELECT 47 UNION ALL SELECT 48 UNION ALL SELECT 49
    ) t
    LEFT JOIN player_inventory pi ON pi.slot_index = t.slot_index 
        AND pi.player_id = 1 
        AND pi.is_equipped = FALSE
    WHERE pi.inventory_id IS NULL
    LIMIT 1
);

-- Inserir os 4 itens no inventário
INSERT INTO player_inventory (
    player_id,
    item_template_id,
    quantity,
    slot_index,
    is_equipped,
    durability,
    custom_properties,
    acquired_at
) VALUES
    (1, @ring_id, 1, @slot_ring, FALSE, 100.0, '[]', NOW()),
    (1, @amulet_id, 1, @slot_amulet, FALSE, 100.0, '[]', NOW()),
    (1, @necklace_id, 1, @slot_necklace, FALSE, 100.0, '[]', NOW()),
    (1, @earring_id, 1, @slot_earring, FALSE, 100.0, '[]', NOW());

-- Adicionar item_id = 6 ao inventário (se ainda não estiver)
INSERT INTO player_inventory (
    player_id,
    item_template_id,
    quantity,
    slot_index,
    is_equipped,
    durability,
    custom_properties,
    acquired_at
)
SELECT 
    1,
    6,
    1,
    (
        SELECT MIN(t.slot_index)
        FROM (
            SELECT 0 as slot_index UNION ALL SELECT 1 UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT 4
            UNION ALL SELECT 5 UNION ALL SELECT 6 UNION ALL SELECT 7 UNION ALL SELECT 8 UNION ALL SELECT 9
            UNION ALL SELECT 10 UNION ALL SELECT 11 UNION ALL SELECT 12 UNION ALL SELECT 13 UNION ALL SELECT 14
            UNION ALL SELECT 15 UNION ALL SELECT 16 UNION ALL SELECT 17 UNION ALL SELECT 18 UNION ALL SELECT 19
            UNION ALL SELECT 20 UNION ALL SELECT 21 UNION ALL SELECT 22 UNION ALL SELECT 23 UNION ALL SELECT 24
            UNION ALL SELECT 25 UNION ALL SELECT 26 UNION ALL SELECT 27 UNION ALL SELECT 28 UNION ALL SELECT 29
            UNION ALL SELECT 30 UNION ALL SELECT 31 UNION ALL SELECT 32 UNION ALL SELECT 33 UNION ALL SELECT 34
            UNION ALL SELECT 35 UNION ALL SELECT 36 UNION ALL SELECT 37 UNION ALL SELECT 38 UNION ALL SELECT 39
            UNION ALL SELECT 40 UNION ALL SELECT 41 UNION ALL SELECT 42 UNION ALL SELECT 43 UNION ALL SELECT 44
            UNION ALL SELECT 45 UNION ALL SELECT 46 UNION ALL SELECT 47 UNION ALL SELECT 48 UNION ALL SELECT 49
        ) t
        LEFT JOIN player_inventory pi ON pi.slot_index = t.slot_index 
            AND pi.player_id = 1 
            AND pi.is_equipped = FALSE
        WHERE pi.inventory_id IS NULL
        LIMIT 1
    ),
    FALSE,
    100.0,
    '[]',
    NOW()
WHERE NOT EXISTS (
    SELECT 1 FROM player_inventory 
    WHERE player_id = 1 
    AND item_template_id = 6 
    AND is_equipped = FALSE
);

-- Verificar os itens criados
SELECT 
    it.item_id,
    it.item_name,
    it.rarity,
    it.required_level,
    it.value,
    it.equipment_slot,
    it.stats_json,
    pi.inventory_id,
    pi.slot_index
FROM item_templates it
LEFT JOIN player_inventory pi ON pi.item_template_id = it.item_id AND pi.player_id = 1
WHERE it.item_name LIKE '%Mestre%' OR it.item_id = 6
ORDER BY it.item_id;

