<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { http_response_code(200); exit; }
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
requireAdminAuth($data);
require_once __DIR__ . '/../../helpers/admin_audit_helper.php';

$guildId = (int)($data['guild_id'] ?? 0);
$newLeaderId = (int)($data['new_leader_player_id'] ?? 0);
$operator = adminOperatorName();
if ($guildId <= 0 || $newLeaderId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'guild_id e new_leader_player_id obrigatórios']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    $mem = $pdo->prepare('SELECT player_id FROM guild_members WHERE guild_id = ? AND player_id = ? FOR UPDATE');
    $mem->execute([$guildId, $newLeaderId]);
    if (!$mem->fetch()) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Novo líder precisa ser membro']);
        exit;
    }
    $prev = $pdo->prepare('SELECT guild_leader_id FROM guilds WHERE guild_id = ? FOR UPDATE');
    $prev->execute([$guildId]);
    $prevRow = $prev->fetch(PDO::FETCH_ASSOC);
    $oldLeaderId = $prevRow ? (int)$prevRow['guild_leader_id'] : 0;
    $pdo->prepare('UPDATE guilds SET guild_leader_id = ? WHERE guild_id = ?')->execute([$newLeaderId, $guildId]);
    try {
        if ($oldLeaderId > 0 && $oldLeaderId !== $newLeaderId) {
            $pdo->prepare("UPDATE guild_members SET member_rank = 'officer' WHERE guild_id = ? AND player_id = ?")
                ->execute([$guildId, $oldLeaderId]);
        }
        $pdo->prepare("UPDATE guild_members SET member_rank = 'master' WHERE guild_id = ? AND player_id = ?")
            ->execute([$guildId, $newLeaderId]);
    } catch (Throwable $ignored) {
    }
    $pdo->commit();
    logAdminAudit($pdo, $operator, 'transfer_owner_admin', "guild={$guildId};leader={$newLeaderId}", 'guild', $guildId);
    echo json_encode(['success' => true], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) $pdo->rollBack();
    error_log('[admin/transfer_owner_admin] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
