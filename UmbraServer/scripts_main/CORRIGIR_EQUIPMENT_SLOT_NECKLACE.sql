-- ============================================
-- CORREÇÃO: Atualizar equipment_slot para "necklace"
-- ============================================
-- Este script corrige o equipment_slot do item "Colar do Mestre" de "amulet" para "necklace"
-- 
-- IMPORTANTE: Execute primeiro o script atualizar_enum_equipment_slot.sql para incluir 'necklace' no ENUM

USE umbra_eternum;

-- ============================================
-- PASSO 1: Atualizar ENUM para incluir 'necklace' (se ainda não estiver)
-- ============================================
-- Se o ENUM já incluir 'necklace', este passo pode ser ignorado
-- Execute o script: www/umbra_api/scripts/atualizar_enum_equipment_slot.sql

-- ============================================
-- PASSO 2: Atualizar o equipment_slot do item "Colar do Mestre"
-- ============================================
UPDATE item_templates
SET equipment_slot = 'necklace'
WHERE item_name = 'Colar do Mestre'
  AND equipment_slot != 'necklace';

-- ============================================
-- PASSO 3: Verificar se a atualização foi bem-sucedida
-- ============================================
SELECT 
    item_id,
    item_name,
    equipment_slot,
    item_subtype
FROM item_templates
WHERE item_name = 'Colar do Mestre';

