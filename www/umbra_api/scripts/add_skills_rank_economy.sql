-- Progressão de skills R2: economia de pontos.
-- Custo do rank N  = skill_cost + rank_cost_step × (N − 1)   (padrão: 1, 2, 3, 4, 5)
-- Nível do rank N  = required_level + rank_level_step × (N − 1), limitado ao nível máximo
-- Ultimates: skill_cost 3, max_rank 3 (custos 3, 4, 5).
-- Idempotente MySQL 5.7+.
-- Uso: mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_skills_rank_economy.sql

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'rank_cost_step'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE skills ADD COLUMN rank_cost_step TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT ''Pontos extras por rank: custo(N) = skill_cost + step × (N − 1)'' AFTER skill_cost',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'rank_level_step'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE skills ADD COLUMN rank_level_step TINYINT UNSIGNED NOT NULL DEFAULT 4 COMMENT ''Níveis extras por rank: nível(N) = required_level + step × (N − 1)'' AFTER rank_cost_step',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

-- Ultimates: 3 ranks, custo 3/4/5.
UPDATE skills SET skill_cost = 3, max_rank = 3 WHERE type_id = 6;

-- Personagens com ultimate acima do novo máximo ficam no rank 3 (pontos gastos mantidos;
-- o reset de pontos devolve tudo).
UPDATE player_skills ps
INNER JOIN skills s ON s.skill_id = ps.skill_id
SET ps.current_rank = s.max_rank
WHERE ps.current_rank > s.max_rank;
