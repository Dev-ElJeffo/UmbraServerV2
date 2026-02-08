<?php
/**
 * Helper para operações de trade
 * Limpa automaticamente solicitações expiradas e sessões órfãs
 */

/**
 * Limpa trade_requests expiradas (status='pending' e expires_at passou)
 * e cancela trade_sessions ativas há mais de 10 minutos (sessões órfãs/abandonadas)
 * Deve ser chamado no início de endpoints de trade para evitar bloqueios
 *
 * @param PDO $pdo Conexão com o banco
 * @param int $sessionTimeoutMinutes Tempo em minutos para considerar sessão abandonada (padrão 1)
 * @return array ['expired_requests' => int, 'cancelled_sessions' => int]
 */
function cleanupExpiredTrades(PDO $pdo, int $sessionTimeoutMinutes = 1): array {
    $result = ['expired_requests' => 0, 'cancelled_sessions' => 0];

    try {
        // 1. Marcar solicitações pendentes como expiradas
        $stmt = $pdo->prepare("
            UPDATE trade_requests 
            SET status = 'expired' 
            WHERE status = 'pending' AND expires_at IS NOT NULL AND expires_at < NOW()
        ");
        $stmt->execute();
        $result['expired_requests'] = $stmt->rowCount();

        // 2. Cancelar sessões ativas há muito tempo (jogador não respondeu ou abandonou)
        $stmt = $pdo->prepare("
            UPDATE trade_sessions 
            SET status = 'cancelled', completed_at = NOW() 
            WHERE status = 'active' 
            AND created_at < DATE_SUB(NOW(), INTERVAL ? MINUTE)
        ");
        $stmt->execute([$sessionTimeoutMinutes]);
        $result['cancelled_sessions'] = $stmt->rowCount();

        if ($result['expired_requests'] > 0 || $result['cancelled_sessions'] > 0) {
            error_log("[trade_helper] Cleanup: expired_requests={$result['expired_requests']}, cancelled_sessions={$result['cancelled_sessions']}");
        }
    } catch (PDOException $e) {
        error_log("[trade_helper] Erro no cleanup: " . $e->getMessage());
    }

    return $result;
}
