<?php
/**
 * Umbra Eternum - API de Skills
 * Endpoint: reset_skill_points.php
 * Método: POST
 *
 * R2: devolve todos os pontos de skill (desaprende tudo, menos o ataque básico) por gold.
 * Custo = nível × UMBRA_SKILL_RESET_GOLD_PER_LEVEL. Com "preview": true só retorna o custo.
 * Também limpa da skillbar os slots das skills desaprendidas.
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

/** Gold por nível cobrado no reset. */
const UMBRA_SKILL_RESET_GOLD_PER_LEVEL = 100;

try {
    $data = json_decode(file_get_contents('php://input'), true);
    if (!is_array($data)) {
        $data = [];
    }

    $jwtResult = validateJWTRequest($data, $_SERVER);
    if (!$jwtResult['valid']) {
        http_response_code(401);
        echo json_encode(['success' => false, 'message' => $jwtResult['error']], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $playerId = (int)($jwtResult['payload']['player_id'] ?? 0);
    if ($playerId <= 0) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'player_id não encontrado no token'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $previewOnly = !empty($data['preview']);

    $pdo = getConnection();
    $pdo->beginTransaction();

    try {
        $stmt = $pdo->prepare("SELECT id, level, gold FROM players WHERE id = :player_id FOR UPDATE");
        $stmt->execute([':player_id' => $playerId]);
        $player = $stmt->fetch(PDO::FETCH_ASSOC);
        if (!$player) {
            $pdo->rollBack();
            http_response_code(404);
            echo json_encode(['success' => false, 'message' => 'Jogador não encontrado'], JSON_UNESCAPED_UNICODE);
            exit;
        }

        $goldCost = max(0, (int)$player['level']) * UMBRA_SKILL_RESET_GOLD_PER_LEVEL;
        $gold = (int)$player['gold'];

        $stmt = $pdo->prepare("
            SELECT total_points_earned, points_spent
            FROM player_skill_points
            WHERE player_id = :player_id
            FOR UPDATE
        ");
        $stmt->execute([':player_id' => $playerId]);
        $points = $stmt->fetch(PDO::FETCH_ASSOC);
        $totalEarned = $points ? (int)$points['total_points_earned'] : 0;
        $spent = $points ? (int)$points['points_spent'] : 0;

        if ($previewOnly) {
            $pdo->rollBack();
            echo json_encode([
                'success' => true,
                'message' => 'Prévia do reset',
                'data' => [
                    'gold_cost' => $goldCost,
                    'gold' => $gold,
                    'points_refunded' => $spent,
                    'can_reset' => $gold >= $goldCost && $spent > 0
                ]
            ], JSON_UNESCAPED_UNICODE);
            exit;
        }

        if ($spent <= 0) {
            $pdo->rollBack();
            http_response_code(409);
            echo json_encode(['success' => false, 'message' => 'Nenhum ponto de skill para devolver'], JSON_UNESCAPED_UNICODE);
            exit;
        }
        if ($gold < $goldCost) {
            $pdo->rollBack();
            http_response_code(403);
            echo json_encode([
                'success' => false,
                'message' => "Gold insuficiente. Necessário: {$goldCost}"
            ], JSON_UNESCAPED_UNICODE);
            exit;
        }

        // Skillbar: remove os slots de skills que serão desaprendidas (itens ficam).
        $stmt = $pdo->prepare("
            DELETE sb FROM player_skillbar sb
            INNER JOIN skills s ON s.skill_id = sb.skill_id
            WHERE sb.player_id = :player_id AND COALESCE(s.is_basic_attack, 0) = 0
              AND sb.item_template_id IS NULL
        ");
        $stmt->execute([':player_id' => $playerId]);

        $stmt = $pdo->prepare("
            DELETE ps FROM player_skills ps
            INNER JOIN skills s ON s.skill_id = ps.skill_id
            WHERE ps.player_id = :player_id AND COALESCE(s.is_basic_attack, 0) = 0
        ");
        $stmt->execute([':player_id' => $playerId]);
        $skillsRemoved = $stmt->rowCount();

        $stmt = $pdo->prepare("
            UPDATE player_skill_points
            SET points_spent = 0, points_available = total_points_earned
            WHERE player_id = :player_id
        ");
        $stmt->execute([':player_id' => $playerId]);

        if ($goldCost > 0) {
            $stmt = $pdo->prepare("UPDATE players SET gold = gold - :cost WHERE id = :player_id");
            $stmt->execute([':cost' => $goldCost, ':player_id' => $playerId]);
        }

        $pdo->commit();

        echo json_encode([
            'success' => true,
            'message' => 'Pontos de skill redistribuíveis novamente!',
            'data' => [
                'gold_cost' => $goldCost,
                'gold' => $gold - $goldCost,
                'skills_removed' => $skillsRemoved,
                'skill_points' => [
                    'total_earned' => $totalEarned,
                    'spent' => 0,
                    'available' => $totalEarned
                ]
            ]
        ], JSON_UNESCAPED_UNICODE);
    } catch (Exception $e) {
        if ($pdo->inTransaction()) {
            $pdo->rollBack();
        }
        throw $e;
    }
} catch (Exception $e) {
    error_log('[skills/reset_skill_points] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor'], JSON_UNESCAPED_UNICODE);
}
