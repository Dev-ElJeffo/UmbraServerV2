-- ============================================
-- VERIFICAR REQUISITOS PARA EQUIPAR ITENS
-- ============================================
-- Este script verifica se os itens no banco de dados
-- preenchem os requisitos para serem equipados

USE umbra_eternum;

-- ============================================
-- 1. VERIFICAR ESTRUTURA DA TABELA item_templates
-- ============================================
SELECT 
    'Verificando estrutura da tabela item_templates...' AS Status;

DESCRIBE item_templates;

-- ============================================
-- 2. VERIFICAR ITENS NO BANCO
-- ============================================
SELECT 
    'Itens no banco de dados:' AS Status;

SELECT 
    item_id,
    item_name,
    item_type,
    equipment_slot,
    required_level,
    rarity,
    CASE 
        WHEN equipment_slot = 'none' OR equipment_slot IS NULL THEN '❌ NÃO EQUIPÁVEL'
        WHEN required_level IS NULL OR required_level = 0 THEN '⚠️ SEM NÍVEL REQUERIDO'
        ELSE '✅ PODE SER EQUIPADO'
    END AS status_equipamento
FROM item_templates
ORDER BY item_id;

-- ============================================
-- 3. VERIFICAR ITENS NO INVENTÁRIO DO JOGADOR
-- ============================================
SELECT 
    'Itens no inventário do jogador:' AS Status;

SELECT 
    pi.inventory_id,
    pi.player_id,
    pi.slot_index,
    pi.quantity,
    pi.is_equipped,
    it.item_name,
    it.equipment_slot,
    it.required_level,
    p.level AS player_level,
    CASE 
        WHEN it.equipment_slot = 'none' OR it.equipment_slot IS NULL THEN '❌ NÃO EQUIPÁVEL'
        WHEN p.level < it.required_level THEN CONCAT('⚠️ NÍVEL INSUFICIENTE (Requer: ', it.required_level, ', Jogador: ', p.level, ')')
        WHEN pi.is_equipped = 1 THEN '✅ JÁ EQUIPADO'
        ELSE '✅ PODE SER EQUIPADO'
    END AS status_equipamento
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
INNER JOIN players p ON pi.player_id = p.id
ORDER BY pi.player_id, pi.slot_index;

-- ============================================
-- 4. VERIFICAR NÍVEL DO JOGADOR
-- ============================================
SELECT 
    'Níveis dos jogadores:' AS Status;

SELECT 
    id AS player_id,
    character_name,
    level,
    CASE 
        WHEN level IS NULL OR level = 0 THEN '⚠️ NÍVEL INVÁLIDO'
        ELSE '✅ OK'
    END AS status_level
FROM players
ORDER BY id;

-- ============================================
-- 5. RESUMO DE PROBLEMAS
-- ============================================
SELECT 
    'RESUMO DE PROBLEMAS:' AS Status;

-- Itens não equipáveis
SELECT 
    'Itens não equipáveis (equipment_slot = none):' AS Problema,
    COUNT(*) AS Quantidade
FROM item_templates
WHERE equipment_slot = 'none' OR equipment_slot IS NULL;

-- Itens sem nível requerido
SELECT 
    'Itens sem nível requerido:' AS Problema,
    COUNT(*) AS Quantidade
FROM item_templates
WHERE (required_level IS NULL OR required_level = 0) 
AND (equipment_slot != 'none' AND equipment_slot IS NOT NULL);

-- Itens no inventário que não podem ser equipados
SELECT 
    'Itens no inventário que não podem ser equipados:' AS Problema,
    COUNT(*) AS Quantidade
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE it.equipment_slot = 'none' OR it.equipment_slot IS NULL;

-- Itens no inventário com nível insuficiente
SELECT 
    'Itens no inventário com nível insuficiente:' AS Problema,
    COUNT(*) AS Quantidade
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
INNER JOIN players p ON pi.player_id = p.id
WHERE (it.equipment_slot != 'none' AND it.equipment_slot IS NOT NULL)
AND p.level < it.required_level;

