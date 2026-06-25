<?php
/**
 * POST /api/combat/dot_remove.php
 * Remove DoT/HoT ativo.
 *
 * Body (JSON):
 * {
 *   "token": "jwt...",
 *   "target_player_id": 2,
 *   "source_player_id": 1,
 *   "source_skill_id": 1,
 *   "effect_id": 0
 * }
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

$target_player_id = isset($data['target_player_id']) ? (int)$data['target_player_id'] : 0;
$source_player_id = isset($data['source_player_id']) ? (int)$data['source_player_id'] : 0;
$source_skill_id = isset($data['source_skill_id']) ? (int)$data['source_skill_id'] : 0;
$effect_id = isset($data['effect_id']) ? (int)$data['effect_id'] : 0;

if ($target_player_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'target_player_id inválido']);
    exit;
}

try {
    $pdo = getConnection();

    $sql = 'DELETE FROM active_dots WHERE target_player_id = :target_id';
    $params = ['target_id' => $target_player_id];

    if ($source_skill_id > 0) {
        $sql .= ' AND skill_id = :skill_id';
        $params['skill_id'] = $source_skill_id;
    }
    if ($source_player_id > 0) {
        $sql .= ' AND source_player_id = :source_id';
        $params['source_id'] = $source_player_id;
    }
    if ($effect_id > 0) {
        $sql .= ' AND effect_id = :effect_id';
        $params['effect_id'] = $effect_id;
    }

    $stmt = $pdo->prepare($sql);
    $stmt->execute($params);
    $removed = $stmt->rowCount();

    echo json_encode([
        'success' => true,
        'message' => 'DoT/HoT removido',
        'data' => [
            'removed_count' => $removed,
            'target_player_id' => $target_player_id,
        ],
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('[dot_remove] PDO: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Throwable $e) {
    error_log('[dot_remove] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
