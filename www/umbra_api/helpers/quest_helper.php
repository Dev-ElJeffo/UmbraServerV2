<?php
/**
 * Engine autoritativa de quests (aceitar, progresso, turn-in, recompensas).
 */

require_once __DIR__ . '/experience_helper.php';
require_once __DIR__ . '/npc_vendor_helper.php';

const QUEST_AREA_MARGIN = 50.0;

function questDecodeParams(?string $json): array
{
    if ($json === null || $json === '') {
        return [];
    }
    $decoded = json_decode($json, true);
    return is_array($decoded) ? $decoded : [];
}

function questLoadQuestRow(PDO $pdo, int $quest_id): ?array
{
    $stmt = $pdo->prepare('SELECT * FROM quests WHERE quest_id = ? AND is_active = 1 LIMIT 1');
    $stmt->execute([$quest_id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    return $row ?: null;
}

function questGetPlayerQuestRow(PDO $pdo, int $player_id, int $quest_id): ?array
{
    $stmt = $pdo->prepare('
        SELECT * FROM player_quests
        WHERE player_id = ? AND quest_id = ?
        ORDER BY player_quest_id DESC
        LIMIT 1
    ');
    $stmt->execute([$player_id, $quest_id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    return $row ?: null;
}

function questPlayerCompletedQuest(PDO $pdo, int $player_id, int $quest_id): bool
{
    $stmt = $pdo->prepare("
        SELECT 1 FROM player_quests
        WHERE player_id = ? AND quest_id = ? AND status = 'completed'
        LIMIT 1
    ");
    $stmt->execute([$player_id, $quest_id]);
    return (bool)$stmt->fetchColumn();
}

function questResolveAvailability(PDO $pdo, int $player_id, array $questRow, ?array $playerQuestRow): string
{
    if ($playerQuestRow && in_array((string)$playerQuestRow['status'], ['active', 'ready'], true)) {
        return (string)$playerQuestRow['status'];
    }
    // completed/failed/null: não bloquear se repeatable — permite nova run.
    if (!empty($questRow['prerequisite_quest_id'])) {
        if (!questPlayerCompletedQuest($pdo, $player_id, (int)$questRow['prerequisite_quest_id'])) {
            return 'locked';
        }
    }
    if (empty($questRow['repeatable']) && questPlayerCompletedQuest($pdo, $player_id, (int)$questRow['quest_id'])) {
        return 'completed';
    }
    $lvlStmt = $pdo->prepare('SELECT level FROM players WHERE id = ? LIMIT 1');
    $lvlStmt->execute([$player_id]);
    $playerLevel = (int)($lvlStmt->fetchColumn() ?: 1);
    if ($playerLevel < (int)$questRow['min_level']) {
        return 'locked';
    }
    foreach (questLoadStartRequirements($pdo, (int)$questRow['quest_id']) as $req) {
        $itemId = (int)($req['item_template_id'] ?? 0);
        $need = max(1, (int)($req['quantity'] ?? 1));
        if ($itemId <= 0) {
            continue;
        }
        if (questCountPlayerItem($pdo, $player_id, $itemId) < $need) {
            return 'locked';
        }
    }
    return 'available';
}

function questLoadObjectives(PDO $pdo, int $quest_id): array
{
    $stmt = $pdo->prepare('
        SELECT objective_id, sort_order, objective_type, description, params_json
        FROM quest_objectives
        WHERE quest_id = ?
        ORDER BY sort_order ASC, objective_id ASC
    ');
    $stmt->execute([$quest_id]);
    return $stmt->fetchAll(PDO::FETCH_ASSOC) ?: [];
}

function questLoadRewards(PDO $pdo, int $quest_id): array
{
    $stmt = $pdo->prepare('
        SELECT qr.reward_id, qr.reward_type, qr.amount, qr.item_template_id, qr.quantity,
               qr.choice_group_id, qr.sort_order,
               it.item_name, it.icon_path
        FROM quest_rewards qr
        LEFT JOIN item_templates it ON it.item_id = qr.item_template_id
        WHERE qr.quest_id = ?
        ORDER BY qr.sort_order ASC, qr.reward_id ASC
    ');
    $stmt->execute([$quest_id]);
    return $stmt->fetchAll(PDO::FETCH_ASSOC) ?: [];
}

function questLoadRewardChoices(PDO $pdo, int $quest_id): array
{
    $stmt = $pdo->prepare('
        SELECT qrc.choice_id, qrc.choice_group_id, qrc.label, qrc.reward_type, qrc.amount,
               qrc.item_template_id, qrc.quantity, qrc.sort_order,
               it.item_name, it.icon_path
        FROM quest_reward_choices qrc
        LEFT JOIN item_templates it ON it.item_id = qrc.item_template_id
        WHERE qrc.quest_id = ?
        ORDER BY qrc.choice_group_id ASC, qrc.sort_order ASC, qrc.choice_id ASC
    ');
    $stmt->execute([$quest_id]);
    return $stmt->fetchAll(PDO::FETCH_ASSOC) ?: [];
}

function questLoadAcceptGrants(PDO $pdo, int $quest_id): array
{
    $stmt = $pdo->prepare('
        SELECT g.grant_id, g.quest_id, g.item_template_id, g.quantity, g.sort_order,
               it.item_name, it.icon_path
        FROM quest_accept_grants g
        LEFT JOIN item_templates it ON it.item_id = g.item_template_id
        WHERE g.quest_id = ?
        ORDER BY g.sort_order ASC, g.grant_id ASC
    ');
    $stmt->execute([$quest_id]);
    return $stmt->fetchAll(PDO::FETCH_ASSOC) ?: [];
}

function questLoadStartRequirements(PDO $pdo, int $quest_id): array
{
    $stmt = $pdo->prepare('
        SELECT r.requirement_id, r.quest_id, r.item_template_id, r.quantity, r.sort_order,
               it.item_name, it.icon_path
        FROM quest_start_requirements r
        LEFT JOIN item_templates it ON it.item_id = r.item_template_id
        WHERE r.quest_id = ?
        ORDER BY r.sort_order ASC, r.requirement_id ASC
    ');
    $stmt->execute([$quest_id]);
    return $stmt->fetchAll(PDO::FETCH_ASSOC) ?: [];
}

function questFormatItemQtyRow(array $row, string $idKey): array
{
    return [
        $idKey => (int)($row[$idKey] ?? 0),
        'item_template_id' => (int)($row['item_template_id'] ?? 0),
        'item_name' => $row['item_name'] ?? '',
        'icon_path' => $row['icon_path'] ?? '',
        'quantity' => max(1, (int)($row['quantity'] ?? 1)),
        'sort_order' => (int)($row['sort_order'] ?? 0),
    ];
}

function questPlayerMeetsStartRequirements(PDO $pdo, int $player_id, int $quest_id): bool
{
    foreach (questLoadStartRequirements($pdo, $quest_id) as $req) {
        $itemId = (int)($req['item_template_id'] ?? 0);
        $need = max(1, (int)($req['quantity'] ?? 1));
        if ($itemId <= 0) {
            continue;
        }
        if (questCountPlayerItem($pdo, $player_id, $itemId) < $need) {
            return false;
        }
    }
    return true;
}

function questApplyAcceptGrants(PDO $pdo, int $player_id, int $quest_id): void
{
    foreach (questLoadAcceptGrants($pdo, $quest_id) as $grant) {
        $itemId = (int)($grant['item_template_id'] ?? 0);
        $qty = max(1, (int)($grant['quantity'] ?? 1));
        if ($itemId <= 0) {
            continue;
        }
        if (!questGrantItemToPlayer($pdo, $player_id, $itemId, $qty)) {
            throw new RuntimeException('Inventário cheio ou item inválido ao conceder item de aceite.');
        }
    }
}

function questRevokeAcceptGrants(PDO $pdo, int $player_id, int $quest_id): void
{
    foreach (questLoadAcceptGrants($pdo, $quest_id) as $grant) {
        $itemId = (int)($grant['item_template_id'] ?? 0);
        $qty = max(1, (int)($grant['quantity'] ?? 1));
        if ($itemId <= 0) {
            continue;
        }
        // Best-effort: remove até a quantidade concedida (não falha o abandon).
        questRemoveDeliverItems($pdo, $player_id, $itemId, min($qty, questCountPlayerItem($pdo, $player_id, $itemId)));
    }
}

function questCountPlayerItem(PDO $pdo, int $player_id, int $item_template_id): int
{
    $stmt = $pdo->prepare('
        SELECT COALESCE(SUM(quantity), 0)
        FROM player_inventory
        WHERE player_id = ? AND item_template_id = ? AND auction_listing_id IS NULL
    ');
    $stmt->execute([$player_id, $item_template_id]);
    return (int)$stmt->fetchColumn();
}

function questFormatRewardRow(array $row): array
{
    return [
        'reward_id' => (int)($row['reward_id'] ?? 0),
        'reward_type' => $row['reward_type'] ?? '',
        'amount' => (int)($row['amount'] ?? 0),
        'item_template_id' => isset($row['item_template_id']) ? (int)$row['item_template_id'] : 0,
        'item_name' => $row['item_name'] ?? '',
        'icon_path' => $row['icon_path'] ?? '',
        'quantity' => (int)($row['quantity'] ?? 1),
        'choice_group_id' => isset($row['choice_group_id']) ? (int)$row['choice_group_id'] : 0,
        'label' => $row['label'] ?? '',
        'choice_id' => isset($row['choice_id']) ? (int)$row['choice_id'] : 0,
    ];
}

function questFormatObjectiveForApi(array $objRow, ?array $progressRow = null): array
{
    $params = questDecodeParams($objRow['params_json'] ?? null);
    $required = (int)($params['required_count'] ?? 1);
    if ($required < 1) {
        $required = 1;
    }
    $current = (int)($progressRow['current_count'] ?? 0);
    $completed = !empty($progressRow['is_completed']);
    return [
        'objective_id' => (int)$objRow['objective_id'],
        'sort_order' => (int)$objRow['sort_order'],
        'objective_type' => $objRow['objective_type'],
        'description' => $objRow['description'] ?? '',
        'params' => $params,
        'current_count' => $current,
        'required_count' => $required,
        'is_completed' => $completed,
    ];
}

function questLoadPlayerObjectiveMap(PDO $pdo, int $player_quest_id): array
{
    $stmt = $pdo->prepare('
        SELECT objective_id, current_count, is_completed
        FROM player_quest_objectives
        WHERE player_quest_id = ?
    ');
    $stmt->execute([$player_quest_id]);
    $map = [];
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        $map[(int)$row['objective_id']] = $row;
    }
    return $map;
}

function questRefreshCollectProgress(PDO $pdo, int $player_id, int $player_quest_id): void
{
    $stmt = $pdo->prepare('
        SELECT pqo.player_objective_id, qo.objective_id, qo.objective_type, qo.params_json
        FROM player_quest_objectives pqo
        INNER JOIN quest_objectives qo ON qo.objective_id = pqo.objective_id
        WHERE pqo.player_quest_id = ? AND pqo.is_completed = 0 AND qo.objective_type = \'collect\'
    ');
    $stmt->execute([$player_quest_id]);
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        $params = questDecodeParams($row['params_json'] ?? null);
        $itemId = (int)($params['item_template_id'] ?? 0);
        $required = max(1, (int)($params['required_count'] ?? 1));
        if ($itemId <= 0) {
            continue;
        }
        $have = questCountPlayerItem($pdo, $player_id, $itemId);
        $count = min($have, $required);
        $done = $have >= $required ? 1 : 0;
        $upd = $pdo->prepare('
            UPDATE player_quest_objectives
            SET current_count = ?, is_completed = ?
            WHERE player_objective_id = ?
        ');
        $upd->execute([$count, $done, (int)$row['player_objective_id']]);
    }
}

function questRefreshDeliverReadiness(PDO $pdo, int $player_id, int $player_quest_id): void
{
    $stmt = $pdo->prepare('
        SELECT pqo.player_objective_id, qo.objective_id, qo.objective_type, qo.params_json
        FROM player_quest_objectives pqo
        INNER JOIN quest_objectives qo ON qo.objective_id = pqo.objective_id
        WHERE pqo.player_quest_id = ? AND qo.objective_type = \'deliver\'
    ');
    $stmt->execute([$player_quest_id]);
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        $params = questDecodeParams($row['params_json'] ?? null);
        $itemId = (int)($params['item_template_id'] ?? 0);
        $required = max(1, (int)($params['required_count'] ?? 1));
        if ($itemId <= 0) {
            continue;
        }
        $have = questCountPlayerItem($pdo, $player_id, $itemId);
        $count = min($have, $required);
        $done = $have >= $required ? 1 : 0;
        $upd = $pdo->prepare('
            UPDATE player_quest_objectives
            SET current_count = ?, is_completed = ?
            WHERE player_objective_id = ?
        ');
        $upd->execute([$count, $done, (int)$row['player_objective_id']]);
    }
}

function questRefreshPlayerQuestProgress(PDO $pdo, int $player_id, int $player_quest_id): void
{
    questRefreshCollectProgress($pdo, $player_id, $player_quest_id);
    questRefreshDeliverReadiness($pdo, $player_id, $player_quest_id);
    questPromoteToReadyIfComplete($pdo, $player_quest_id);
}

function questAllObjectivesComplete(PDO $pdo, int $player_quest_id): bool
{
    // Se a quest tem objetivos definidos mas o player não tem linhas de progresso
    // (ex.: CASCADE após editar quest no Manager), NÃO considerar completa.
    $stmt = $pdo->prepare('
        SELECT
            (SELECT COUNT(*)
             FROM quest_objectives qo
             INNER JOIN player_quests pq ON pq.quest_id = qo.quest_id
             WHERE pq.player_quest_id = ?) AS need_count,
            (SELECT COUNT(*)
             FROM player_quest_objectives
             WHERE player_quest_id = ?) AS have_count,
            (SELECT COUNT(*)
             FROM player_quest_objectives
             WHERE player_quest_id = ? AND is_completed = 0) AS incomplete_count
    ');
    $stmt->execute([$player_quest_id, $player_quest_id, $player_quest_id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC) ?: [];
    $need = (int)($row['need_count'] ?? 0);
    $have = (int)($row['have_count'] ?? 0);
    $incomplete = (int)($row['incomplete_count'] ?? 0);
    if ($need > 0 && $have < $need) {
        return false;
    }
    return $incomplete === 0;
}

/**
 * Recria player_quest_objectives após REPLACE de objetivos no admin,
 * e volta quests órfãs (ready sem progresso) para active.
 */
function questResyncPlayerObjectivesForQuest(PDO $pdo, int $quest_id): void
{
    $objStmt = $pdo->prepare('
        SELECT objective_id, objective_type, params_json
        FROM quest_objectives
        WHERE quest_id = ?
        ORDER BY sort_order ASC, objective_id ASC
    ');
    $objStmt->execute([$quest_id]);
    $objectives = $objStmt->fetchAll(PDO::FETCH_ASSOC);

    $pqStmt = $pdo->prepare("
        SELECT player_quest_id, player_id, status
        FROM player_quests
        WHERE quest_id = ? AND status IN ('active', 'ready')
    ");
    $pqStmt->execute([$quest_id]);
    $players = $pqStmt->fetchAll(PDO::FETCH_ASSOC);
    if (empty($players)) {
        return;
    }

    $ins = $pdo->prepare('
        INSERT INTO player_quest_objectives (player_quest_id, objective_id, current_count, is_completed)
        VALUES (?, ?, ?, ?)
        ON DUPLICATE KEY UPDATE objective_id = VALUES(objective_id)
    ');
    // Unique is (player_quest_id, objective_id) — ON DUPLICATE keeps existing counts.

    foreach ($players as $pq) {
        $playerQuestId = (int)$pq['player_quest_id'];
        $playerId = (int)$pq['player_id'];

        // Remove progresso de objetivos que não existem mais nesta quest.
        $pdo->prepare('
            DELETE pqo FROM player_quest_objectives pqo
            LEFT JOIN quest_objectives qo ON qo.objective_id = pqo.objective_id AND qo.quest_id = ?
            WHERE pqo.player_quest_id = ? AND qo.objective_id IS NULL
        ')->execute([$quest_id, $playerQuestId]);

        foreach ($objectives as $obj) {
            $objectiveId = (int)$obj['objective_id'];
            $exists = $pdo->prepare('
                SELECT 1 FROM player_quest_objectives
                WHERE player_quest_id = ? AND objective_id = ? LIMIT 1
            ');
            $exists->execute([$playerQuestId, $objectiveId]);
            if ($exists->fetchColumn()) {
                continue;
            }
            $params = questDecodeParams($obj['params_json'] ?? null);
            $initial = 0;
            $done = 0;
            if (($obj['objective_type'] ?? '') === 'talk') {
                $initial = 1;
                $done = 1;
            } elseif (($obj['objective_type'] ?? '') === 'collect') {
                $itemId = (int)($params['item_template_id'] ?? 0);
                $required = max(1, (int)($params['required_count'] ?? 1));
                $have = $itemId > 0 ? questCountPlayerItem($pdo, $playerId, $itemId) : 0;
                $initial = min($have, $required);
                $done = $have >= $required ? 1 : 0;
            }
            $ins->execute([$playerQuestId, $objectiveId, $initial, $done]);
        }

        questRefreshPlayerQuestProgress($pdo, $playerId, $playerQuestId);

        // Se ficou "ready" sem completar de verdade, volta para active.
        if (!questAllObjectivesComplete($pdo, $playerQuestId)) {
            $pdo->prepare("
                UPDATE player_quests SET status = 'active'
                WHERE player_quest_id = ? AND status = 'ready'
            ")->execute([$playerQuestId]);
        }
    }
}

function questPromoteToReadyIfComplete(PDO $pdo, int $player_quest_id): void
{
    if (!questAllObjectivesComplete($pdo, $player_quest_id)) {
        return;
    }
    $pdo->prepare("
        UPDATE player_quests SET status = 'ready'
        WHERE player_quest_id = ? AND status = 'active'
    ")->execute([$player_quest_id]);
}

function questDistance3d(float $x1, float $y1, float $z1, float $x2, float $y2, float $z2): float
{
    $dx = $x1 - $x2;
    $dy = $y1 - $y2;
    $dz = $z1 - $z2;
    return sqrt($dx * $dx + $dy * $dy + $dz * $dz);
}

function questValidateAreaObjective(array $params, int $zone_id, float $pos_x, float $pos_y, float $pos_z): bool
{
    $targetZone = (int)($params['zone_id'] ?? 0);
    if ($targetZone > 0 && $zone_id > 0 && $targetZone !== $zone_id) {
        return false;
    }
    $tx = (float)($params['pos_x'] ?? 0);
    $ty = (float)($params['pos_y'] ?? 0);
    $tz = (float)($params['pos_z'] ?? 0);
    $radius = (float)($params['radius'] ?? 300) + QUEST_AREA_MARGIN;
    return questDistance3d($pos_x, $pos_y, $pos_z, $tx, $ty, $tz) <= $radius;
}

function questGrantItemToPlayer(PDO $pdo, int $player_id, int $item_template_id, int $quantity): bool
{
    if ($item_template_id <= 0 || $quantity <= 0) {
        return false;
    }
    $tpl = $pdo->prepare('SELECT item_id, max_stack_size FROM item_templates WHERE item_id = ? LIMIT 1');
    $tpl->execute([$item_template_id]);
    $template = $tpl->fetch(PDO::FETCH_ASSOC);
    if (!$template) {
        return false;
    }
    $remaining = $quantity;
    $maxStack = max(1, (int)$template['max_stack_size']);
    while ($remaining > 0) {
        $chunk = min($remaining, $maxStack);
        $slot = npcVendorFindFreeSlot($pdo, $player_id);
        if ($slot === null) {
            return false;
        }
        $ins = $pdo->prepare('
            INSERT INTO player_inventory (player_id, item_template_id, quantity, slot_index)
            VALUES (?, ?, ?, ?)
        ');
        $ins->execute([$player_id, $item_template_id, $chunk, $slot]);
        $remaining -= $chunk;
    }
    return true;
}

function questApplyReward(PDO $pdo, int $player_id, string $reward_type, int $amount, ?int $item_template_id, int $quantity): array
{
    $result = ['reward_type' => $reward_type, 'amount' => $amount, 'item_template_id' => $item_template_id, 'quantity' => $quantity];
    if ($reward_type === 'gold' && $amount > 0) {
        $pdo->prepare('UPDATE players SET gold = gold + ? WHERE id = ?')->execute([$amount, $player_id]);
        $goldStmt = $pdo->prepare('SELECT gold FROM players WHERE id = ?');
        $goldStmt->execute([$player_id]);
        $result['new_gold'] = (int)$goldStmt->fetchColumn();
    } elseif ($reward_type === 'experience' && $amount > 0) {
        $exp = umbra_grant_experience($pdo, $player_id, $amount, 'quest_reward', false);
        if ($exp === null) {
            throw new RuntimeException('Falha ao conceder experiência da recompensa de quest.');
        }
        $result['experience_grant'] = $exp;
    } elseif ($reward_type === 'item' && $item_template_id) {
        if (!questGrantItemToPlayer($pdo, $player_id, $item_template_id, max(1, $quantity))) {
            throw new RuntimeException('Inventário cheio ao conceder item de recompensa.');
        }
    }
    return $result;
}

function questValidateDeliverItems(PDO $pdo, int $player_id, int $quest_id): ?string
{
    $deliverStmt = $pdo->prepare('
        SELECT qo.params_json FROM quest_objectives qo
        WHERE qo.quest_id = ? AND qo.objective_type = \'deliver\'
    ');
    $deliverStmt->execute([$quest_id]);
    foreach ($deliverStmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        $params = questDecodeParams($row['params_json'] ?? null);
        $itemId = (int)($params['item_template_id'] ?? 0);
        $required = max(1, (int)($params['required_count'] ?? 1));
        $have = questCountPlayerItem($pdo, $player_id, $itemId);
        if ($have < $required) {
            return 'Itens insuficientes para entregar a quest.';
        }
    }
    return null;
}

function questRemoveDeliverObjectives(PDO $pdo, int $player_id, int $quest_id): bool
{
    $deliverStmt = $pdo->prepare('
        SELECT qo.params_json FROM quest_objectives qo
        WHERE qo.quest_id = ? AND qo.objective_type = \'deliver\'
    ');
    $deliverStmt->execute([$quest_id]);
    foreach ($deliverStmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        $params = questDecodeParams($row['params_json'] ?? null);
        $itemId = (int)($params['item_template_id'] ?? 0);
        $required = max(1, (int)($params['required_count'] ?? 1));
        if (!questRemoveDeliverItems($pdo, $player_id, $itemId, $required)) {
            return false;
        }
    }
    return true;
}

function questRemoveDeliverItems(PDO $pdo, int $player_id, int $item_template_id, int $required): bool
{
    if ($item_template_id <= 0 || $required <= 0) {
        return true;
    }
    $have = questCountPlayerItem($pdo, $player_id, $item_template_id);
    if ($have < $required) {
        return false;
    }
    $remaining = $required;
    $stmt = $pdo->prepare('
        SELECT inventory_id, quantity FROM player_inventory
        WHERE player_id = ? AND item_template_id = ? AND auction_listing_id IS NULL
        ORDER BY slot_index ASC
    ');
    $stmt->execute([$player_id, $item_template_id]);
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        if ($remaining <= 0) {
            break;
        }
        $invId = (int)$row['inventory_id'];
        $qty = (int)$row['quantity'];
        if ($qty <= $remaining) {
            $pdo->prepare('DELETE FROM player_inventory WHERE inventory_id = ?')->execute([$invId]);
            $remaining -= $qty;
        } else {
            $pdo->prepare('UPDATE player_inventory SET quantity = quantity - ? WHERE inventory_id = ?')
                ->execute([$remaining, $invId]);
            $remaining = 0;
        }
    }
    return $remaining <= 0;
}

function questResolveTurnInNpcLabel(PDO $pdo, int $npc_template_id): string
{
    if ($npc_template_id <= 0) {
        return '';
    }
    $stmt = $pdo->prepare('
        SELECT COALESCE(NULLIF(TRIM(nv.vendor_display_name), \'\'), nt.npc_name) AS label
        FROM npc_templates nt
        LEFT JOIN npc_vendors nv ON nv.npc_template_id = nt.npc_template_id
        WHERE nt.npc_template_id = ?
        LIMIT 1
    ');
    $stmt->execute([$npc_template_id]);
    return (string)($stmt->fetchColumn() ?: '');
}

function questBuildDetailPayload(PDO $pdo, int $player_id, array $questRow, ?array $playerQuestRow): array
{
    $quest_id = (int)$questRow['quest_id'];
    $status = questResolveAvailability($pdo, $player_id, $questRow, $playerQuestRow);
    // Reoferta de repeatable: não mostrar progresso/id da run completed antiga.
    $progressRow = $playerQuestRow;
    if ($status === 'available' && $playerQuestRow
        && in_array((string)$playerQuestRow['status'], ['completed', 'failed'], true)) {
        $progressRow = null;
    }
    $objectives = questLoadObjectives($pdo, $quest_id);
    $progressMap = $progressRow ? questLoadPlayerObjectiveMap($pdo, (int)$progressRow['player_quest_id']) : [];
    $objOut = [];
    $seenObjectiveIds = [];
    foreach ($objectives as $obj) {
        $pid = (int)$obj['objective_id'];
        if (isset($seenObjectiveIds[$pid])) {
            continue;
        }
        $seenObjectiveIds[$pid] = true;
        $objOut[] = questFormatObjectiveForApi($obj, $progressMap[$pid] ?? null);
    }
    $rewards = array_map('questFormatRewardRow', questLoadRewards($pdo, $quest_id));
    $choices = array_map('questFormatRewardRow', questLoadRewardChoices($pdo, $quest_id));
    $acceptGrants = array_map(
        static fn(array $row) => questFormatItemQtyRow($row, 'grant_id'),
        questLoadAcceptGrants($pdo, $quest_id)
    );
    $startReqs = array_map(
        static fn(array $row) => questFormatItemQtyRow($row, 'requirement_id'),
        questLoadStartRequirements($pdo, $quest_id)
    );
    $turnInTemplateId = (int)($questRow['turn_in_npc_template_id'] ?? 0);
    $turnInNpcName = questResolveTurnInNpcLabel($pdo, $turnInTemplateId);
    return [
        'quest_id' => $quest_id,
        'quest_key' => $questRow['quest_key'],
        'title' => $questRow['title'],
        'description' => $questRow['description'] ?? '',
        'offer_text' => $questRow['offer_text'] ?? '',
        'turn_in_text' => $questRow['turn_in_text'] ?? '',
        'turn_in_npc_template_id' => $turnInTemplateId,
        'turn_in_npc_name' => $turnInNpcName,
        'min_level' => (int)$questRow['min_level'],
        'repeatable' => !empty($questRow['repeatable']),
        'status' => $status,
        'player_quest_id' => $progressRow ? (int)$progressRow['player_quest_id'] : 0,
        'objectives' => $objOut,
        'rewards' => $rewards,
        'reward_choices' => $choices,
        'accept_grants' => $acceptGrants,
        'start_requirements' => $startReqs,
    ];
}

function questGetNpcOffers(PDO $pdo, int $player_id, int $npc_template_id): array
{
    $stmt = $pdo->prepare('
        SELECT q.*, nqo.sort_order AS offer_sort
        FROM npc_quest_offers nqo
        INNER JOIN quests q ON q.quest_id = nqo.quest_id
        WHERE nqo.npc_template_id = ? AND q.is_active = 1
        ORDER BY nqo.sort_order ASC, q.quest_id ASC
    ');
    $stmt->execute([$npc_template_id]);
    $offers = [];
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $questRow) {
        $playerQuest = questGetPlayerQuestRow($pdo, $player_id, (int)$questRow['quest_id']);
        if ($playerQuest && in_array($playerQuest['status'], ['active', 'ready'], true)) {
            $status = (string)$playerQuest['status'];
        } else {
            $status = questResolveAvailability($pdo, $player_id, $questRow, $playerQuest);
        }
        // Oculta quests bloqueadas (prerequisite/min_level) ou já concluídas (não repetíveis).
        if (in_array($status, ['locked', 'completed'], true)) {
            continue;
        }
        $offers[] = [
            'quest_id' => (int)$questRow['quest_id'],
            'quest_key' => $questRow['quest_key'],
            'title' => $questRow['title'],
            'offer_text' => $questRow['offer_text'] ?? '',
            'status' => $status,
            'sort_order' => (int)$questRow['offer_sort'],
        ];
    }
    return $offers;
}

function questAccept(PDO $pdo, int $player_id, int $quest_id, int $npc_template_id = 0): array
{
    $quest = questLoadQuestRow($pdo, $quest_id);
    if (!$quest) {
        return ['ok' => false, 'message' => 'Quest não encontrada.'];
    }
    $existing = questGetPlayerQuestRow($pdo, $player_id, $quest_id);
    if ($existing && in_array($existing['status'], ['active', 'ready'], true)) {
        return ['ok' => false, 'message' => 'Quest já está ativa.'];
    }
    $avail = questResolveAvailability($pdo, $player_id, $quest, $existing);
    if ($avail !== 'available') {
        return ['ok' => false, 'message' => 'Quest indisponível (' . $avail . ').'];
    }
    if (!questPlayerMeetsStartRequirements($pdo, $player_id, $quest_id)) {
        return ['ok' => false, 'message' => 'Você não possui o item necessário para iniciar esta quest.'];
    }
    if ($npc_template_id > 0) {
        $offerStmt = $pdo->prepare('SELECT 1 FROM npc_quest_offers WHERE npc_template_id = ? AND quest_id = ? LIMIT 1');
        $offerStmt->execute([$npc_template_id, $quest_id]);
        if (!$offerStmt->fetchColumn()) {
            return ['ok' => false, 'message' => 'Este NPC não oferece esta quest.'];
        }
    }
    try {
        $pdo->beginTransaction();
        if (!questPlayerMeetsStartRequirements($pdo, $player_id, $quest_id)) {
            throw new RuntimeException('Você não possui o item necessário para iniciar esta quest.');
        }
        $pdo->prepare('INSERT INTO player_quests (player_id, quest_id, status) VALUES (?, ?, \'active\')')
            ->execute([$player_id, $quest_id]);
        $player_quest_id = (int)$pdo->lastInsertId();
        foreach (questLoadObjectives($pdo, $quest_id) as $obj) {
            $params = questDecodeParams($obj['params_json'] ?? null);
            $initial = 0;
            $done = 0;
            if ($obj['objective_type'] === 'talk') {
                $done = 1;
                $initial = 1;
            } elseif ($obj['objective_type'] === 'collect') {
                $itemId = (int)($params['item_template_id'] ?? 0);
                $required = max(1, (int)($params['required_count'] ?? 1));
                $have = $itemId > 0 ? questCountPlayerItem($pdo, $player_id, $itemId) : 0;
                $initial = min($have, $required);
                $done = $have >= $required ? 1 : 0;
            }
            $pdo->prepare('
                INSERT INTO player_quest_objectives (player_quest_id, objective_id, current_count, is_completed)
                VALUES (?, ?, ?, ?)
            ')->execute([$player_quest_id, (int)$obj['objective_id'], $initial, $done]);
        }
        questApplyAcceptGrants($pdo, $player_id, $quest_id);
        // Re-sincroniza collect após grants (item concedido pode completar objetivo).
        foreach (questLoadObjectives($pdo, $quest_id) as $obj) {
            if ($obj['objective_type'] !== 'collect') {
                continue;
            }
            $params = questDecodeParams($obj['params_json'] ?? null);
            $itemId = (int)($params['item_template_id'] ?? 0);
            $required = max(1, (int)($params['required_count'] ?? 1));
            $have = $itemId > 0 ? questCountPlayerItem($pdo, $player_id, $itemId) : 0;
            $initial = min($have, $required);
            $done = $have >= $required ? 1 : 0;
            $pdo->prepare('
                UPDATE player_quest_objectives
                SET current_count = ?, is_completed = ?
                WHERE player_quest_id = ? AND objective_id = ?
            ')->execute([$initial, $done, $player_quest_id, (int)$obj['objective_id']]);
        }
        questRefreshPlayerQuestProgress($pdo, $player_id, $player_quest_id);
        $pdo->commit();
        $pq = questGetPlayerQuestRow($pdo, $player_id, $quest_id);
        return ['ok' => true, 'detail' => questBuildDetailPayload($pdo, $player_id, $quest, $pq)];
    } catch (Throwable $e) {
        if ($pdo->inTransaction()) {
            $pdo->rollBack();
        }
        error_log('questAccept: ' . $e->getMessage());
        $msg = $e instanceof RuntimeException ? $e->getMessage() : 'Falha ao aceitar quest.';
        return ['ok' => false, 'message' => $msg];
    }
}

function questAbandon(PDO $pdo, int $player_id, int $quest_id): array
{
    $pq = questGetPlayerQuestRow($pdo, $player_id, $quest_id);
    if (!$pq || !in_array($pq['status'], ['active', 'ready'], true)) {
        return ['ok' => false, 'message' => 'Quest não está ativa.'];
    }
    try {
        $pdo->beginTransaction();
        questRevokeAcceptGrants($pdo, $player_id, $quest_id);
        $pdo->prepare('DELETE FROM player_quests WHERE player_quest_id = ?')->execute([(int)$pq['player_quest_id']]);
        $pdo->commit();
        return ['ok' => true, 'quest_id' => $quest_id];
    } catch (Throwable $e) {
        if ($pdo->inTransaction()) {
            $pdo->rollBack();
        }
        error_log('questAbandon: ' . $e->getMessage());
        return ['ok' => false, 'message' => 'Falha ao abandonar quest.'];
    }
}

function questReportProgress(PDO $pdo, int $player_id, int $quest_id, string $objective_type, ?array $client_pos, int $zone_id = 0, int $item_template_id = 0): array
{
    $pq = questGetPlayerQuestRow($pdo, $player_id, $quest_id);
    if (!$pq || $pq['status'] !== 'active') {
        return ['ok' => false, 'message' => 'Quest não está ativa.'];
    }
    if (!in_array($objective_type, ['reach_area', 'use_item_at'], true)) {
        return ['ok' => false, 'message' => 'Tipo de progresso inválido.'];
    }
    if ($client_pos === null) {
        return ['ok' => false, 'message' => 'Informe pos_x, pos_y e pos_z.'];
    }
    $player_quest_id = (int)$pq['player_quest_id'];
    $stmt = $pdo->prepare('
        SELECT pqo.player_objective_id, qo.objective_id, qo.params_json
        FROM player_quest_objectives pqo
        INNER JOIN quest_objectives qo ON qo.objective_id = pqo.objective_id
        WHERE pqo.player_quest_id = ? AND pqo.is_completed = 0 AND qo.objective_type = ?
    ');
    $stmt->execute([$player_quest_id, $objective_type]);
    $updated = false;
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        $params = questDecodeParams($row['params_json'] ?? null);
        if ($objective_type === 'use_item_at') {
            $needItem = (int)($params['item_template_id'] ?? 0);
            if ($item_template_id > 0 && $needItem !== $item_template_id) {
                continue;
            }
            if ($needItem > 0 && questCountPlayerItem($pdo, $player_id, $needItem) < 1) {
                continue;
            }
        }
        if (!questValidateAreaObjective($params, $zone_id, $client_pos['pos_x'], $client_pos['pos_y'], $client_pos['pos_z'])) {
            continue;
        }
        $required = max(1, (int)($params['required_count'] ?? 1));
        $pdo->prepare('
            UPDATE player_quest_objectives
            SET current_count = ?, is_completed = 1
            WHERE player_objective_id = ?
        ')->execute([$required, (int)$row['player_objective_id']]);
        $updated = true;
    }
    if ($updated) {
        questPromoteToReadyIfComplete($pdo, $player_quest_id);
    }
    $quest = questLoadQuestRow($pdo, $quest_id);
    $pq = questGetPlayerQuestRow($pdo, $player_id, $quest_id);
    return ['ok' => true, 'updated' => $updated, 'detail' => questBuildDetailPayload($pdo, $player_id, $quest, $pq)];
}

function questHasPendingChoiceRewards(PDO $pdo, int $quest_id): bool
{
    $stmt = $pdo->prepare('SELECT 1 FROM quest_reward_choices WHERE quest_id = ? LIMIT 1');
    $stmt->execute([$quest_id]);
    return (bool)$stmt->fetchColumn();
}

function questTurnIn(PDO $pdo, int $player_id, int $quest_id, int $npc_template_id = 0): array
{
    $pq = questGetPlayerQuestRow($pdo, $player_id, $quest_id);
    if (!$pq || $pq['status'] !== 'ready') {
        return ['ok' => false, 'message' => 'Quest não está pronta para entrega.'];
    }
    $quest = questLoadQuestRow($pdo, $quest_id);
    if (!$quest) {
        return ['ok' => false, 'message' => 'Quest não encontrada.'];
    }
    if ($npc_template_id > 0 && !empty($quest['turn_in_npc_template_id'])) {
        if ((int)$quest['turn_in_npc_template_id'] !== $npc_template_id) {
            return ['ok' => false, 'message' => 'Entregue esta quest ao NPC correto.'];
        }
    }
    $player_quest_id = (int)$pq['player_quest_id'];
    $deliverError = questValidateDeliverItems($pdo, $player_id, $quest_id);
    if ($deliverError !== null) {
        return ['ok' => false, 'message' => $deliverError];
    }
    if (questHasPendingChoiceRewards($pdo, $quest_id)) {
        return [
            'ok' => true,
            'needs_reward_choice' => true,
            'quest_id' => $quest_id,
            'player_quest_id' => $player_quest_id,
            'reward_choices' => array_map('questFormatRewardRow', questLoadRewardChoices($pdo, $quest_id)),
        ];
    }
    try {
        $pdo->beginTransaction();
        if (!questRemoveDeliverObjectives($pdo, $player_id, $quest_id)) {
            throw new RuntimeException('Itens insuficientes para entregar a quest.');
        }
        $grants = [];
        foreach (questLoadRewards($pdo, $quest_id) as $reward) {
            if (!empty($reward['choice_group_id'])) {
                continue;
            }
            $grants[] = questApplyReward(
                $pdo,
                $player_id,
                $reward['reward_type'],
                (int)$reward['amount'],
                isset($reward['item_template_id']) ? (int)$reward['item_template_id'] : null,
                (int)($reward['quantity'] ?? 1)
            );
        }
        $pdo->prepare("
            UPDATE player_quests SET status = 'completed', completed_at = NOW()
            WHERE player_quest_id = ?
        ")->execute([$player_quest_id]);
        $pdo->commit();
        $goldStmt = $pdo->prepare('SELECT gold FROM players WHERE id = ?');
        $goldStmt->execute([$player_id]);
        return [
            'ok' => true,
            'needs_reward_choice' => false,
            'quest_id' => $quest_id,
            'rewards_granted' => $grants,
            'new_gold' => (int)$goldStmt->fetchColumn(),
        ];
    } catch (Throwable $e) {
        if ($pdo->inTransaction()) {
            $pdo->rollBack();
        }
        error_log('questTurnIn: ' . $e->getMessage());
        return ['ok' => false, 'message' => $e->getMessage()];
    }
}

function questChooseRewardAndComplete(PDO $pdo, int $player_id, int $quest_id, int $choice_id): array
{
    $pq = questGetPlayerQuestRow($pdo, $player_id, $quest_id);
    if (!$pq || $pq['status'] !== 'ready') {
        return ['ok' => false, 'message' => 'Quest não está pronta.'];
    }
    $choiceStmt = $pdo->prepare('SELECT * FROM quest_reward_choices WHERE choice_id = ? AND quest_id = ? LIMIT 1');
    $choiceStmt->execute([$choice_id, $quest_id]);
    $choice = $choiceStmt->fetch(PDO::FETCH_ASSOC);
    if (!$choice) {
        return ['ok' => false, 'message' => 'Escolha de recompensa inválida.'];
    }
    $player_quest_id = (int)$pq['player_quest_id'];
    try {
        $pdo->beginTransaction();
        if (!questRemoveDeliverObjectives($pdo, $player_id, $quest_id)) {
            throw new RuntimeException('Itens insuficientes para entregar a quest.');
        }
        $grant = questApplyReward(
            $pdo,
            $player_id,
            $choice['reward_type'],
            (int)$choice['amount'],
            isset($choice['item_template_id']) ? (int)$choice['item_template_id'] : null,
            (int)($choice['quantity'] ?? 1)
        );
        foreach (questLoadRewards($pdo, $quest_id) as $reward) {
            if (!empty($reward['choice_group_id'])) {
                continue;
            }
            $grantFixed = questApplyReward(
                $pdo,
                $player_id,
                $reward['reward_type'],
                (int)$reward['amount'],
                isset($reward['item_template_id']) ? (int)$reward['item_template_id'] : null,
                (int)($reward['quantity'] ?? 1)
            );
            $grant['fixed_rewards'][] = $grantFixed;
        }
        $chosenJson = json_encode(['choice_id' => $choice_id, 'choice_group_id' => (int)$choice['choice_group_id']]);
        $pdo->prepare("
            UPDATE player_quests
            SET status = 'completed', completed_at = NOW(), chosen_rewards_json = ?
            WHERE player_quest_id = ?
        ")->execute([$chosenJson, $player_quest_id]);
        $pdo->commit();
        $goldStmt = $pdo->prepare('SELECT gold FROM players WHERE id = ?');
        $goldStmt->execute([$player_id]);
        return [
            'ok' => true,
            'quest_id' => $quest_id,
            'reward_granted' => $grant,
            'new_gold' => (int)$goldStmt->fetchColumn(),
        ];
    } catch (Throwable $e) {
        if ($pdo->inTransaction()) {
            $pdo->rollBack();
        }
        error_log('questChooseReward: ' . $e->getMessage());
        return ['ok' => false, 'message' => $e->getMessage()];
    }
}

function questGetJournal(PDO $pdo, int $player_id): array
{
    $stmt = $pdo->prepare("
        SELECT pq.*, q.quest_key, q.title, q.description
        FROM player_quests pq
        INNER JOIN quests q ON q.quest_id = pq.quest_id
        WHERE pq.player_id = ? AND pq.status IN ('active', 'ready', 'completed')
        ORDER BY FIELD(pq.status, 'ready', 'active', 'completed'), pq.accepted_at DESC
    ");
    $stmt->execute([$player_id]);
    $entries = [];
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        if (in_array($row['status'], ['active', 'ready'], true)) {
            questRefreshPlayerQuestProgress($pdo, $player_id, (int)$row['player_quest_id']);
            $fresh = questGetPlayerQuestRow($pdo, $player_id, (int)$row['quest_id']);
            if ($fresh) {
                $row = array_merge($row, $fresh);
            }
        }
        $quest = questLoadQuestRow($pdo, (int)$row['quest_id']);
        if (!$quest) {
            continue;
        }
        $detail = questBuildDetailPayload($pdo, $player_id, $quest, $row);
        $currentObjective = '';
        foreach ($detail['objectives'] as $obj) {
            if (empty($obj['is_completed'])) {
                $req = (int)$obj['required_count'];
                $cur = (int)$obj['current_count'];
                $currentObjective = $obj['description'] . ($req > 1 ? " ({$cur}/{$req})" : '');
                break;
            }
        }
        $entries[] = [
            'quest_id' => (int)$row['quest_id'],
            'title' => $row['title'],
            'status' => $row['status'],
            'current_objective' => $currentObjective,
            'detail' => $detail,
        ];
    }
    return $entries;
}

function questCountOffersForNpc(PDO $pdo, int $npc_template_id): int
{
    $stmt = $pdo->prepare('
        SELECT COUNT(*) FROM npc_quest_offers nqo
        INNER JOIN quests q ON q.quest_id = nqo.quest_id
        WHERE nqo.npc_template_id = ? AND q.is_active = 1
    ');
    $stmt->execute([$npc_template_id]);
    return (int)$stmt->fetchColumn();
}
