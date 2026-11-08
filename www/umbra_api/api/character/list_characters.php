<?php
/**
 * API: Listar Personagens de uma Conta
 * Método: POST
 * Parâmetros: token (JWT) OU account_id (deprecated, requer token)
 * 
 * ATUALIZADO: Valida token JWT antes de processar
 * Usa account_id do token JWT, não do cliente (segurança)
 */

error_reporting(0);
ini_set('display_errors', '0');

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

// Carregar JWT helper
$jwt_helper_paths = [
    __DIR__ . '/../common/jwt_helper.php',
    __DIR__ . '/../../common/jwt_helper.php',
];

$jwt_loaded = false;
foreach ($jwt_helper_paths as $path) {
    if (file_exists($path)) {
        require_once $path;
        $jwt_loaded = true;
        break;
    }
}

if (!$jwt_loaded) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro de configuração do servidor (JWT helper não encontrado)']);
    exit;
}

// Carregar database.php
$db_loaded = false;
$possible_paths = [
    __DIR__ . '/../config/database.php',
    __DIR__ . '/../../config/database.php',
    'C:/wamp64/www/umbra_api/config/database.php',
];

foreach ($possible_paths as $path) {
    if (file_exists($path)) {
        require_once $path;
        $db_loaded = true;
        break;
    }
}

if (!$db_loaded) {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro de configuração do servidor']);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Método não permitido. Use POST.']);
    exit;
}

$json = file_get_contents('php://input');
$data = json_decode($json, true);

// ✅ VALIDAÇÃO JWT (SEGURANÇA CRÍTICA)
$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error']]);
    exit;
}

// ✅ Usar account_id do token JWT, não do cliente (segurança)
$account_id = intval($validation['payload']['account_id']);

// Verificação adicional: se cliente forneceu account_id, deve corresponder ao token
if (isset($data['account_id']) && intval($data['account_id']) !== $account_id) {
    http_response_code(403);
    echo json_encode(['success' => false, 'message' => 'account_id fornecido não corresponde ao token']);
    exit;
}

try {
    $pdo = getConnection();
    
    if (!$pdo) {
        throw new Exception("Falha na conexão");
    }
    
    // Verificar se a conta existe (accounts.id, não account_id)
    $stmt = $pdo->prepare("SELECT id FROM accounts WHERE id = ?");
    $stmt->execute([$account_id]);
    
    if (!$stmt->fetch()) {
        echo json_encode(['success' => false, 'message' => 'Conta não encontrada']);
        exit;
    }
    
    // Buscar personagens (usando nomes de colunas corretos)
    $stmt = $pdo->prepare("
        SELECT 
            id,
            account_id,
            character_name,
            level,
            experience,
            current_zone,
            pos_x,
            pos_y,
            pos_z,
            health,
            max_health,
            mana,
            max_mana,
            stamina,
            max_stamina,
            strength,
            dexterity,
            intelligence,
            vitality,
            class_id,
            COALESCE(hair, 0) AS hair,
            COALESCE(head, 0) AS head,
            created_at,
            last_played_at
        FROM players
        WHERE account_id = ?
        ORDER BY last_played_at DESC, created_at DESC
    ");
    $stmt->execute([$account_id]);
    $players = $stmt->fetchAll(PDO::FETCH_ASSOC);

    require_once __DIR__ . '/../../helpers/item_visual_helper.php';
    
    // Formatar dados
    $formatted_players = [];
    foreach ($players as $player) {
        $player_id = intval($player['id']);
        $class_id = intval($player['class_id'] ?? 0);
        $equipped_visual = aggregate_player_equipped_visual($pdo, $player_id, $class_id);

        $formatted_players[] = [
            'player_id' => $player_id,
            'account_id' => intval($player['account_id']),
            'character_name' => $player['character_name'],
            'class_id' => intval($player['class_id'] ?? 0),
            'hair' => intval($player['hair'] ?? 0),
            'head' => intval($player['head'] ?? 0),
            'equipped_visual' => $equipped_visual,
            'level' => intval($player['level']),
            'experience' => intval($player['experience']),
            'current_zone' => $player['current_zone'],
            'position' => [
                'x' => floatval($player['pos_x']),
                'y' => floatval($player['pos_y']),
                'z' => floatval($player['pos_z'])
            ],
            'stats' => [
                'health' => intval($player['health']),
                'max_health' => intval($player['max_health']),
                'mana' => intval($player['mana']),
                'max_mana' => intval($player['max_mana']),
                'stamina' => intval($player['stamina']),
                'max_stamina' => intval($player['max_stamina']),
                'strength' => intval($player['strength']),
                'dexterity' => intval($player['dexterity']),
                'intelligence' => intval($player['intelligence']),
                'vitality' => intval($player['vitality'])
            ],
            'created_at' => $player['created_at'],
            'last_login' => $player['last_played_at']
        ];
    }
    
    echo json_encode([
        'success' => true,
        'count' => count($formatted_players),
        'max_characters' => 5,
        'players' => $formatted_players
    ]);
    
} catch (Exception $e) {
    error_log("List Characters Error: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao buscar personagens']);
}
