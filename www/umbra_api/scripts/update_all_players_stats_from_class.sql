-- Script para atualizar stats base de TODOS os players baseado em suas classes
-- Útil para sincronizar stats após atribuir classes
-- Execute este script no MySQL Workbench ou phpMyAdmin

USE umbra_eternum;

-- Atualizar todos os players que têm uma classe atribuída
UPDATE players p
INNER JOIN classes c ON p.class_id = c.class_id
SET 
    p.strength = c.base_strength,
    p.dexterity = c.base_dexterity,
    p.intelligence = c.base_intelligence,
    p.vitality = c.base_vitality,
    p.luck = c.base_luck,
    p.max_health = c.base_health,
    p.health = c.base_health,
    p.max_mana = c.base_mana,
    p.mana = c.base_mana,
    p.max_stamina = c.base_stamina,
    p.stamina = c.base_stamina
WHERE p.class_id IS NOT NULL;

-- Verificar quantos players foram atualizados
SELECT 
    COUNT(*) as players_updated
FROM players
WHERE class_id IS NOT NULL;

-- Verificar alguns exemplos
SELECT 
    p.id,
    p.character_name,
    c.class_name,
    p.strength,
    p.dexterity,
    p.intelligence,
    p.vitality,
    p.luck,
    p.max_health,
    p.max_mana,
    p.max_stamina
FROM players p
INNER JOIN classes c ON p.class_id = c.class_id
ORDER BY p.id
LIMIT 10;

