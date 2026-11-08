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

require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

$questId = (int)($data['quest_id'] ?? 0);
$offerId = (int)($data['offer_id'] ?? 0);
$unlinkOnly = !empty($data['unlink_only']);
$npcTemplateId = (int)($data['npc_template_id'] ?? 0);
$force = !empty($data['force']);

if ($unlinkOnly) {
    if ($offerId <= 0 && ($questId <= 0 || $npcTemplateId <= 0)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'offer_id ou (quest_id + npc_template_id) para unlink'], JSON_UNESCAPED_UNICODE);
        exit;
    }
} elseif ($questId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'quest_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();

    if ($unlinkOnly) {
        if ($offerId > 0) {
            $stmt = $pdo->prepare('DELETE FROM npc_quest_offers WHERE offer_id = :id');
            $stmt->execute([':id' => $offerId]);
        } else {
            $stmt = $pdo->prepare('DELETE FROM npc_quest_offers WHERE quest_id = :qid AND npc_template_id = :tid');
            $stmt->execute([':qid' => $questId, ':tid' => $npcTemplateId]);
        }
        if ($stmt->rowCount() <= 0) {
            http_response_code(404);
            echo json_encode(['success' => false, 'message' => 'Oferta não encontrada'], JSON_UNESCAPED_UNICODE);
            exit;
        }
        echo json_encode([
            'success' => true,
            'message' => 'Oferta desvinculada',
            'unlink_only' => true,
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $chk = $pdo->prepare('SELECT quest_id FROM quests WHERE quest_id = :id LIMIT 1');
    $chk->execute([':id' => $questId]);
    if (!$chk->fetch()) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Quest não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $progress = 0;
    try {
        $p = $pdo->prepare('SELECT COUNT(*) FROM player_quests WHERE quest_id = :id');
        $p->execute([':id' => $questId]);
        $progress = (int)$p->fetchColumn();
    } catch (Throwable $ignored) {
    }
    if ($progress > 0 && !$force) {
        http_response_code(409);
        echo json_encode([
            'success' => false,
            'message' => "Quest com progresso de $progress jogador(es). Envie force=1 para apagar.",
            'player_quests' => $progress,
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $stmt = $pdo->prepare('DELETE FROM quests WHERE quest_id = :id');
    $stmt->execute([':id' => $questId]);
    if ($stmt->rowCount() <= 0) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Quest não encontrada'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    auditAdminWrite('delete_quest', "quest_id={$questId};force=" . ($force ? '1' : '0'), 'quest', $questId);
    echo json_encode([
        'success' => true,
        'message' => 'Quest removida',
        'quest_id' => $questId,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/delete_quest] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
