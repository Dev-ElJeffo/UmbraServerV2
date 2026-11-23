-- =============================================================================
-- Correção de dados das skills (achados da revisão das descrições).
-- Valores de referência: docs_main/R4_PROPOSTA_KITS.md.
--
-- 1) power_coef × DAMAGE: a zone usa o value_percent de cada DAMAGE; power_coef aparece no
--    tooltip/livro ("Poder"). Os dois ficam iguais ao valor aprovado na R4.
-- 2) Alvo errado: Oração de Campo e Milagre das Cinzas curavam só o caster (target_override
--    SELF); redirect_to_self só vale em reação e sai das skills que não são reação.
-- 3) Duração: skill.duration_ms alinhado aos efeitos (Escudo de Voto 30s, Barreira 60s);
--    buffs sem duração em skill ativa ganham os 5s que a zone já aplicava por padrão.
-- 4) target_stat vazio em DOT vira 'health'.
--
-- Backup: skills_dados_backup_v1 e skill_rank_scaling_backup_v1 (criados só na 1ª execução).
-- Desfazer:
--   UPDATE skills s JOIN skills_dados_backup_v1 b ON b.skill_id = s.skill_id
--      SET s.power_coef = b.power_coef, s.duration_ms = b.duration_ms,
--          s.effects_json = b.effects_json, s.description = b.description;
--   UPDATE skill_rank_scaling r JOIN skill_rank_scaling_backup_v1 b ON b.scaling_id = r.scaling_id
--      SET r.extra_effects_json = b.extra_effects_json;
-- Idempotente: cada UPDATE confere o tipo do efeito no índice antes de alterar.
-- Depois: Manager > Skills > Recarregar no Zone (ou reiniciar a zone).
-- =============================================================================

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS skills_dados_backup_v1 AS
  SELECT skill_id, power_coef, duration_ms, effects_json, description FROM skills;
CREATE TABLE IF NOT EXISTS skill_rank_scaling_backup_v1 AS
  SELECT scaling_id, skill_id, `rank`, extra_effects_json FROM skill_rank_scaling;

-- 1) Dano: power_coef e DAMAGE iguais ao valor da R4 ------------------------------
UPDATE skills SET power_coef = 155 WHERE skill_key = 'TEMP_HOLY_STRIKE';
UPDATE skills SET power_coef = 200 WHERE skill_key = 'TEMP_JUDGMENT';
UPDATE skills SET power_coef = 120, effects_json = JSON_SET(effects_json, '$[0].value_percent', 120)
 WHERE skill_key = 'DMAGE_CRIMSON_DRAIN' AND JSON_UNQUOTE(JSON_EXTRACT(effects_json, '$[0].type')) = 'DAMAGE';
UPDATE skills SET power_coef = 250, effects_json = JSON_SET(effects_json, '$[0].value_percent', 250)
 WHERE skill_key = 'DMAGE_MINOR_PORTAL' AND JSON_UNQUOTE(JSON_EXTRACT(effects_json, '$[0].type')) = 'DAMAGE';

-- 2) Alvo -----------------------------------------------------------------------
-- Oração de Campo: cura o aliado (sem alvo, o caster) 25% + 40% do ataque mágico (R4).
UPDATE skills SET power_coef = 40,
       effects_json = JSON_REMOVE(effects_json, '$[0].target_override')
 WHERE skill_key = 'TEMP_FIELD_PRAYER' AND JSON_UNQUOTE(JSON_EXTRACT(effects_json, '$[0].type')) = 'HEAL';
-- Milagre das Cinzas: a cura vale para os aliados na área, como os buffs de defesa.
UPDATE skills SET effects_json = JSON_REMOVE(effects_json, '$[0].target_override', '$[0].conditions_json')
 WHERE skill_key = 'TEMP_ASH_MIRACLE' AND JSON_UNQUOTE(JSON_EXTRACT(effects_json, '$[0].type')) = 'HEAL';
-- redirect_to_self sem efeito fora de reação.
UPDATE skills SET effects_json = JSON_REMOVE(effects_json, '$[1].conditions_json')
 WHERE skill_key = 'DMAGE_CRIMSON_DRAIN' AND JSON_UNQUOTE(JSON_EXTRACT(effects_json, '$[1].type')) = 'LIFESTEAL';
UPDATE skills SET effects_json = JSON_REMOVE(effects_json, '$[0].conditions_json')
 WHERE skill_key = 'ALCH_NEUTRALIZER' AND JSON_UNQUOTE(JSON_EXTRACT(effects_json, '$[0].type')) = 'CLEANSE';
-- Interposição redireciona para você parte do dano do aliado (não reflete no agressor).
UPDATE skills SET description = 'Reação: quando um aliado leva dano, você assume parte dele no lugar.'
 WHERE skill_key = 'TEMP_INTERPOSITION';

-- 3) Duração --------------------------------------------------------------------
UPDATE skills SET duration_ms = 30000 WHERE skill_key = 'TEMP_VOW_SHIELD';
UPDATE skills SET duration_ms = 60000 WHERE skill_key = 'TEMP_RUNIC_BARRIER';
UPDATE skills SET effects_json = JSON_SET(effects_json, '$[1].duration_ms', 5000)
 WHERE skill_key IN ('MONK_TECH_CUT', 'MONK_DISCIPLINE_SEQUENCE', 'ASSN_PRECISE_BLADE')
   AND JSON_UNQUOTE(JSON_EXTRACT(effects_json, '$[1].type')) = 'BUFF_STAT'
   AND COALESCE(JSON_EXTRACT(effects_json, '$[1].duration_ms'), 0) = 0;
-- Marcos de resistência do Golpe Consagrado e do Julgamento (buff em si, skill sem duração).
UPDATE skill_rank_scaling r JOIN skills s ON s.skill_id = r.skill_id
   SET r.extra_effects_json = JSON_SET(r.extra_effects_json, '$[0].duration_ms', 5000)
 WHERE s.skill_key IN ('TEMP_HOLY_STRIKE', 'TEMP_JUDGMENT')
   AND JSON_UNQUOTE(JSON_EXTRACT(r.extra_effects_json, '$[0].type')) = 'BUFF_STAT'
   AND JSON_EXTRACT(r.extra_effects_json, '$[0].duration_ms') IS NULL;

-- 4) target_stat vazio em DOT ------------------------------------------------------
UPDATE skills SET effects_json = JSON_SET(effects_json, '$[1].target_stat', 'health')
 WHERE skill_key = 'TEMP_HOLY_STRIKE' AND JSON_UNQUOTE(JSON_EXTRACT(effects_json, '$[1].type')) = 'DOT';
UPDATE skills SET effects_json = JSON_SET(effects_json, '$[2].target_stat', 'health')
 WHERE skill_key = 'TEMP_VOW_MARK' AND JSON_UNQUOTE(JSON_EXTRACT(effects_json, '$[2].type')) = 'DOT';

-- Conferência
SELECT skill_key, power_coef, duration_ms, effects_json FROM skills
 WHERE skill_key IN ('TEMP_HOLY_STRIKE', 'TEMP_JUDGMENT', 'DMAGE_CRIMSON_DRAIN', 'DMAGE_MINOR_PORTAL',
                     'TEMP_FIELD_PRAYER', 'TEMP_ASH_MIRACLE', 'TEMP_VOW_SHIELD', 'TEMP_RUNIC_BARRIER',
                     'MONK_TECH_CUT', 'MONK_DISCIPLINE_SEQUENCE', 'ASSN_PRECISE_BLADE', 'ALCH_NEUTRALIZER',
                     'TEMP_VOW_MARK');
