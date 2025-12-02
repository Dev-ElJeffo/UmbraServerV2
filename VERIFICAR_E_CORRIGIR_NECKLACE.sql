-- ============================================
-- VERIFICAR E CORRIGIR ITEM NECKLACE
-- ============================================
-- Este script verifica e corrige o item "Colar do Mestre"
-- para garantir que está com equipment_slot = 'necklace' e equipado

USE umbra_eternum;

-- ============================================
-- 1. VERIFICAR STATUS ATUAL
-- ============================================
SELECT 
    'STATUS ATUAL DO ITEM:' AS Status;

SELECT 
    it.item_id,
    it.item_name,
    it.equipment_slot AS 'equipment_slot_no_template',
    pi.inventory_id,
    pi.is_equipped,
    pi.slot_index
FROM item_templates it
LEFT JOIN player_inventory pi ON it.item_id = pi.item_template_id AND pi.player_id = 1
WHERE it.item_name = 'Colar do Mestre';

-- ============================================
-- 2. VERIFICAR SE O ENUM INCLUI 'necklace'
-- ============================================
SELECT 
    'VERIFICANDO ENUM equipment_slot:' AS Status;

SHOW COLUMNS FROM item_templates WHERE Field = 'equipment_slot';

-- ============================================
-- 3. ATUALIZAR equipment_slot PARA 'necklace' (se necessário)
-- ============================================
-- NOTA: Se o ENUM não incluir 'necklace', execute primeiro:
-- www/umbra_api/scripts/atualizar_enum_equipment_slot.sql

UPDATE item_templates
SET equipment_slot = 'necklace'
WHERE item_name = 'Colar do Mestre'
  AND equipment_slot != 'necklace';

-- ============================================
-- 4. VERIFICAR SE O ITEM ESTÁ EQUIPADO
-- ============================================
SELECT 
    'VERIFICANDO SE ITEM ESTÁ EQUIPADO:' AS Status;

SELECT 
    pi.inventory_id,
    pi.player_id,
    pi.is_equipped,
    it.item_name,
    it.equipment_slot
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.player_id = 1
  AND it.item_name = 'Colar do Mestre';

-- ============================================
-- 5. SE O ITEM NÃO ESTIVER EQUIPADO, EQUIPAR
-- ============================================
-- Descomente as linhas abaixo se precisar equipar o item:

-- UPDATE player_inventory pi
-- INNER JOIN item_templates it ON pi.item_template_id = it.item_id
-- SET pi.is_equipped = TRUE
-- WHERE pi.player_id = 1
--   AND it.item_name = 'Colar do Mestre'
--   AND pi.is_equipped = FALSE;

-- ============================================
-- 6. VERIFICAR STATUS FINAL
-- ============================================
SELECT 
    'STATUS FINAL:' AS Status;

SELECT 
    it.item_id,
    it.item_name,
    it.equipment_slot,
    pi.inventory_id,
    pi.is_equipped,
    CASE 
        WHEN it.equipment_slot = 'necklace' AND pi.is_equipped = 1 
        THEN '✅ CORRETO'
        WHEN it.equipment_slot != 'necklace' 
        THEN '❌ equipment_slot incorreto'
        WHEN pi.is_equipped = 0 
        THEN '❌ Item não está equipado'
        ELSE '⚠️ Verificar manualmente'
    END AS status
FROM item_templates it
LEFT JOIN player_inventory pi ON it.item_id = pi.item_template_id AND pi.player_id = 1
WHERE it.item_name = 'Colar do Mestre';

