<?php
/**
 * Helpers compartilhados para create/update quest admin.
 */
require_once __DIR__ . '/../../helpers/quest_helper.php';

function adminQuestAllowedObjectiveTypes(): array
{
    return ['talk', 'kill', 'collect', 'deliver', 'reach_area', 'use_item_at'];
}

function adminQuestAllowedRewardTypes(): array
{
    return ['gold', 'experience', 'item'];
}

function adminQuestEncodeParams($params): string
{
    if (is_string($params)) {
        $decoded = json_decode($params, true);
        if (json_last_error() === JSON_ERROR_NONE) {
            return json_encode($decoded, JSON_UNESCAPED_UNICODE);
        }
        return '{}';
    }
    if (!is_array($params) && !is_object($params)) {
        return '{}';
    }
    return json_encode($params, JSON_UNESCAPED_UNICODE);
}

function adminQuestReplaceObjectives(PDO $pdo, int $questId, array $objectives): void
{
    $pdo->prepare('DELETE FROM quest_objectives WHERE quest_id = :id')->execute([':id' => $questId]);
    if (!empty($objectives)) {
        $allowed = adminQuestAllowedObjectiveTypes();
        $ins = $pdo->prepare(
            'INSERT INTO quest_objectives (quest_id, sort_order, objective_type, description, params_json)
             VALUES (:qid, :sort, :type, :desc, :params)'
        );
        foreach ($objectives as $i => $obj) {
            if (!is_array($obj)) {
                continue;
            }
            $type = (string)($obj['objective_type'] ?? '');
            if (!in_array($type, $allowed, true)) {
                throw new InvalidArgumentException("objective_type inválido: {$type}");
            }
            $sort = isset($obj['sort_order']) ? (int)$obj['sort_order'] : (int)$i;
            $desc = (string)($obj['description'] ?? '');
            $params = $obj['params'] ?? ($obj['params_json'] ?? []);
            $ins->execute([
                ':qid' => $questId,
                ':sort' => $sort,
                ':type' => $type,
                ':desc' => $desc,
                ':params' => adminQuestEncodeParams($params),
            ]);
        }
    }

    // DELETE em quest_objectives faz CASCADE em player_quest_objectives.
    // Recria progresso para quem já tinha a quest ativa/pronta.
    questResyncPlayerObjectivesForQuest($pdo, $questId);
}

function adminQuestReplaceRewards(PDO $pdo, int $questId, array $rewards): void
{
    $pdo->prepare('DELETE FROM quest_rewards WHERE quest_id = :id')->execute([':id' => $questId]);
    if (empty($rewards)) {
        return;
    }
    $allowed = adminQuestAllowedRewardTypes();
    $ins = $pdo->prepare(
        'INSERT INTO quest_rewards
            (quest_id, reward_type, amount, item_template_id, quantity, choice_group_id, sort_order)
         VALUES
            (:qid, :type, :amount, :item, :qty, NULL, :sort)'
    );
    foreach ($rewards as $i => $rew) {
        if (!is_array($rew)) {
            continue;
        }
        $type = (string)($rew['reward_type'] ?? '');
        if (!in_array($type, $allowed, true)) {
            throw new InvalidArgumentException("reward_type inválido: {$type}");
        }
        $amount = (int)($rew['amount'] ?? 0);
        $qty = (int)($rew['quantity'] ?? 1);
        if ($qty < 1) {
            $qty = 1;
        }
        $itemId = null;
        if ($type === 'item') {
            $itemId = (int)($rew['item_template_id'] ?? 0);
            if ($itemId <= 0) {
                throw new InvalidArgumentException('item_template_id obrigatório para reward item');
            }
        }
        $sort = isset($rew['sort_order']) ? (int)$rew['sort_order'] : (int)$i;
        $ins->execute([
            ':qid' => $questId,
            ':type' => $type,
            ':amount' => $amount,
            ':item' => $itemId,
            ':qty' => $qty,
            ':sort' => $sort,
        ]);
    }
}

function adminQuestReplaceItemQtyList(PDO $pdo, int $questId, string $table, string $idCol, array $rows): void
{
    $allowed = ['quest_accept_grants' => true, 'quest_start_requirements' => true];
    if (!isset($allowed[$table])) {
        throw new InvalidArgumentException("tabela inválida: {$table}");
    }
    $pdo->prepare("DELETE FROM {$table} WHERE quest_id = :id")->execute([':id' => $questId]);
    if (empty($rows)) {
        return;
    }
    $ins = $pdo->prepare(
        "INSERT INTO {$table} (quest_id, item_template_id, quantity, sort_order)
         VALUES (:qid, :item, :qty, :sort)"
    );
    foreach ($rows as $i => $row) {
        if (!is_array($row)) {
            continue;
        }
        $itemId = (int)($row['item_template_id'] ?? 0);
        if ($itemId <= 0) {
            throw new InvalidArgumentException('item_template_id obrigatório');
        }
        $qty = (int)($row['quantity'] ?? 1);
        if ($qty < 1) {
            $qty = 1;
        }
        $sort = isset($row['sort_order']) ? (int)$row['sort_order'] : (int)$i;
        $ins->execute([
            ':qid' => $questId,
            ':item' => $itemId,
            ':qty' => $qty,
            ':sort' => $sort,
        ]);
    }
}

function adminQuestReplaceAcceptGrants(PDO $pdo, int $questId, array $rows): void
{
    adminQuestReplaceItemQtyList($pdo, $questId, 'quest_accept_grants', 'grant_id', $rows);
}

function adminQuestReplaceStartRequirements(PDO $pdo, int $questId, array $rows): void
{
    adminQuestReplaceItemQtyList($pdo, $questId, 'quest_start_requirements', 'requirement_id', $rows);
}
