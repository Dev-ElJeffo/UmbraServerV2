<?php
/**
 * POST /api/companion/get_player_companions.php
 * Lista os companheiros do personagem (janela de companheiros no UE).
 *
 * Body (JSON): { "token": "jwt...", "player_id": 1 }
 * Resposta: { success, companions: [ {companion_id, companion_key, name, description, companion_type,
 *             icon_path, required_level, is_active, loot_min_rarity, loot_gold, expires_at} ] }
 * Invocar/recolher/filtro vão pela zone (opcode 124), não por aqui.
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

$data = json_decode(file_get_contents('php://input'), true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado'], JSON_UNESCAPED_UNICODE);
    exit;
}

$player_id = isset($data['player_id']) && is_numeric($data['player_id'])
    ? (int)$data['player_id']
    : (int)($validation['payload']['player_id'] ?? 0);
$account_id = (int)($validation['payload']['account_id'] ?? 0);

if ($player_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Player ID ou Account ID ausente'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();

    $own = $pdo->prepare('SELECT account_id FROM players WHERE id = :pid');
    $own->execute([':pid' => $player_id]);
    $owner = $own->fetchColumn();
    if ($owner === false) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Personagem não encontrado'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    if ((int)$owner !== $account_id) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Acesso negado: personagem não pertence à sua conta'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $stmt = $pdo->prepare(
        'SELECT ct.companion_id, ct.companion_key, ct.name, ct.description, ct.companion_type, ct.icon_path,
                ct.required_level, pc.is_active, pc.loot_min_rarity, pc.loot_gold, pc.expires_at
           FROM player_companions pc
           JOIN companion_templates ct ON ct.companion_id = pc.companion_id
          WHERE pc.player_id = :pid
            AND ct.is_active = 1
            AND (pc.expires_at IS NULL OR pc.expires_at > NOW())
          ORDER BY ct.companion_type, ct.name'
    );
    $stmt->execute([':pid' => $player_id]);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        $r['companion_id'] = (int)$r['companion_id'];
        $r['required_level'] = (int)$r['required_level'];
        $r['is_active'] = (int)$r['is_active'];
        $r['loot_min_rarity'] = (int)$r['loot_min_rarity'];
        $r['loot_gold'] = (int)$r['loot_gold'];
    }
    unset($r);

    echo json_encode([
        'success' => true,
        'message' => 'Companheiros carregados',
        'companions' => $rows,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[companion/get_player_companions] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
