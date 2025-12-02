-- ============================================
-- ADICIONAR ITENS - SLOTS VAZIOS: 0, 1, 5, 10, 11...
-- ============================================

-- Colar do Mestre no slot 0
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
    (SELECT item_id FROM item_templates WHERE item_name = 'Colar do Mestre' LIMIT 1),
    1,
    0,
    FALSE,
    100.0,
    '[]',
    NOW()
WHERE NOT EXISTS (
    SELECT 1 
    FROM player_inventory pi
    INNER JOIN item_templates it ON pi.item_template_id = it.item_id
    WHERE pi.player_id = 1 
    AND it.item_name = 'Colar do Mestre'
    AND pi.is_equipped = FALSE
)
AND EXISTS (SELECT item_id FROM item_templates WHERE item_name = 'Colar do Mestre' LIMIT 1);

-- Brincos do Mestre no slot 1
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
    (SELECT item_id FROM item_templates WHERE item_name = 'Brincos do Mestre' LIMIT 1),
    1,
    1,
    FALSE,
    100.0,
    '[]',
    NOW()
WHERE NOT EXISTS (
    SELECT 1 
    FROM player_inventory pi
    INNER JOIN item_templates it ON pi.item_template_id = it.item_id
    WHERE pi.player_id = 1 
    AND it.item_name = 'Brincos do Mestre'
    AND pi.is_equipped = FALSE
)
AND EXISTS (SELECT item_id FROM item_templates WHERE item_name = 'Brincos do Mestre' LIMIT 1);

-- Botas Élficas (item_id = 6) no slot 5
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
    5,
    FALSE,
    100.0,
    '[]',
    NOW()
WHERE NOT EXISTS (
    SELECT 1 
    FROM player_inventory pi
    WHERE pi.player_id = 1 
    AND pi.item_template_id = 6
    AND pi.is_equipped = FALSE
)
AND EXISTS (SELECT item_id FROM item_templates WHERE item_id = 6 LIMIT 1);

