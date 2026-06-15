<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST');

include_once '../../config/database.php';
include_once 'verify_admin.php';

$response = array();

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents("php://input"));
    
    if (!empty($data->admin_username) && !empty($data->target_user_id)) {
        try {
            $auth = authenticateAdminObjectRequest($data, $_SERVER);
            if (empty($auth['success'])) {
                http_response_code($auth['http_code'] ?? 403);
                echo json_encode($auth);
                exit();
            }
            $adminCheck = $auth;

            $database = new Database();
            $db = $database->connect();
            // Verificar se o alvo existe
            $query = "SELECT id, username, banned FROM accounts WHERE id = :target_id";
            $stmt = $db->prepare($query);
            $stmt->bindParam(':target_id', $data->target_user_id);
            $stmt->execute();
            
            if ($stmt->rowCount() == 0) {
                $response['success'] = false;
                $response['message'] = 'Conta alvo não encontrada';
                echo json_encode($response);
                exit();
            }
            
            $target = $stmt->fetch(PDO::FETCH_ASSOC);
            
            // Verificar se está banido
            if ($target['banned'] == 0) {
                $response['success'] = false;
                $response['message'] = 'Esta conta não está banida';
                echo json_encode($response);
                exit();
            }
            
            // Desbanir conta
            $query = "UPDATE accounts 
                     SET banned = 0, ban_reason = NULL 
                     WHERE id = :target_id";
            $stmt = $db->prepare($query);
            $stmt->bindParam(':target_id', $data->target_user_id);
            
            if ($stmt->execute()) {
                $response['success'] = true;
                $response['message'] = "Conta '{$target['username']}' foi desbanida com sucesso";
                $response['unbanned_user'] = [
                    'id' => $target['id'],
                    'username' => $target['username']
                ];
            } else {
                $response['success'] = false;
                $response['message'] = 'Erro ao desbanir conta';
            }
            
        } catch (Exception $e) {
            $response['success'] = false;
            $response['message'] = 'Erro: ' . $e->getMessage();
        }
    } else {
        $response['success'] = false;
        $response['message'] = 'admin_username e target_user_id são obrigatórios';
    }
} else {
    $response['success'] = false;
    $response['message'] = 'Método não permitido. Use POST';
}

echo json_encode($response, JSON_PRETTY_PRINT);
?>

