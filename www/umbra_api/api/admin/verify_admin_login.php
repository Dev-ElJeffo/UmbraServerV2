<?php
/**
 * POST /api/admin/verify_admin_login.php
 * Valida username+senha admin, emite JWT e role (super|ops|content).
 */
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(204);
    exit;
}

include_once '../../config/database.php';
include_once '../../helpers/jwt_helper.php';
include_once 'verify_admin.php';

$response = ['success' => false, 'message' => 'Método não permitido. Use POST'];

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $raw = file_get_contents('php://input');
    $data = json_decode($raw);

    if (empty($data->admin_username) || empty($data->password)) {
        $response = ['success' => false, 'message' => 'admin_username e password são obrigatórios'];
    } else {
        try {
            $database = new Database();
            $db = $database->connect();

            $query = "SELECT id, username, email, password_hash, isadmin, banned, ban_reason,
                             COALESCE(admin_role, 'super') AS admin_role
                      FROM accounts WHERE username = :username LIMIT 1";
            try {
                $stmt = $db->prepare($query);
                $stmt->bindParam(':username', $data->admin_username);
                $stmt->execute();
            } catch (Exception $e) {
                // Coluna admin_role ainda não existe
                $query = "SELECT id, username, email, password_hash, isadmin, banned, ban_reason
                          FROM accounts WHERE username = :username LIMIT 1";
                $stmt = $db->prepare($query);
                $stmt->bindParam(':username', $data->admin_username);
                $stmt->execute();
            }

            if ($stmt->rowCount() === 0) {
                http_response_code(403);
                echo json_encode(['success' => false, 'message' => 'Usuário não encontrado'], JSON_UNESCAPED_UNICODE);
                exit;
            }

            $account = $stmt->fetch(PDO::FETCH_ASSOC);
            if (!empty($account['banned'])) {
                http_response_code(403);
                echo json_encode(['success' => false, 'message' => 'Conta banida'], JSON_UNESCAPED_UNICODE);
                exit;
            }
            if ((int)$account['isadmin'] !== 1) {
                http_response_code(403);
                echo json_encode(['success' => false, 'message' => 'Acesso negado. Apenas administradores'], JSON_UNESCAPED_UNICODE);
                exit;
            }
            if (!password_verify($data->password, $account['password_hash'])) {
                http_response_code(403);
                echo json_encode(['success' => false, 'message' => 'Senha inválida'], JSON_UNESCAPED_UNICODE);
                exit;
            }

            $role = strtolower(trim($account['admin_role'] ?? 'super'));
            if (!in_array($role, ['super', 'ops', 'content'], true)) {
                $role = 'super';
            }

            $token = generateJWT([
                'account_id' => (int)$account['id'],
                'username' => $account['username'],
                'isadmin' => 1,
                'admin_role' => $role,
                'scope' => 'umbra_manager',
            ], 12);

            if (!$token) {
                http_response_code(500);
                echo json_encode(['success' => false, 'message' => 'Falha ao gerar JWT'], JSON_UNESCAPED_UNICODE);
                exit;
            }

            $response = [
                'success' => true,
                'token' => $token,
                'role' => $role,
                'admin' => [
                    'id' => (int)$account['id'],
                    'username' => $account['username'],
                    'email' => $account['email'],
                    'role' => $role,
                ],
            ];
        } catch (Exception $e) {
            http_response_code(500);
            $response = ['success' => false, 'message' => 'Erro: ' . $e->getMessage()];
        }
    }
}

echo json_encode($response, JSON_UNESCAPED_UNICODE);
