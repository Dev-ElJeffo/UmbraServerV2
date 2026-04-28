-- ============================================
-- UMBRA ETERNUM - SISTEMA DE REFINAÇÃO (VERSÃO SEGURA)
-- ============================================
-- Esta versão verifica se colunas já existem antes de adicionar
-- Pode ser executada múltiplas vezes sem erros
-- Versão: 1.0 (Safe)
-- Data: 2026-04-27

USE umbra_eternum;

DELIMITER $$

-- ============================================
-- PARTE 1: ALTERAÇÕES NA TABELA item_templates
-- ============================================

-- Procedure para adicionar can_be_refined
DROP PROCEDURE IF EXISTS add_can_be_refined$$
CREATE PROCEDURE add_can_be_refined()
BEGIN
    IF NOT EXISTS (
        SELECT * FROM information_schema.COLUMNS 
        WHERE TABLE_SCHEMA = 'umbra_eternum' 
        AND TABLE_NAME = 'item_templates' 
        AND COLUMN_NAME = 'can_be_refined'
    ) THEN
        ALTER TABLE item_templates
        ADD COLUMN can_be_refined BOOLEAN DEFAULT FALSE COMMENT 'Se o item pode ser refinado de +0 a +12';
        SELECT 'Coluna can_be_refined criada' AS Status;
    ELSE
        SELECT 'Coluna can_be_refined já existe' AS Status;
    END IF;
END$$

-- Procedure para adicionar tradeable
DROP PROCEDURE IF EXISTS add_tradeable$$
CREATE PROCEDURE add_tradeable()
BEGIN
    IF NOT EXISTS (
        SELECT * FROM information_schema.COLUMNS 
        WHERE TABLE_SCHEMA = 'umbra_eternum' 
        AND TABLE_NAME = 'item_templates' 
        AND COLUMN_NAME = 'tradeable'
    ) THEN
        ALTER TABLE item_templates
        ADD COLUMN tradeable BOOLEAN DEFAULT TRUE COMMENT 'Se o item pode ser negociado (loja/leilão/trade)';
        SELECT 'Coluna tradeable criada' AS Status;
    ELSE
        SELECT 'Coluna tradeable já existe' AS Status;
    END IF;
END$$

-- Procedure para adicionar item_category
DROP PROCEDURE IF EXISTS add_item_category$$
CREATE PROCEDURE add_item_category()
BEGIN
    IF NOT EXISTS (
        SELECT * FROM information_schema.COLUMNS 
        WHERE TABLE_SCHEMA = 'umbra_eternum' 
        AND TABLE_NAME = 'item_templates' 
        AND COLUMN_NAME = 'item_category'
    ) THEN
        ALTER TABLE item_templates
        ADD COLUMN item_category ENUM('equipment', 'consumable', 'material', 'upgrade', 'quest', 'misc') DEFAULT 'misc' COMMENT 'Categoria do item';
        SELECT 'Coluna item_category criada' AS Status;
    ELSE
        SELECT 'Coluna item_category já existe' AS Status;
    END IF;
END$$

-- Executar procedures
CALL add_can_be_refined()$$
CALL add_tradeable()$$
CALL add_item_category()$$

-- Limpar procedures
DROP PROCEDURE IF EXISTS add_can_be_refined$$
DROP PROCEDURE IF EXISTS add_tradeable$$
DROP PROCEDURE IF EXISTS add_item_category$$

DELIMITER ;

-- Criar índices (ignorar erros se já existirem)
SET @sql = 'ALTER TABLE item_templates ADD INDEX idx_can_be_refined (can_be_refined)';
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @sql = 'ALTER TABLE item_templates ADD INDEX idx_tradeable (tradeable)';
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @sql = 'ALTER TABLE item_templates ADD INDEX idx_item_category (item_category)';
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- ============================================
-- PARTE 2: ATUALIZAR ITENS EXISTENTES
-- ============================================

-- Marcar armas e armaduras como refináveis e equipment
UPDATE item_templates 
SET can_be_refined = TRUE, item_category = 'equipment'
WHERE item_type IN ('weapon', 'armor') AND equipment_slot != 'none';

-- Definir categoria para outros tipos
UPDATE item_templates SET item_category = 'consumable' WHERE item_type = 'consumable' AND item_category = 'misc';
UPDATE item_templates SET item_category = 'material' WHERE item_type = 'material' AND item_category = 'misc';
UPDATE item_templates SET item_category = 'quest' WHERE item_type = 'quest' AND item_category = 'misc';

-- ============================================
-- PARTE 3: ALTERAÇÕES NA TABELA player_inventory
-- ============================================

DELIMITER $$

-- Procedure para adicionar refinement_level
DROP PROCEDURE IF EXISTS add_refinement_level$$
CREATE PROCEDURE add_refinement_level()
BEGIN
    IF NOT EXISTS (
        SELECT * FROM information_schema.COLUMNS 
        WHERE TABLE_SCHEMA = 'umbra_eternum' 
        AND TABLE_NAME = 'player_inventory' 
        AND COLUMN_NAME = 'refinement_level'
    ) THEN
        ALTER TABLE player_inventory
        ADD COLUMN refinement_level TINYINT UNSIGNED DEFAULT 0 COMMENT 'Nível de refinação (+0 a +12)';
        SELECT 'Coluna refinement_level criada' AS Status;
    ELSE
        SELECT 'Coluna refinement_level já existe' AS Status;
    END IF;
END$$

-- Procedure para adicionar refinement_bonus_stats
DROP PROCEDURE IF EXISTS add_refinement_bonus_stats$$
CREATE PROCEDURE add_refinement_bonus_stats()
BEGIN
    IF NOT EXISTS (
        SELECT * FROM information_schema.COLUMNS 
        WHERE TABLE_SCHEMA = 'umbra_eternum' 
        AND TABLE_NAME = 'player_inventory' 
        AND COLUMN_NAME = 'refinement_bonus_stats'
    ) THEN
        ALTER TABLE player_inventory
        ADD COLUMN refinement_bonus_stats JSON COMMENT 'Stats bônus calculados da refinação';
        SELECT 'Coluna refinement_bonus_stats criada' AS Status;
    ELSE
        SELECT 'Coluna refinement_bonus_stats já existe' AS Status;
    END IF;
END$$

-- Executar procedures
CALL add_refinement_level()$$
CALL add_refinement_bonus_stats()$$

-- Limpar procedures
DROP PROCEDURE IF EXISTS add_refinement_level$$
DROP PROCEDURE IF EXISTS add_refinement_bonus_stats$$

DELIMITER ;

-- Criar índice
SET @sql = 'ALTER TABLE player_inventory ADD INDEX idx_refinement_level (refinement_level)';
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

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
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
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
) 
SELECT * FROM (SELECT
    'Fragmento de Energia Umbral' AS item_name,
    'Um fragmento cristalizado de energia sombria. Usado para refinar equipamentos de +0 a +6. A energia umbral flui através dele, fortalecendo armas e armaduras.' AS item_description,
    'material' AS item_type,
    'upgrade_material' AS item_subtype,
    'none' AS equipment_slot,
    999 AS max_stack_size,
    '/Game/UI/Icons/Items/ICO_UmbralFragment' AS icon_path,
    'uncommon' AS rarity,
    100 AS value,
    0.1 AS weight,
    'upgrade' AS item_category,
    TRUE AS tradeable,
    FALSE AS can_be_refined
) AS tmp
WHERE NOT EXISTS (
    SELECT 1 FROM item_templates WHERE item_name = 'Fragmento de Energia Umbral'
) LIMIT 1;

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
)
SELECT * FROM (SELECT
    'Pedra de Energia Umbral' AS item_name,
    'Uma pedra de energia concentrada, pulsando com poder sombrio. Necessária para refinamentos avançados de +7 a +12. Extremamente rara e valiosa.' AS item_description,
    'material' AS item_type,
    'upgrade_material' AS item_subtype,
    'none' AS equipment_slot,
    999 AS max_stack_size,
    '/Game/UI/Icons/Items/ICO_UmbralStone' AS icon_path,
    'rare' AS rarity,
    500 AS value,
    0.2 AS weight,
    'upgrade' AS item_category,
    TRUE AS tradeable,
    FALSE AS can_be_refined
) AS tmp
WHERE NOT EXISTS (
    SELECT 1 FROM item_templates WHERE item_name = 'Pedra de Energia Umbral'
) LIMIT 1;

-- ============================================
-- PARTE 6: POPULAR TABELA refinement_config
-- ============================================

-- Obter IDs dos itens de refinação criados
SET @fragment_id = (SELECT item_id FROM item_templates WHERE item_name = 'Fragmento de Energia Umbral' LIMIT 1);
SET @stone_id = (SELECT item_id FROM item_templates WHERE item_name = 'Pedra de Energia Umbral' LIMIT 1);

-- Popular configurações de refinação (apenas se ainda não existirem)
INSERT IGNORE INTO refinement_config (refinement_level, success_rate, required_item_id, required_item_quantity, stat_bonus_multiplier) VALUES
(0, 1.00, @fragment_id, 1, 1.00),
(1, 1.00, @fragment_id, 1, 1.05),
(2, 1.00, @fragment_id, 2, 1.10),
(3, 1.00, @fragment_id, 2, 1.15),
(4, 1.00, @fragment_id, 3, 1.20),
(5, 1.00, @fragment_id, 3, 1.25),
(6, 1.00, @fragment_id, 4, 1.30),
(7, 0.75, @stone_id, 1, 1.35),
(8, 0.60, @stone_id, 2, 1.40),
(9, 0.45, @stone_id, 3, 1.45),
(10, 0.30, @stone_id, 4, 1.50),
(11, 0.20, @stone_id, 5, 1.55),
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
-- FIM DO SCRIPT
-- ============================================

SELECT 'Script de refinação executado com sucesso!' AS Status;
