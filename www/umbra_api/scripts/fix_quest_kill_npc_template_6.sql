-- Ajusta objetivo kill da quest de treino para npc_template_id = 6.
-- NÃO altera zone_id: neste projeto a zone do servidor é 0 (zone_server sem argumento ou zone_id=0).

UPDATE quest_objectives qo
INNER JOIN quests q ON q.quest_id = qo.quest_id
SET qo.params_json = JSON_SET(
  COALESCE(qo.params_json, JSON_OBJECT()),
  '$.npc_template_id', 6,
  '$.required_count', COALESCE(CAST(JSON_UNQUOTE(JSON_EXTRACT(qo.params_json, '$.required_count')) AS UNSIGNED), 3)
)
WHERE q.quest_key = 'quest_merchant_kill_training'
  AND qo.objective_type = 'kill';

-- Reseta instância de treino morta para testar respawn (mantém zone_id atual, ex.: 0)
UPDATE npc_instances
SET current_health = (SELECT max_health FROM npc_templates WHERE npc_template_id = npc_instances.npc_template_id),
    current_mana = (SELECT max_mana FROM npc_templates WHERE npc_template_id = npc_instances.npc_template_id),
    is_dead = 0,
    respawn_at = NULL
WHERE npc_template_id = 6 AND is_dead = 1;
