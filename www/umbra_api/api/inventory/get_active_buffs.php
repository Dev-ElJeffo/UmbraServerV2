<?php
/**
 * POST /api/inventory/get_active_buffs.php
 * Lista buffs ativos de poções (player_item_buffs) com dados do template.
 *
 * Body (JSON):
 * { "token": "jwt...", "player_id": 1 }
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$player_id = null;
if (isset($data['player_id']) && is_numeric($data['player_id'])) {
    $player_id = (int)$data['player_id'];
} else {
    $player_id = $validation['payload']['player_id'] ?? null;
}

$account_id = $validation['payload']['account_id'] ?? null;

if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Player ID não encontrado']);
    exit;
}

if (!$account_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Account ID não encontrado no token']);
    exit;
}

try {
    $pdo = getConnection();

    $check_stmt = $pdo->prepare('SELECT id, account_id FROM players WHERE id = :player_id');
    $check_stmt->execute(['player_id' => $player_id]);
    $player_row = $check_stmt->fetch(PDO::FETCH_ASSOC);

    if (!$player_row) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Personagem não encontrado']);
        exit;
    }
    if ((int)$player_row['account_id'] !== (int)$account_id) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Acesso negado: personagem não pertence à sua conta']);
        exit;
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
        WHERE pib.player_id = :player_id
          AND pib.expires_at_ms > :now_ms
        ORDER BY pib.expires_at_ms ASC
    ");
    $stmt->execute(['player_id' => $player_id, 'now_ms' => $now_ms]);

    $buffs = [];
    while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        $buffs[] = [
            'buff_key' => $row['buff_key'],
            'bonus_value' => (int)$row['bonus_value'],
            'duration_ms' => (int)$row['duration_ms'],
            'started_at_ms' => (int)$row['started_at_ms'],
            'expires_at_ms' => (int)$row['expires_at_ms'],
            'item_template_id' => (int)$row['item_template_id'],
            'item_subtype' => $row['item_subtype'],
            'item_name' => $row['item_name'],
            'item_description' => $row['item_description'],
            'icon_path' => $row['icon_path']
        ];
    }

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'buffs' => $buffs,
        'server_time_ms' => $now_ms
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao listar buffs: ' . $e->getMessage()]);
}
