<?php
/**
 * Autenticação admin compartilhada (UmbraManager admin_username ou JWT).
 * Uso: $data = json_decode(...); require_once 'require_admin_auth.php'; requireAdminAuth($data);
 */
require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';
require_once __DIR__ . '/verify_admin.php';

function requireAdminAuth(array $data): void
{
    $authOk = false;
    if (!empty($data['admin_username'])) {
        try {
            $database = new Database();
            $authDb = $database->connect();
            $adminCheck = verifyAdmin($authDb, $data['admin_username']);
            if (!empty($adminCheck['success'])) {
                $authOk = true;
            } else {
                http_response_code(403);
                echo json_encode($adminCheck, JSON_UNESCAPED_UNICODE);
                exit;
            }
        } catch (Exception $e) {
            http_response_code(500);
            echo json_encode(['success' => false, 'message' => 'Erro de autenticação: ' . $e->getMessage()], JSON_UNESCAPED_UNICODE);
            exit;
        }
    }
    if (!$authOk) {
        $validation = validateJWTRequest($data, $_SERVER);
        if (!$validation['valid']) {
            http_response_code(401);
            echo json_encode([
                'success' => false,
                'message' => $validation['error'] ?? 'Token inválido ou expirado (forneça admin_username ou token)',
            ], JSON_UNESCAPED_UNICODE);
            exit;
        }
    }
}
