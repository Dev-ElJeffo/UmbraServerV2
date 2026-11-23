-- Progressão de skills R1: crescimento por rank.
-- valor(r) = base × (1 + crescimento × (r − 1)), aplicado a todos os efeitos base da skill.
-- rank_growth_percent: NULL = padrão por tipo de efeito (dano/cura 12,5%, stat 10%,
-- controle = duração 10% + chance 5 pontos/rank); 0 = a skill não escala.
-- Também zera power_coef_bonus: antes ele SOMAVA com os +10%/rank implícitos (contagem dupla);
-- agora o crescimento cobre o dano e o bônus fica só como ajuste fino opcional.
-- Idempotente MySQL 5.7+.
-- Uso: mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_skills_rank_growth.sql

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skills' AND COLUMN_NAME = 'rank_growth_percent'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE skills ADD COLUMN rank_growth_percent DECIMAL(5,2) NULL DEFAULT NULL COMMENT ''% de crescimento por rank; NULL = padrão por tipo de efeito; 0 = não escala'' AFTER max_rank',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;

UPDATE skill_rank_scaling SET power_coef_bonus = 0 WHERE power_coef_bonus <> 0;
