-- Itens de teste de encantamento: 1 extrator + 1 cristal por stat_key.
-- Idempotente. Cristais de teste usam o mesmo item_subtype=enchant_crystal
-- (o apply lê stats_json.enchant_force_stat e aplica esse afixo no valor máximo da tabela).
-- O "Cristal de Encantamento" genérico (roll aleatório) NÃO é alterado.

USE umbra_eternum;

INSERT INTO item_templates (
  item_name, item_description, item_type, item_subtype, equipment_slot,
  max_stack_size, icon_path, rarity, value, weight, item_category, tradeable, can_be_refined
)
SELECT
  'Extrator de Encanto',
  'Remove um afixo escolhido do equipamento, liberando o slot para um novo cristal.',
  'material', 'enchant_extractor', 'none',
  999, '/Game/UI/Icons/Items/ICO_EnchantExtractor', 'rare', 200, 0.1, 'upgrade', TRUE, FALSE
WHERE NOT EXISTS (
  SELECT 1 FROM item_templates WHERE item_subtype = 'enchant_extractor' LIMIT 1
);

INSERT INTO item_templates (
  item_name, item_description, item_type, item_subtype, equipment_slot,
  max_stack_size, icon_path, rarity, value, weight, item_category, tradeable, can_be_refined, stats_json
)
SELECT
  CONCAT('Cristal de Teste — ', w.display_name),
  CONCAT('TESTE: aplica o afixo ', w.display_name, ' (', w.stat_key, ') no valor máximo configurado. Não sorteia aleatório.'),
  'material', 'enchant_crystal', 'none',
  999, '/Game/UI/Icons/Items/ICO_EnchantCrystal', 'rare', 1, 0.1, 'upgrade', TRUE, FALSE,
  JSON_OBJECT('enchant_force_stat', w.stat_key)
FROM enchant_stat_weights w
WHERE NOT EXISTS (
  SELECT 1 FROM item_templates t
  WHERE t.item_subtype = 'enchant_crystal'
    AND t.item_name = CONCAT('Cristal de Teste — ', w.display_name)
);

-- Reaplica stats_json se o cristal de teste já existia sem force_stat
UPDATE item_templates t
INNER JOIN enchant_stat_weights w
  ON t.item_name = CONCAT('Cristal de Teste — ', w.display_name)
SET
  t.item_subtype = 'enchant_crystal',
  t.item_category = 'upgrade',
  t.stats_json = JSON_OBJECT('enchant_force_stat', w.stat_key),
  t.item_description = CONCAT('TESTE: aplica o afixo ', w.display_name, ' (', w.stat_key, ') no valor máximo configurado.')
WHERE t.item_subtype = 'enchant_crystal';

SELECT item_id, item_name, item_subtype, stats_json
FROM item_templates
WHERE item_subtype IN ('enchant_crystal', 'enchant_extractor')
ORDER BY item_subtype, item_name;
