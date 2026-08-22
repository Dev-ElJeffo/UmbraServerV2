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

$classId = (int)($data['class_id'] ?? 0);
$skills = $data['skills'] ?? null;
if ($classId <= 0 || !is_array($skills)) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'class_id e skills[] obrigatórios'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();
    $upd = $pdo->prepare(
        'UPDATE skills SET cast_anim_path = :path WHERE skill_id = :sid AND class_id = :cid'
    );
    $isBasic = $pdo->prepare(
        'SELECT is_basic_attack FROM skills WHERE skill_id = :sid AND class_id = :cid LIMIT 1'
    );
    $updBasic = $pdo->prepare(
        'UPDATE basic_attacks SET cast_anim_path = :path WHERE class_id = :cid'
    );
    $updated = 0;
    foreach ($skills as $row) {
        $sid = (int)($row['skill_id'] ?? 0);
        if ($sid <= 0) {
            continue;
        }
        $path = trim((string)($row['cast_anim_path'] ?? ''));
        $pathOrNull = $path === '' ? null : $path;
        $upd->execute([
            ':path' => $pathOrNull,
            ':sid' => $sid,
            ':cid' => $classId,
        ]);
        $updated += $upd->rowCount() > 0 ? 1 : 0;

        // Ataque básico do player (opcode 99) lê basic_attacks, não só skills.
        $isBasic->execute([':sid' => $sid, ':cid' => $classId]);
        $flag = $isBasic->fetchColumn();
        if ($flag !== false && (int)$flag === 1) {
            $updBasic->execute([
                ':path' => $pathOrNull,
                ':cid' => $classId,
            ]);
        }
    }

    echo json_encode([
        'success' => true,
        'message' => 'Animações das skills atualizadas',
        'class_id' => $classId,
        'updated' => $updated,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/upsert_class_skill_anims] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao salvar anims das skills'], JSON_UNESCAPED_UNICODE);
}
