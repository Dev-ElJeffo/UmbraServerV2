<?php
/**
 * POST /api/character/get_public_info.php
 * Obtém informações públicas de um jogador (para inspeção).
 * Retorna a MESMA estrutura que get_character_info.php (chave "character") para o cliente UE parsear igual.
 *
 * Suporta GET (query params) e POST (JSON body)
 * - player_id: ID do jogador a inspecionar
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/../../helpers/character_info_helper.php';

$active_buffs_helper_path = __DIR__ . '/../../helpers/active_buffs_helper.php';
if (is_file($active_buffs_helper_path)) {
    require_once $active_buffs_helper_path;
}

// Ler dados do body JSON (POST) ou headers (GET)
$data = [];
$json = file_get_contents('php://input');
if (!empty($json)) {
    $data = json_decode($json, true) ?: [];
}

// Se não tem token no body, tentar do header
if (!isset($data['token'])) {
    $headers = getallheaders();
    if (isset($headers['Authorization'])) {
        $data['token'] = str_replace('Bearer ', '', $headers['Authorization']);
    }
}

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

// Obter player_id: primeiro do body JSON (POST), depois de query params (GET)
$target_player_id = null;
if (isset($data['player_id']) && is_numeric($data['player_id'])) {
    $target_player_id = intval($data['player_id']);
} else {
    $target_player_id = isset($_GET['player_id']) ? intval($_GET['player_id']) : null;
}

if (!$target_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id é obrigatório']);
    exit;
}

try {
    $pdo = getConnection();

    // Usar o mesmo helper que get_character_info; não criar stat_points para outros jogadores
    $character = get_character_info_data($pdo, $target_player_id, ['create_stat_points_if_missing' => false]);

    if ($character === null) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não encontrado']);
        exit;
    }

    $character['active_buffs'] = [];
    $character['skill_buffs'] = [];
    $character['passive_buffs'] = [];
    $character['passive_skill_defs'] = [];
    $character['dot_buffs'] = [];
    try {
        if (function_exists('fetch_active_buffs_for_player')) {
            $character['active_buffs'] = fetch_active_buffs_for_player($pdo, $target_player_id);
        }
        if (function_exists('fetch_skill_active_buffs_for_player')) {
            $character['skill_buffs'] = fetch_skill_active_buffs_for_player($pdo, $target_player_id, true);
        }
        $base_health = (int)($character['base_health'] ?? $character['max_health'] ?? 100);
        $current_health = (int)($character['current_health'] ?? $character['health'] ?? 0);
        $health_pct = ($current_health > 0 && $base_health > 0)
            ? (int)floor($current_health * 100 / $base_health)
            : 100;
        if (function_exists('fetch_passive_display_buffs_for_player')) {
            $character['passive_buffs'] = fetch_passive_display_buffs_for_player($pdo, $target_player_id, $health_pct);
        }
        if (function_exists('fetch_passive_skill_defs_for_player')) {
            $character['passive_skill_defs'] = fetch_passive_skill_defs_for_player($pdo, $target_player_id);
        }
        if (function_exists('fetch_active_dots_for_player')) {
            $character['dot_buffs'] = fetch_active_dots_for_player($pdo, $target_player_id);
        }
    } catch (\Throwable $e) {
        error_log("[get_public_info] active_buffs falhou player_id={$target_player_id}: " . $e->getMessage());
        $character['active_buffs'] = [];
        $character['skill_buffs'] = [];
        $character['passive_buffs'] = [];
        $character['passive_skill_defs'] = [];
        $character['dot_buffs'] = [];
    }

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'character' => $character
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    error_log("Erro ao obter informações públicas: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
