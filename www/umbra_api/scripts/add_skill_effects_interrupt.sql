-- Revisão de skills, fase B: novo tipo de efeito INTERRUPT (chance de cancelar o cast em
-- curso do alvo; duration_ms > 0 = silêncio após interromper). Idempotente MySQL 5.7+.
-- Uso: mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_skill_effects_interrupt.sql

SET @has_interrupt = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'skill_effects'
    AND COLUMN_NAME = 'effect_type' AND COLUMN_TYPE LIKE '%''INTERRUPT''%'
);
SET @sql = IF(@has_interrupt = 0,
  'ALTER TABLE skill_effects MODIFY COLUMN effect_type ENUM(''DAMAGE'',''HEAL'',''SHIELD'',''BUFF_STAT'',''DEBUFF_STAT'',''DOT'',''HOT'',''CLEANSE'',''DISPEL'',''STUN'',''SILENCE'',''SLOW'',''ROOT'',''KNOCKBACK'',''TAUNT'',''STEALTH'',''INVULNERABLE'',''LIFESTEAL'',''MANASTEAL'',''SUMMON'',''TELEPORT'',''EXECUTE'',''REFLECT'',''COOLDOWN_RESET'',''RESOURCE_RESTORE'',''INTERRUPT'') NOT NULL',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
