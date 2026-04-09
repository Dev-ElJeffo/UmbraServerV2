<?php
/**
 * GET /api/social/get_pending_invites.php
 * Obtém convites/solicitações pendentes do jogador
 * 
 * Query params:
 * - type: 'party', 'trade', 'friend', 'duel' ou 'all'
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/../../helpers/trade_helper.php';

$data = [];
$headers = getallheaders();
if (isset($headers['Authorization'])) {
    $data['token'] = str_replace('Bearer ', '', $headers['Authorization']);
}

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$account_id = intval($validation['payload']['account_id'] ?? 0);
$player_id = intval($validation['payload']['player_id'] ?? 0);
$type = $_GET['type'] ?? 'all';

if ($player_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem ativo. Selecione o personagem novamente.']);
    exit;
}

try {
    $pdo = getConnection();

    $ownStmt = $pdo->prepare('SELECT id FROM players WHERE id = ? AND account_id = ? LIMIT 1');
    $ownStmt->execute([$player_id, $account_id]);
    if (!$ownStmt->fetch()) {
        http_response_code(403);
        echo json_encode([
            'success' => false,
            'message' => 'Personagem do token não pertence à conta. Selecione o personagem novamente.',
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    // Limpar solicitações expiradas e sessões abandonadas (evita listar expiradas)
    cleanupExpiredTrades($pdo);

    // Marcar convites de grupo expirados (limpeza no banco)
    $pdo->exec("UPDATE party_invites SET status = 'expired' WHERE status = 'pending' AND (expires_at IS NULL OR expires_at < NOW())");

    $result = [];

    // Party Invites removidos: convites pendentes de grupo não são mais listados ou carregados ao logar.
    // Convites são recebidos apenas via WebSocket em tempo real.
    $result['party_invites'] = [];
    
    // Trade Requests (útil para teste/debug; em produção o cliente usa WebSocket tempo real)
    if ($type == 'all' || $type == 'trade') {
        $trade_query = $pdo->prepare("
            SELECT tr.request_id, tr.trade_session_id, tr.from_player_id, tr.created_at, tr.expires_at,
                   p.character_name as from_player_name
            FROM trade_requests tr
            INNER JOIN players p ON tr.from_player_id = p.id
            WHERE tr.to_player_id = :player_id AND tr.status = 'pending'
            AND (tr.expires_at IS NULL OR tr.expires_at > NOW())
            ORDER BY tr.created_at DESC
        ");
        $trade_query->execute(['player_id' => $player_id]);
        $result['trade_requests'] = $trade_query->fetchAll(PDO::FETCH_ASSOC);
    }
    
    // Friend Requests
    if ($type == 'all' || $type == 'friend') {
        $friend_query = $pdo->prepare("
            SELECT fr.request_id, fr.from_player_id, fr.created_at,
                   p.character_name as from_player_name
            FROM friend_requests fr
            INNER JOIN players p ON fr.from_player_id = p.id
            WHERE fr.to_player_id = :player_id AND fr.status = 'pending'
            ORDER BY fr.created_at DESC
        ");
        $friend_query->execute(['player_id' => $player_id]);
        $result['friend_requests'] = $friend_query->fetchAll(PDO::FETCH_ASSOC);
    }
    
    // Duel Requests
    if ($type == 'all' || $type == 'duel') {
        $duel_query = $pdo->prepare("
            SELECT dr.request_id, dr.challenger_id, dr.created_at,
                   p.character_name as challenger_name
            FROM duel_requests dr
            INNER JOIN players p ON dr.challenger_id = p.id
            WHERE dr.opponent_id = :player_id AND dr.status = 'pending'
            ORDER BY dr.created_at DESC
        ");
        $duel_query->execute(['player_id' => $player_id]);
        $result['duel_requests'] = $duel_query->fetchAll(PDO::FETCH_ASSOC);
    }
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'data' => $result
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    error_log("Erro ao obter convites pendentes: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
