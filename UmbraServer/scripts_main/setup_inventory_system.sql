-- ============================================
-- UMBRA ETERNUM - SISTEMA DE INVENTÁRIO
-- ============================================
-- Este script cria as tabelas necessárias para o sistema de inventário
-- Execute este script após o setup_database.sql

USE umbra_eternum;

-- ============================================
-- TABELA: item_templates
-- ============================================
-- Armazena os templates/definições de todos os itens do jogo
CREATE TABLE IF NOT EXISTS item_templates (
    item_id INT PRIMARY KEY AUTO_INCREMENT,
    item_name VARCHAR(100) NOT NULL,
    item_description TEXT,
    item_type ENUM('weapon', 'armor', 'consumable', 'material', 'quest', 'misc') NOT NULL DEFAULT 'misc',
    item_subtype VARCHAR(50) COMMENT 'Ex: sword, helmet, health_potion, ore, etc.',
    icon_path VARCHAR(255) COMMENT 'Caminho para o ícone do item no Content Browser',
    
    -- Propriedades de empilhamento
    max_stack_size INT NOT NULL DEFAULT 1 COMMENT 'Quantidade máxima em um slot (1 = não empilhável)',
    
    -- Propriedades de equipamento
    equipment_slot ENUM('head', 'chest', 'legs', 'feet', 'hands', 'main_hand', 'off_hand', 'ring', 'amulet', 'none') DEFAULT 'none',
    required_level INT DEFAULT 1 COMMENT 'Nível mínimo para usar o item',
    
    -- Estatísticas (JSON para flexibilidade)
    stats_json JSON COMMENT 'Estatísticas do item: {"damage": 10, "defense": 5, "health_restore": 50, etc.}',
    
    -- Propriedades visuais/economia
    rarity ENUM('common', 'uncommon', 'rare', 'epic', 'legendary') DEFAULT 'common',
    value INT DEFAULT 0 COMMENT 'Valor em moeda do jogo para venda/compra',
    weight FLOAT DEFAULT 0.0 COMMENT 'Peso do item (para sistema de carga)',
    
    -- Timestamps
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    -- Índices
    INDEX idx_item_type (item_type),
    INDEX idx_item_subtype (item_subtype),
    INDEX idx_rarity (rarity)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================
-- TABELA: player_inventory
-- ============================================
-- Armazena as instâncias de itens no inventário de cada jogador
CREATE TABLE IF NOT EXISTS player_inventory (
    inventory_id INT PRIMARY KEY AUTO_INCREMENT,
    player_id BIGINT UNSIGNED NOT NULL,
    item_template_id INT NOT NULL,
    
    -- Quantidade e posição
    quantity INT NOT NULL DEFAULT 1 COMMENT 'Quantidade do item (para itens empilháveis)',
    slot_index INT NOT NULL COMMENT 'Índice do slot no inventário (0-49 para inventário de 50 slots)',
    auction_listing_id INT UNSIGNED NULL DEFAULT NULL COMMENT 'Anúncio ativo no mercado; NULL = item na bolsa',
    
    -- Estado do item
    is_equipped BOOLEAN DEFAULT FALSE COMMENT 'Se o item está equipado',
    durability FLOAT DEFAULT 100.0 COMMENT 'Durabilidade do item (100 = novo, 0 = quebrado)',
    
    -- Propriedades únicas do item (JSON para flexibilidade)
    custom_properties JSON COMMENT 'Propriedades únicas desta instância: {"enchantment": "fire", "bonus_stats": {}, etc.}',
    
    -- Timestamps
    acquired_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'Quando o item foi adquirido',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    -- Chaves estrangeiras
    FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE,
    FOREIGN KEY (item_template_id) REFERENCES item_templates(item_id) ON DELETE RESTRICT,
    
    -- Índices
    INDEX idx_player_inventory (player_id),
    INDEX idx_slot_index (player_id, slot_index),
    INDEX idx_player_inventory_auction (auction_listing_id),
    INDEX idx_equipped (player_id, is_equipped),
    
    -- Constraint: Um jogador não pode ter dois itens no mesmo slot
    UNIQUE KEY unique_player_slot (player_id, slot_index)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================
-- INSERIR ITENS DE EXEMPLO/TESTE
-- ============================================

-- Armas
INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, equipment_slot, max_stack_size, stats_json, rarity, value, required_level, icon_path) VALUES
('Espada de Ferro', 'Uma espada comum feita de ferro.', 'weapon', 'sword', 'main_hand', 1, '{"damage": 15, "attack_speed": 1.2}', 'common', 50, 1, '/Game/UI/Icons/Items/ICO_IronSword'),
('Espada Flamejante', 'Uma espada lendária envolta em chamas eternas.', 'weapon', 'sword', 'main_hand', 1, '{"damage": 50, "attack_speed": 1.5, "fire_damage": 25}', 'legendary', 5000, 20, '/Game/UI/Icons/Items/ICO_FlameSword'),
('Arco Longo', 'Um arco de madeira resistente.', 'weapon', 'bow', 'main_hand', 1, '{"damage": 20, "attack_speed": 0.8, "range": 30}', 'uncommon', 120, 5, '/Game/UI/Icons/Items/ICO_LongBow');

-- Armaduras
INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, equipment_slot, max_stack_size, stats_json, rarity, value, required_level, icon_path) VALUES
('Capacete de Couro', 'Um capacete simples de couro.', 'armor', 'helmet', 'head', 1, '{"defense": 5, "health": 10}', 'common', 30, 1, '/Game/UI/Icons/Items/ICO_LeatherHelmet'),
('Peitoral de Aço', 'Uma armadura de aço que protege o torso.', 'armor', 'chestplate', 'chest', 1, '{"defense": 25, "health": 50}', 'rare', 500, 10, '/Game/UI/Icons/Items/ICO_SteelChestplate'),
('Botas Élficas', 'Botas leves que aumentam a velocidade.', 'armor', 'boots', 'feet', 1, '{"defense": 8, "speed": 15}', 'epic', 800, 15, '/Game/UI/Icons/Items/ICO_ElvenBoots');

-- Consumíveis
INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, equipment_slot, max_stack_size, stats_json, rarity, value, required_level, icon_path) VALUES
('Poção de Vida Menor', 'Restaura 50 pontos de vida.', 'consumable', 'health_potion', 'none', 20, '{"health_restore": 50}', 'common', 10, 1, '/Game/UI/Icons/Items/ICO_HealthPotionSmall'),
('Poção de Vida Maior', 'Restaura 150 pontos de vida.', 'consumable', 'health_potion', 'none', 10, '{"health_restore": 150}', 'uncommon', 50, 5, '/Game/UI/Icons/Items/ICO_HealthPotionLarge'),
('Poção de Mana', 'Restaura 100 pontos de mana.', 'consumable', 'mana_potion', 'none', 20, '{"mana_restore": 100}', 'common', 15, 1, '/Game/UI/Icons/Items/ICO_ManaPotion'),
('Elixir de Força', 'Aumenta força em 20% por 5 minutos.', 'consumable', 'buff_potion', 'none', 5, '{"strength_buff": 20, "duration": 300}', 'rare', 100, 10, '/Game/UI/Icons/Items/ICO_StrengthElixir');

-- Materiais
INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, equipment_slot, max_stack_size, stats_json, rarity, value, required_level, icon_path) VALUES
('Minério de Ferro', 'Minério bruto de ferro. Pode ser fundido.', 'material', 'ore', 'none', 99, '{}', 'common', 5, 1, '/Game/UI/Icons/Items/ICO_IronOre'),
('Cristal de Mana', 'Um cristal pulsante de energia mágica.', 'material', 'crystal', 'none', 50, '{}', 'rare', 50, 5, '/Game/UI/Icons/Items/ICO_ManaCrystal'),
('Couro de Lobo', 'Couro de boa qualidade. Útil para artesanato.', 'material', 'leather', 'none', 50, '{}', 'common', 8, 1, '/Game/UI/Icons/Items/ICO_WolfLeather');

-- Itens de Quest
INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, equipment_slot, max_stack_size, stats_json, rarity, value, required_level, icon_path) VALUES
('Medalhão Antigo', 'Um medalhão misterioso com símbolos desconhecidos.', 'quest', 'key_item', 'none', 1, '{}', 'epic', 0, 1, '/Game/UI/Icons/Items/ICO_AncientMedallion'),
('Chave da Torre', 'Chave ornamentada que abre a Torre de Umbra.', 'quest', 'key_item', 'none', 1, '{}', 'uncommon', 0, 1, '/Game/UI/Icons/Items/ICO_TowerKey');

-- Diversos
INSERT INTO item_templates (item_name, item_description, item_type, item_subtype, equipment_slot, max_stack_size, stats_json, rarity, value, required_level, icon_path) VALUES
('Moeda de Ouro', 'Moeda de ouro padrão do reino.', 'misc', 'currency', 'none', 999, '{}', 'common', 1, 1, '/Game/UI/Icons/Items/ICO_GoldCoin'),
('Gema Brilhante', 'Uma gema valiosa. Pode ser vendida por um bom preço.', 'misc', 'gem', 'none', 99, '{}', 'rare', 200, 1, '/Game/UI/Icons/Items/ICO_BrightGem');

-- ============================================
-- INSERIR INVENTÁRIO DE TESTE PARA JOGADOR 1
-- ============================================
-- Dar alguns itens iniciais ao primeiro jogador (player_id = 1)
-- Slots 0-9: Barra rápida
-- Slots 10-49: Inventário principal

-- Barra rápida (slots 0-9)
INSERT INTO player_inventory (player_id, item_template_id, quantity, slot_index, is_equipped) VALUES
(1, 1, 1, 0, FALSE),  -- Espada de Ferro no slot 0
(1, 7, 5, 1, FALSE),  -- 5x Poção de Vida Menor no slot 1
(1, 9, 3, 2, FALSE);  -- 3x Poção de Mana no slot 2

-- Inventário principal (slots 10-49)
INSERT INTO player_inventory (player_id, item_template_id, quantity, slot_index, is_equipped) VALUES
(1, 4, 1, 10, FALSE),  -- Capacete de Couro
(1, 5, 1, 11, FALSE),  -- Peitoral de Aço
(1, 11, 20, 12, FALSE), -- 20x Minério de Ferro
(1, 13, 10, 13, FALSE), -- 10x Couro de Lobo
(1, 16, 50, 14, FALSE); -- 50x Moeda de Ouro

-- ============================================
-- CONSULTAS ÚTEIS PARA TESTES
-- ============================================

-- Ver todos os templates de itens por tipo
-- SELECT * FROM item_templates WHERE item_type = 'weapon';
-- SELECT * FROM item_templates WHERE item_type = 'armor';
-- SELECT * FROM item_templates WHERE item_type = 'consumable';

-- Ver inventário completo de um jogador com informações dos itens
-- SELECT 
--     pi.inventory_id,
--     pi.slot_index,
--     pi.quantity,
--     pi.is_equipped,
--     pi.durability,
--     it.item_name,
--     it.item_type,
--     it.item_description,
--     it.rarity,
--     it.stats_json
-- FROM player_inventory pi
-- JOIN item_templates it ON pi.item_template_id = it.item_id
-- WHERE pi.player_id = 1
-- ORDER BY pi.slot_index;

-- Ver todos os itens equipados de um jogador
-- SELECT 
--     it.item_name,
--     it.equipment_slot,
--     pi.durability
-- FROM player_inventory pi
-- JOIN item_templates it ON pi.item_template_id = it.item_id
-- WHERE pi.player_id = 1 AND pi.is_equipped = TRUE;

-- ============================================
-- FIM DO SCRIPT
-- ============================================

