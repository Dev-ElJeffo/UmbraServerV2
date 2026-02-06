-- Script para corrigir itens equipados que ainda ocupam slots no inventário
-- Executar uma vez para limpar dados inconsistentes
-- 
-- IMPORTANTE: Itens equipados usam slot_index = -inventory_id para garantir unicidade
-- (evita conflito com a constraint unique_player_slot)

-- Mostrar itens afetados antes da correção
SELECT 
    pi.inventory_id,
    pi.player_id,
    pi.slot_index,
    pi.is_equipped,
    it.item_name,
    it.equipment_slot
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE AND pi.slot_index >= 0;

-- Desabilitar safe mode temporariamente
SET SQL_SAFE_UPDATES = 0;

-- Atualizar: itens equipados devem ter slot_index = -inventory_id (negativo único)
-- Isso garante que cada item equipado tenha um slot_index único
UPDATE player_inventory 
SET slot_index = -inventory_id 
WHERE is_equipped = TRUE AND slot_index >= 0;

-- Reabilitar safe mode
SET SQL_SAFE_UPDATES = 1;

-- Verificar resultado
SELECT 
    pi.inventory_id,
    pi.player_id,
    pi.slot_index,
    pi.is_equipped,
    it.item_name
FROM player_inventory pi
INNER JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.is_equipped = TRUE;

-- Verificar se ainda há inconsistências (itens equipados com slot >= 0)
SELECT 
    COUNT(*) as inconsistencias_restantes
FROM player_inventory 
WHERE is_equipped = TRUE AND slot_index >= 0;
