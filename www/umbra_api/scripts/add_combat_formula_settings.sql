-- =============================================================================
-- Fórmulas de combate (revisão de balanceamento): atributos em PONTOS convertidos em %
-- com retorno decrescente, x / (x + K). Nenhum valor zera o outro; todo ponto vale algo.
--
-- combat_formula_settings  linha única (id = 1), editada no Manager (aba Fórmulas de combate)
--                          e recarregada na zone via reload_combat_formulas.
-- pvp_settings.pvp_damage_mult  multiplicador global de dano PvP (antes 0,7 fixo no código).
--
-- Crítico:  5% + 70% x C/(C+crit_k), vezes (1 - R/(R+crit_resist_k)), teto crit_cap_pct
-- Duplo:    60% x D/(D+double_k), vezes (1 - R/(R+double_resist_k)); golpe extra = double_damage_pct
-- Acerto:   80% + 15% x A/(A+hit_accuracy_k) - 40% x E/(E+hit_dodge_k), entre hit_min e hit_max
-- Defesa:   DEF/(DEF + defense_k_base + defense_k_per_level x nível do atacante), teto defense_cap_pct
-- Atributo: escala% da skill x S/(S+attribute_scaling_k)
-- Variação: dano/cura/escudo x [1 - v%, 1 + v%]
-- Idempotente: CREATE ... IF NOT EXISTS, INSERT IGNORE e coluna via INFORMATION_SCHEMA.
-- =============================================================================

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS combat_formula_settings (
  id TINYINT UNSIGNED NOT NULL DEFAULT 1,
  crit_base_pct FLOAT NOT NULL DEFAULT 5 COMMENT 'Crítico mínimo com 0 pontos',
  crit_bonus_max_pct FLOAT NOT NULL DEFAULT 70 COMMENT 'Quanto os pontos de crítico somam no máximo',
  crit_k FLOAT NOT NULL DEFAULT 400 COMMENT 'Pontos de crítico para metade do bônus',
  crit_resist_k FLOAT NOT NULL DEFAULT 400 COMMENT 'Pontos de resistência para cortar o crítico pela metade',
  crit_cap_pct FLOAT NOT NULL DEFAULT 75 COMMENT 'Teto da chance final de crítico',
  double_max_pct FLOAT NOT NULL DEFAULT 60,
  double_k FLOAT NOT NULL DEFAULT 400,
  double_resist_k FLOAT NOT NULL DEFAULT 400,
  double_damage_pct INT UNSIGNED NOT NULL DEFAULT 80 COMMENT 'Golpe extra do duplo (% do golpe)',
  hit_base_pct FLOAT NOT NULL DEFAULT 80,
  hit_accuracy_bonus_pct FLOAT NOT NULL DEFAULT 15,
  hit_accuracy_k FLOAT NOT NULL DEFAULT 300,
  hit_dodge_max_pct FLOAT NOT NULL DEFAULT 40,
  hit_dodge_k FLOAT NOT NULL DEFAULT 300,
  hit_min_pct FLOAT NOT NULL DEFAULT 5,
  hit_max_pct FLOAT NOT NULL DEFAULT 95,
  defense_k_base FLOAT NOT NULL DEFAULT 500,
  defense_k_per_level FLOAT NOT NULL DEFAULT 50 COMMENT 'K da defesa cresce com o nível do atacante',
  defense_cap_pct FLOAT NOT NULL DEFAULT 75,
  attribute_scaling_k FLOAT NOT NULL DEFAULT 300,
  damage_variance_pct FLOAT NOT NULL DEFAULT 7 COMMENT '0 = dano fixo',
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT IGNORE INTO combat_formula_settings (id) VALUES (1);

SET @col_exists := (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'pvp_settings' AND COLUMN_NAME = 'pvp_damage_mult'
);
SET @sql := IF(@col_exists = 0,
  'ALTER TABLE pvp_settings ADD COLUMN pvp_damage_mult FLOAT NOT NULL DEFAULT 0.7 COMMENT ''Multiplicador global de dano PvP'' AFTER open_world_pvp',
  'SELECT 1');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
