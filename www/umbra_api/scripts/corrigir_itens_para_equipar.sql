-- ============================================
-- CORRIGIR ITENS PARA PODEREM SER EQUIPADOS
-- ============================================
-- Este script corrige itens no banco de dados
-- para que possam ser equipados

USE umbra_eternum;

-- ============================================
-- 1. VERIFICAR ITENS ATUAIS
-- ============================================
SELECT 
    'ITENS ANTES DA CORREÇÃO:' AS Status;

SELECT 
    item_id,
    item_name,
    item_type,
    equipment_slot,
    required_level
FROM item_templates
ORDER BY item_id;

-- ============================================
-- 2. CORRIGIR ITENS COM equipment_slot = 'none'
-- ============================================
-- Vamos definir equipment_slot baseado no item_type e item_subtype

-- Armas (weapon) -> main_hand
UPDATE item_templates
SET equipment_slot = 'main_hand'
WHERE equipment_slot = 'none' 
AND item_type = 'weapon'
AND (item_subtype LIKE '%sword%' OR item_subtype LIKE '%axe%' OR item_subtype LIKE '%mace%' OR item_subtype LIKE '%staff%' OR item_subtype LIKE '%wand%' OR item_subtype LIKE '%bow%' OR item_subtype LIKE '%dagger%');

-- Escudos -> off_hand
UPDATE item_templates
SET equipment_slot = 'off_hand'
WHERE equipment_slot = 'none' 
AND (item_type = 'armor' AND item_subtype LIKE '%shield%')
OR (item_subtype LIKE '%shield%');

-- Armaduras (armor) -> baseado no subtype
UPDATE item_templates
SET equipment_slot = 'head'
WHERE equipment_slot = 'none' 
AND item_type = 'armor'
AND (item_subtype LIKE '%helmet%' OR item_subtype LIKE '%hat%' OR item_subtype LIKE '%cap%' OR item_subtype LIKE '%head%');

UPDATE item_templates
SET equipment_slot = 'chest'
WHERE equipment_slot = 'none' 
AND item_type = 'armor'
AND (item_subtype LIKE '%chest%' OR item_subtype LIKE '%armor%' OR item_subtype LIKE '%plate%' OR item_subtype LIKE '%mail%' OR item_subtype LIKE '%robe%');

UPDATE item_templates
SET equipment_slot = 'hands'
WHERE equipment_slot = 'none' 
AND item_type = 'armor'
AND (item_subtype LIKE '%glove%' OR item_subtype LIKE '%gauntlet%' OR item_subtype LIKE '%hand%');

UPDATE item_templates
SET equipment_slot = 'feet'
WHERE equipment_slot = 'none' 
AND item_type = 'armor'
AND (item_subtype LIKE '%boot%' OR item_subtype LIKE '%shoe%' OR item_subtype LIKE '%foot%' OR item_subtype LIKE '%greave%');

-- Acessórios
UPDATE item_templates
SET equipment_slot = 'ring'
WHERE equipment_slot = 'none' 
AND (item_subtype LIKE '%ring%' OR item_name LIKE '%ring%');

UPDATE item_templates
SET equipment_slot = 'amulet'
WHERE equipment_slot = 'none' 
AND (item_subtype LIKE '%amulet%' OR item_subtype LIKE '%necklace%' OR item_subtype LIKE '%pendant%' OR item_name LIKE '%amulet%' OR item_name LIKE '%necklace%');

-- ============================================
-- 3. DEFINIR required_level PARA ITENS SEM NÍVEL
-- ============================================
-- Se required_level for NULL ou 0, definir como 1 (nível mínimo)

UPDATE item_templates
SET required_level = 1
WHERE (required_level IS NULL OR required_level = 0)
AND (equipment_slot != 'none' AND equipment_slot IS NOT NULL);

-- ============================================
-- 4. VERIFICAR ITENS APÓS CORREÇÃO
-- ============================================
SELECT 
    'ITENS APÓS CORREÇÃO:' AS Status;

SELECT 
    item_id,
    item_name,
    item_type,
    item_subtype,
    equipment_slot,
    required_level,
    CASE 
        WHEN equipment_slot = 'none' OR equipment_slot IS NULL THEN '❌ NÃO EQUIPÁVEL'
        WHEN required_level IS NULL OR required_level = 0 THEN '⚠️ SEM NÍVEL REQUERIDO'
        ELSE '✅ PODE SER EQUIPADO'
    END AS status_equipamento
FROM item_templates
ORDER BY item_id;

-- ============================================
-- 5. VERIFICAR NÍVEL DOS JOGADORES
-- ============================================
SELECT 
    'NÍVEIS DOS JOGADORES:' AS Status;

SELECT 
    id AS player_id,
    character_name,
    level,
    CASE 
        WHEN level IS NULL OR level = 0 THEN '⚠️ DEFINIR NÍVEL MÍNIMO'
        ELSE '✅ OK'
    END AS status_level
FROM players
ORDER BY id;

-- Se algum jogador tiver nível NULL ou 0, definir como 1
UPDATE players
SET level = 1
WHERE level IS NULL OR level = 0;

-- ============================================
-- 6. RESUMO FINAL
-- ============================================
SELECT 
    'RESUMO FINAL:' AS Status;

SELECT 
    COUNT(*) AS total_itens,
    SUM(CASE WHEN equipment_slot != 'none' AND equipment_slot IS NOT NULL THEN 1 ELSE 0 END) AS itens_equipaveis,
    SUM(CASE WHEN equipment_slot = 'none' OR equipment_slot IS NULL THEN 1 ELSE 0 END) AS itens_nao_equipaveis
FROM item_templates;

