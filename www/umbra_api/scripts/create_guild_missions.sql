-- =============================================================================
-- Missões de guilda (etapa 3b).
--
-- guild_mission_templates     cadastro do Manager (tipo/params iguais aos objetivos de quest)
-- guild_missions              instância por guilda e período (daily = data; weekly = segunda-feira)
-- guild_mission_contributions contribuição de cada membro + se já resgatou a recompensa
-- sp_guild_missions_ensure    sorteia as missões do período (3 diárias + 2 semanais) por peso e nível
--                             da guilda. Usada pelo PHP e pela zone: uma regra só.
--
-- EXP de guilda: concedido uma vez por missão (xp_awarded) num UPDATE atômico (zone ou PHP).
-- Idempotente: CREATE ... IF NOT EXISTS, seeds com WHERE NOT EXISTS, DROP/CREATE da procedure.
-- =============================================================================

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS guild_mission_templates (
  mission_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  mission_key VARCHAR(64) NOT NULL,
  title VARCHAR(120) NOT NULL,
  description VARCHAR(255) NOT NULL DEFAULT '',
  period ENUM('daily','weekly') NOT NULL DEFAULT 'daily',
  objective_type ENUM('kill','kill_any','earn_gold','use_consumable','use_skill','craft_item','refine_item') NOT NULL,
  params_json JSON NULL COMMENT 'Mesmo formato dos objetivos de quest (sem required_count)',
  required_total INT UNSIGNED NOT NULL DEFAULT 100 COMMENT 'Meta somada da guilda',
  min_guild_level INT UNSIGNED NOT NULL DEFAULT 1,
  reward_guild_xp INT UNSIGNED NOT NULL DEFAULT 0,
  reward_gold INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Por membro que contribuiu o mínimo',
  reward_item_id INT NULL,
  reward_item_qty INT UNSIGNED NOT NULL DEFAULT 1,
  min_contribution INT UNSIGNED NOT NULL DEFAULT 1 COMMENT 'Contribuição mínima para resgatar',
  weight INT UNSIGNED NOT NULL DEFAULT 10 COMMENT 'Peso no sorteio',
  is_active TINYINT(1) NOT NULL DEFAULT 1,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (mission_id),
  UNIQUE KEY uq_guild_mission_key (mission_key)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS guild_missions (
  guild_mission_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  guild_id BIGINT UNSIGNED NOT NULL,
  mission_id INT UNSIGNED NOT NULL,
  period ENUM('daily','weekly') NOT NULL,
  period_key DATE NOT NULL COMMENT 'daily = o dia; weekly = a segunda-feira da semana',
  progress INT UNSIGNED NOT NULL DEFAULT 0,
  required_total INT UNSIGNED NOT NULL DEFAULT 1 COMMENT 'Cópia da meta no sorteio',
  status ENUM('active','completed') NOT NULL DEFAULT 'active',
  xp_awarded TINYINT(1) NOT NULL DEFAULT 0,
  started_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  completed_at TIMESTAMP NULL DEFAULT NULL,
  PRIMARY KEY (guild_mission_id),
  UNIQUE KEY uq_guild_mission_period (guild_id, mission_id, period_key),
  KEY idx_guild_missions_guild (guild_id, period, period_key),
  CONSTRAINT fk_guild_missions_guild FOREIGN KEY (guild_id) REFERENCES guilds(guild_id) ON DELETE CASCADE,
  CONSTRAINT fk_guild_missions_tpl FOREIGN KEY (mission_id) REFERENCES guild_mission_templates(mission_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS guild_mission_contributions (
  guild_mission_id BIGINT UNSIGNED NOT NULL,
  player_id BIGINT UNSIGNED NOT NULL,
  amount INT UNSIGNED NOT NULL DEFAULT 0,
  reward_claimed TINYINT(1) NOT NULL DEFAULT 0,
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (guild_mission_id, player_id),
  CONSTRAINT fk_gmc_mission FOREIGN KEY (guild_mission_id) REFERENCES guild_missions(guild_mission_id) ON DELETE CASCADE,
  CONSTRAINT fk_gmc_player FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Sorteio do período (3 diárias + 2 semanais). FOR UPDATE na guilda serializa chamadas simultâneas.
DROP PROCEDURE IF EXISTS sp_guild_missions_ensure;
DELIMITER $$
CREATE PROCEDURE sp_guild_missions_ensure(IN p_guild_id BIGINT UNSIGNED)
BEGIN
  DECLARE v_level INT DEFAULT 1;
  DECLARE v_day DATE;
  DECLARE v_week DATE;
  DECLARE v_have INT DEFAULT 0;

  START TRANSACTION;
  SELECT guild_level INTO v_level FROM guilds WHERE guild_id = p_guild_id FOR UPDATE;
  SET v_day = CURDATE();
  SET v_week = DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY);

  SELECT COUNT(*) INTO v_have FROM guild_missions
   WHERE guild_id = p_guild_id AND period = 'daily' AND period_key = v_day;
  IF v_have = 0 THEN
    INSERT IGNORE INTO guild_missions (guild_id, mission_id, period, period_key, progress, required_total, status)
    SELECT p_guild_id, t.mission_id, 'daily', v_day, 0, t.required_total, 'active'
      FROM guild_mission_templates t
     WHERE t.is_active = 1 AND t.period = 'daily' AND t.min_guild_level <= IFNULL(v_level, 1)
     ORDER BY -LOG(1 - RAND()) / GREATEST(1, t.weight)
     LIMIT 3;
  END IF;

  SELECT COUNT(*) INTO v_have FROM guild_missions
   WHERE guild_id = p_guild_id AND period = 'weekly' AND period_key = v_week;
  IF v_have = 0 THEN
    INSERT IGNORE INTO guild_missions (guild_id, mission_id, period, period_key, progress, required_total, status)
    SELECT p_guild_id, t.mission_id, 'weekly', v_week, 0, t.required_total, 'active'
      FROM guild_mission_templates t
     WHERE t.is_active = 1 AND t.period = 'weekly' AND t.min_guild_level <= IFNULL(v_level, 1)
     ORDER BY -LOG(1 - RAND()) / GREATEST(1, t.weight)
     LIMIT 2;
  END IF;
  COMMIT;
END$$
DELIMITER ;

-- Seeds (ajustar metas/recompensas no Manager → Missões de guilda)
INSERT INTO guild_mission_templates (mission_key, title, description, period, objective_type, params_json, required_total, reward_guild_xp, reward_gold, min_contribution, weight)
SELECT * FROM (
  SELECT 'GM_DAILY_HUNT' AS k, 'Caçada da guilda' AS t, 'Derrotem monstros de qualquer nível.' AS d, 'daily' AS p, 'kill_any' AS o, JSON_OBJECT('min_level', 0, 'max_level', 0, 'zone_id', 0) AS pj, 60 AS req, 300 AS xp, 100 AS g, 5 AS mc, 10 AS w
  UNION ALL SELECT 'GM_DAILY_GOLD', 'Cofre da guilda', 'Saqueiem ouro dos inimigos.', 'daily', 'earn_gold', JSON_OBJECT(), 2000, 250, 80, 100, 10
  UNION ALL SELECT 'GM_DAILY_SKILLS', 'Treino coletivo', 'Usem habilidades em combate.', 'daily', 'use_skill', JSON_OBJECT('skill_id', 0), 150, 200, 60, 10, 10
  UNION ALL SELECT 'GM_DAILY_POTIONS', 'Suprimentos de campanha', 'Usem poções durante as batalhas.', 'daily', 'use_consumable', JSON_OBJECT('item_template_id', 0, 'item_subtype', ''), 30, 150, 50, 2, 6
  UNION ALL SELECT 'GM_WEEKLY_HUNT', 'Expedição semanal', 'Derrotem uma grande quantidade de monstros.', 'weekly', 'kill_any', JSON_OBJECT('min_level', 0, 'max_level', 0, 'zone_id', 0), 600, 2000, 500, 30, 10
  UNION ALL SELECT 'GM_WEEKLY_CRAFT', 'Oficina da guilda', 'Fabriquem itens na forja e no laboratório.', 'weekly', 'craft_item', JSON_OBJECT('item_template_id', 0), 40, 1500, 400, 3, 10
) s
WHERE NOT EXISTS (SELECT 1 FROM guild_mission_templates gm WHERE gm.mission_key = s.k);

-- Conferência
SELECT mission_id, mission_key, period, objective_type, required_total, reward_guild_xp FROM guild_mission_templates;
