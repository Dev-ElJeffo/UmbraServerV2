<?php
/**
 * Helpers compartilhados para create/update quest admin.
 */
require_once __DIR__ . '/quest_schema_helpers.php';
require_once __DIR__ . '/../../helpers/quest_helper.php';

function adminQuestAllowedObjectiveTypes(): array
{
    return [
        'talk', 'kill', 'collect', 'deliver', 'reach_area', 'use_item_at',
        // Quests v2 (add_quests_v2.sql)
        'kill_any', 'use_skill', 'use_consumable', 'earn_gold', 'reach_level', 'craft_item', 'refine_item',
    ];
}

function adminQuestAllowedRewardTypes(): array
{
    // companion: amount = companion_id; guild_xp: vai para a guilda do jogador.
    return ['gold', 'experience', 'item', 'skill_points', 'stat_points', 'companion', 'guild_xp'];
}

/** Campos v2 do quest (create/update): normaliza e devolve [coluna => valor]. */
function adminQuestV2Fields(array $data): array
{
    $out = [];
    if (array_key_exists('repeat_type', $data)) {
        $rt = strtolower(trim((string)$data['repeat_type']));
        if (!in_array($rt, ['none', 'always', 'daily', 'weekly'], true)) {
            throw new InvalidArgumentException('repeat_type: none, always, daily ou weekly');
        }
        $out['repeat_type'] = $rt;
        $out['repeatable'] = $rt === 'none' ? 0 : 1;  // compat com quem ainda lê repeatable
    }
    if (array_key_exists('max_level', $data)) {
        $out['max_level'] = max(0, (int)$data['max_level']);
    }
    if (array_key_exists('class_ids', $data)) {
        $raw = $data['class_ids'];
        if (is_string($raw)) {
            $raw = preg_split('/[\s,;]+/', $raw, -1, PREG_SPLIT_NO_EMPTY);
        }
        $ids = array_values(array_unique(array_filter(array_map('intval', (array)$raw), static fn($v) => $v > 0)));
        $out['class_ids'] = $ids ? json_encode($ids) : null;
    }
    if (array_key_exists('quest_category', $data)) {
        $cat = strtolower(trim((string)$data['quest_category']));
        if (!in_array($cat, ['main', 'side', 'daily', 'event'], true)) {
            throw new InvalidArgumentException('quest_category: main, side, daily ou event');
        }
        $out['quest_category'] = $cat;
    }
    if (array_key_exists('time_limit_minutes', $data)) {
        $out['time_limit_minutes'] = max(0, min(10080, (int)$data['time_limit_minutes']));
    }
    return $out;
}

function adminQuestEncodeParams($params, string $type = ''): string
{
    if (is_string($params)) {
        $decoded = json_decode($params, true);
        if (json_last_error() !== JSON_ERROR_NONE) {
            throw new InvalidArgumentException('params JSON inválido: ' . json_last_error_msg());
        }
        $params = $decoded;
    }
    if (!is_array($params) && !is_object($params)) {
        $params = [];
    }
    if ($type !== '') {
        $params = quest_validate_params($type, $params);
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
                ':params' => adminQuestEncodeParams($params, $type),
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

/**
 * Substitui todas as opções de escolha de recompensa da quest.
 * Payload row: choice_group_id, label?, reward_type, amount?, item_template_id?, quantity?, sort_order?
 */
function adminQuestReplaceRewardChoices(PDO $pdo, int $questId, array $choices): void
{
    $pdo->prepare('DELETE FROM quest_reward_choices WHERE quest_id = :id')->execute([':id' => $questId]);
    if (empty($choices)) {
        return;
    }
    $allowed = adminQuestAllowedRewardTypes();
    $ins = $pdo->prepare(
        'INSERT INTO quest_reward_choices
            (quest_id, choice_group_id, label, reward_type, amount, item_template_id, quantity, sort_order)
         VALUES
            (:qid, :gid, :label, :type, :amount, :item, :qty, :sort)'
    );
    foreach ($choices as $i => $row) {
        if (!is_array($row)) {
            continue;
        }
        $type = (string)($row['reward_type'] ?? '');
        if (!in_array($type, $allowed, true)) {
            throw new InvalidArgumentException("reward_type inválido em reward_choices: {$type}");
        }
        $gid = (int)($row['choice_group_id'] ?? 0);
        if ($gid <= 0) {
            throw new InvalidArgumentException('choice_group_id deve ser >= 1 em reward_choices');
        }
        $amount = (int)($row['amount'] ?? 0);
        $qty = (int)($row['quantity'] ?? 1);
        if ($qty < 1) {
            $qty = 1;
        }
        $itemId = null;
        if ($type === 'item') {
            $itemId = (int)($row['item_template_id'] ?? 0);
            if ($itemId <= 0) {
                throw new InvalidArgumentException('item_template_id obrigatório para reward_choices item');
            }
        }
        $label = trim((string)($row['label'] ?? ''));
        $sort = isset($row['sort_order']) ? (int)$row['sort_order'] : (int)$i;
        $ins->execute([
            ':qid' => $questId,
            ':gid' => $gid,
            ':label' => $label,
            ':type' => $type,
            ':amount' => $amount,
            ':item' => $itemId,
            ':qty' => $qty,
            ':sort' => $sort,
        ]);
    }
}
