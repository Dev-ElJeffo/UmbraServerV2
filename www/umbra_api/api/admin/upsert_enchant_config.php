<?php
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

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
require_once __DIR__ . '/../../config/database.php';

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    if (!empty($data['stats']) && is_array($data['stats'])) {
        $upd = $pdo->prepare('
            UPDATE enchant_stat_weights
            SET weight = :w, value_min = :vmin, value_max = :vmax, tier = :tier, display_name = COALESCE(NULLIF(:dn, \'\'), display_name)
            WHERE stat_key = :k
        ');
        foreach ($data['stats'] as $row) {
            if (!is_array($row) || empty($row['stat_key'])) {
                continue;
            }
            $min = (int)($row['value_min'] ?? 1);
            $max = (int)($row['value_max'] ?? 1);
            if ($max < $min) {
                $tmp = $min;
                $min = $max;
                $max = $tmp;
            }
            $tier = (int)($row['tier'] ?? 1);
            if ($tier < 1) {
                $tier = 1;
            }
            if ($tier > 4) {
                $tier = 4;
            }
            $upd->execute([
                'w' => max(0, (int)($row['weight'] ?? 0)),
                'vmin' => $min,
                'vmax' => $max,
                'tier' => $tier,
                'dn' => (string)($row['display_name'] ?? ''),
                'k' => (string)$row['stat_key'],
            ]);
        }
    }

    if (isset($data['slot_weights']) && is_array($data['slot_weights'])) {
        $slotUpd = $pdo->prepare('
            INSERT INTO enchant_slot_chances (slot_count, weight) VALUES (:n, :w)
            ON DUPLICATE KEY UPDATE weight = VALUES(weight)
        ');
        foreach ([0, 1, 2, 3] as $n) {
            $key = (string)$n;
            $w = $data['slot_weights'][$n] ?? ($data['slot_weights'][$key] ?? null);
            if ($w === null) {
                continue;
            }
            $slotUpd->execute(['n' => $n, 'w' => max(0, (int)$w)]);
        }
    }

    $pdo->commit();
    echo json_encode(['success' => true, 'message' => 'Configuração de encantos salva'], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo instanceof PDO && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('upsert_enchant_config: ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao salvar config de encantos'], JSON_UNESCAPED_UNICODE);
}
