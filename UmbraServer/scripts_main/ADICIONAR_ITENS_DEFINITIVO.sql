-- ============================================
-- ADICIONAR ITENS - VERSÃO DEFINITIVA
-- ============================================

-- Verificar slots ocupados primeiro
SELECT slot_index FROM player_inventory WHERE player_id = 1 AND is_equipped = FALSE ORDER BY slot_index;

-- Verificar se os itens já existem
SELECT 
    it.item_id,
    it.item_name,
    pi.inventory_id,
    pi.slot_index
FROM item_templates it
LEFT JOIN player_inventory pi ON pi.item_template_id = it.item_id AND pi.player_id = 1 AND pi.is_equipped = FALSE
WHERE it.item_name LIKE '%Mestre%' OR it.item_id = 6;

-- Colar do Mestre - slot 0
INSERT IGNORE INTO player_inventory (
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
WHERE EXISTS (SELECT item_id FROM item_templates WHERE item_name = 'Colar do Mestre' LIMIT 1)
AND NOT EXISTS (
    SELECT 1 FROM player_inventory 
    WHERE player_id = 1 AND slot_index = 0 AND is_equipped = FALSE
)
AND NOT EXISTS (
    SELECT 1 FROM player_inventory pi
    INNER JOIN item_templates it ON pi.item_template_id = it.item_id
    WHERE pi.player_id = 1 AND it.item_name = 'Colar do Mestre' AND pi.is_equipped = FALSE
);

-- Brincos do Mestre - slot 1
INSERT IGNORE INTO player_inventory (
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
WHERE EXISTS (SELECT item_id FROM item_templates WHERE item_name = 'Brincos do Mestre' LIMIT 1)
AND NOT EXISTS (
    SELECT 1 FROM player_inventory 
    WHERE player_id = 1 AND slot_index = 1 AND is_equipped = FALSE
)
AND NOT EXISTS (
    SELECT 1 FROM player_inventory pi
    INNER JOIN item_templates it ON pi.item_template_id = it.item_id
    WHERE pi.player_id = 1 AND it.item_name = 'Brincos do Mestre' AND pi.is_equipped = FALSE
);

-- Botas Élficas (item_id = 6) - slot 5
INSERT IGNORE INTO player_inventory (
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
WHERE EXISTS (SELECT item_id FROM item_templates WHERE item_id = 6 LIMIT 1)
AND NOT EXISTS (
    SELECT 1 FROM player_inventory 
    WHERE player_id = 1 AND slot_index = 5 AND is_equipped = FALSE
)
AND NOT EXISTS (
    SELECT 1 FROM player_inventory 
    WHERE player_id = 1 AND item_template_id = 6 AND is_equipped = FALSE
);

