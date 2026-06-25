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
            $database = new Database();
            $db = $database->connect();
            $adminCheck = verifyAdmin($db, $data->admin_username);

            if (!$adminCheck['success']) {
                http_response_code(403);
                echo json_encode($adminCheck);
                exit();
            }

            $response = [
                'success' => true,
                'admin' => $adminCheck['admin'],
            ];
        } catch (Exception $e) {
            $response = ['success' => false, 'message' => 'Erro: ' . $e->getMessage()];
        }
    }
}

echo json_encode($response);
