-- =============================================================================
-- Quests v2 (etapa 3a): objetivos e recompensas novos, repetição diária/semanal,
-- restrições (nível máximo, classe), categoria e tempo limite.
--
-- Objetivos novos: kill_any, use_skill, use_consumable, earn_gold, reach_level,
--                  craft_item, refine_item (kill aceita npc_template_ids: [..]).
-- Recompensas novas: skill_points, stat_points, companion (amount = companion_id), guild_xp.
-- quests.repeat_type: none | always | daily | weekly  (repeatable = 1 vira 'always').
-- player_skill_points.bonus_points: pontos de skill de quest, somados ao nível × 3
--   (trigger, zone e PHP usam level*3 + bonus_points; antes o level up apagava o bônus).
--
-- Idempotente: MODIFY de ENUM é estável; colunas via INFORMATION_SCHEMA + PREPARE/EXECUTE.
-- =============================================================================

SET NAMES utf8mb4;

ALTER TABLE quest_objectives MODIFY COLUMN objective_type
  ENUM('talk','kill','collect','deliver','reach_area','use_item_at',
       'kill_any','use_skill','use_consumable','earn_gold','reach_level','craft_item','refine_item') NOT NULL;

ALTER TABLE quest_rewards MODIFY COLUMN reward_type
  ENUM('gold','experience','item','skill_points','stat_points','companion','guild_xp') NOT NULL;

ALTER TABLE quest_reward_choices MODIFY COLUMN reward_type
  ENUM('gold','experience','item','skill_points','stat_points','companion','guild_xp') NOT NULL;

-- quests.repeat_type ---------------------------------------------------------------
SET @c = (SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'quests' AND COLUMN_NAME = 'repeat_type');
SET @sql = IF(@c = 0,
  'ALTER TABLE quests ADD COLUMN repeat_type ENUM(''none'',''always'',''daily'',''weekly'') NOT NULL DEFAULT ''none'' COMMENT ''none=uma vez; always=repete na hora; daily=1x por dia; weekly=1x por semana'' AFTER repeatable',
  'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;
-- Migração única: só quando a coluna acabou de nascer (@c = 0).
SET @sql = IF(@c = 0, 'UPDATE quests SET repeat_type = ''always'' WHERE repeatable = 1', 'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;

-- quests.max_level ----------------------------------------------------------------
SET @c = (SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'quests' AND COLUMN_NAME = 'max_level');
SET @sql = IF(@c = 0,
  'ALTER TABLE quests ADD COLUMN max_level INT UNSIGNED NOT NULL DEFAULT 0 COMMENT ''0 = sem limite'' AFTER min_level',
  'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;

-- quests.class_ids ----------------------------------------------------------------
SET @c = (SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'quests' AND COLUMN_NAME = 'class_ids');
SET @sql = IF(@c = 0,
  'ALTER TABLE quests ADD COLUMN class_ids JSON NULL COMMENT ''[class_id,...]; NULL/vazio = todas as classes'' AFTER max_level',
  'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;

-- quests.quest_category -----------------------------------------------------------
SET @c = (SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'quests' AND COLUMN_NAME = 'quest_category');
SET @sql = IF(@c = 0,
  'ALTER TABLE quests ADD COLUMN quest_category ENUM(''main'',''side'',''daily'',''event'') NOT NULL DEFAULT ''side'' COMMENT ''Agrupamento no diário'' AFTER class_ids',
  'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;

-- quests.time_limit_minutes -------------------------------------------------------
SET @c = (SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'quests' AND COLUMN_NAME = 'time_limit_minutes');
SET @sql = IF(@c = 0,
  'ALTER TABLE quests ADD COLUMN time_limit_minutes INT UNSIGNED NOT NULL DEFAULT 0 COMMENT ''0 = sem limite; estourou = failed'' AFTER quest_category',
  'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;

-- player_skill_points.bonus_points ------------------------------------------------
SET @c = (SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'player_skill_points' AND COLUMN_NAME = 'bonus_points');
SET @sql = IF(@c = 0,
  'ALTER TABLE player_skill_points ADD COLUMN bonus_points INT UNSIGNED NOT NULL DEFAULT 0 COMMENT ''Pontos de skill de quest (somam ao nível × 3)'' AFTER points_available',
  'SELECT 1');
PREPARE s FROM @sql; EXECUTE s; DEALLOCATE PREPARE s;

-- Trigger de level up: preserva o bônus de quest.
DROP TRIGGER IF EXISTS trg_update_skill_points_on_level;
DELIMITER $$
CREATE TRIGGER trg_update_skill_points_on_level AFTER UPDATE ON players
FOR EACH ROW
BEGIN
    IF NEW.level > OLD.level THEN
        UPDATE player_skill_points
        SET total_points_earned = (NEW.level * 3) + bonus_points,
            points_available = (NEW.level * 3) + bonus_points - points_spent
        WHERE player_id = NEW.id;
    END IF;
END$$
DELIMITER ;

-- Conferência
SELECT quest_id, quest_key, repeatable, repeat_type, quest_category FROM quests ORDER BY quest_id;
