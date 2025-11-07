<?php
// Suprimir warnings para garantir JSON puro
error_reporting(E_ERROR | E_PARSE);
ini_set('display_errors', '0');

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../utils/auth.php';
require_once __DIR__ . '/../../utils/response.php';

header('Content-Type: application/json');

// Verificar método
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    sendErrorResponse(405, 'Método não permitido');
    exit;
}

// Obter dados do body
$input = json_decode(file_get_contents('php://input'), true);

if (!$input) {
    sendErrorResponse(400, 'JSON inválido');
    exit;
}

// Validar token
$token = $input['token'] ?? '';
$accountId = validateToken($token);

if (!$accountId) {
    sendErrorResponse(401, 'Token inválido ou expirado');
    exit;
}

// Validar campos obrigatórios
$playerId = $input['player_id'] ?? 0;
$posX = $input['pos_x'] ?? null;
$posY = $input['pos_y'] ?? null;
$posZ = $input['pos_z'] ?? null;
$currentZone = $input['current_zone'] ?? 'Tutorial';

if ($playerId <= 0 || $posX === null || $posY === null || $posZ === null) {
    sendErrorResponse(400, 'Campos obrigatórios: player_id, pos_x, pos_y, pos_z');
    exit;
}

try {
    // Verificar se o player pertence à conta
    $stmt = $pdo->prepare("SELECT account_id FROM players WHERE id = ?");
    $stmt->execute([$playerId]);
    $player = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$player || $player['account_id'] != $accountId) {
        sendErrorResponse(403, 'Player não pertence à sua conta');
        exit;
    }
    
    // Atualizar posição
    $stmt = $pdo->prepare("
        UPDATE players 
        SET pos_x = ?, pos_y = ?, pos_z = ?, current_zone = ?, last_played_at = NOW()
        WHERE id = ?
    ");
    
    $success = $stmt->execute([$posX, $posY, $posZ, $currentZone, $playerId]);
    
    if ($success) {
        sendSuccessResponse([
            'message' => 'Posição atualizada com sucesso',
            'player_id' => $playerId,
            'position' => [
                'x' => $posX,
                'y' => $posY,
                'z' => $posZ
            ],
            'current_zone' => $currentZone
        ]);
    } else {
        sendErrorResponse(500, 'Erro ao atualizar posição');
    }
} catch (PDOException $e) {
    error_log("Erro ao atualizar posição: " . $e->getMessage());
    sendErrorResponse(500, 'Erro interno do servidor');
}
?>

