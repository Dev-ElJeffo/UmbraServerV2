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

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
require_once __DIR__ . '/../../helpers/admin_audit_helper.php';

$guildId = (int)($data['guild_id'] ?? 0);
$playerId = (int)($data['player_id'] ?? 0);
$operator = !empty($data['admin_username']) ? (string)$data['admin_username'] : 'admin';
if ($guildId <= 0 || $playerId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'guild_id e player_id obrigatórios']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    $st = $pdo->prepare('SELECT guild_leader_id FROM guilds WHERE guild_id = ? FOR UPDATE');
    $st->execute([$guildId]);
    $g = $st->fetch(PDO::FETCH_ASSOC);
    if (!$g) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Guild não encontrada']);
        exit;
    }
    if ((int)$g['guild_leader_id'] === $playerId) {
        $pdo->rollBack();
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Não pode kickar o líder; use transfer ou disband']);
        exit;
    }
    $del = $pdo->prepare('DELETE FROM guild_members WHERE guild_id = ? AND player_id = ?');
    $del->execute([$guildId, $playerId]);
    if ($del->rowCount() === 0) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Membro não encontrado']);
        exit;
    }
    $pdo->prepare('UPDATE guilds SET member_count = GREATEST(member_count - 1, 0) WHERE guild_id = ?')
        ->execute([$guildId]);
    $pdo->commit();
    logAdminAudit($pdo, $operator, 'kick_guild_member', "guild={$guildId};player={$playerId}", 'guild', $guildId);
    echo json_encode(['success' => true], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) $pdo->rollBack();
    error_log('[admin/kick_guild_member] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno']);
}
