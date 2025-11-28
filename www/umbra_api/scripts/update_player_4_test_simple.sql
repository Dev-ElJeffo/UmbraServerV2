-- Script SIMPLES para atualizar player_id = 1 com dados de teste
-- Execute este script no MySQL Workbench

USE umbra_eternum;

-- Passo 1: Atualizar classe, facção e stats PvP
UPDATE players
SET 
    class_id = 1,  -- Barbarian (primeira classe inserida)
    faction_id = 1,  -- Novarra (primeira facção inserida)
    selected_class = 1,  -- Barbarian
    pvp = 50,
    honor = 1000,
    chaos = 1
WHERE id = 1;

-- Passo 2: Atualizar stats base do player com stats da classe Barbarian
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
WHERE p.id = 1;

-- Verificar resultado completo
SELECT 
    p.id,
    p.character_name,
    c.class_name,
    f.faction_name,
    p.strength,
    p.dexterity,
    p.intelligence,
    p.vitality,
    p.luck,
    p.max_health,
    p.max_mana,
    p.max_stamina,
    p.pvp,
    p.honor,
    p.chaos
FROM players p
LEFT JOIN classes c ON p.class_id = c.class_id
LEFT JOIN factions f ON p.faction_id = f.faction_id
WHERE p.id = 1;

