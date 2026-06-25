<?php
/**
 * GET /api/inventory/get_item_template_filters.php
 * Retorna valores distintos de item_type, item_subtype e rarity para ComboBoxes (leilão / filtros).
 * Autenticação: token em query ou Authorization (mesmo padrão JWT).
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

$data = [];
if (!empty($_GET['token'])) {
    $data['token'] = $_GET['token'];
}

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$account_id = (int)($validation['payload']['account_id'] ?? 0);
$player_id = (int)($validation['payload']['player_id'] ?? 0);

if ($player_id <= 0 || $account_id <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token sem personagem ativo.']);
    exit;
}

try {
    $pdo = getConnection();

    $stmt = $pdo->prepare('SELECT id FROM players WHERE id = ? AND account_id = ? LIMIT 1');
    $stmt->execute([$player_id, $account_id]);
    if (!$stmt->fetch()) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Personagem do token não pertence à conta.']);
        exit;
    }

    $types = $pdo->query("SELECT DISTINCT item_type FROM item_templates WHERE item_type IS NOT NULL AND item_type != '' ORDER BY item_type ASC")
        ->fetchAll(PDO::FETCH_COLUMN);
    $subtypes = $pdo->query("SELECT DISTINCT item_subtype FROM item_templates WHERE item_subtype IS NOT NULL AND item_subtype != '' ORDER BY item_subtype ASC")
        ->fetchAll(PDO::FETCH_COLUMN);
    $rarities = $pdo->query("SELECT DISTINCT rarity FROM item_templates ORDER BY rarity DESC")
        ->fetchAll(PDO::FETCH_COLUMN);

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'item_types' => array_values($types),
        'item_subtypes' => array_values($subtypes),
        'rarities' => array_values($rarities),
    ], JSON_UNESCAPED_UNICODE);
} catch (PDOException $e) {
    error_log('get_item_template_filters: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao carregar filtros', 'error' => $e->getMessage()]);
}
