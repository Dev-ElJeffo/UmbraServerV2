<?php
/**
 * Audit admin central (MySQL admin_audit).
 */

require_once __DIR__ . '/../config/database.php';

function adminAuditEnsureTable(PDO $pdo): void
{
    $pdo->exec("
        CREATE TABLE IF NOT EXISTS admin_audit (
          id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
          created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
          operator_account_id BIGINT UNSIGNED NULL,
          operator_name VARCHAR(64) NOT NULL DEFAULT '',
          action VARCHAR(64) NOT NULL,
          target_type VARCHAR(32) NULL,
          target_id BIGINT NULL,
          details TEXT NULL,
          ip_address VARCHAR(45) NULL,
          payload_json JSON NULL,
          PRIMARY KEY (id),
          INDEX idx_admin_audit_created (created_at),
          INDEX idx_admin_audit_action (action),
          INDEX idx_admin_audit_operator (operator_name),
          INDEX idx_admin_audit_target (target_type, target_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
    ");
}

/**
 * @param array<string,mixed>|null $payload
 */
function logAdminAudit(
    PDO $pdo,
    string $operatorName,
    string $action,
    ?string $details = null,
    ?string $targetType = null,
    ?int $targetId = null,
    ?int $operatorAccountId = null,
    ?array $payload = null
): void {
    try {
        adminAuditEnsureTable($pdo);
        $ip = $_SERVER['REMOTE_ADDR'] ?? null;
        $payloadJson = $payload !== null ? json_encode($payload, JSON_UNESCAPED_UNICODE) : null;
        $st = $pdo->prepare('
            INSERT INTO admin_audit
              (operator_account_id, operator_name, action, target_type, target_id, details, ip_address, payload_json)
            VALUES
              (:oid, :oname, :action, :ttype, :tid, :details, :ip, :payload)
        ');
        $st->execute([
            ':oid' => $operatorAccountId,
            ':oname' => mb_substr($operatorName, 0, 64),
            ':action' => mb_substr($action, 0, 64),
            ':ttype' => $targetType !== null ? mb_substr($targetType, 0, 32) : null,
            ':tid' => $targetId,
            ':details' => $details,
            ':ip' => $ip !== null ? mb_substr($ip, 0, 45) : null,
            ':payload' => $payloadJson,
        ]);
    } catch (Throwable $e) {
        error_log('[logAdminAudit] ' . $e->getMessage());
    }
}
