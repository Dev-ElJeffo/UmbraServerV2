<?php
/**
 * API: Criar Personagem
 * Método: POST
 * Parâmetros: token (JWT), character_name, class_id, hair, head
 * 
 * ATUALIZADO: Valida token JWT antes de processar
 * Usa account_id do token JWT, não do cliente (segurança)
 * Aceita class_id, hair e head para personalização do personagem
 * Cria personagem com stats baseados na classe selecionada
 */

// Habilitar logs temporariamente para debug
error_reporting(E_ALL);
ini_set('display_errors', '0'); // Não mostrar na tela, mas logar
ini_set('log_errors', '1');

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

// Validar campos obrigatórios
if (empty($data['character_name'])) {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'message' => 'character_name é obrigatório'
    ]);
    exit;
}

if (empty($data['class_id']) || !isset($data['class_id'])) {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'message' => 'class_id é obrigatório'
    ]);
    exit;
}

if (!isset($data['hair']) || $data['hair'] === null || $data['hair'] === '') {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'message' => 'hair é obrigatório'
    ]);
    exit;
}

if (!isset($data['head']) || $data['head'] === null || $data['head'] === '') {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'message' => 'head é obrigatório'
    ]);
    exit;
}

$character_name = trim($data['character_name']);
$class_id = intval($data['class_id']);
$hair = intval($data['hair']);
$head = intval($data['head']);

// Validações de nome
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

// Validações de class_id
if ($class_id <= 0) {
    echo json_encode(['success' => false, 'message' => 'class_id deve ser um número positivo']);
    exit;
}

// Validações de hair e head (devem ser >= 0)
if ($hair < 0) {
    echo json_encode(['success' => false, 'message' => 'hair deve ser um número maior ou igual a 0']);
    exit;
}

if ($head < 0) {
    echo json_encode(['success' => false, 'message' => 'head deve ser um número maior ou igual a 0']);
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
    
    // Verificar se a classe existe e buscar seus stats
    $stmt = $pdo->prepare("
        SELECT 
            class_id,
            base_strength,
            base_dexterity,
            base_intelligence,
            base_vitality,
            base_luck,
            base_health,
            base_mana,
            base_stamina,
            base_physical_attack,
            base_magic_attack,
            base_physical_defense,
            base_magic_defense,
            base_accuracy,
            base_dodge,
            base_critical,
            base_movement,
            base_critical_resistance,
            base_double_attack_resistance,
            base_double_attack_rate
        FROM classes
        WHERE class_id = ?
    ");
    $stmt->execute([$class_id]);
    $class_data = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$class_data) {
        echo json_encode(['success' => false, 'message' => 'Classe não encontrada']);
        exit;
    }
    
    // Usar stats da classe para criar o personagem
    $base_strength = intval($class_data['base_strength']);
    $base_dexterity = intval($class_data['base_dexterity']);
    $base_intelligence = intval($class_data['base_intelligence']);
    $base_vitality = intval($class_data['base_vitality']);
    $base_luck = intval($class_data['base_luck']);
    $base_health = intval($class_data['base_health']);
    $base_mana = intval($class_data['base_mana']);
    $base_stamina = intval($class_data['base_stamina']);
    
    // Criar personagem com stats da classe e campos hair/head
    // Estrutura da tabela: account_id, character_name, level, experience, next_level_exp, pos_x, pos_y, pos_z, current_zone,
    // health, max_health, mana, max_mana, stamina, max_stamina, strength, dexterity, intelligence, vitality,
    // hair, head, class_id, faction_id, current_guild_id, equipped_title_id, selected_class, luck, pvp, chaos, honor, created_at
    $stmt = $pdo->prepare("
        INSERT INTO players (
            account_id,
            character_name,
            level,
            experience,
            next_level_exp,
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
            hair,
            head,
            class_id,
            faction_id,
            current_guild_id,
            equipped_title_id,
            selected_class,
            luck,
            pvp,
            chaos,
            honor,
            created_at
        ) VALUES (
            ?, ?, 1, 0, 1000,
            0.0, 0.0, 0.0,
            'Tutorial',
            ?, ?,
            ?, ?,
            ?, ?,
            ?, ?, ?, ?,
            ?, ?,
            ?,
            NULL, NULL, NULL, ?,
            ?,
            0, 0, 0,
            NOW()
        )
    ");
    
    // Preparar valores para execução
    $execute_values = [
        $account_id,           // account_id
        $character_name,       // character_name
        $base_health,          // health
        $base_health,          // max_health
        $base_mana,            // mana
        $base_mana,            // max_mana
        $base_stamina,         // stamina
        $base_stamina,         // max_stamina
        $base_strength,        // strength
        $base_dexterity,       // dexterity
        $base_intelligence,    // intelligence
        $base_vitality,        // vitality
        $hair,                 // hair
        $head,                 // head
        $class_id,             // class_id
        $class_id,             // selected_class
        $base_luck             // luck
    ];
    
    error_log("Create Character - Account ID: " . $account_id);
    error_log("Create Character - Character Name: " . $character_name);
    error_log("Create Character - Class ID: " . $class_id);
    error_log("Create Character - Hair: " . $hair);
    error_log("Create Character - Head: " . $head);
    error_log("Create Character - Values count: " . count($execute_values));
    error_log("Create Character - Values: " . print_r($execute_values, true));
    
    $result = $stmt->execute($execute_values);
    
    if (!$result) {
        $error_info = $stmt->errorInfo();
        error_log("Create Character - SQL Error: " . print_r($error_info, true));
        throw new PDOException("Erro ao executar INSERT: " . $error_info[2], intval($error_info[0]));
    }
    
    $player_id = $pdo->lastInsertId();
    
    // Buscar personagem criado
    $stmt = $pdo->prepare("
        SELECT 
            id,
            account_id,
            character_name,
            class_id,
            hair,
            head,
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
            luck,
            created_at,
            last_played_at
        FROM players
        WHERE id = ?
    ");
    
    $stmt->execute([$player_id]);
    $player = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$player) {
        throw new Exception("Personagem criado mas não foi possível recuperar os dados");
    }
    
    echo json_encode([
        'success' => true,
        'message' => 'Personagem criado com sucesso!',
        'player' => [
            'player_id' => intval($player['id']),
            'account_id' => intval($player['account_id']),
            'character_name' => $player['character_name'],
            'class_id' => intval($player['class_id']),
            'hair' => intval($player['hair']),
            'head' => intval($player['head']),
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
                'vitality' => intval($player['vitality']),
                'luck' => intval($player['luck'])
            ],
            'created_at' => $player['created_at'],
            'last_login' => $player['last_played_at']
        ]
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    error_log("Create Character PDO Error: " . $e->getMessage());
    error_log("SQL State: " . $e->getCode());
    if (isset($stmt)) {
        error_log("SQL Error Info: " . print_r($stmt->errorInfo(), true));
    }
    error_log("Stack trace: " . $e->getTraceAsString());
    http_response_code(500);
    
    // Retornar erro detalhado para debug
    echo json_encode([
        'success' => false, 
        'message' => 'Erro ao criar personagem',
        'error' => $e->getMessage(),
        'sql_state' => $e->getCode(),
        'error_info' => isset($stmt) ? $stmt->errorInfo() : null
    ], JSON_UNESCAPED_UNICODE);
} catch (Exception $e) {
    error_log("Create Character Error: " . $e->getMessage());
    error_log("Stack trace: " . $e->getTraceAsString());
    http_response_code(500);
    
    // Retornar erro detalhado para debug
    echo json_encode([
        'success' => false, 
        'message' => 'Erro ao criar personagem',
        'error' => $e->getMessage(),
        'file' => $e->getFile(),
        'line' => $e->getLine()
    ], JSON_UNESCAPED_UNICODE);
}
