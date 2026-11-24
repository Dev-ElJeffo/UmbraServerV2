<?php
/**
 * POST /api/admin/save_dungeon_difficulties.php — substitui as dificuldades de uma dungeon.
 * Body: instance_template_id, difficulties: [{difficulty_key, name, sort_order, min_level, hp_mult,
 *       damage_mult, reward_mult, is_active}]. Chave repetida = erro. Depois: reload_instances.
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

require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

$templateId = (int)($data['instance_template_id'] ?? 0);
$list = $data['difficulties'] ?? null;
if ($templateId <= 0 || !is_array($list)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'instance_template_id e difficulties são obrigatórios'],
        JSON_UNESCAPED_UNICODE);
    exit;
}
if (count($list) > 8) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'No máximo 8 dificuldades por dungeon'], JSON_UNESCAPED_UNICODE);
    exit;
}

$clean = [];
$seen = [];
foreach ($list as $i => $d) {
    $key = strtolower(trim((string)($d['difficulty_key'] ?? '')));
    if (!preg_match('/^[a-z0-9_]{1,32}$/', $key)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Linha ' . ($i + 1) . ': chave inválida (a-z, 0-9, _)'],
            JSON_UNESCAPED_UNICODE);
        exit;
    }
    if (isset($seen[$key])) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => "Chave repetida: {$key}"], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $seen[$key] = true;
    $name = trim((string)($d['name'] ?? ''));
    $clean[] = [
        'key' => $key,
        'name' => mb_substr($name === '' ? $key : $name, 0, 64),
        'sort' => (int)($d['sort_order'] ?? $i),
        'min_level' => max(1, (int)($d['min_level'] ?? 1)),
        'hp' => max(0.1, min(100.0, (float)($d['hp_mult'] ?? 1.0))),
        'dmg' => max(0.1, min(100.0, (float)($d['damage_mult'] ?? 1.0))),
        'reward' => max(0.0, min(100.0, (float)($d['reward_mult'] ?? 1.0))),
        'active' => ((int)($d['is_active'] ?? 1)) !== 0 ? 1 : 0,
    ];
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    $pdo->prepare('DELETE FROM dungeon_difficulties WHERE instance_template_id = ?')->execute([$templateId]);
    $ins = $pdo->prepare(
        'INSERT INTO dungeon_difficulties
           (instance_template_id, difficulty_key, name, sort_order, min_level, hp_mult, damage_mult, reward_mult, is_active)
         VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)'
    );
    foreach ($clean as $c) {
        $ins->execute([$templateId, $c['key'], $c['name'], $c['sort'], $c['min_level'], $c['hp'], $c['dmg'],
            $c['reward'], $c['active']]);
    }
    $pdo->commit();
    auditAdminWrite('save_dungeon_difficulties', "instance_template_id={$templateId} n=" . count($clean),
        'instance_template', $templateId);
    echo json_encode(['success' => true, 'message' => 'Dificuldades salvas', 'total' => count($clean)],
        JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[admin/save_dungeon_difficulties] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
