-- Script para atualizar stats base do player baseado na classe
-- Atualiza: strength, dexterity, intelligence, vitality, luck, health, mana, stamina
-- Execute este script no MySQL Workbench ou phpMyAdmin

USE umbra_eternum;

-- Atualizar player_id = 1 com stats do Barbarian
UPDATE players p
INNER JOIN classes c ON p.class_id = c.class_id
SET 
    p.strength = c.base_strength,
    p.dexterity = c.base_dexterity,
    p.intelligence = c.base_intelligence,
    p.vitality = c.base_vitality,
    p.luck = c.base_luck,
    p.max_health = c.base_health,
    p.health = c.base_health,  -- Atualizar health atual também
    p.max_mana = c.base_mana,
    p.mana = c.base_mana,  -- Atualizar mana atual também
    p.max_stamina = c.base_stamina,
    p.stamina = c.base_stamina  -- Atualizar stamina atual também
WHERE p.id = 1;

-- Verificar se foi atualizado corretamente
SELECT 
    p.id,
    p.character_name,
    c.class_name,
    p.strength,
    p.dexterity,
    p.intelligence,
    p.vitality,
    p.luck,
    p.health,
    p.max_health,
    p.mana,
    p.max_mana,
    p.stamina,
    p.max_stamina,
    c.base_strength,
    c.base_dexterity,
    c.base_intelligence,
    c.base_vitality,
    c.base_luck,
    c.base_health,
    c.base_mana,
    c.base_stamina
FROM players p
INNER JOIN classes c ON p.class_id = c.class_id
WHERE p.id = 1;

-- Os valores de p.* devem corresponder aos valores de c.base_*

