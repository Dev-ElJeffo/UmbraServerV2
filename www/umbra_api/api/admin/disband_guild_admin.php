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
$operator = !empty($data['admin_username']) ? (string)$data['admin_username'] : 'admin';
if ($guildId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'guild_id obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    $pdo->beginTransaction();
    $st = $pdo->prepare('SELECT guild_name FROM guilds WHERE guild_id = ? FOR UPDATE');
    $st->execute([$guildId]);
    $name = $st->fetchColumn();
    if (!$name) {
        $pdo->rollBack();
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Guild não encontrada']);
        exit;
    }
    $pdo->prepare('DELETE FROM guild_members WHERE guild_id = ?')->execute([$guildId]);
    $pdo->prepare('DELETE FROM guild_invites WHERE guild_id = ?')->execute([$guildId]);
    $pdo->prepare('DELETE FROM guilds WHERE guild_id = ?')->execute([$guildId]);
    $pdo->commit();
    logAdminAudit($pdo, $operator, 'disband_guild_admin', "guild={$guildId};name={$name}", 'guild', $guildId);
    echo json_encode(['success' => true, 'guild_name' => $name], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) $pdo->rollBack();
    error_log('[admin/disband_guild_admin] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno: ' . $e->getMessage()]);
}
