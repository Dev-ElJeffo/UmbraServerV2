<?php
ini_set('display_errors', '0');
error_reporting(E_ALL);
ob_start();

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
    $data['admin_username'] = $_GET['admin_username'] ?? $data['admin_username'] ?? null;
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/stat_key_mapping.php';
require_once __DIR__ . '/../../helpers/enchant_helper.php';

try {
    $pdo = getConnection();
    if (!$pdo) {
        throw new RuntimeException('Falha na conexão com o banco');
    }
    $stats = $pdo->query('
        SELECT stat_key, display_name, weight, value_min, value_max, tier
        FROM enchant_stat_weights
        ORDER BY tier ASC, weight DESC, stat_key ASC
    ')->fetchAll(PDO::FETCH_ASSOC);
    if (!is_array($stats)) {
        $stats = [];
    }

    $weightSum = 0;
    foreach ($stats as $row) {
        $weightSum += max(0, (int)$row['weight']);
    }

    $outStats = [];
    foreach ($stats as $row) {
        $w = (int)$row['weight'];
        $outStats[] = [
            'stat_key' => $row['stat_key'],
            'display_name' => $row['display_name'] ?: enchant_display_name((string)$row['stat_key']),
            'weight' => $w,
            'weight_percent' => $weightSum > 0 ? round($w * 100.0 / $weightSum, 2) : 0,
            'value_min' => (int)$row['value_min'],
            'value_max' => (int)$row['value_max'],
            'tier' => (int)$row['tier'],
        ];
    }

    $slots = [0 => 0, 1 => 0, 2 => 0, 3 => 0];
    $slotRows = $pdo->query('SELECT slot_count, weight FROM enchant_slot_chances')->fetchAll(PDO::FETCH_ASSOC);
    $slotSum = 0;
    foreach ($slotRows as $r) {
        $n = (int)$r['slot_count'];
        if ($n >= 0 && $n <= 3) {
            $slots[$n] = (int)$r['weight'];
            $slotSum += (int)$r['weight'];
        }
    }
    $slotsObj = [
        '0' => $slots[0],
        '1' => $slots[1],
        '2' => $slots[2],
        '3' => $slots[3],
    ];
    $slotPercents = [];
    foreach ($slotsObj as $n => $w) {
        $slotPercents[$n] = $slotSum > 0 ? round($w * 100.0 / $slotSum, 2) : 0;
    }

    while (ob_get_level() > 0) {
        ob_end_clean();
    }
    echo json_encode([
        'success' => true,
        'stats' => $outStats,
        'slot_weights' => $slotsObj,
        'slot_percents' => $slotPercents,
        'weight_sum' => $weightSum,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    while (ob_get_level() > 0) {
        ob_end_clean();
    }
    error_log('get_enchant_config: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao carregar config de encantos'], JSON_UNESCAPED_UNICODE);
}
