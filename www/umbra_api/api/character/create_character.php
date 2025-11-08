<?php
/**
 * API: Criar Personagem
 * Método: POST
 * Parâmetros: token (JWT), character_name
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

if (empty($data['character_name'])) {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'message' => 'character_name é obrigatório'
    ]);
    exit;
}
$character_name = trim($data['character_name']);

// Validações
if (strlen($character_name) < 3) {
    echo json_encode(['success' => false, 'message' => 'Nome do personagem deve ter no mínimo 3 caracteres']);
    exit;
}

if (strlen($character_name) > 20) {
    echo json_encode(['success' => false, 'message' => 'Nome do personagem deve ter no máximo 20 caracteres']);
    exit;
}

if (!preg_match('/^[a-zA-Z0-9_]+$/', $character_name)) {
    echo json_encode(['success' => false, 'message' => 'Nome do personagem deve conter apenas letras, números e underscore']);
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
    
    // Verificar se o nome já existe
    $stmt = $pdo->prepare("SELECT id FROM players WHERE character_name = ?");
    $stmt->execute([$character_name]);
    
    if ($stmt->fetch()) {
        echo json_encode(['success' => false, 'message' => 'Nome de personagem já está em uso']);
        exit;
    }
    
    // Verificar limite de personagens
    $stmt = $pdo->prepare("SELECT COUNT(*) as total FROM players WHERE account_id = ?");
    $stmt->execute([$account_id]);
    $result = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if ($result['total'] >= 5) {
        echo json_encode(['success' => false, 'message' => 'Limite de 5 personagens por conta atingido']);
        exit;
    }
    
    // Criar personagem com valores padrão para TODAS as colunas
    $stmt = $pdo->prepare("
        INSERT INTO players (
            account_id,
            character_name,
            level,
            experience,
            pos_x,
            pos_y,
            pos_z,
            current_zone,
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
            created_at
        ) VALUES (
            ?, ?, 
            1, 0,
            0.0, 0.0, 0.0,
            'Tutorial',
            100, 100,
            50, 50,
            100, 100,
            10, 10, 10, 10,
            NOW()
        )
    ");
    
    $stmt->execute([$account_id, $character_name]);
    
    $player_id = $pdo->lastInsertId();
    
    // Buscar personagem criado
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
            created_at,
            last_played_at
        FROM players
        WHERE id = ?
    ");
    $stmt->execute([$player_id]);
    $player = $stmt->fetch(PDO::FETCH_ASSOC);
    
    echo json_encode([
        'success' => true,
        'message' => 'Personagem criado com sucesso!',
        'player' => [
            'player_id' => intval($player['id']),
            'account_id' => intval($player['account_id']),
            'character_name' => $player['character_name'],
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
        ]
    ]);
    
} catch (Exception $e) {
    error_log("Create Character Error: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao criar personagem']);
}
