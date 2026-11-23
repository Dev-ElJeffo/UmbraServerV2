-- Revisão de skills, fase C: correção dos dados das skills das classes.
-- Idempotente: cada UPDATE regrava o mesmo valor final; o INTERRUPT só é adicionado se
-- o efeito ainda não existir. Depende de add_skill_effects_interrupt.sql (fase B) só
-- para a tabela skill_effects; os efeitos abaixo ficam em skills.effects_json.
-- Uso: mysql -u root -p umbra_eternum < www\umbra_api\scripts\fix_skills_revisao_v1.sql
-- Depois: reload_skills na zone (salvar qualquer skill no Manager) ou reiniciar a zone.

SET NAMES utf8mb4;

-- ============================================================================
-- Alchemist
-- ============================================================================

-- Tônico de Campo: era BUFF + AREA (inimigos) com override ALLY. Vira cura em área de
-- aliados (o caster sempre entra), escalando com MAG_ATK.
UPDATE skills SET type_id = 1, target_id = 6, power_coef = 40, area_radius = 400,
  effects_json = '[{"type": "HEAL", "target_stat": "health", "value_percent": 20, "chance_percent": 100}]'
WHERE skill_key = 'ALCH_FIELD_TONIC';

-- Elixir de Força: o override AREA + include_caster=false mandava o buff para os inimigos.
UPDATE skills SET is_interrupt = 0, area_radius = 0,
  effects_json = '[{"type": "BUFF_STAT", "value_flat": 30, "duration_ms": 60000, "target_stat": "strength", "value_percent": 15, "chance_percent": 100}]'
WHERE skill_key = 'ALCH_STRENGTH_ELIXIR';

-- Neutralizador: redirect_to_self só vale em REACTION; o self-cast de ALLY já cobre.
UPDATE skills SET
  effects_json = '[{"type": "CLEANSE", "value_flat": 3, "chance_percent": 100}]'
WHERE skill_key = 'ALCH_NEUTRALIZER';

-- Curas que não escalavam (power_coef = % do MAG_ATK somado à cura).
UPDATE skills SET power_coef = 40 WHERE skill_key = 'ALCH_RESTORING_MIST';
UPDATE skills SET power_coef = 50 WHERE skill_key = 'ALCH_SURVIVAL_PROTOCOL';

-- Bomba Disruptiva: 50% de interromper casts na área.
UPDATE skills SET effects_json = JSON_ARRAY_APPEND(effects_json, '$',
  JSON_OBJECT('type', 'INTERRUPT', 'chance_percent', 50, 'duration_ms', 0))
WHERE skill_key = 'ALCH_DISRUPTIVE_BOMB'
  AND JSON_SEARCH(effects_json, 'one', 'INTERRUPT', NULL, '$[*].type') IS NULL;

-- ============================================================================
-- Templar
-- ============================================================================

-- Oração de Campo: override SELF impedia curar o aliado selecionado.
UPDATE skills SET power_coef = 40,
  effects_json = '[{"type": "HEAL", "target_stat": "health", "value_percent": 25, "chance_percent": 100}]'
WHERE skill_key = 'TEMP_FIELD_PRAYER';

-- Milagre das Cinzas: a cura (override SELF) agora vai para o grupo na área.
UPDATE skills SET power_coef = 60,
  effects_json = '[{"type": "HEAL", "target_stat": "health", "value_percent": 35, "chance_percent": 100}, {"type": "BUFF_STAT", "duration_ms": 35000, "target_stat": "physical_defense", "value_percent": 50, "chance_percent": 100}, {"type": "BUFF_STAT", "duration_ms": 35000, "target_stat": "magic_defense", "value_percent": 50, "chance_percent": 100}]'
WHERE skill_key = 'TEMP_ASH_MIRACLE';

-- Aura do Guardião: sem duração caía nos 5s padrão; dura o mesmo que o cooldown.
UPDATE skills SET duration_ms = 60000,
  effects_json = '[{"type": "BUFF_STAT", "value_flat": 20, "duration_ms": 60000, "target_stat": "critical_resistance", "value_percent": 10, "chance_percent": 100}]'
WHERE skill_key = 'TEMP_GUARDIAN_AURA';

-- Chama do Voto: overrides PARTY redundantes (a skill já é AREA_ALLY com o caster).
UPDATE skills SET
  effects_json = '[{"type": "BUFF_STAT", "duration_ms": 60000, "target_stat": "physical_attack", "value_percent": 30, "chance_percent": 100}, {"type": "BUFF_STAT", "duration_ms": 60000, "target_stat": "magic_attack", "value_percent": 30, "chance_percent": 100}]'
WHERE skill_key = 'TEMP_VOW_FLAME';

-- Golpe Consagrado: 40% de interromper.
UPDATE skills SET effects_json = JSON_ARRAY_APPEND(effects_json, '$',
  JSON_OBJECT('type', 'INTERRUPT', 'chance_percent', 40, 'duration_ms', 0))
WHERE skill_key = 'TEMP_HOLY_STRIKE'
  AND JSON_SEARCH(effects_json, 'one', 'INTERRUPT', NULL, '$[*].type') IS NULL;

-- ============================================================================
-- Barbarian
-- ============================================================================

-- Rugido de Guerra: buff próprio, o checkbox "Interrompe" não fazia sentido.
UPDATE skills SET is_interrupt = 0 WHERE skill_key = 'BARB_WAR_CRY';

-- Golpe da Ruína: buff de double attack sem duração explícita (era 5s implícito).
UPDATE skills SET
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 180, "chance_percent": 100}, {"type": "BUFF_STAT", "duration_ms": 5000, "target_stat": "double_attack_rate", "value_percent": 25, "chance_percent": 100}]'
WHERE skill_key = 'BARB_RUIN_STRIKE';

-- Impacto Esmagador: 60% de interromper + 1,5s de silêncio.
UPDATE skills SET effects_json = JSON_ARRAY_APPEND(effects_json, '$',
  JSON_OBJECT('type', 'INTERRUPT', 'chance_percent', 60, 'duration_ms', 1500))
WHERE skill_key = 'BARB_CRUSHING_IMPACT'
  AND JSON_SEARCH(effects_json, 'one', 'INTERRUPT', NULL, '$[*].type') IS NULL;

-- ============================================================================
-- Assassin
-- ============================================================================

-- Lâmina Precisa: precisão no caster com duração explícita.
UPDATE skills SET
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 140, "chance_percent": 100}, {"type": "BUFF_STAT", "value_flat": 30, "duration_ms": 5000, "target_stat": "accuracy", "chance_percent": 100}]'
WHERE skill_key = 'ASSN_PRECISE_BLADE';

-- Golpe nas Sombras: 50% de interromper + 1s de silêncio.
UPDATE skills SET effects_json = JSON_ARRAY_APPEND(effects_json, '$',
  JSON_OBJECT('type', 'INTERRUPT', 'chance_percent', 50, 'duration_ms', 1000))
WHERE skill_key = 'ASSN_SHADOW_STRIKE'
  AND JSON_SEARCH(effects_json, 'one', 'INTERRUPT', NULL, '$[*].type') IS NULL;

-- ============================================================================
-- Martial
-- ============================================================================

-- Corte Técnico: precisão no caster com duração explícita + 40% de interromper.
UPDATE skills SET
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 150, "chance_percent": 100}, {"type": "BUFF_STAT", "value_flat": 20, "duration_ms": 5000, "target_stat": "accuracy", "chance_percent": 100}, {"type": "INTERRUPT", "chance_percent": 40, "duration_ms": 0}]'
WHERE skill_key = 'MONK_TECH_CUT';

-- Sequência Disciplinada: double attack no caster com duração explícita.
UPDATE skills SET
  effects_json = '[{"type": "DAMAGE", "target_stat": "health", "value_percent": 130, "chance_percent": 100}, {"type": "BUFF_STAT", "duration_ms": 5000, "target_stat": "double_attack_rate", "value_percent": 30, "chance_percent": 100}]'
WHERE skill_key = 'MONK_DISCIPLINE_SEQUENCE';

-- ============================================================================
-- Dark Mage
-- ============================================================================

-- Torção da Realidade: 50% de interromper + 2s de silêncio (contrafeitiço).
UPDATE skills SET effects_json = JSON_ARRAY_APPEND(effects_json, '$',
  JSON_OBJECT('type', 'INTERRUPT', 'chance_percent', 50, 'duration_ms', 2000))
WHERE skill_key = 'DMAGE_REALITY_TWIST'
  AND JSON_SEARCH(effects_json, 'one', 'INTERRUPT', NULL, '$[*].type') IS NULL;

-- Conferência
SELECT skill_key, type_id, target_id, power_coef, area_radius, is_interrupt, effects_json
FROM skills
WHERE skill_key IN ('ALCH_FIELD_TONIC', 'ALCH_STRENGTH_ELIXIR', 'ALCH_NEUTRALIZER',
  'ALCH_DISRUPTIVE_BOMB', 'TEMP_FIELD_PRAYER', 'TEMP_ASH_MIRACLE', 'TEMP_GUARDIAN_AURA',
  'TEMP_VOW_FLAME', 'TEMP_HOLY_STRIKE', 'BARB_RUIN_STRIKE', 'BARB_CRUSHING_IMPACT',
  'ASSN_PRECISE_BLADE', 'ASSN_SHADOW_STRIKE', 'MONK_TECH_CUT', 'MONK_DISCIPLINE_SEQUENCE',
  'DMAGE_REALITY_TWIST');
