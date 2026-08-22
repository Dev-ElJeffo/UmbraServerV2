<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    if (isset($_GET['admin_username'])) {
        $data['admin_username'] = $_GET['admin_username'];
    }
}

require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/class_admin_helpers.php';
requireAdminAuth($data);

try {
    $pdo = getConnection();
    $anim = class_has_anim_set_column($pdo) ? ', anim_set_json' : '';
    $stmt = $pdo->query(
        "SELECT class_id, class_name, class_description,
                base_strength, base_dexterity, base_intelligence, base_vitality, base_luck,
                base_health, base_mana, base_stamina,
                base_physical_attack, base_magic_attack, base_physical_defense, base_magic_defense,
                base_accuracy, base_dodge, base_critical, base_movement,
                base_critical_resistance, base_double_attack_resistance, base_double_attack_rate,
                created_at
                $anim
         FROM classes
         ORDER BY class_id ASC"
    );
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    foreach ($rows as &$r) {
        if (isset($r['anim_set_json']) && is_string($r['anim_set_json']) && $r['anim_set_json'] !== '') {
            $decoded = json_decode($r['anim_set_json'], true);
            if (json_last_error() === JSON_ERROR_NONE) {
                $r['anim_set_json'] = $decoded;
            }
        }
        foreach ($r as $k => $v) {
            if (is_numeric($v) && $k !== 'class_name' && $k !== 'class_description' && $k !== 'created_at' && $k !== 'anim_set_json') {
                $r[$k] = (int)$v;
            }
        }
    }
    unset($r);

    echo json_encode([
        'success' => true,
        'classes' => $rows,
        'total' => count($rows),
    ], JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
} catch (Throwable $e) {
    error_log('[admin/list_classes] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao listar classes'], JSON_UNESCAPED_UNICODE);
}
