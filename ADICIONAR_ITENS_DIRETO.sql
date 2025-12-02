-- ============================================
-- ADICIONAR ITENS - VERSÃO DIRETA
-- ============================================

-- Colar do Mestre
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
    (
        SELECT t.slot_index
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
        LEFT JOIN player_inventory pi ON pi.player_id = 1 
            AND pi.slot_index = t.slot_index 
            AND pi.is_equipped = FALSE
        WHERE pi.inventory_id IS NULL
        ORDER BY t.slot_index ASC
        LIMIT 1
    ),
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

-- Brincos do Mestre
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
    (
        SELECT t.slot_index
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
        LEFT JOIN player_inventory pi ON pi.player_id = 1 
            AND pi.slot_index = t.slot_index 
            AND pi.is_equipped = FALSE
        WHERE pi.inventory_id IS NULL
        ORDER BY t.slot_index ASC
        LIMIT 1
    ),
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

-- Verificar resultado
SELECT 
    it.item_id,
    it.item_name,
    pi.inventory_id,
    pi.slot_index,
    pi.is_equipped
FROM item_templates it
LEFT JOIN player_inventory pi ON pi.item_template_id = it.item_id AND pi.player_id = 1
WHERE it.item_name LIKE '%Mestre%'
ORDER BY it.item_id;

