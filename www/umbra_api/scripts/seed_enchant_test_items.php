<?php
/**
 * Cria o Extrator de Encanto e um Cristal de Teste para cada stat em enchant_stat_weights.
 *
 * CLI:
 *   php seed_enchant_test_items.php
 *   php seed_enchant_test_items.php --player_id=1 --qty=20
 *
 * Browser (WAMP):
 *   http://localhost/umbra_api/scripts/seed_enchant_test_items.php?player_id=1&qty=20
 */

require_once __DIR__ . '/../config/database.php';
require_once __DIR__ . '/../helpers/stat_key_mapping.php';
require_once __DIR__ . '/../helpers/enchant_helper.php';

$isCli = (PHP_SAPI === 'cli');
if (!$isCli) {
    header('Content-Type: text/plain; charset=utf-8');
}

$playerId = 0;
$qty = 20;
if ($isCli) {
    foreach (array_slice($argv, 1) as $arg) {
        if (preg_match('/^--player_id=(\d+)$/', $arg, $m)) {
            $playerId = (int)$m[1];
        }
        if (preg_match('/^--qty=(\d+)$/', $arg, $m)) {
            $qty = max(1, (int)$m[1]);
        }
    }
} else {
    $playerId = (int)($_GET['player_id'] ?? 0);
    $qty = max(1, (int)($_GET['qty'] ?? 20));
}

function seed_out(string $line): void
{
    echo $line . (PHP_SAPI === 'cli' ? PHP_EOL : "\n");
}

function seed_find_empty_bag_slot(PDO $pdo, int $playerId): ?int
{
    $st = $pdo->prepare('
        SELECT slot_index FROM player_inventory
        WHERE player_id = ? AND slot_index BETWEEN 0 AND 49
    ');
    $st->execute([$playerId]);
    $used = [];
    foreach ($st->fetchAll(PDO::FETCH_COLUMN) as $idx) {
        $used[(int)$idx] = true;
    }
    for ($i = 0; $i < 50; $i++) {
        if (empty($used[$i])) {
            return $i;
        }
    }
    return null;
}

function seed_grant_stack(PDO $pdo, int $playerId, int $templateId, int $qty, int $maxStack): bool
{
    $qty = min($qty, $maxStack);
    $st = $pdo->prepare('
        SELECT inventory_id, quantity FROM player_inventory
        WHERE player_id = :pid AND item_template_id = :tid
          AND is_equipped = 0 AND slot_index BETWEEN 0 AND 49
          AND (auction_listing_id IS NULL)
        ORDER BY quantity ASC
        LIMIT 1
        FOR UPDATE
    ');
    $st->execute(['pid' => $playerId, 'tid' => $templateId]);
    $row = $st->fetch(PDO::FETCH_ASSOC);
    if ($row) {
        $newQty = min($maxStack, (int)$row['quantity'] + $qty);
        $pdo->prepare('UPDATE player_inventory SET quantity = ? WHERE inventory_id = ?')
            ->execute([$newQty, $row['inventory_id']]);
        return true;
    }
    $slot = seed_find_empty_bag_slot($pdo, $playerId);
    if ($slot === null) {
        return false;
    }
    $ins = $pdo->prepare('
        INSERT INTO player_inventory (player_id, item_template_id, quantity, slot_index, is_equipped)
        VALUES (:pid, :tid, :qty, :slot, 0)
    ');
    $ins->execute([
        'pid' => $playerId,
        'tid' => $templateId,
        'qty' => $qty,
        'slot' => $slot,
    ]);
    return true;
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
    seed_out("Extrator de Encanto item_id={$extractorId}");

    $stats = $pdo->query('SELECT stat_key, display_name, value_max FROM enchant_stat_weights ORDER BY tier ASC, stat_key ASC')
        ->fetchAll(PDO::FETCH_ASSOC);
    if (!$stats) {
        throw new RuntimeException('enchant_stat_weights vazio. Rode add_item_enchantment_system.sql primeiro.');
    }

    $findByName = $pdo->prepare('SELECT item_id FROM item_templates WHERE item_name = ? LIMIT 1');
    $created = [];
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
        }
        $created[] = ['item_id' => $id, 'name' => $name, 'stat_key' => $key];
        seed_out("Cristal item_id={$id}  {$name}  force={$key}");
    }

    $granted = 0;
    $failedGrant = [];
    if ($playerId > 0) {
        $exists = $pdo->prepare('SELECT id FROM players WHERE id = ? LIMIT 1');
        $exists->execute([$playerId]);
        if (!$exists->fetchColumn()) {
            throw new RuntimeException("player_id={$playerId} não existe.");
        }
        $maxSt = $pdo->prepare('SELECT max_stack_size FROM item_templates WHERE item_id = ?');
        $maxSt->execute([$extractorId]);
        $extMax = max(1, (int)$maxSt->fetchColumn());
        if (!seed_grant_stack($pdo, $playerId, $extractorId, $qty, $extMax)) {
            $failedGrant[] = 'Extrator de Encanto (bag cheia)';
        } else {
            $granted++;
            seed_out("Grant: Extrator x{$qty} -> player {$playerId}");
        }
        foreach ($created as $c) {
            $maxSt->execute([$c['item_id']]);
            $cMax = max(1, (int)$maxSt->fetchColumn());
            if (!seed_grant_stack($pdo, $playerId, $c['item_id'], $qty, $cMax)) {
                $failedGrant[] = $c['name'] . ' (bag cheia)';
            } else {
                $granted++;
                seed_out("Grant: {$c['name']} x{$qty} -> player {$playerId}");
            }
        }
    }

    $pdo->commit();
    seed_out('');
    seed_out('OK. Extrator + ' . count($created) . ' cristais de teste.');
    if ($playerId > 0) {
        seed_out("Itens enviados à bag do player_id={$playerId}: {$granted}");
        if ($failedGrant) {
            seed_out('Falhou (sem slot): ' . implode(', ', $failedGrant));
        }
    } else {
        seed_out('Templates criados. Para enviar à bag: php seed_enchant_test_items.php --player_id=SEU_ID --qty=20');
    }
} catch (Throwable $e) {
    if (isset($pdo) && $pdo instanceof PDO && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    http_response_code(500);
    seed_out('ERRO: ' . $e->getMessage());
    exit(1);
}
