-- ============================================
-- UMBRA ETERNUM - SISTEMA DE REFINAÇÃO
-- ============================================
-- Este script adiciona suporte completo para refinação de itens
-- Versão: 1.0
-- Data: 2026-04-27

USE umbra_eternum;

-- ============================================
-- PARTE 1: ALTERAÇÕES NA TABELA item_templates
-- ============================================

-- Adicionar campo can_be_refined (se item pode ser refinado)
-- Ignorar erro se coluna já existir
ALTER TABLE item_templates
ADD COLUMN can_be_refined BOOLEAN DEFAULT FALSE COMMENT 'Se o item pode ser refinado de +0 a +12';

-- Adicionar campo tradeable (se item pode ser negociado)
ALTER TABLE item_templates
ADD COLUMN tradeable BOOLEAN DEFAULT TRUE COMMENT 'Se o item pode ser negociado (loja/leilão/trade)';

-- Adicionar campo item_category (categoria do item)
ALTER TABLE item_templates
ADD COLUMN item_category ENUM('equipment', 'consumable', 'material', 'upgrade', 'quest', 'misc') DEFAULT 'misc' COMMENT 'Categoria do item';

-- Criar índices para melhor performance (ignorar erro se já existir)
ALTER TABLE item_templates ADD INDEX idx_can_be_refined (can_be_refined);
ALTER TABLE item_templates ADD INDEX idx_tradeable (tradeable);
ALTER TABLE item_templates ADD INDEX idx_item_category (item_category);

-- ============================================
-- PARTE 2: ATUALIZAR ITENS EXISTENTES
-- ============================================

-- Marcar armas e armaduras como refináveis e equipment
UPDATE item_templates 
SET can_be_refined = TRUE, item_category = 'equipment'
WHERE item_type IN ('weapon', 'armor') AND equipment_slot != 'none';

-- Definir categoria para outros tipos
UPDATE item_templates SET item_category = 'consumable' WHERE item_type = 'consumable';
UPDATE item_templates SET item_category = 'material' WHERE item_type = 'material';
UPDATE item_templates SET item_category = 'quest' WHERE item_type = 'quest';
UPDATE item_templates SET item_category = 'misc' WHERE item_type = 'misc';

-- ============================================
-- PARTE 3: ALTERAÇÕES NA TABELA player_inventory
-- ============================================

-- Adicionar campo refinement_level (nível de refinação +0 a +12)
ALTER TABLE player_inventory
ADD COLUMN refinement_level TINYINT UNSIGNED DEFAULT 0 COMMENT 'Nível de refinação (+0 a +12)';

-- Adicionar campo refinement_bonus_stats (stats bônus da refinação em JSON)
ALTER TABLE player_inventory
ADD COLUMN refinement_bonus_stats JSON COMMENT 'Stats bônus calculados da refinação (ex: {"attack": 5, "defense": 2})';

-- Criar índice para refinement_level
ALTER TABLE player_inventory ADD INDEX idx_refinement_level (refinement_level);

-- ============================================
-- PARTE 4: CRIAR TABELA refinement_config
-- ============================================

CREATE TABLE IF NOT EXISTS refinement_config (
    refinement_level TINYINT UNSIGNED PRIMARY KEY COMMENT 'Nível de refinação atual (+0 a +12)',
    success_rate FLOAT NOT NULL COMMENT 'Taxa de sucesso (0.0 a 1.0)',
    required_item_id INT NOT NULL COMMENT 'ID do item de refinação necessário',
    required_item_quantity INT NOT NULL DEFAULT 1 COMMENT 'Quantidade do item necessária',
    stat_bonus_multiplier FLOAT NOT NULL COMMENT 'Multiplicador de stats (ex: 1.05 = +5%, 1.10 = +10%)',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (required_item_id) REFERENCES item_templates(item_id) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================
-- PARTE 5: CRIAR ITENS DE REFINAÇÃO
-- ============================================

-- Inserir Fragmento de Energia Umbral (usado de +0 a +6)
INSERT INTO item_templates (
    item_name, 
    item_description, 
    item_type, 
    item_subtype, 
    equipment_slot, 
    max_stack_size, 
    icon_path, 
    rarity, 
    value, 
    weight,
    item_category, 
    tradeable,
    can_be_refined
) VALUES (
    'Fragmento de Energia Umbral',
    'Um fragmento cristalizado de energia sombria. Usado para refinar equipamentos de +0 a +6. A energia umbral flui através dele, fortalecendo armas e armaduras.',
    'material',
    'upgrade_material',
    'none',
    999,
    '/Game/UI/Icons/Items/ICO_UmbralFragment',
    'uncommon',
    100,
    0.1,
    'upgrade',
    TRUE,
    FALSE
) ON DUPLICATE KEY UPDATE item_name = item_name;

-- Inserir Pedra de Energia Umbral (usado de +7 a +12)
INSERT INTO item_templates (
    item_name, 
    item_description, 
    item_type, 
    item_subtype, 
    equipment_slot, 
    max_stack_size, 
    icon_path, 
    rarity, 
    value, 
    weight,
    item_category, 
    tradeable,
    can_be_refined
) VALUES (
    'Pedra de Energia Umbral',
    'Uma pedra de energia concentrada, pulsando com poder sombrio. Necessária para refinamentos avançados de +7 a +12. Extremamente rara e valiosa.',
    'material',
    'upgrade_material',
    'none',
    999,
    '/Game/UI/Icons/Items/ICO_UmbralStone',
    'rare',
    500,
    0.2,
    'upgrade',
    TRUE,
    FALSE
) ON DUPLICATE KEY UPDATE item_name = item_name;

-- ============================================
-- PARTE 6: POPULAR TABELA refinement_config
-- ============================================

-- Obter IDs dos itens de refinação criados
SET @fragment_id = (SELECT item_id FROM item_templates WHERE item_name = 'Fragmento de Energia Umbral' LIMIT 1);
SET @stone_id = (SELECT item_id FROM item_templates WHERE item_name = 'Pedra de Energia Umbral' LIMIT 1);

-- Limpar tabela se já existir dados (para reexecução do script)
DELETE FROM refinement_config;

-- Popular configurações de refinação
INSERT INTO refinement_config (refinement_level, success_rate, required_item_id, required_item_quantity, stat_bonus_multiplier) VALUES
-- +0 → +1: 100% sucesso, 1 fragmento, sem bônus ainda
(0, 1.00, @fragment_id, 1, 1.00),

-- +1 → +2: 100% sucesso, 1 fragmento, +5% stats
(1, 1.00, @fragment_id, 1, 1.05),

-- +2 → +3: 100% sucesso, 2 fragmentos, +10% stats
(2, 1.00, @fragment_id, 2, 1.10),

-- +3 → +4: 100% sucesso, 2 fragmentos, +15% stats
(3, 1.00, @fragment_id, 2, 1.15),

-- +4 → +5: 100% sucesso, 3 fragmentos, +20% stats
(4, 1.00, @fragment_id, 3, 1.20),

-- +5 → +6: 100% sucesso, 3 fragmentos, +25% stats
(5, 1.00, @fragment_id, 3, 1.25),

-- +6 → +7: 100% sucesso, 4 fragmentos, +30% stats
(6, 1.00, @fragment_id, 4, 1.30),

-- +7 → +8: 75% sucesso, 1 pedra, +35% stats
(7, 0.75, @stone_id, 1, 1.35),

-- +8 → +9: 60% sucesso, 2 pedras, +40% stats
(8, 0.60, @stone_id, 2, 1.40),

-- +9 → +10: 45% sucesso, 3 pedras, +45% stats
(9, 0.45, @stone_id, 3, 1.45),

-- +10 → +11: 30% sucesso, 4 pedras, +50% stats
(10, 0.30, @stone_id, 4, 1.50),

-- +11 → +12: 20% sucesso, 5 pedras, +55% stats
(11, 0.20, @stone_id, 5, 1.55),

-- +12 é o máximo (esta linha serve apenas como referência)
(12, 0.00, @stone_id, 0, 1.60);

-- ============================================
-- PARTE 7: VERIFICAÇÕES E RELATÓRIO
-- ============================================

-- Mostrar itens refináveis
SELECT 
    'Itens Refináveis' AS Categoria,
    COUNT(*) AS Total
FROM item_templates
WHERE can_be_refined = TRUE;

-- Mostrar itens de refinação
SELECT 
    'Itens de Refinação Criados' AS Status,
    item_id,
    item_name,
    rarity,
    max_stack_size
FROM item_templates
WHERE item_category = 'upgrade';

-- Mostrar configuração de refinação
SELECT 
    CONCAT('+', refinement_level, ' → +', refinement_level + 1) AS Transicao,
    CONCAT(ROUND(success_rate * 100, 0), '%') AS Taxa_Sucesso,
    required_item_quantity AS Qtd_Material,
    CONCAT('+', ROUND((stat_bonus_multiplier - 1.0) * 100, 0), '%') AS Bonus_Stats,
    it.item_name AS Material_Necessario
FROM refinement_config rc
JOIN item_templates it ON rc.required_item_id = it.item_id
WHERE rc.refinement_level < 12
ORDER BY rc.refinement_level;

-- ============================================
-- CONSULTAS ÚTEIS PARA DESENVOLVIMENTO
-- ============================================

-- Verificar estrutura da tabela item_templates
-- DESCRIBE item_templates;

-- Verificar estrutura da tabela player_inventory
-- DESCRIBE player_inventory;

-- Verificar estrutura da tabela refinement_config
-- DESCRIBE refinement_config;

-- Testar refinação (exemplo - NÃO EXECUTAR EM PRODUÇÃO)
-- UPDATE player_inventory 
-- SET refinement_level = 5, 
--     refinement_bonus_stats = '{"attack": 10, "defense": 5}'
-- WHERE inventory_id = 1;

-- Ver itens refinados de um jogador
-- SELECT 
--     pi.inventory_id,
--     it.item_name,
--     CONCAT(it.item_name, ' +', pi.refinement_level) AS nome_completo,
--     pi.refinement_level,
--     pi.refinement_bonus_stats
-- FROM player_inventory pi
-- JOIN item_templates it ON pi.item_template_id = it.item_id
-- WHERE pi.player_id = 1 AND pi.refinement_level > 0;

-- ============================================
-- FIM DO SCRIPT
-- ============================================

SELECT 'Script de refinação executado com sucesso!' AS Status;
