-- Progressão de skills R4: kits revisados por classe (docs_main/R4_PROPOSTA_KITS.md).
-- Idempotente: regrava os valores finais; marcos (ranks 3/5) recriados por umbra_r4_ranks.
-- As skills novas reaproveitam a linha (skill_id, nível, ícone) da skill que substituem.
-- Uso: mysql -u root -p umbra_eternum < www\umbra_api\scripts\fix_skills_r4_kits.sql
-- Depois: reiniciar a zone (ou reload_skills) e relogar no cliente.

SET NAMES utf8mb4;

DROP PROCEDURE IF EXISTS umbra_r4_ranks;
DELIMITER $$
-- Recria os 5 ranks da skill: marco do rank 3 (cd3/dur3/ex3, vale também no 4) e do rank 5
-- (cd5/dur5/ex5, cumulativos: passe o total). Extras valem do rank em que aparecem em diante.
CREATE PROCEDURE umbra_r4_ranks(IN p_key VARCHAR(64), IN p_cd3 INT, IN p_cd5 INT,
                                IN p_dur3 INT, IN p_dur5 INT, IN p_ex3 JSON, IN p_ex5 JSON)
BEGIN
  DECLARE v_id INT DEFAULT NULL;
  SELECT skill_id INTO v_id FROM skills WHERE skill_key = p_key LIMIT 1;
  IF v_id IS NOT NULL THEN
    DELETE FROM skill_rank_scaling WHERE skill_id = v_id;
    INSERT INTO skill_rank_scaling
      (skill_id, `rank`, power_coef_bonus, resource_cost_bonus, cooldown_reduction_ms,
       duration_bonus_ms, extra_effects_json)
    VALUES
      (v_id, 1, 0, 0, 0, 0, JSON_ARRAY()),
      (v_id, 2, 0, 0, 0, 0, JSON_ARRAY()),
      (v_id, 3, 0, 0, p_cd3, p_dur3, COALESCE(p_ex3, JSON_ARRAY())),
      (v_id, 4, 0, 0, p_cd3, p_dur3, JSON_ARRAY()),
      (v_id, 5, 0, 0, p_cd5, p_dur5, COALESCE(p_ex5, JSON_ARRAY()));
  END IF;
END$$
DELIMITER ;

-- ============================================================================
-- Barbarian
-- ============================================================================

UPDATE skills SET is_interrupt = 0,
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 180}, {"type": "BUFF_STAT", "duration_ms": 5000, "target_stat": "double_attack_rate", "value_percent": 25}]'
WHERE skill_key = 'BARB_RUIN_STRIKE';
CALL umbra_r4_ranks('BARB_RUIN_STRIKE', 500, 500, 0, 0, NULL,
  '[{"type": "STUN", "duration_ms": 1000, "chance_percent": 30}]');

UPDATE skills SET
  description = 'Pele e ossos endurecidos: mais defesa e resistência a críticos.',
  effects_json = '[{"type": "BUFF_STAT", "value_flat": 50, "target_stat": "physical_defense", "value_percent": 15}, {"type": "BUFF_STAT", "value_flat": 30, "target_stat": "critical_resistance", "value_percent": 10}]'
WHERE skill_key = 'BARB_HARDENED_SKIN';

UPDATE skills SET is_interrupt = 0,
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 140}, {"type": "BUFF_STAT", "duration_ms": 3000, "target_stat": "movement_speed", "value_percent": 30}, {"type": "INTERRUPT", "chance_percent": 50}]'
WHERE skill_key = 'BARB_WILD_CHARGE';
CALL umbra_r4_ranks('BARB_WILD_CHARGE', 0, 0, 0, 0,
  '[{"type": "ROOT", "duration_ms": 1000}]', '[{"type": "STUN", "duration_ms": 1200}]');

UPDATE skills SET target_id = 6, area_radius = 500, is_interrupt = 0
WHERE skill_key = 'BARB_WAR_CRY';
CALL umbra_r4_ranks('BARB_WAR_CRY', 0, 0, 0, 20000,
  '[{"type": "BUFF_STAT", "duration_ms": 60000, "target_stat": "stun_resist", "value_percent": 15}]', NULL);

-- Pisotear (substitui Ossos de Ferro)
UPDATE skills SET skill_key = 'BARB_STOMP', skill_name = 'Pisotear', type_id = 1, target_id = 4,
  power_coef = 90, resource_cost = 35, cooldown_ms = 14000, cast_time_ms = 0, duration_ms = 0,
  range_min = 0, range_max = 0, area_radius = 300, requires_target = 0, is_interrupt = 0,
  can_crit = 1, damage_type = 'PHYSICAL', scaling_stat_id = 1, element_id = 1,
  description = 'Pisa com força: dano em área ao redor, lentidão e chance de interromper.',
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 90}, {"type": "SLOW", "duration_ms": 3000, "value_percent": -30}, {"type": "INTERRUPT", "chance_percent": 40}]'
WHERE skill_id = 6;
CALL umbra_r4_ranks('BARB_STOMP', 2000, 2000, 0, 0, NULL, '[{"type": "ROOT", "duration_ms": 1000}]');

CALL umbra_r4_ranks('BARB_RENDING_CUT', 1000, 1000, 0, 0, NULL,
  '[{"type": "DEBUFF_STAT", "duration_ms": 6000, "target_stat": "physical_defense", "value_percent": -15}]');

-- Arremesso de Corrente (substitui Passo da Caçadora)
UPDATE skills SET skill_key = 'BARB_CHAIN_THROW', skill_name = 'Arremesso de Corrente', type_id = 1,
  target_id = 2, power_coef = 80, resource_cost = 30, cooldown_ms = 15000, cast_time_ms = 0,
  duration_ms = 0, range_min = 0, range_max = 800, area_radius = 0, requires_target = 1,
  is_interrupt = 0, can_crit = 1, damage_type = 'PHYSICAL', scaling_stat_id = 1, element_id = 1,
  description = 'Arremessa uma corrente que puxa o inimigo até você e o deixa lento.',
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 80}, {"type": "KNOCKBACK", "value_flat": -500}, {"type": "SLOW", "duration_ms": 2000, "value_percent": -40}]'
WHERE skill_id = 8;
CALL umbra_r4_ranks('BARB_CHAIN_THROW', 0, 0, 0, 0,
  '[{"type": "ROOT", "duration_ms": 1000}]', '[{"type": "STUN", "duration_ms": 1000}]');

CALL umbra_r4_ranks('BARB_INSTINCT_RECOVERY', 0, 0, 0, 0,
  '[{"type": "HOT", "duration_ms": 6000, "target_stat": "health", "value_percent": 10, "tick_interval_ms": 2000}]',
  '[{"type": "CLEANSE", "value_flat": 3}]');

CALL umbra_r4_ranks('BARB_CRIMSON_FRENZY', 0, 0, 0, 0, NULL,
  '[{"type": "BUFF_STAT", "duration_ms": 12000, "target_stat": "lifesteal", "value_percent": 10}]');

CALL umbra_r4_ranks('BARB_CRUSHING_IMPACT', 2000, 2000, 0, 0, NULL,
  '[{"type": "STUN", "duration_ms": 1500, "chance_percent": 50}]');

UPDATE skills SET
  description = 'Instinto de caçador: mais precisão e esquiva.',
  effects_json = '[{"type": "BUFF_STAT", "value_flat": 40, "target_stat": "accuracy", "value_percent": 10}, {"type": "BUFF_STAT", "value_flat": 15, "target_stat": "dodge"}]'
WHERE skill_key = 'BARB_RELENTLESS_HUNT';

CALL umbra_r4_ranks('BARB_AVATAR_RUIN', 0, 0, 0, 0,
  '[{"type": "BUFF_STAT", "duration_ms": 20000, "target_stat": "stun_resist", "value_percent": 30}]', NULL);

-- ============================================================================
-- Templar
-- ============================================================================

-- Golpe Consagrado / Julgamento: mantêm a progressão de resistência; ganham o marco do rank 5.
UPDATE skill_rank_scaling r INNER JOIN skills s ON s.skill_id = r.skill_id
SET r.extra_effects_json = JSON_ARRAY_APPEND(COALESCE(r.extra_effects_json, JSON_ARRAY()), '$',
  JSON_OBJECT('type', 'SILENCE', 'duration_ms', 1000, 'chance_percent', 40))
WHERE s.skill_key = 'TEMP_HOLY_STRIKE' AND r.`rank` = 5
  AND JSON_SEARCH(COALESCE(r.extra_effects_json, JSON_ARRAY()), 'one', 'SILENCE', NULL, '$[*].type') IS NULL;

UPDATE skill_rank_scaling r INNER JOIN skills s ON s.skill_id = r.skill_id
SET r.extra_effects_json = JSON_ARRAY_APPEND(COALESCE(r.extra_effects_json, JSON_ARRAY()), '$',
  JSON_OBJECT('type', 'STUN', 'duration_ms', 1500))
WHERE s.skill_key = 'TEMP_JUDGMENT' AND r.`rank` = 5
  AND JSON_SEARCH(COALESCE(r.extra_effects_json, JSON_ARRAY()), 'one', 'STUN', NULL, '$[*].type') IS NULL;

UPDATE skills SET
  description = 'Voto de proteção: aumenta defesa física e mágica.',
  effects_json = '[{"type": "BUFF_STAT", "duration_ms": 30000, "target_stat": "physical_defense", "value_percent": 30}, {"type": "BUFF_STAT", "duration_ms": 30000, "target_stat": "magic_defense", "value_percent": 30}]'
WHERE skill_key = 'TEMP_VOW_SHIELD';
CALL umbra_r4_ranks('TEMP_VOW_SHIELD', 0, 0, 0, 0, NULL,
  '[{"type": "BUFF_STAT", "duration_ms": 30000, "target_stat": "damage_reduction", "value_percent": 10}]');

-- Desafio (substitui Égide Espiritual)
UPDATE skills SET skill_key = 'TEMP_CHALLENGE', skill_name = 'Desafio', type_id = 1, target_id = 4,
  power_coef = 0, resource_cost = 25, cooldown_ms = 15000, cast_time_ms = 0, duration_ms = 4000,
  range_min = 0, range_max = 0, area_radius = 400, requires_target = 0, is_interrupt = 0,
  description = 'Provoca os inimigos ao redor e reduz o dano recebido por alguns segundos.',
  effects_json = '[{"type": "TAUNT", "duration_ms": 4000}, {"type": "BUFF_STAT", "duration_ms": 4000, "target_stat": "damage_reduction", "value_percent": 15}]'
WHERE skill_id = 48;
CALL umbra_r4_ranks('TEMP_CHALLENGE', 0, 0, 0, 0,
  '[{"type": "DEBUFF_STAT", "duration_ms": 6000, "target_stat": "physical_attack", "value_percent": -10}]',
  '[{"type": "BUFF_STAT", "duration_ms": 4000, "target_stat": "damage_reduction", "value_percent": 10}]');

UPDATE skills SET
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 160}, {"type": "DEBUFF_STAT", "duration_ms": 6000, "target_stat": "magic_defense", "value_percent": -15}]'
WHERE skill_key = 'TEMP_ASH_LIGHT';
CALL umbra_r4_ranks('TEMP_ASH_LIGHT', 1000, 1000, 0, 0, NULL,
  '[{"type": "SILENCE", "duration_ms": 1500, "chance_percent": 40}]');

CALL umbra_r4_ranks('TEMP_GUARDIAN_STEP', 0, 0, 0, 0,
  '[{"type": "BUFF_STAT", "duration_ms": 6000, "target_stat": "movement_speed", "value_percent": 20}]',
  '[{"type": "CLEANSE", "value_flat": 2}]');

-- Barreira Rúnica: escudo 5000 fixo era alto demais (mantém a progressão de resistência).
UPDATE skills SET
  effects_json = '[{"type": "SHIELD", "value_flat": 400, "duration_ms": 60000, "value_percent": 25}]'
WHERE skill_key = 'TEMP_RUNIC_BARRIER';

CALL umbra_r4_ranks('TEMP_FIELD_PRAYER', 0, 0, 0, 0, NULL, '[{"type": "CLEANSE", "value_flat": 3}]');
CALL umbra_r4_ranks('TEMP_VOW_MARK', 0, 0, 0, 0, NULL, '[{"type": "ROOT", "duration_ms": 1000}]');
CALL umbra_r4_ranks('TEMP_ASH_MIRACLE', 0, 0, 0, 0, '[{"type": "CLEANSE", "value_flat": 3}]', NULL);

-- ============================================================================
-- DarkMage
-- ============================================================================

UPDATE skills SET damage_type = 'MAGIC' WHERE skill_key IN
  ('DMAGE_CRIMSON_MIST', 'DMAGE_RUPTURE_SEAL', 'DMAGE_CORRUPTION_ORB', 'DMAGE_MINOR_PORTAL');

CALL umbra_r4_ranks('DMAGE_CRIMSON_MIST', 500, 500, 0, 0, NULL,
  '[{"type": "SLOW", "duration_ms": 3000, "value_percent": -30}]');
CALL umbra_r4_ranks('DMAGE_DIMENSIONAL_MARK', 0, 0, 0, 0, NULL,
  '[{"type": "DOT", "duration_ms": 10000, "target_stat": "health", "value_percent": 15, "tick_interval_ms": 2000}]');

-- Correntes Sombrias (substitui Véu Sombrio)
UPDATE skills SET skill_key = 'DMAGE_SHADOW_CHAINS', skill_name = 'Correntes Sombrias', type_id = 1,
  target_id = 2, power_coef = 60, resource_cost = 30, cooldown_ms = 12000, cast_time_ms = 0,
  duration_ms = 0, range_min = 0, range_max = 1200, area_radius = 0, requires_target = 1,
  is_interrupt = 0, can_crit = 1, damage_type = 'MAGIC', scaling_stat_id = 2, element_id = 2,
  description = 'Correntes de sombra prendem o inimigo no lugar.',
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 60}, {"type": "ROOT", "duration_ms": 2000}]'
WHERE skill_id = 18;
CALL umbra_r4_ranks('DMAGE_SHADOW_CHAINS', 2000, 2000, 0, 0, NULL, '[{"type": "SILENCE", "duration_ms": 1000}]');

CALL umbra_r4_ranks('DMAGE_RUPTURE_SEAL', 0, 0, 0, 0, NULL, '[{"type": "SILENCE", "duration_ms": 1000}]');
CALL umbra_r4_ranks('DMAGE_CORRUPTION_ORB', 0, 0, 0, 0, NULL,
  '[{"type": "SLOW", "duration_ms": 3000, "value_percent": -30}]');
CALL umbra_r4_ranks('DMAGE_RUBY_CONVERGENCE', 0, 10000, 0, 0, NULL, NULL);

UPDATE skills SET effects_json = JSON_ARRAY_APPEND(effects_json, '$',
  JSON_OBJECT('type', 'SLOW', 'duration_ms', 4000, 'value_percent', -30))
WHERE skill_key = 'DMAGE_REALITY_TWIST'
  AND JSON_SEARCH(effects_json, 'one', 'SLOW', NULL, '$[*].type') IS NULL;

CALL umbra_r4_ranks('DMAGE_RIFT_STEP', 3000, 3000, 0, 0, NULL, NULL);
CALL umbra_r4_ranks('DMAGE_ARCANE_COLLAPSE', 0, 0, 0, 0, NULL,
  '[{"type": "STUN", "duration_ms": 1000, "chance_percent": 30}]');

-- Onda do Vazio (substitui Afinidade da Névoa)
UPDATE skills SET skill_key = 'DMAGE_VOID_WAVE', skill_name = 'Onda do Vazio', type_id = 1,
  target_id = 4, power_coef = 90, resource_cost = 40, cooldown_ms = 18000, cast_time_ms = 0,
  duration_ms = 0, range_min = 0, range_max = 0, area_radius = 400, requires_target = 0,
  is_interrupt = 0, can_crit = 1, damage_type = 'MAGIC', scaling_stat_id = 2, element_id = 2,
  description = 'Explosão do vazio ao redor: dano e repulsão dos inimigos.',
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 90}, {"type": "KNOCKBACK", "value_flat": 400}]'
WHERE skill_id = 29;
CALL umbra_r4_ranks('DMAGE_VOID_WAVE', 3000, 3000, 0, 0, NULL,
  '[{"type": "SLOW", "duration_ms": 3000, "value_percent": -30}]');

-- Portal Menor: stun vai para o rank 3 (ultimate tem 3 ranks).
CALL umbra_r4_ranks('DMAGE_MINOR_PORTAL', 0, 0, 0, 0,
  '[{"type": "STUN", "duration_ms": 1000, "chance_percent": 50}]', NULL);

-- ============================================================================
-- Alchemist
-- ============================================================================

-- Tônico de Campo sai do Nv 1 (linha 61) para o Nv 7 (linha 63), levando os visuais.
UPDATE skills t INNER JOIN skills s ON s.skill_id = 61
SET t.icon_path = s.icon_path, t.cast_anim_path = s.cast_anim_path, t.vfx_path = s.vfx_path,
    t.hit_vfx_path = s.hit_vfx_path, t.buff_vfx_path = s.buff_vfx_path, t.sfx_path = s.sfx_path
WHERE t.skill_id = 63 AND s.skill_key = 'ALCH_FIELD_TONIC';

-- Frasco Ácido (linha 61, Nv 1)
UPDATE skills SET skill_key = 'ALCH_ACID_FLASK', skill_name = 'Frasco Ácido', type_id = 1,
  target_id = 2, power_coef = 120, resource_cost = 15, cooldown_ms = 4000, cast_time_ms = 0,
  duration_ms = 0, range_min = 0, range_max = 900, area_radius = 0, requires_target = 1,
  is_interrupt = 0, can_crit = 1, damage_type = 'MAGIC', scaling_stat_id = 2, element_id = 5,
  description = 'Arremessa um frasco ácido: dano e corrosão contínua.',
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 120}, {"type": "DOT", "duration_ms": 6000, "target_stat": "health", "value_percent": 30, "tick_interval_ms": 2000}]'
WHERE skill_id = 61;
CALL umbra_r4_ranks('ALCH_ACID_FLASK', 0, 0, 0, 0,
  '[{"type": "DEBUFF_STAT", "duration_ms": 6000, "target_stat": "physical_defense", "value_percent": -10}]',
  '[{"type": "SLOW", "duration_ms": 3000, "value_percent": -30}]');

-- Tônico de Campo (linha 63, Nv 7)
UPDATE skills SET skill_key = 'ALCH_FIELD_TONIC', skill_name = 'Tônico de Campo', type_id = 1,
  target_id = 6, power_coef = 40, resource_cost = 20, cooldown_ms = 6000, cast_time_ms = 600,
  duration_ms = 0, range_min = 0, range_max = 250, area_radius = 400, requires_target = 0,
  is_interrupt = 0, damage_type = 'MAGIC', scaling_stat_id = 2,
  description = 'Borrifa um tônico que cura você e os aliados próximos.',
  effects_json = '[{"type": "HEAL", "target_stat": "health", "value_percent": 20}]'
WHERE skill_id = 63;
CALL umbra_r4_ranks('ALCH_FIELD_TONIC', 0, 0, 0, 0, NULL,
  '[{"type": "HOT", "duration_ms": 6000, "target_stat": "health", "value_percent": 8, "tick_interval_ms": 2000}]');

-- Elixir Ofensivo (linha 62)
UPDATE skills SET skill_key = 'ALCH_OFFENSIVE_ELIXIR', skill_name = 'Elixir Ofensivo', type_id = 3,
  target_id = 3, power_coef = 0, resource_cost = 25, cooldown_ms = 20000, cast_time_ms = 500,
  duration_ms = 60000, range_min = 0, range_max = 600, area_radius = 0, requires_target = 0,
  is_interrupt = 0,
  description = 'Elixir que aumenta força, inteligência e crítico do aliado.',
  effects_json = '[{"type": "BUFF_STAT", "value_flat": 30, "duration_ms": 60000, "target_stat": "strength", "value_percent": 15}, {"type": "BUFF_STAT", "value_flat": 30, "duration_ms": 60000, "target_stat": "intelligence", "value_percent": 15}, {"type": "BUFF_STAT", "value_flat": 10, "duration_ms": 60000, "target_stat": "critical_chance"}]'
WHERE skill_id = 62;
CALL umbra_r4_ranks('ALCH_OFFENSIVE_ELIXIR', 0, 0, 0, 30000, NULL, NULL);

-- Elixir Defensivo (linha 64)
UPDATE skills SET skill_key = 'ALCH_DEFENSIVE_ELIXIR', skill_name = 'Elixir Defensivo', type_id = 3,
  target_id = 3, power_coef = 0, resource_cost = 25, cooldown_ms = 20000, cast_time_ms = 500,
  duration_ms = 60000, range_min = 0, range_max = 600, area_radius = 0, requires_target = 0,
  is_interrupt = 0,
  description = 'Elixir que aumenta vitalidade, vida e defesa do aliado.',
  effects_json = '[{"type": "BUFF_STAT", "value_flat": 25, "duration_ms": 60000, "target_stat": "vitality", "value_percent": 12}, {"type": "BUFF_STAT", "value_flat": 100, "duration_ms": 60000, "target_stat": "max_health"}, {"type": "BUFF_STAT", "duration_ms": 60000, "target_stat": "physical_defense", "value_percent": 10}]'
WHERE skill_id = 64;
CALL umbra_r4_ranks('ALCH_DEFENSIVE_ELIXIR', 0, 0, 0, 30000, NULL, NULL);

-- Gás Paralisante (linha 65)
UPDATE skills SET skill_key = 'ALCH_PARALYZING_GAS', skill_name = 'Gás Paralisante', type_id = 1,
  target_id = 4, power_coef = 50, resource_cost = 35, cooldown_ms = 14000, cast_time_ms = 0,
  duration_ms = 0, range_min = 0, range_max = 900, area_radius = 300, requires_target = 0,
  is_interrupt = 0, can_crit = 1, damage_type = 'MAGIC', scaling_stat_id = 2, element_id = 5,
  description = 'Nuvem de gás que fere e deixa os inimigos lentos.',
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 50}, {"type": "SLOW", "duration_ms": 4000, "value_percent": -40}]'
WHERE skill_id = 65;
CALL umbra_r4_ranks('ALCH_PARALYZING_GAS', 0, 0, 0, 0,
  '[{"type": "ROOT", "duration_ms": 1500}]', '[{"type": "SILENCE", "duration_ms": 1500}]');

-- Bomba Incendiária (linha 66)
UPDATE skills SET skill_key = 'ALCH_INCENDIARY_BOMB', skill_name = 'Bomba Incendiária', type_id = 1,
  target_id = 4, power_coef = 110, resource_cost = 40, cooldown_ms = 12000, cast_time_ms = 0,
  duration_ms = 0, range_min = 0, range_max = 900, area_radius = 250, requires_target = 0,
  is_interrupt = 0, can_crit = 1, damage_type = 'MAGIC', scaling_stat_id = 2, element_id = 3,
  description = 'Bomba que explode em chamas e queima os inimigos na área.',
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 110}, {"type": "DOT", "duration_ms": 6000, "target_stat": "health", "value_percent": 35, "tick_interval_ms": 2000}]'
WHERE skill_id = 66;
CALL umbra_r4_ranks('ALCH_INCENDIARY_BOMB', 2000, 2000, 0, 0, NULL,
  '[{"type": "INTERRUPT", "chance_percent": 50}]');

CALL umbra_r4_ranks('ALCH_RESTORING_MIST', 0, 0, 0, 0, NULL, '[{"type": "CLEANSE", "value_flat": 3}]');
CALL umbra_r4_ranks('ALCH_PROTECTIVE_COAT', 0, 0, 0, 0, NULL,
  '[{"type": "BUFF_STAT", "duration_ms": 30000, "target_stat": "damage_reduction", "value_percent": 10}]');
CALL umbra_r4_ranks('ALCH_NEUTRALIZER', 0, 0, 0, 0, NULL,
  '[{"type": "HOT", "duration_ms": 6000, "target_stat": "health", "value_percent": 10, "tick_interval_ms": 2000}]');
CALL umbra_r4_ranks('ALCH_DISRUPTIVE_BOMB', 0, 0, 0, 0, NULL,
  '[{"type": "STUN", "duration_ms": 1000, "chance_percent": 30}]');
CALL umbra_r4_ranks('ALCH_SURVIVAL_PROTOCOL', 0, 0, 0, 0, '[{"type": "CLEANSE", "value_flat": 3}]', NULL);

-- Buffs de aliado: sem alvo = self-cast (fase A), não exigem alvo selecionado.
UPDATE skills SET requires_target = 0 WHERE class_id = 4 AND target_id IN (3, 5, 6);

-- ============================================================================
-- Assassin
-- ============================================================================

UPDATE skills SET
  description = 'Golpe vindo das sombras. Saindo da furtividade, o crítico é garantido.',
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 160, "conditions_json": {"crit_if_stealthed": true}}, {"type": "INTERRUPT", "duration_ms": 1000, "chance_percent": 50}]'
WHERE skill_key = 'ASSN_SHADOW_STRIKE';
CALL umbra_r4_ranks('ASSN_SHADOW_STRIKE', 500, 500, 0, 0, NULL, '[{"type": "SILENCE", "duration_ms": 1500}]');

UPDATE skills SET duration_ms = 5000,
  description = 'Fica furtivo e mais rápido. Atacar ou usar habilidade ofensiva revela você.',
  effects_json = '[{"type": "STEALTH", "duration_ms": 5000}, {"type": "BUFF_STAT", "duration_ms": 5000, "target_stat": "movement_speed", "value_percent": 20}]'
WHERE skill_key = 'ASSN_INVISIBLE_STEP';
CALL umbra_r4_ranks('ASSN_INVISIBLE_STEP', 0, 0, 2000, 2000, NULL,
  '[{"type": "BUFF_STAT", "duration_ms": 7000, "target_stat": "movement_speed", "value_percent": 15}]');

CALL umbra_r4_ranks('ASSN_PRECISE_BLADE', 0, 0, 0, 0, NULL,
  '[{"type": "BUFF_STAT", "duration_ms": 5000, "target_stat": "ignore_dodge", "value_percent": 20}]');
CALL umbra_r4_ranks('ASSN_DOUBLE_ATTACK', 0, 0, 0, 0, NULL,
  '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 60}]');
CALL umbra_r4_ranks('ASSN_TENDON_CUT', 0, 0, 0, 0, '[{"type": "ROOT", "duration_ms": 1000}]', NULL);
CALL umbra_r4_ranks('ASSN_DEEP_BLEED', 0, 0, 0, 0, NULL,
  '[{"type": "DEBUFF_STAT", "duration_ms": 10000, "target_stat": "physical_defense", "value_percent": -10}]');

-- Bomba de Fumaça (substitui Véu de Fumaça)
UPDATE skills SET skill_key = 'ASSN_SMOKE_BOMB', skill_name = 'Bomba de Fumaça', type_id = 1,
  target_id = 4, power_coef = 0, resource_cost = 30, cooldown_ms = 25000, cast_time_ms = 0,
  duration_ms = 0, range_min = 0, range_max = 0, area_radius = 300, requires_target = 0,
  is_interrupt = 0, can_move_while_casting = 1,
  description = 'Fumaça ao redor que silencia e cega os inimigos.',
  effects_json = '[{"type": "SILENCE", "duration_ms": 2000}, {"type": "DEBUFF_STAT", "duration_ms": 4000, "target_stat": "accuracy", "value_percent": -20}]'
WHERE skill_id = 85;
CALL umbra_r4_ranks('ASSN_SMOKE_BOMB', 5000, 5000, 0, 0, NULL, '[{"type": "STEALTH", "duration_ms": 2000}]');

CALL umbra_r4_ranks('ASSN_PREDATOR_EYE', 0, 0, 0, 0, NULL,
  '[{"type": "BUFF_STAT", "duration_ms": 10000, "target_stat": "critical_chance", "value_percent": 15}]');
CALL umbra_r4_ranks('ASSN_PERFECT_KILL', 0, 0, 0, 0,
  '[{"type": "EXECUTE", "value_percent": 50, "conditions_json": {"target_health_below_percent": 30}}]', NULL);

-- ============================================================================
-- Martial
-- ============================================================================

CALL umbra_r4_ranks('MONK_TECH_CUT', 500, 500, 0, 0, NULL, NULL);
CALL umbra_r4_ranks('MONK_HIGH_GUARD', 0, 0, 0, 0, NULL,
  '[{"type": "BUFF_STAT", "duration_ms": 8000, "target_stat": "damage_reduction", "value_percent": 10}]');
CALL umbra_r4_ranks('MONK_DISCIPLINE_SEQUENCE', 0, 0, 0, 0, NULL,
  '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 70}]');
CALL umbra_r4_ranks('MONK_TACTICAL_STEP', 0, 0, 0, 0, NULL, '[{"type": "CLEANSE", "value_flat": 2}]');
CALL umbra_r4_ranks('MONK_COMBAT_BREATH', 0, 0, 0, 0, NULL, '[{"type": "CLEANSE", "value_flat": 3}]');

-- Chute Giratório (substitui Foco de Batalha)
UPDATE skills SET skill_key = 'MONK_SPINNING_KICK', skill_name = 'Chute Giratório', type_id = 1,
  target_id = 4, power_coef = 120, resource_cost = 30, cooldown_ms = 15000, cast_time_ms = 0,
  duration_ms = 0, range_min = 0, range_max = 0, area_radius = 250, requires_target = 0,
  is_interrupt = 0, can_crit = 1, damage_type = 'PHYSICAL', scaling_stat_id = 1, element_id = 1,
  description = 'Chute giratório que atinge e empurra os inimigos ao redor.',
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 120}, {"type": "KNOCKBACK", "value_flat": 350}, {"type": "INTERRUPT", "chance_percent": 40}]'
WHERE skill_id = 40;
CALL umbra_r4_ranks('MONK_SPINNING_KICK', 3000, 3000, 0, 0, NULL,
  '[{"type": "STUN", "duration_ms": 1000, "chance_percent": 30}]');

-- Varredura (substitui Defesa Rotacional)
UPDATE skills SET skill_key = 'MONK_SWEEP', skill_name = 'Varredura', type_id = 1,
  target_id = 2, power_coef = 100, resource_cost = 30, cooldown_ms = 14000, cast_time_ms = 0,
  duration_ms = 0, range_min = 0, range_max = 200, area_radius = 0, requires_target = 1,
  is_interrupt = 0, can_crit = 1, damage_type = 'PHYSICAL', scaling_stat_id = 1, element_id = 1,
  description = 'Rasteira que derruba o inimigo e o prende no lugar.',
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 100}, {"type": "ROOT", "duration_ms": 1500}]'
WHERE skill_id = 41;
CALL umbra_r4_ranks('MONK_SWEEP', 2000, 2000, 0, 0, NULL,
  '[{"type": "SLOW", "duration_ms": 4000, "value_percent": -30}]');

UPDATE skills SET
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 180}, {"type": "EXECUTE", "value_percent": 20, "conditions_json": {"target_health_below_percent": 30}}, {"type": "LIFESTEAL", "value_percent": 20}]'
WHERE skill_key = 'MONK_MARTIAL_EXECUTION';

CALL umbra_r4_ranks('MONK_VETERAN_FORM', 0, 0, 0, 0,
  '[{"type": "BUFF_STAT", "duration_ms": 20000, "target_stat": "stun_resist", "value_percent": 30}]', NULL);

DROP PROCEDURE IF EXISTS umbra_r4_ranks;

-- Conferência
SELECT class_id, required_level, skill_key, skill_name, type_id, target_id, range_max, area_radius
FROM skills WHERE class_id IS NOT NULL AND is_basic_attack = 0
ORDER BY class_id, required_level;
