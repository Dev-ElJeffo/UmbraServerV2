-- Script para criar tabelas de informações do personagem
-- Classes, Factions, Titles, Guilds
-- Execute este script no MySQL Workbench ou phpMyAdmin

USE umbra_eternum;

-- ============================================================================
-- TABELA: classes
-- ============================================================================
CREATE TABLE IF NOT EXISTS classes (
    class_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    class_name VARCHAR(50) NOT NULL UNIQUE,
    class_description TEXT,
    -- Stats iniciais da classe
    base_strength INT UNSIGNED DEFAULT 10,
    base_dexterity INT UNSIGNED DEFAULT 10,
    base_intelligence INT UNSIGNED DEFAULT 10,
    base_vitality INT UNSIGNED DEFAULT 10,
    base_luck INT UNSIGNED DEFAULT 10,
    base_health INT UNSIGNED DEFAULT 100,
    base_mana INT UNSIGNED DEFAULT 50,
    base_stamina INT UNSIGNED DEFAULT 100,
    -- Stats de combate iniciais
    base_physical_attack INT DEFAULT 0,
    base_magic_attack INT DEFAULT 0,
    base_physical_defense INT DEFAULT 0,
    base_magic_defense INT DEFAULT 0,
    base_accuracy INT DEFAULT 0,
    base_dodge INT DEFAULT 0,
    base_critical INT DEFAULT 0,
    base_movement INT DEFAULT 0,
    base_resistance INT DEFAULT 0,
    base_double_attack_rate INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (class_id),
    INDEX idx_class_name (class_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: factions
-- ============================================================================
CREATE TABLE IF NOT EXISTS factions (
    faction_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    faction_name VARCHAR(50) NOT NULL UNIQUE,
    faction_description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (faction_id),
    INDEX idx_faction_name (faction_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: titles
-- ============================================================================
CREATE TABLE IF NOT EXISTS titles (
    title_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    title_name VARCHAR(100) NOT NULL UNIQUE,
    title_description TEXT,
    title_status VARCHAR(50) DEFAULT 'common',
    -- Stats bonus do título (opcional)
    bonus_strength INT DEFAULT 0,
    bonus_dexterity INT DEFAULT 0,
    bonus_intelligence INT DEFAULT 0,
    bonus_vitality INT DEFAULT 0,
    bonus_luck INT DEFAULT 0,
    bonus_health INT DEFAULT 0,
    bonus_mana INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (title_id),
    INDEX idx_title_name (title_name),
    INDEX idx_title_status (title_status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- TABELA: guilds
-- ============================================================================
CREATE TABLE IF NOT EXISTS guilds (
    guild_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    guild_name VARCHAR(50) NOT NULL UNIQUE,
    guild_description TEXT,
    guild_icon VARCHAR(255) DEFAULT NULL,
    guild_leader_id BIGINT UNSIGNED DEFAULT NULL,
    founded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (guild_id),
    INDEX idx_guild_name (guild_name),
    INDEX idx_guild_leader (guild_leader_id),
    FOREIGN KEY (guild_leader_id) REFERENCES players(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- ADICIONAR CAMPOS À TABELA players
-- ============================================================================

-- Relacionamentos
ALTER TABLE players
ADD COLUMN class_id BIGINT UNSIGNED DEFAULT NULL AFTER vitality,
ADD COLUMN faction_id BIGINT UNSIGNED DEFAULT NULL AFTER class_id,
ADD COLUMN current_guild_id BIGINT UNSIGNED DEFAULT NULL AFTER faction_id,
ADD COLUMN equipped_title_id BIGINT UNSIGNED DEFAULT NULL AFTER current_guild_id,
ADD COLUMN selected_class BIGINT UNSIGNED DEFAULT NULL AFTER equipped_title_id;

-- Stats PvP/Social
ALTER TABLE players
ADD COLUMN pvp INT UNSIGNED DEFAULT 0 AFTER luck,
ADD COLUMN chaos INT UNSIGNED DEFAULT 0 AFTER pvp,
ADD COLUMN honor INT UNSIGNED DEFAULT 0 AFTER chaos;

-- Foreign Keys
ALTER TABLE players
ADD CONSTRAINT fk_player_class FOREIGN KEY (class_id) REFERENCES classes(class_id) ON DELETE SET NULL,
ADD CONSTRAINT fk_player_faction FOREIGN KEY (faction_id) REFERENCES factions(faction_id) ON DELETE SET NULL,
ADD CONSTRAINT fk_player_guild FOREIGN KEY (current_guild_id) REFERENCES guilds(guild_id) ON DELETE SET NULL,
ADD CONSTRAINT fk_player_title FOREIGN KEY (equipped_title_id) REFERENCES titles(title_id) ON DELETE SET NULL,
ADD CONSTRAINT fk_player_selected_class FOREIGN KEY (selected_class) REFERENCES classes(class_id) ON DELETE SET NULL;

-- Índices
CREATE INDEX idx_player_class ON players(class_id);
CREATE INDEX idx_player_faction ON players(faction_id);
CREATE INDEX idx_player_guild ON players(current_guild_id);
CREATE INDEX idx_player_title ON players(equipped_title_id);
CREATE INDEX idx_player_selected_class ON players(selected_class);

-- ============================================================================
-- INSERIR DADOS INICIAIS: CLASSES
-- ============================================================================

INSERT INTO classes (class_name, class_description, base_strength, base_dexterity, base_intelligence, base_vitality, base_luck, base_health, base_mana, base_stamina, base_physical_attack, base_magic_attack, base_physical_defense, base_magic_defense, base_accuracy, base_dodge, base_critical, base_movement, base_resistance, base_double_attack_rate) VALUES
-- Barbarian: Focado em força e defesa física
('Barbarian', 'Guerreiro selvagem focado em combate corpo a corpo e força bruta', 20, 12, 8, 18, 10, 150, 30, 120, 15, 2, 12, 5, 8, 6, 10, 5, 3, 5),

-- Templar: Balanceado, focado em defesa
('Templar', 'Cavaleiro sagrado equilibrado em ataque e defesa', 15, 10, 12, 20, 12, 180, 60, 110, 10, 8, 15, 10, 10, 8, 8, 4, 8, 3),

-- DarkMage: Focado em magia e inteligência
('DarkMage', 'Mago das trevas especializado em magia ofensiva', 8, 10, 22, 10, 12, 80, 150, 90, 3, 20, 5, 15, 12, 5, 15, 3, 12, 2),

-- Cleric: Focado em suporte e magia curativa
('Cleric', 'Sacerdote especializado em cura e magia de suporte', 10, 10, 18, 15, 15, 120, 140, 100, 5, 15, 8, 12, 10, 7, 8, 4, 15, 2),

-- Assassin: Focado em agilidade e crítico
('Assassin', 'Assassino furtivo especializado em ataques rápidos e críticos', 12, 22, 10, 10, 18, 100, 40, 130, 12, 5, 6, 4, 18, 20, 25, 8, 2, 15),

-- Monk: Balanceado, focado em agilidade e resistência
('Monk', 'Monge equilibrado focado em velocidade e resistência', 14, 18, 12, 16, 14, 130, 70, 140, 10, 8, 10, 8, 15, 15, 12, 7, 10, 8);

-- ============================================================================
-- INSERIR DADOS INICIAIS: FACTIONS
-- ============================================================================

INSERT INTO factions (faction_name, faction_description) VALUES
('Novarra', 'Reino de luz e ordem, dedicado à justiça e proteção dos inocentes'),
('Eldros', 'Império das sombras, focado em poder e dominação através da força');

-- ============================================================================
-- INSERIR DADOS INICIAIS: TITLES (5 títulos completos)
-- ============================================================================

INSERT INTO titles (title_name, title_description, title_status, bonus_strength, bonus_dexterity, bonus_intelligence, bonus_vitality, bonus_luck, bonus_health, bonus_mana) VALUES
('Novato', 'Um iniciante em sua jornada. Este título não oferece bônus, mas marca o início de uma grande aventura.', 'common', 0, 0, 0, 0, 0, 0, 0),

('Veterano', 'Um guerreiro experiente que já enfrentou muitos desafios. +2 em todos os atributos base.', 'uncommon', 2, 2, 2, 2, 2, 20, 10),

('Lendário', 'Um herói cujas façanhas são conhecidas em todo o reino. +5 em todos os atributos base e +50 de vida.', 'rare', 5, 5, 5, 5, 5, 50, 25),

('Mestre das Sombras', 'Um especialista em combate furtivo. +8 em Destreza, +5 em Sorte e +10% de chance de crítico.', 'epic', 0, 8, 0, 0, 5, 0, 0),

('Guardião Eterno', 'O protetor supremo, cuja força é lendária. +10 em Força, +8 em Vitalidade e +100 de vida máxima.', 'legendary', 10, 0, 0, 8, 0, 100, 0);

-- ============================================================================
-- VERIFICAÇÃO
-- ============================================================================

-- Verificar classes criadas
SELECT class_id, class_name, base_strength, base_dexterity, base_intelligence, base_vitality, base_luck 
FROM classes 
ORDER BY class_id;

-- Verificar factions criadas
SELECT faction_id, faction_name FROM factions ORDER BY faction_id;

-- Verificar titles criados
SELECT title_id, title_name, title_status FROM titles ORDER BY title_id;

-- Verificar estrutura da tabela players
SELECT COLUMN_NAME, DATA_TYPE, IS_NULLABLE, COLUMN_DEFAULT
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = 'umbra_eternum'
  AND TABLE_NAME = 'players'
  AND COLUMN_NAME IN ('class_id', 'faction_id', 'current_guild_id', 'equipped_title_id', 'selected_class', 'pvp', 'chaos', 'honor')
ORDER BY ORDINAL_POSITION;

