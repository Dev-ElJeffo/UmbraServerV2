-- ============================================
-- ADICIONAR ITENS - VERSÃO QUE FUNCIONA
-- ============================================

-- PASSO 1: Ver qual é o primeiro slot vazio
SELECT 
    MIN(t.slot_index) as primeiro_slot_vazio
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
WHERE t.slot_index NOT IN (
    SELECT COALESCE(slot_index, -1)
    FROM player_inventory 
    WHERE player_id = 1 AND is_equipped = FALSE
);

-- PASSO 2: Se a query acima retornou um número (ex: 10), use esse número abaixo
-- Colar do Mestre - SUBSTITUA 10 pelo número que apareceu acima
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
VALUES (
    1,
    (SELECT item_id FROM item_templates WHERE item_name = 'Colar do Mestre' LIMIT 1),
    1,
    10,  -- SUBSTITUA PELO NÚMERO QUE APARECEU NA QUERY ACIMA
    FALSE,
    100.0,
    '[]',
    NOW()
)
ON DUPLICATE KEY UPDATE slot_index = slot_index;  -- Se der erro, não faz nada

-- PASSO 3: Execute a query do PASSO 1 de novo para pegar o próximo slot vazio
-- Brincos do Mestre - SUBSTITUA pelo próximo número vazio
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
VALUES (
    1,
    (SELECT item_id FROM item_templates WHERE item_name = 'Brincos do Mestre' LIMIT 1),
    1,
    11,  -- SUBSTITUA PELO PRÓXIMO NÚMERO VAZIO
    FALSE,
    100.0,
    '[]',
    NOW()
)
ON DUPLICATE KEY UPDATE slot_index = slot_index;

