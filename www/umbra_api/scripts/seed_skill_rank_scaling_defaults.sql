-- ============================================================================
-- UMBRA ETERNUM - Seed skill_rank_scaling (defaults + CC showcase)
-- Alinha power_coef_bonus ao fallback C++: +10% do base por rank acima 1
-- (rank 2 = +10%, rank 3 = +20%, ...). Idempotente via INSERT IGNORE.
-- Nota: `rank` é palavra reservada no MySQL 8 — sempre usar backticks.
-- ============================================================================

SET NAMES utf8mb4;

-- Defaults: ranks 2..max_rank para skills enabled sem row existente
INSERT IGNORE INTO skill_rank_scaling
  (skill_id, `rank`, power_coef_bonus, resource_cost_bonus, cooldown_reduction_ms, duration_bonus_ms, extra_effects_json)
SELECT
  s.skill_id,
  r.rank_n,
  ROUND(s.power_coef * 0.1 * (r.rank_n - 1)),
  0,
  0,
  0,
  NULL
FROM skills s
CROSS JOIN (
  SELECT 2 AS rank_n UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5
  UNION ALL SELECT 6 UNION ALL SELECT 7 UNION ALL SELECT 8 UNION ALL SELECT 9 UNION ALL SELECT 10
) r
WHERE s.is_enabled = 1
  AND r.rank_n <= GREATEST(1, s.max_rank)
  AND r.rank_n >= 2;

-- Showcase CC: Golpe da Ruína (BARB) — SILENCE no rank 3, STUN no rank 5
UPDATE skill_rank_scaling srs
JOIN skills s ON s.skill_id = srs.skill_id
SET
  srs.extra_effects_json = CASE
    WHEN srs.`rank` = 3 THEN JSON_ARRAY(
      JSON_OBJECT('type', 'SILENCE', 'duration_ms', 1500, 'chance_percent', 100)
    )
    WHEN srs.`rank` = 5 THEN JSON_ARRAY(
      JSON_OBJECT('type', 'STUN', 'duration_ms', 1000, 'chance_percent', 100)
    )
    ELSE srs.extra_effects_json
  END
WHERE s.skill_key = 'BARB_RUIN_STRIKE'
  AND srs.`rank` IN (3, 5);

-- Fallback se a skill de showcase não existir: primeira skill ACTIVE enabled
UPDATE skill_rank_scaling srs
JOIN (
  SELECT skill_id FROM skills
  WHERE is_enabled = 1 AND type_id = 1
  ORDER BY skill_id ASC
  LIMIT 1
) pick ON pick.skill_id = srs.skill_id
LEFT JOIN skills barb ON barb.skill_key = 'BARB_RUIN_STRIKE'
SET
  srs.extra_effects_json = CASE
    WHEN srs.`rank` = 3 THEN JSON_ARRAY(
      JSON_OBJECT('type', 'SILENCE', 'duration_ms', 1500, 'chance_percent', 100)
    )
    WHEN srs.`rank` = 5 THEN JSON_ARRAY(
      JSON_OBJECT('type', 'STUN', 'duration_ms', 1000, 'chance_percent', 100)
    )
    ELSE srs.extra_effects_json
  END
WHERE barb.skill_id IS NULL
  AND srs.`rank` IN (3, 5);
