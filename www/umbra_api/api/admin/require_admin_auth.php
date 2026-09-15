<?php
/**
 * Autenticação admin do UmbraManager: JWT obrigatório + isadmin no banco + role.
 */
require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/../../helpers/admin_audit_helper.php';

function admin_decode_json_body(?string $raw = null): array
{
    if ($raw === null) {
        $raw = file_get_contents('php://input');
    }
    if ($raw === false) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Falha ao ler o corpo da requisição'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    $trim = trim($raw);
    if ($trim === '') {
        return [];
    }
    $data = json_decode($trim, true);
    if (!is_array($data)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'JSON inválido'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    return $data;
}

function adminRolesForScript(?string $script = null): array
{
    $script = $script ?? basename($_SERVER['SCRIPT_FILENAME'] ?? '');
    $super = ['super'];
    $ops = ['super', 'ops'];
    $content = ['super', 'content'];
    $all = ['super', 'ops', 'content'];

    $map = [
        'list_accounts.php' => $super,
        'ban_account.php' => $super,
        'unban_account.php' => $super,
        'get_game_rates.php' => $super,
        'set_game_rates.php' => $super,
        'admin_send_mail.php' => $super,
        'admin_send_mail_all.php' => $super,
        'list_mail_admin.php' => $super,
        'list_guilds.php' => $super,
        'get_guild.php' => $super,
        'kick_guild_member.php' => $super,
        'disband_guild_admin.php' => $super,
        'transfer_owner_admin.php' => $super,
        'list_auctions_admin.php' => $super,
        'force_cancel_auction.php' => $super,
        'expire_stale.php' => $super,
        'server_status.php' => $ops,
        'project_state_summary.php' => $ops,
        'player_inspector.php' => $ops,
        'list_admin_audit.php' => $ops,
        'log_admin_audit.php' => $all,
        'reload_skills.php' => $ops,
    ];

    return $map[$script] ?? $content;
}

function currentAdmin(): array
{
    return is_array($GLOBALS['umbra_admin'] ?? null) ? $GLOBALS['umbra_admin'] : [];
}

function adminOperatorName(): string
{
    $admin = currentAdmin();
    $name = trim((string)($admin['username'] ?? ''));
    return $name !== '' ? $name : 'admin';
}

function authenticateAdminRequest(array $data): array
{
    $validation = verifyAdminFromJWT($data, $_SERVER);
    if (empty($validation['valid']) || empty($validation['is_admin'])) {
        return [
            'ok' => false,
            'http' => 401,
            'message' => $validation['error'] ?? 'Token inválido ou expirado',
        ];
    }

    $scope = (string)($validation['scope'] ?? '');
    if ($scope !== 'umbra_manager') {
        return [
            'ok' => false,
            'http' => 403,
            'message' => 'Token sem permissão de UmbraManager',
        ];
    }

    $role = strtolower(trim((string)($validation['admin_role'] ?? 'content')));
    if (!in_array($role, ['super', 'ops', 'content'], true)) {
        $role = 'content';
    }

    return [
        'ok' => true,
        'admin' => [
            'id' => (int)($validation['account_id'] ?? 0),
            'username' => (string)($validation['account']['username'] ?? ''),
            'email' => (string)($validation['account']['email'] ?? ''),
            'role' => $role,
        ],
    ];
}

function requireAdminAuth(array $data, ?array $allowedRoles = null): array
{
    $auth = authenticateAdminRequest($data);
    if (!$auth['ok']) {
        http_response_code((int)($auth['http'] ?? 401));
        echo json_encode([
            'success' => false,
            'message' => $auth['message'] ?? 'Não autenticado',
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $admin = $auth['admin'];
    $roles = $allowedRoles ?? adminRolesForScript();
    if (!in_array($admin['role'], $roles, true)) {
        http_response_code(403);
        echo json_encode([
            'success' => false,
            'message' => 'Permissão insuficiente para esta operação',
            'required_roles' => $roles,
            'role' => $admin['role'],
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $GLOBALS['umbra_admin'] = $admin;
    return $admin;
}

function requireAdminRole(array $allowedRoles): array
{
    $admin = currentAdmin();
    if ($admin === []) {
        http_response_code(401);
        echo json_encode(['success' => false, 'message' => 'Não autenticado'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    if (!in_array($admin['role'], $allowedRoles, true)) {
        http_response_code(403);
        echo json_encode([
            'success' => false,
            'message' => 'Permissão insuficiente para esta operação',
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }
    return $admin;
}

function auditAdminWrite(string $action, ?string $details = null, ?string $targetType = null, ?int $targetId = null, ?array $payload = null): void
{
    $admin = currentAdmin();
    if ($admin === []) {
        return;
    }
    try {
        $pdo = getConnection();
        if (!$pdo) {
            return;
        }
        logAdminAudit(
            $pdo,
            (string)$admin['username'],
            $action,
            $details,
            $targetType,
            $targetId,
            (int)$admin['id'],
            $payload
        );
    } catch (Throwable $e) {
        error_log('[auditAdminWrite] ' . $e->getMessage());
    }
}
