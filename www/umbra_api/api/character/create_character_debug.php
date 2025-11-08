<?php
/**
 * API DEBUG: Criar Personagem
 * Mostra erros detalhados
 */

// ATIVAR ERROS PARA DEBUG
error_reporting(E_ALL);
ini_set('display_errors', '1');

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type');

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
    echo json_encode(['success' => false, 'message' => 'database.php não encontrado']);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Método não permitido']);
    exit;
}

$json = file_get_contents('php://input');
$data = json_decode($json, true);

// Log de entrada
$debug_info = [
    'input_raw' => $json,
    'input_decoded' => $data,
    'account_id' => isset($data['account_id']) ? $data['account_id'] : null,
    'character_name' => isset($data['character_name']) ? $data['character_name'] : null
];

if (empty($data['account_id']) || empty($data['character_name'])) {
    echo json_encode([
        'success' => false,
        'message' => 'Dados incompletos',
        'debug' => $debug_info
    ]);
    exit;
}

$account_id = intval($data['account_id']);
$character_name = trim($data['character_name']);

try {
    $pdo = getConnection();
    
    if (!$pdo) {
        echo json_encode([
            'success' => false,
            'message' => 'getConnection retornou null',
            'debug' => $debug_info
        ]);
        exit;
    }
    
    $debug_info['connection'] = 'OK';
    
    // Verificar conta (accounts.id, não account_id)
    $stmt = $pdo->prepare("SELECT id FROM accounts WHERE id = ?");
    $stmt->execute([$account_id]);
    $account = $stmt->fetch();
    
    $debug_info['account_exists'] = $account ? 'SIM' : 'NAO';
    
    if (!$account) {
        echo json_encode([
            'success' => false,
            'message' => 'Conta não encontrada',
            'debug' => $debug_info
        ]);
        exit;
    }
    
    // Verificar nome
    $stmt = $pdo->prepare("SELECT id FROM players WHERE character_name = ?");
    $stmt->execute([$character_name]);
    $existing = $stmt->fetch();
    
    $debug_info['name_available'] = $existing ? 'NAO (já existe)' : 'SIM';
    
    if ($existing) {
        echo json_encode([
            'success' => false,
            'message' => 'Nome já em uso',
            'debug' => $debug_info
        ]);
        exit;
    }
    
    // Verificar limite
    $stmt = $pdo->prepare("SELECT COUNT(*) as total FROM players WHERE account_id = ?");
    $stmt->execute([$account_id]);
    $result = $stmt->fetch();
    $total = $result['total'];
    
    $debug_info['character_count'] = $total;
    
    if ($total >= 5) {
        echo json_encode([
            'success' => false,
            'message' => 'Limite atingido',
            'debug' => $debug_info
        ]);
        exit;
    }
    
    // Tentar criar
    $debug_info['status'] = 'Tentando INSERT...';
    
    $sql = "
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
    ";
    
    $debug_info['sql'] = $sql;
    $debug_info['params'] = [$account_id, $character_name];
    
    $stmt = $pdo->prepare($sql);
    $result = $stmt->execute([$account_id, $character_name]);
    
    $debug_info['insert_result'] = $result ? 'SUCCESS' : 'FAILED';
    $debug_info['last_insert_id'] = $pdo->lastInsertId();
    
    if (!$result) {
        echo json_encode([
            'success' => false,
            'message' => 'INSERT falhou',
            'debug' => $debug_info,
            'error_info' => $stmt->errorInfo()
        ]);
        exit;
    }
    
    $player_id = $pdo->lastInsertId();
    
    // Buscar personagem criado
    $stmt = $pdo->prepare("SELECT * FROM players WHERE id = ?");
    $stmt->execute([$player_id]);
    $player = $stmt->fetch();
    
    echo json_encode([
        'success' => true,
        'message' => 'Personagem criado com sucesso!',
        'debug' => $debug_info,
        'player' => $player
    ], JSON_PRETTY_PRINT);
    
} catch (PDOException $e) {
    echo json_encode([
        'success' => false,
        'message' => 'ERRO PDO',
        'error' => $e->getMessage(),
        'code' => $e->getCode(),
        'file' => $e->getFile(),
        'line' => $e->getLine(),
        'debug' => $debug_info
    ], JSON_PRETTY_PRINT);
} catch (Exception $e) {
    echo json_encode([
        'success' => false,
        'message' => 'ERRO GERAL',
        'error' => $e->getMessage(),
        'type' => get_class($e),
        'debug' => $debug_info
    ], JSON_PRETTY_PRINT);
}

