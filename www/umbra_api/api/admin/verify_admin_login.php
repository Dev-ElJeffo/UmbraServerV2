<?php
/**
 * POST /api/admin/verify_admin_login.php
 * Valida se admin_username existe, não está banido e isadmin=1.
 */
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST');

include_once '../../config/database.php';
include_once 'verify_admin.php';

$response = ['success' => false, 'message' => 'Método não permitido. Use POST'];

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents('php://input'));

    if (empty($data->admin_username)) {
        $response = ['success' => false, 'message' => 'admin_username é obrigatório'];
    } else {
        try {
            $auth = authenticateAdminObjectRequest($data, $_SERVER);
            if (empty($auth['success'])) {
                http_response_code($auth['http_code'] ?? 403);
                echo json_encode($auth);
                exit();
            }

            $response = [
                'success' => true,
                'admin' => $auth['admin'],
            ];
        } catch (Exception $e) {
            $response = ['success' => false, 'message' => 'Erro: ' . $e->getMessage()];
        }
    }
}

echo json_encode($response);
