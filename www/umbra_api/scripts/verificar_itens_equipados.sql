-- ============================================
-- VERIFICAR ITENS EQUIPADOS NO BANCO
-- ============================================
-- Este script verifica o estado atual dos itens equipados

USE umbra_eternum;

-- ============================================
-- 1. VERIFICAR ITENS COM is_equipped = TRUE
-- ============================================
SELECT 
    'ITENS MARCADOS COMO EQUIPADOS:' AS Status;

SELECT 
    pi.inventory_id,
    pi.player_id,
    pi.slot_index,
    pi.is_equipped,
    pi.item_template_id,
    it.item_name,
    it.equipment_slot,
    it.required_level,
    CASE 
        WHEN it.equipment_slot = 'none' OR it.equipment_slot IS NULL THEN '❌ SLOT INVÁLIDO'
        WHEN it.equipment_slot NOT IN ('head', 'chest', 'legs', 'feet', 'hands', 'main_hand', 'off_hand', 'ring', 'amulet', 'earring', 'bracelet', 'mount') THEN '⚠️ SLOT DESCONHECIDO'
        ELSE '✅ OK'
    END AS status_slot
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE
ORDER BY pi.player_id, it.equipment_slot;

-- ============================================
-- 2. VERIFICAR SE HÁ MÚLTIPLOS ITENS NO MESMO SLOT
-- ============================================
SELECT 
    'MÚLTIPLOS ITENS NO MESMO SLOT (PROBLEMA):' AS Status;

SELECT 
    pi.player_id,
    it.equipment_slot,
    COUNT(*) AS quantidade,
    GROUP_CONCAT(pi.inventory_id) AS inventory_ids
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE
AND it.equipment_slot != 'none'
AND it.equipment_slot IS NOT NULL
GROUP BY pi.player_id, it.equipment_slot
HAVING COUNT(*) > 1;

-- ============================================
-- 3. VERIFICAR ITENS COM SLOT INVÁLIDO MAS EQUIPADOS
-- ============================================
SELECT 
    'ITENS EQUIPADOS COM SLOT INVÁLIDO:' AS Status;

SELECT 
    pi.inventory_id,
    pi.player_id,
    it.item_name,
    it.equipment_slot,
    pi.is_equipped
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE
AND (it.equipment_slot = 'none' OR it.equipment_slot IS NULL);

-- ============================================
-- 4. SIMULAR O QUE get_character_info.php RETORNA
-- ============================================
SELECT 
    'SIMULAÇÃO: equipped_items (como retornado pela API):' AS Status;

SELECT 
    it.equipment_slot AS slot_name,
    pi.inventory_id,
    it.item_name,
    it.equipment_slot,
    CASE 
        WHEN it.equipment_slot = 'none' OR it.equipment_slot IS NULL THEN '❌ NÃO SERÁ RETORNADO'
        ELSE '✅ SERÁ RETORNADO'
    END AS sera_retornado
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.player_id = 1  -- ALTERE PARA O ID DO SEU JOGADOR
AND pi.is_equipped = TRUE
ORDER BY it.equipment_slot ASC;

-- ============================================
-- 5. CORRIGIR ITENS COM PROBLEMAS
-- ============================================
-- DESEQUIPAR ITENS COM SLOT INVÁLIDO
UPDATE player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
SET pi.is_equipped = FALSE
WHERE pi.is_equipped = TRUE
AND (it.equipment_slot = 'none' OR it.equipment_slot IS NULL);

-- DESEQUIPAR ITENS DUPLICADOS (manter apenas o primeiro)
-- NOTA: Execute manualmente se necessário, pois pode haver lógica específica

