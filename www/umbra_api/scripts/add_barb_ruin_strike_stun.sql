-- Garante STUN no effects_json base do Golpe da Ruína (tooltip + combate mesmo sem extra_effects de rank).
UPDATE skills
SET effects_json = JSON_ARRAY_APPEND(
  COALESCE(effects_json, JSON_ARRAY()),
  '$',
  JSON_OBJECT('type', 'STUN', 'duration_ms', 2000, 'chance_percent', 100)
)
WHERE skill_key = 'BARB_RUIN_STRIKE'
  AND JSON_SEARCH(COALESCE(effects_json, JSON_ARRAY()), 'one', 'STUN', NULL, '$[*].type') IS NULL;
