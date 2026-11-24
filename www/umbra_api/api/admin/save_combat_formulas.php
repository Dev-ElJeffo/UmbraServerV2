<?php
/**
 * POST /api/admin/save_combat_formulas.php — grava as constantes das fórmulas de combate (id = 1).
 * Body: settings{...} com as chaves de combat_formula_defaults(). Valor fora da faixa é recusado.
 * Depois: reload_combat_formulas nas zones.
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
require_once __DIR__ . '/../../helpers/combat_formula_helper.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

function formula_fail(string $msg): void
{
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $msg], JSON_UNESCAPED_UNICODE);
    exit;
}

$in = is_array($data['settings'] ?? null) ? $data['settings'] : [];
$defaults = combat_formula_defaults();

// chave => [mínimo, máximo, rótulo]
$ranges = [
    'crit_base_pct' => [0, 100, 'Crítico base (%)'],
    'crit_bonus_max_pct' => [0, 100, 'Bônus máximo de crítico (%)'],
    'crit_k' => [1, 100000, 'K do crítico'],
    'crit_resist_k' => [1, 100000, 'K da resistência a crítico'],
    'crit_cap_pct' => [0, 100, 'Teto do crítico (%)'],
    'double_max_pct' => [0, 100, 'Duplo máximo (%)'],
    'double_k' => [1, 100000, 'K do duplo'],
    'double_resist_k' => [1, 100000, 'K da resistência a duplo'],
    'double_damage_pct' => [0, 500, 'Golpe extra do duplo (%)'],
    'hit_base_pct' => [0, 100, 'Acerto base (%)'],
    'hit_accuracy_bonus_pct' => [0, 100, 'Bônus máximo de precisão (%)'],
    'hit_accuracy_k' => [1, 100000, 'K da precisão'],
    'hit_dodge_max_pct' => [0, 100, 'Esquiva máxima (%)'],
    'hit_dodge_k' => [1, 100000, 'K da esquiva'],
    'hit_min_pct' => [0, 100, 'Acerto mínimo (%)'],
    'hit_max_pct' => [0, 100, 'Acerto máximo (%)'],
    'defense_k_base' => [1, 1000000, 'K base da defesa'],
    'defense_k_per_level' => [0, 100000, 'K da defesa por nível'],
    'defense_cap_pct' => [0, 95, 'Teto da defesa (%)'],
    'attribute_scaling_k' => [1, 100000, 'K da escala de atributo'],
    'damage_variance_pct' => [0, 50, 'Variação de dano (%)'],
];

$s = [];
foreach ($ranges as $key => [$min, $max, $label]) {
    $v = isset($in[$key]) ? (float)$in[$key] : (float)$defaults[$key];
    if ($v < $min || $v > $max) {
        formula_fail("{$label}: use um valor entre {$min} e {$max}");
    }
    $s[$key] = $key === 'double_damage_pct' ? (int)round($v) : $v;
}
if ($s['hit_min_pct'] > $s['hit_max_pct']) {
    formula_fail('Acerto mínimo não pode ser maior que o máximo');
}

try {
    $pdo = getConnection();
    $cols = array_keys($s);
    $insertCols = implode(', ', $cols);
    $params = implode(', ', array_map(static fn($c) => ':' . $c, $cols));
    $updates = implode(', ', array_map(static fn($c) => "{$c} = VALUES({$c})", $cols));
    $stmt = $pdo->prepare(
        "INSERT INTO combat_formula_settings (id, {$insertCols}) VALUES (1, {$params})
         ON DUPLICATE KEY UPDATE {$updates}"
    );
    $stmt->execute($s);
    auditAdminWrite('save_combat_formulas', 'crit_k=' . $s['crit_k'] . ' def_k_lvl=' . $s['defense_k_per_level']
        . ' var=' . $s['damage_variance_pct'], 'combat_formula_settings', 1, $s);
    echo json_encode(['success' => true, 'message' => 'Fórmulas de combate salvas', 'settings' => $s],
        JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/save_combat_formulas] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
