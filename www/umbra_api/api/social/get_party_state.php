<?php
/**
 * POST /api/social/get_party_state.php
 * Obtém estado do grupo (membros com nome, level, HP, MP)
 * Parâmetros: token, party_id (ou obtém do jogador atual)
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/../../helpers/character_info_helper.php';

// Helper de buffs é opcional (pode não existir no deploy WAMP antigo)
$active_buffs_helper_path = __DIR__ . '/../../helpers/active_buffs_helper.php';
if (is_file($active_buffs_helper_path)) {
    require_once $active_buffs_helper_path;
}

/**
 * @return array<int, array<string, mixed>>
 */
function party_state_fetch_active_buffs(PDO $pdo, int $player_id): array
{
    if (function_exists('fetch_active_buffs_for_player')) {
        return fetch_active_buffs_for_player($pdo, $player_id);
    }

    $now_ms = (int)round(microtime(true) * 1000);
    $stmt = $pdo->prepare("
        SELECT
            pib.buff_key,
            pib.bonus_value,
            pib.duration_ms,
            pib.started_at_ms,
            pib.expires_at_ms,
            pib.item_template_id,
            pib.item_subtype,
            it.item_name,
            it.item_description,
            it.icon_path
        FROM player_item_buffs pib
        INNER JOIN item_templates it ON pib.item_template_id = it.item_id
        WHERE pib.player_id = :player_id AND pib.expires_at_ms > :now_ms
        ORDER BY pib.expires_at_ms ASC
    ");
    $stmt->execute(['player_id' => $player_id, 'now_ms' => $now_ms]);
    $rows = [];
    while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        $rows[] = [
            'buff_key' => $row['buff_key'],
            'bonus_value' => (int)$row['bonus_value'],
            'duration_ms' => (int)$row['duration_ms'],
            'started_at_ms' => (int)$row['started_at_ms'],
            'expires_at_ms' => (int)$row['expires_at_ms'],
            'item_template_id' => (int)$row['item_template_id'],
            'item_subtype' => $row['item_subtype'],
            'item_name' => $row['item_name'],
            'item_description' => $row['item_description'],
            'icon_path' => $row['icon_path'],
        ];
    }
    return $rows;
}

/**
 * Fallback quando get_character_info_data falha.
 */
function load_party_member_fallback(PDO $pdo, int $mid, int $leader_id): ?array
{
    $stmt = $pdo->prepare("
        SELECT
            p.id as player_id,
            p.character_name,
            COALESCE(p.level, 1) as level,
            COALESCE(p.health, 100) as current_health,
            COALESCE(p.max_health, 100) as base_max_health,
            COALESCE(p.mana, 50) as current_mana,
            COALESCE(p.max_mana, 50) as base_max_mana,
            COALESCE(p.vitality, 10) as base_vitality,
            COALESCE(p.intelligence, 10) as base_intelligence,
            COALESCE(psp.vitality_points, 0) as vitality_points,
            COALESCE(psp.intelligence_points, 0) as intelligence_points,
            c.class_name
        FROM players p
        LEFT JOIN classes c ON p.class_id = c.class_id
        LEFT JOIN player_stat_points psp ON p.id = psp.player_id
        WHERE p.id = ?
    ");
    $stmt->execute([$mid]);
    $r = $stmt->fetch(PDO::FETCH_ASSOC);
    if (!$r) {
        return null;
    }

    $level = (int)$r['level'];
    $base_max_health = (int)$r['base_max_health'];
    $base_max_mana = (int)$r['base_max_mana'];
    $total_vitality = (int)$r['base_vitality'] + (int)$r['vitality_points'];
    $total_intelligence = (int)$r['base_intelligence'] + (int)$r['intelligence_points'];
    $total_health_bonus = 0;
    $total_mana_bonus = 0;

    try {
        $equip_stmt = $pdo->prepare("
            SELECT
                JSON_EXTRACT(it.stats_json, '$.vitality') as vitality,
                JSON_EXTRACT(it.stats_json, '$.intelligence') as intelligence,
                JSON_EXTRACT(it.stats_json, '$.health_bonus') as health_bonus,
                JSON_EXTRACT(it.stats_json, '$.mana_bonus') as mana_bonus
            FROM player_inventory pi
            INNER JOIN item_templates it ON pi.item_template_id = it.item_id
            WHERE pi.player_id = ? AND pi.is_equipped = TRUE
        ");
        $equip_stmt->execute([$mid]);
        foreach ($equip_stmt->fetchAll(PDO::FETCH_ASSOC) as $es) {
            $total_vitality += (int)($es['vitality'] ?? 0);
            $total_intelligence += (int)($es['intelligence'] ?? 0);
            $total_health_bonus += (int)($es['health_bonus'] ?? 0);
            $total_mana_bonus += (int)($es['mana_bonus'] ?? 0);
        }
    } catch (\Throwable $ignored) {
    }

    $level_hp_bonus = $level * 20;
    $level_mp_bonus = $level * 20;
    $total_health_bonus += (int)floor($total_vitality / 10) * 30;
    $total_mana_bonus += (int)floor($total_intelligence / 10) * 30;

    return [
        'player_id' => $mid,
        'character_name' => $r['character_name'] ?? '',
        'level' => $level,
        'current_health' => (int)$r['current_health'],
        'max_health' => $base_max_health + $level_hp_bonus + $total_health_bonus,
        'current_mana' => (int)$r['current_mana'],
        'max_mana' => $base_max_mana + $level_mp_bonus + $total_mana_bonus,
        'class_name' => $r['class_name'] ?? '',
        'is_leader' => ($mid === $leader_id),
    ];
}

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$player_id = (int)($validation['payload']['player_id'] ?? 0);
$party_id = isset($data['party_id']) ? (int)$data['party_id'] : 0;

if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

try {
    $pdo = getConnection();

    if (!$party_id) {
        $find = $pdo->prepare("SELECT party_id FROM party_members WHERE player_id = ?");
        $find->execute([$player_id]);
        $row = $find->fetch(PDO::FETCH_ASSOC);
        $party_id = $row ? (int)$row['party_id'] : 0;
    }

    if (!$party_id) {
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'in_party' => false,
            'party_id' => 0,
            'leader_id' => 0,
            'members' => []
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $check = $pdo->prepare("SELECT 1 FROM party_members WHERE party_id = ? AND player_id = ?");
    $check->execute([$party_id, $player_id]);
    if (!$check->fetch()) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Você não pertence a este grupo']);
        exit;
    }

    $party_stmt = $pdo->prepare("SELECT leader_id FROM parties WHERE party_id = ?");
    $party_stmt->execute([$party_id]);
    $party = $party_stmt->fetch(PDO::FETCH_ASSOC);
    if (!$party) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Grupo não encontrado']);
        exit;
    }

    $leader_id = (int)$party['leader_id'];

    $members_stmt = $pdo->prepare("
        SELECT p.id as player_id
        FROM party_members pm
        INNER JOIN players p ON pm.player_id = p.id
        WHERE pm.party_id = ?
        ORDER BY pm.joined_at ASC
    ");
    $members_stmt->execute([$party_id]);
    $member_rows = $members_stmt->fetchAll(PDO::FETCH_ASSOC);

    $members = [];
    foreach ($member_rows as $r) {
        $mid = (int)$r['player_id'];
        $member_payload = null;

        try {
            $character = get_character_info_data($pdo, $mid, ['create_stat_points_if_missing' => false]);
            if ($character !== null) {
                $health = $character['stats']['health'] ?? [];
                $mana = $character['stats']['mana'] ?? [];
                $member_payload = [
                    'player_id' => $mid,
                    'character_name' => $character['character_name'] ?? '',
                    'level' => (int)($character['level'] ?? 1),
                    'current_health' => (int)($health['current'] ?? 0),
                    'max_health' => (int)($health['max_total'] ?? 0),
                    'current_mana' => (int)($mana['current'] ?? 0),
                    'max_mana' => (int)($mana['max_total'] ?? 0),
                    'class_name' => $character['class']['class_name'] ?? '',
                    'is_leader' => ($mid === $leader_id),
                ];
            }
        } catch (\Throwable $e) {
            error_log("[get_party_state] helper falhou player_id={$mid}: " . $e->getMessage());
        }

        if ($member_payload === null) {
            try {
                $member_payload = load_party_member_fallback($pdo, $mid, $leader_id);
            } catch (\Throwable $e) {
                error_log("[get_party_state] fallback falhou player_id={$mid}: " . $e->getMessage());
            }
        }

        if ($member_payload === null) {
            continue;
        }

        try {
            $member_payload['active_buffs'] = party_state_fetch_active_buffs($pdo, $mid);
            $health_pct = 100;
            if (isset($member_payload['current_health'], $member_payload['max_health'])) {
                $cur = (int)$member_payload['current_health'];
                $max = max(1, (int)$member_payload['max_health']);
                $health_pct = $cur > 0 ? (int)floor($cur * 100 / $max) : 100;
            }
            if (function_exists('fetch_skill_active_buffs_for_player')) {
                $member_payload['skill_buffs'] = fetch_skill_active_buffs_for_player($pdo, $mid, true);
            } else {
                $member_payload['skill_buffs'] = [];
            }
            if (function_exists('fetch_passive_display_buffs_for_player')) {
                $member_payload['passive_buffs'] = fetch_passive_display_buffs_for_player($pdo, $mid, $health_pct);
            } else {
                $member_payload['passive_buffs'] = [];
            }
            if (function_exists('fetch_active_dots_for_player')) {
                $member_payload['dot_buffs'] = fetch_active_dots_for_player($pdo, $mid);
            } else {
                $member_payload['dot_buffs'] = [];
            }
        } catch (\Throwable $e) {
            $member_payload['active_buffs'] = [];
            $member_payload['skill_buffs'] = [];
            $member_payload['passive_buffs'] = [];
            $member_payload['dot_buffs'] = [];
        }

        $members[] = $member_payload;
    }

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'in_party' => true,
        'party_id' => (int)$party_id,
        'leader_id' => $leader_id,
        'members' => $members
    ], JSON_UNESCAPED_UNICODE);

} catch (\Throwable $e) {
    error_log("Erro ao obter estado do grupo: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
