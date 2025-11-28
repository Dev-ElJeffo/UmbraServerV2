-- Script para atualizar dados de teste do player_id = 1
-- Adiciona: Classe Barbarian, Facção Novarra, PvP stats
-- Execute este script no MySQL Workbench ou phpMyAdmin

USE umbra_eternum;

-- Passo 1: Atualizar classe, facção e stats PvP
UPDATE players p
SET 
    p.class_id = (SELECT class_id FROM classes WHERE class_name = 'Barbarian' LIMIT 1),
    p.faction_id = (SELECT faction_id FROM factions WHERE faction_name = 'Novarra' LIMIT 1),
    p.selected_class = (SELECT class_id FROM classes WHERE class_name = 'Barbarian' LIMIT 1),
    p.pvp = 50,
    p.honor = 1000,
    p.chaos = 1
WHERE p.id = 1;

-- Passo 2: Atualizar stats base do player com stats da classe
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

-- Verificar se foi atualizado corretamente
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
    p.health,
    p.max_health,
    p.mana,
    p.max_mana,
    p.stamina,
    p.max_stamina,
    p.pvp,
    p.honor,
    p.chaos,
    c.base_strength as class_strength,
    c.base_dexterity as class_dexterity,
    c.base_health as class_health
FROM players p
LEFT JOIN classes c ON p.class_id = c.class_id
LEFT JOIN factions f ON p.faction_id = f.faction_id
WHERE p.id = 1;

-- Deve retornar:
-- id: 4
-- character_name: (nome do personagem)
-- class_name: Barbarian
-- faction_name: Novarra
-- selected_class: 1 (ou o ID da classe Barbarian)
-- pvp: 50
-- honor: 1000
-- chaos: 1

