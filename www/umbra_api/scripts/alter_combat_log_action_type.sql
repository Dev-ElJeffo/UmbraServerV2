-- ============================================================================
-- alter_combat_log_action_type.sql
-- Adiciona DOUBLE e REACTION ao ENUM de combat_log.action_type
-- (código C++ grava esses literais em writeCombatLog / reações).
-- Idempotente: seguro reexecutar se os valores já existirem (MySQL aceita
-- MODIFY com o mesmo conjunto).
-- ============================================================================

USE `umbra_eternum`;

ALTER TABLE `combat_log`
  MODIFY COLUMN `action_type`
    ENUM(
      'DAMAGE',
      'HEAL',
      'BUFF',
      'DEBUFF',
      'DEATH',
      'RESURRECT',
      'MISS',
      'DODGE',
      'BLOCK',
      'CRIT',
      'DOUBLE',
      'REACTION'
    ) NOT NULL;

-- Verificação:
-- SHOW COLUMNS FROM combat_log LIKE 'action_type';
