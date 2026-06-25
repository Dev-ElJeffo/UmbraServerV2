-- Poções de buff de exemplo (ids 10-16)
-- stats_json: {"duration": <segundos>, "<stat>_buff": <valor flat>}
-- use_cooldown_ms curto (1s) para evitar double-click; duração do efeito vem de "duration"

-- Elixir de Força (id 10 — pode já existir no setup_inventory_system.sql)
INSERT INTO item_templates (
    item_id, item_name, item_description, item_type, item_subtype,
    equipment_slot, max_stack_size, stats_json, rarity, value, required_level,
    icon_path, use_cooldown_ms
) VALUES (
    10, 'Elixir de Força', 'Aumenta força em +20 por 5 minutos.', 'consumable', 'buff_potion',
    'none', 5, '{"strength_buff": 20, "duration": 300}', 'rare', 100, 10,
    '/Game/UI/Icons/Items/ICO_StrengthElixir', 1000
) ON DUPLICATE KEY UPDATE
    item_name = VALUES(item_name),
    item_description = VALUES(item_description),
    item_type = VALUES(item_type),
    item_subtype = VALUES(item_subtype),
    stats_json = VALUES(stats_json),
    icon_path = VALUES(icon_path),
    use_cooldown_ms = VALUES(use_cooldown_ms);

INSERT INTO item_templates (
    item_id, item_name, item_description, item_type, item_subtype,
    equipment_slot, max_stack_size, stats_json, rarity, value, required_level,
    icon_path, use_cooldown_ms
) VALUES
(11, 'Elixir de Destreza', 'Aumenta destreza em +20 por 5 minutos.', 'consumable', 'buff_potion',
 'none', 5, '{"dexterity_buff": 20, "duration": 300}', 'rare', 100, 10,
 '/Game/UI/Icons/Items/ICO_DexterityElixir', 1000),
(12, 'Elixir de Inteligência', 'Aumenta inteligência em +20 por 5 minutos.', 'consumable', 'buff_potion',
 'none', 5, '{"intelligence_buff": 20, "duration": 300}', 'rare', 100, 10,
 '/Game/UI/Icons/Items/ICO_IntelligenceElixir', 1000),
(13, 'Elixir de Vitalidade', 'Aumenta vitalidade em +20 por 5 minutos.', 'consumable', 'buff_potion',
 'none', 5, '{"vitality_buff": 20, "duration": 300}', 'rare', 100, 10,
 '/Game/UI/Icons/Items/ICO_VitalityElixir', 1000),
(14, 'Elixir de Sorte', 'Aumenta sorte em +20 por 5 minutos.', 'consumable', 'buff_potion',
 'none', 5, '{"luck_buff": 20, "duration": 300}', 'rare', 100, 10,
 '/Game/UI/Icons/Items/ICO_LuckElixir', 1000),
(15, 'Tônico do Guerreiro', 'Aumenta ataque físico em +30 por 3 minutos.', 'consumable', 'buff_potion',
 'none', 5, '{"attack_buff": 30, "duration": 180}', 'uncommon', 75, 5,
 '/Game/UI/Icons/Items/ICO_WarriorTonic', 1000),
(16, 'Tônico da Defesa', 'Aumenta defesa física em +30 por 3 minutos.', 'consumable', 'buff_potion',
 'none', 5, '{"defense_buff": 30, "duration": 180}', 'uncommon', 75, 5,
 '/Game/UI/Icons/Items/ICO_DefenseTonic', 1000)
ON DUPLICATE KEY UPDATE
    item_name = VALUES(item_name),
    item_description = VALUES(item_description),
    item_subtype = VALUES(item_subtype),
    stats_json = VALUES(stats_json),
    icon_path = VALUES(icon_path),
    use_cooldown_ms = VALUES(use_cooldown_ms);
