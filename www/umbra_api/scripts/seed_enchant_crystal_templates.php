<?php
/**
 * Cria/atualiza templates: Extrator de Encanto + "Cristal de Teste — {stat}".
 * Não envia itens para inventário de nenhum jogador.
 *
 * CLI:
 *   php seed_enchant_crystal_templates.php
 *
 * Browser (WAMP):
 *   http://localhost/umbra_api/scripts/seed_enchant_crystal_templates.php
 */

require_once __DIR__ . '/../config/database.php';
require_once __DIR__ . '/../helpers/stat_key_mapping.php';

$isCli = (PHP_SAPI === 'cli');
if (!$isCli) {
    header('Content-Type: text/plain; charset=utf-8');
}

function crystal_seed_out(string $line): void
{
    echo $line . (PHP_SAPI === 'cli' ? PHP_EOL : "\n");
}

try {
    $pdo = getConnection();
    if (!$pdo) {
        throw new RuntimeException('Sem conexão MySQL (config/database.php).');
    }

    $pdo->beginTransaction();

    $pdo->exec("
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
        )
    ");

    $extractorId = (int)$pdo->query("
        SELECT item_id FROM item_templates WHERE item_subtype = 'enchant_extractor' ORDER BY item_id ASC LIMIT 1
    ")->fetchColumn();
    if ($extractorId <= 0) {
        throw new RuntimeException('Não foi possível criar/encontrar o Extrator de Encanto.');
    }
    crystal_seed_out("Extrator de Encanto item_id={$extractorId}");

    $stats = $pdo->query('SELECT stat_key, display_name, value_max FROM enchant_stat_weights ORDER BY tier ASC, stat_key ASC')
        ->fetchAll(PDO::FETCH_ASSOC);
    if (!$stats) {
        throw new RuntimeException('enchant_stat_weights vazio. Rode add_item_enchantment_system.sql primeiro.');
    }

    $findByName = $pdo->prepare('SELECT item_id FROM item_templates WHERE item_name = ? LIMIT 1');
    $created = 0;
    $updated = 0;

    foreach ($stats as $row) {
        $key = map_target_stat_to_canonical((string)$row['stat_key']);
        $label = (string)($row['display_name'] ?: $key);
        $name = 'Cristal de Teste — ' . $label;
        $desc = sprintf(
            'TESTE: aplica %s (%s) no valor máximo da tabela (até %d). Não sorteia aleatório.',
            $label,
            $key,
            (int)$row['value_max']
        );
        $json = json_encode(['enchant_force_stat' => $key], JSON_UNESCAPED_UNICODE);

        $findByName->execute([$name]);
        $existingId = (int)$findByName->fetchColumn();
        if ($existingId > 0) {
            $pdo->prepare('
                UPDATE item_templates
                SET item_description = :d, item_subtype = \'enchant_crystal\', stats_json = :j, item_category = \'upgrade\'
                WHERE item_id = :id
            ')->execute(['d' => $desc, 'j' => $json, 'id' => $existingId]);
            $id = $existingId;
            $updated++;
        } else {
            $ins = $pdo->prepare('
                INSERT INTO item_templates (
                  item_name, item_description, item_type, item_subtype, equipment_slot,
                  max_stack_size, icon_path, rarity, value, weight, item_category, tradeable, can_be_refined, stats_json
                ) VALUES (
                  :name, :desc, \'material\', \'enchant_crystal\', \'none\',
                  999, \'/Game/UI/Icons/Items/ICO_EnchantCrystal\', \'rare\', 1, 0.1, \'upgrade\', TRUE, FALSE, :json
                )
            ');
            $ins->execute(['name' => $name, 'desc' => $desc, 'json' => $json]);
            $id = (int)$pdo->lastInsertId();
            $created++;
        }

        crystal_seed_out("Cristal item_id={$id}  {$name}  force={$key}");
    }

    $pdo->commit();
    crystal_seed_out('');
    crystal_seed_out('OK. Extrator + ' . count($stats) . ' cristais de teste (templates only).');
    crystal_seed_out("Cristais novos: {$created} | Cristais atualizados: {$updated}");
    crystal_seed_out('Nenhum item foi adicionado ao inventário de jogadores.');
} catch (Throwable $e) {
    if (isset($pdo) && $pdo instanceof PDO && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    http_response_code(500);
    crystal_seed_out('ERRO: ' . $e->getMessage());
    exit(1);
}
