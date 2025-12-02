-- ============================================
-- CORRIGIR ITENS EQUIPADOS COM PROBLEMAS
-- ============================================
-- Este script corrige itens que estão marcados como equipados
-- mas não podem aparecer nos slots (slot inválido, duplicados, etc.)

USE umbra_eternum;

-- ============================================
-- 1. VERIFICAR ESTADO ATUAL
-- ============================================
SELECT 
    'ESTADO ATUAL DOS ITENS EQUIPADOS:' AS Status;

SELECT 
    pi.inventory_id,
    pi.player_id,
    it.item_name,
    it.equipment_slot,
    pi.is_equipped,
    CASE 
        WHEN it.equipment_slot = 'none' OR it.equipment_slot IS NULL THEN '❌ SLOT INVÁLIDO'
        WHEN it.equipment_slot NOT IN ('head', 'chest', 'legs', 'feet', 'hands', 'main_hand', 'off_hand', 'ring', 'amulet', 'earring', 'bracelet', 'mount') THEN '⚠️ SLOT DESCONHECIDO'
        ELSE '✅ OK'
    END AS status
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE
ORDER BY pi.player_id, it.equipment_slot;

-- ============================================
-- 2. DESEQUIPAR ITENS COM SLOT INVÁLIDO
-- ============================================
-- Itens com equipment_slot = 'none' ou NULL não podem ser equipados
-- e não aparecerão nos slots do Character Info

UPDATE player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
SET pi.is_equipped = FALSE
WHERE pi.is_equipped = TRUE
AND (it.equipment_slot = 'none' OR it.equipment_slot IS NULL);

SELECT 
    CONCAT('Itens desequipados (slot inválido): ', ROW_COUNT()) AS Resultado;

-- ============================================
-- 3. VERIFICAR MÚLTIPLOS ITENS NO MESMO SLOT
-- ============================================
SELECT 
    'MÚLTIPLOS ITENS NO MESMO SLOT:' AS Status;

SELECT 
    pi.player_id,
    it.equipment_slot,
    COUNT(*) AS quantidade,
    GROUP_CONCAT(pi.inventory_id ORDER BY pi.inventory_id) AS inventory_ids,
    GROUP_CONCAT(it.item_name ORDER BY pi.inventory_id SEPARATOR ', ') AS item_names
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE
AND it.equipment_slot != 'none'
AND it.equipment_slot IS NOT NULL
GROUP BY pi.player_id, it.equipment_slot
HAVING COUNT(*) > 1;

-- ============================================
-- 4. CORRIGIR MÚLTIPLOS ITENS NO MESMO SLOT
-- ============================================
-- Manter apenas o primeiro item (menor inventory_id)
-- Desequipar os demais

-- Criar tabela temporária com os itens a manter
CREATE TEMPORARY TABLE IF NOT EXISTS temp_items_to_keep AS
SELECT 
    MIN(pi.inventory_id) AS inventory_id,
    pi.player_id,
    it.equipment_slot
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE
AND it.equipment_slot != 'none'
AND it.equipment_slot IS NOT NULL
GROUP BY pi.player_id, it.equipment_slot
HAVING COUNT(*) > 1;

-- Desequipar itens duplicados (exceto o primeiro)
UPDATE player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
INNER JOIN temp_items_to_keep tik ON pi.player_id = tik.player_id 
    AND it.equipment_slot = tik.equipment_slot
SET pi.is_equipped = FALSE
WHERE pi.is_equipped = TRUE
AND pi.inventory_id != tik.inventory_id;

-- Limpar tabela temporária
DROP TEMPORARY TABLE IF EXISTS temp_items_to_keep;

SELECT 
    CONCAT('Itens duplicados desequipados: ', ROW_COUNT()) AS Resultado;

-- ============================================
-- 5. VERIFICAR ESTADO FINAL
-- ============================================
SELECT 
    'ESTADO FINAL APÓS CORREÇÃO:' AS Status;

SELECT 
    pi.inventory_id,
    pi.player_id,
    it.item_name,
    it.equipment_slot,
    pi.is_equipped,
    CASE 
        WHEN it.equipment_slot = 'none' OR it.equipment_slot IS NULL THEN '❌ SLOT INVÁLIDO'
        WHEN it.equipment_slot NOT IN ('head', 'chest', 'legs', 'feet', 'hands', 'main_hand', 'off_hand', 'ring', 'amulet', 'earring', 'bracelet', 'mount') THEN '⚠️ SLOT DESCONHECIDO'
        ELSE '✅ OK'
    END AS status
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE
ORDER BY pi.player_id, it.equipment_slot;

-- ============================================
-- 6. RESUMO
-- ============================================
SELECT 
    'RESUMO:' AS Status;

SELECT 
    COUNT(*) AS total_itens_equipados,
    COUNT(DISTINCT it.equipment_slot) AS slots_ocupados
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE
AND it.equipment_slot != 'none'
AND it.equipment_slot IS NOT NULL;

