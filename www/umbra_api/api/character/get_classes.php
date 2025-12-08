<?php
/**
 * API: Buscar Classes Disponíveis
 * Método: GET
 * 
 * Retorna todas as classes disponíveis no jogo com suas informações completas
 */

error_reporting(0);
ini_set('display_errors', '0');

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

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

try {
    $pdo = getConnection();
    
    if (!$pdo) {
        throw new Exception("Falha na conexão");
    }
    
    // Buscar todas as classes
    $stmt = $pdo->query("
        SELECT 
            class_id,
            class_name,
            class_description,
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
            base_double_attack_rate,
            created_at
        FROM classes
        ORDER BY class_id ASC
    ");
    
    $classes = [];
    while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        $classes[] = [
            'class_id' => intval($row['class_id']),
            'class_name' => $row['class_name'],
            'class_description' => $row['class_description'],
            'base_stats' => [
                'strength' => intval($row['base_strength']),
                'dexterity' => intval($row['base_dexterity']),
                'intelligence' => intval($row['base_intelligence']),
                'vitality' => intval($row['base_vitality']),
                'luck' => intval($row['base_luck'])
            ],
            'base_resources' => [
                'health' => intval($row['base_health']),
                'mana' => intval($row['base_mana']),
                'stamina' => intval($row['base_stamina'])
            ],
            'base_combat' => [
                'physical_attack' => intval($row['base_physical_attack']),
                'magic_attack' => intval($row['base_magic_attack']),
                'physical_defense' => intval($row['base_physical_defense']),
                'magic_defense' => intval($row['base_magic_defense']),
                'accuracy' => intval($row['base_accuracy']),
                'dodge' => intval($row['base_dodge']),
                'critical' => intval($row['base_critical']),
                'movement' => intval($row['base_movement']),
                'critical_resistance' => intval($row['base_critical_resistance']),
                'double_attack_resistance' => intval($row['base_double_attack_resistance']),
                'double_attack_rate' => intval($row['base_double_attack_rate'])
            ],
            'created_at' => $row['created_at']
        ];
    }
    
    echo json_encode([
        'success' => true,
        'message' => 'Classes carregadas com sucesso',
        'classes' => $classes,
        'total' => count($classes)
    ], JSON_UNESCAPED_UNICODE);
    
} catch (Exception $e) {
    error_log("Get Classes Error: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao buscar classes']);
}

