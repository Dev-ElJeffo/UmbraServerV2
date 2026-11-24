-- =============================================================================
-- Dungeons (etapa 5): regras de dungeon sobre as instâncias da etapa 4.
--
-- instance_templates   + is_dungeon, completion_mode, reward_exp, reward_gold, lockout_reset,
--                        completion_close_seconds
-- npc_instances        + dungeon_role (NULL = inimigo comum; trash/boss/final_boss)
-- dungeon_difficulties dificuldades por molde (multiplicadores de vida, dano e recompensa)
-- dungeon_reward_items itens sorteados na conclusão (por dificuldade; '' = todas)
-- dungeon_completions  histórico de conclusões (lockout por período)
-- quest_objectives / guild_mission_templates: + complete_dungeon
--
-- Idempotente: colunas via INFORMATION_SCHEMA, CREATE ... IF NOT EXISTS, seeds com WHERE NOT EXISTS.
-- =============================================================================

SET NAMES utf8mb4;

-- instance_templates -----------------------------------------------------------
SET @c = (SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'instance_templates' AND COLUMN_NAME = 'is_dungeon');
SET @sql = IF(@c = 0,
  'ALTER TABLE instance_templates
     ADD COLUMN is_dungeon TINYINT(1) NOT NULL DEFAULT 0 COMMENT ''Regras de dungeon (chefes, conclusão, lockout)'',
     ADD COLUMN completion_mode ENUM(''final_boss'',''all_bosses'') NOT NULL DEFAULT ''final_boss'' COMMENT ''O que conclui a dungeon'',
     ADD COLUMN reward_exp INT UNSIGNED NOT NULL DEFAULT 0 COMMENT ''EXP de conclusão (x multiplicador da dificuldade)'',
     ADD COLUMN reward_gold INT UNSIGNED NOT NULL DEFAULT 0 COMMENT ''Ouro de conclusão (x multiplicador da dificuldade)'',
     ADD COLUMN lockout_reset ENUM(''none'',''daily'',''weekly'') NOT NULL DEFAULT ''daily'' COMMENT ''Recompensa de conclusão 1x por período'',
     ADD COLUMN completion_close_seconds INT UNSIGNED NOT NULL DEFAULT 300 COMMENT ''Fecha a cópia N s após concluir''',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- npc_instances.dungeon_role ----------------------------------------------------
SET @c = (SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'npc_instances' AND COLUMN_NAME = 'dungeon_role');
SET @sql = IF(@c = 0,
  'ALTER TABLE npc_instances ADD COLUMN dungeon_role ENUM(''trash'',''boss'',''final_boss'') NULL DEFAULT NULL COMMENT ''Papel do NPC de molde na dungeon; NULL = inimigo comum''',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- dificuldades ---------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS dungeon_difficulties (
  difficulty_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  instance_template_id INT UNSIGNED NOT NULL,
  difficulty_key VARCHAR(32) NOT NULL COMMENT 'normal, heroic, ...',
  name VARCHAR(64) NOT NULL,
  sort_order INT NOT NULL DEFAULT 0,
  min_level INT UNSIGNED NOT NULL DEFAULT 1 COMMENT 'Sobrepõe o nível mínimo do molde se maior',
  hp_mult FLOAT NOT NULL DEFAULT 1.0,
  damage_mult FLOAT NOT NULL DEFAULT 1.0,
  reward_mult FLOAT NOT NULL DEFAULT 1.0 COMMENT 'Multiplica EXP e ouro de conclusão',
  is_active TINYINT(1) NOT NULL DEFAULT 1,
  PRIMARY KEY (difficulty_id),
  UNIQUE KEY uq_dungeon_difficulty (instance_template_id, difficulty_key)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- recompensas de conclusão ----------------------------------------------------------
CREATE TABLE IF NOT EXISTS dungeon_reward_items (
  reward_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  instance_template_id INT UNSIGNED NOT NULL,
  difficulty_key VARCHAR(32) NOT NULL DEFAULT '' COMMENT 'Vazio = todas as dificuldades',
  item_template_id INT NOT NULL,
  drop_chance FLOAT NOT NULL DEFAULT 1.0 COMMENT '0..1, sorteado por jogador',
  min_qty INT UNSIGNED NOT NULL DEFAULT 1,
  max_qty INT UNSIGNED NOT NULL DEFAULT 1,
  is_active TINYINT(1) NOT NULL DEFAULT 1,
  PRIMARY KEY (reward_id),
  KEY idx_dungeon_reward_tpl (instance_template_id, difficulty_key)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- conclusões / lockout ---------------------------------------------------------------
CREATE TABLE IF NOT EXISTS dungeon_completions (
  completion_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  player_id INT UNSIGNED NOT NULL,
  instance_template_id INT UNSIGNED NOT NULL,
  difficulty_key VARCHAR(32) NOT NULL,
  period_key DATE NOT NULL COMMENT 'Dia (daily), segunda-feira (weekly) ou 2000-01-01 (none)',
  rewarded TINYINT(1) NOT NULL DEFAULT 1 COMMENT '0 = concluiu em lockout (sem recompensa)',
  duration_seconds INT UNSIGNED NOT NULL DEFAULT 0,
  completed_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (completion_id),
  KEY idx_dungeon_lockout (player_id, instance_template_id, difficulty_key, period_key, rewarded)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- objetivo complete_dungeon (quests v2 e missões de guilda) -------------------------
SET @t = (SELECT COLUMN_TYPE FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'quest_objectives' AND COLUMN_NAME = 'objective_type');
SET @sql = IF(@t IS NOT NULL AND LOCATE('complete_dungeon', @t) = 0,
  'ALTER TABLE quest_objectives MODIFY COLUMN objective_type ENUM(''talk'',''kill'',''collect'',''deliver'',''reach_area'',''use_item_at'',''kill_any'',''use_skill'',''use_consumable'',''earn_gold'',''reach_level'',''craft_item'',''refine_item'',''complete_dungeon'') NOT NULL',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @t = (SELECT COLUMN_TYPE FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'guild_mission_templates' AND COLUMN_NAME = 'objective_type');
SET @sql = IF(@t IS NOT NULL AND LOCATE('complete_dungeon', @t) = 0,
  'ALTER TABLE guild_mission_templates MODIFY COLUMN objective_type ENUM(''kill'',''kill_any'',''earn_gold'',''use_consumable'',''use_skill'',''craft_item'',''refine_item'',''complete_dungeon'') NOT NULL',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- seed: INST_TEST vira dungeon com Normal e Heroica -----------------------------------
UPDATE instance_templates
   SET is_dungeon = 1, reward_exp = 500, reward_gold = 200
 WHERE code = 'INST_TEST' AND is_dungeon = 0;

INSERT INTO dungeon_difficulties (instance_template_id, difficulty_key, name, sort_order, min_level, hp_mult, damage_mult, reward_mult)
SELECT t.instance_template_id, 'normal', 'Normal', 0, 1, 1.0, 1.0, 1.0
  FROM instance_templates t
 WHERE t.code = 'INST_TEST'
   AND NOT EXISTS (SELECT 1 FROM dungeon_difficulties d WHERE d.instance_template_id = t.instance_template_id AND d.difficulty_key = 'normal');

INSERT INTO dungeon_difficulties (instance_template_id, difficulty_key, name, sort_order, min_level, hp_mult, damage_mult, reward_mult)
SELECT t.instance_template_id, 'heroic', 'Heroica', 1, 1, 2.5, 1.8, 2.0
  FROM instance_templates t
 WHERE t.code = 'INST_TEST'
   AND NOT EXISTS (SELECT 1 FROM dungeon_difficulties d WHERE d.instance_template_id = t.instance_template_id AND d.difficulty_key = 'heroic');
