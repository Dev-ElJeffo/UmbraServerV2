<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$levels = $data['levels'] ?? null;
if (!is_array($levels) || count($levels) === 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'levels[] obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();

    $upsertLevel = $pdo->prepare(
        "INSERT INTO player_levels
            (level_number, exp_required, exp_for_next_level, stat_points_gained,
             hp_gain, mp_gain, phys_atk_gain, mag_atk_gain, phys_def_gain, mag_def_gain)
         VALUES
            (:level, :exp_required, :exp_for_next_level, :stat_points_gained,
             :hp_gain, :mp_gain, :phys_atk_gain, :mag_atk_gain, :phys_def_gain, :mag_def_gain)
         ON DUPLICATE KEY UPDATE
            exp_required = VALUES(exp_required),
            exp_for_next_level = VALUES(exp_for_next_level),
            stat_points_gained = VALUES(stat_points_gained),
            hp_gain = VALUES(hp_gain),
            mp_gain = VALUES(mp_gain),
            phys_atk_gain = VALUES(phys_atk_gain),
            mag_atk_gain = VALUES(mag_atk_gain),
            phys_def_gain = VALUES(phys_def_gain),
            mag_def_gain = VALUES(mag_def_gain)"
    );
    $upsertSkill = $pdo->prepare(
        "INSERT INTO skill_points_per_level (`level`, points_granted, cumulative_points)
         VALUES (:level, :granted, 0)
         ON DUPLICATE KEY UPDATE points_granted = VALUES(points_granted)"
    );

    $saved = 0;
    $skillByLevel = [];
    foreach ($levels as $row) {
        if (!is_array($row)) {
            continue;
        }
        $level = (int)($row['level'] ?? $row['level_number'] ?? 0);
        if ($level <= 0) {
            continue;
        }
        $upsertLevel->execute([
            ':level' => $level,
            ':exp_required' => max(0, (int)($row['exp_required'] ?? 0)),
            ':exp_for_next_level' => max(0, (int)($row['exp_for_next_level'] ?? 0)),
            ':stat_points_gained' => max(0, (int)($row['stat_points_gained'] ?? 0)),
            ':hp_gain' => max(0, (int)($row['hp_gain'] ?? 0)),
            ':mp_gain' => max(0, (int)($row['mp_gain'] ?? 0)),
            ':phys_atk_gain' => max(0, (int)($row['phys_atk_gain'] ?? 0)),
            ':mag_atk_gain' => max(0, (int)($row['mag_atk_gain'] ?? 0)),
            ':phys_def_gain' => max(0, (int)($row['phys_def_gain'] ?? 0)),
            ':mag_def_gain' => max(0, (int)($row['mag_def_gain'] ?? 0)),
        ]);
        $granted = max(0, (int)($row['skill_points_granted'] ?? 0));
        $upsertSkill->execute([':level' => $level, ':granted' => $granted]);
        $skillByLevel[$level] = $granted;
        $saved++;
    }

    ksort($skillByLevel, SORT_NUMERIC);
    $running = 0;
    $updCum = $pdo->prepare(
        'UPDATE skill_points_per_level SET cumulative_points = :cum WHERE `level` = :level'
    );
    foreach ($skillByLevel as $level => $granted) {
        $running += $granted;
        $updCum->execute([':cum' => $running, ':level' => $level]);
    }

    $pdo->commit();
    echo json_encode([
        'success' => true,
        'message' => 'Progressão salva',
        'saved' => $saved,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[admin/upsert_progression] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao salvar progressão: ' . $e->getMessage(),
    ], JSON_UNESCAPED_UNICODE);
}
