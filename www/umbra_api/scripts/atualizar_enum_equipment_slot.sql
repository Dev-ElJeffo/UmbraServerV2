-- ============================================
-- ATUALIZAR ENUM equipment_slot
-- ============================================
-- Este script atualiza o ENUM da coluna equipment_slot
-- para incluir todos os slots do C++ (necklace, earring, bracelet, mount)

USE umbra_eternum;

-- ============================================
-- 1. VERIFICAR ENUM ATUAL
-- ============================================
SELECT 
    'ENUM atual da coluna equipment_slot:' AS Status;

SHOW COLUMNS FROM item_templates WHERE Field = 'equipment_slot';

-- ============================================
-- 2. ATUALIZAR ENUM PARA INCLUIR TODOS OS SLOTS
-- ============================================
-- IMPORTANTE: MySQL não permite ALTER ENUM diretamente se houver valores existentes
-- Precisamos modificar a coluna para VARCHAR temporariamente, depois voltar para ENUM

-- Passo 1: Converter para VARCHAR temporariamente
ALTER TABLE item_templates 
MODIFY COLUMN equipment_slot VARCHAR(20) 
COMMENT 'Slot de equipamento: none, head, chest, legs, feet, hands, main_hand, off_hand, ring, amulet, necklace, earring, bracelet, mount';

-- Passo 2: Converter de volta para ENUM com todos os valores
ALTER TABLE item_templates 
MODIFY COLUMN equipment_slot ENUM(
    'none',
    'head',
    'chest',
    'legs',
    'feet',
    'hands',
    'main_hand',
    'off_hand',
    'ring',
    'amulet',
    'necklace',
    'earring',
    'bracelet',
    'mount'
) DEFAULT 'none' 
COMMENT 'Slot de equipamento';

-- ============================================
-- 3. VERIFICAR ENUM ATUALIZADO
-- ============================================
SELECT 
    'ENUM atualizado:' AS Status;

SHOW COLUMNS FROM item_templates WHERE Field = 'equipment_slot';

-- ============================================
-- 4. VERIFICAR SE HÁ VALORES INVÁLIDOS
-- ============================================
SELECT 
    'Valores inválidos (se houver):' AS Status;

SELECT 
    item_id,
    item_name,
    equipment_slot,
    CASE 
        WHEN equipment_slot NOT IN ('none', 'head', 'chest', 'legs', 'feet', 'hands', 'main_hand', 'off_hand', 'ring', 'amulet', 'necklace', 'earring', 'bracelet', 'mount') 
        THEN '❌ VALOR INVÁLIDO'
        ELSE '✅ OK'
    END AS status
FROM item_templates
WHERE equipment_slot NOT IN ('none', 'head', 'chest', 'legs', 'feet', 'hands', 'main_hand', 'off_hand', 'ring', 'amulet', 'necklace', 'earring', 'bracelet', 'mount')
OR equipment_slot IS NULL;

