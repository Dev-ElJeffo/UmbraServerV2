<?php
/**
 * POST /api/social/leave_party.php
 * Sai do grupo atual
 * Parâmetros: token
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

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$player_id = (int)($validation['payload']['player_id'] ?? 0);

if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $stmt = $pdo->prepare("SELECT party_id FROM party_members WHERE player_id = ?");
    $stmt->execute([$player_id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);

    if (!$row) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Você não está em um grupo']);
        exit;
    }

    $party_id = (int)$row['party_id'];

    $delete = $pdo->prepare("DELETE FROM party_members WHERE party_id = ? AND player_id = ?");
    $delete->execute([$party_id, $player_id]);

    $remaining = $pdo->prepare("SELECT COUNT(*) as count FROM party_members WHERE party_id = ?");
    $remaining->execute([$party_id]);
    $count = (int)$remaining->fetch(PDO::FETCH_ASSOC)['count'];

    if ($count === 0) {
        $delete_party = $pdo->prepare("DELETE FROM parties WHERE party_id = ?");
        $delete_party->execute([$party_id]);
    } else {
        $new_leader = $pdo->prepare("SELECT player_id FROM party_members WHERE party_id = ? ORDER BY joined_at ASC LIMIT 1");
        $new_leader->execute([$party_id]);
        $leader_row = $new_leader->fetch(PDO::FETCH_ASSOC);
        if ($leader_row) {
            $update_leader = $pdo->prepare("UPDATE parties SET leader_id = ? WHERE party_id = ? AND leader_id = ?");
            $update_leader->execute([$leader_row['player_id'], $party_id, $player_id]);
        }
    }

    $pdo->commit();

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Você saiu do grupo'
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log("Erro ao sair do grupo: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
