-- Progressão de skills R3: chefe imune a controle (STUN, SILENCE, ROOT, SLOW e lockout de
-- INTERRUPT). O cliente mostra "Imune". Interrupt (cancelar cast), knockback e taunt
-- continuam valendo. Idempotente MySQL 5.7+.
-- Uso: mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_npc_templates_cc_immune.sql
-- Marcar um chefe: UPDATE npc_templates SET cc_immune = 1 WHERE npc_template_id = <id>;
-- (a zone lê no spawn: reiniciar a zone ou reload de instâncias no Manager).

SET @col_exists = (
  SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'npc_templates' AND COLUMN_NAME = 'cc_immune'
);
SET @sql = IF(@col_exists = 0,
  'ALTER TABLE npc_templates ADD COLUMN cc_immune TINYINT(1) NOT NULL DEFAULT 0 COMMENT ''1 = imune a STUN/SILENCE/ROOT/SLOW (chefe)'' AFTER slow_resist',
  'SELECT 1');
PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
