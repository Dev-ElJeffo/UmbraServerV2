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
            $database = new Database();
            $db = $database->connect();
            
            // Verificar se é admin
            $adminCheck = verifyAdmin($db, $data->admin_username);
            
            if (!$adminCheck['success']) {
                http_response_code(403);
                echo json_encode($adminCheck);
                exit();
            }
            
            // Verificar se o alvo existe
            $query = "SELECT id, username, isadmin FROM accounts WHERE id = :target_id";
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
            
            // Não pode banir outro admin
            if ($target['isadmin'] == 1) {
                $response['success'] = false;
                $response['message'] = 'Não é possível banir um administrador';
                echo json_encode($response);
                exit();
            }
            
            // Não pode banir a si mesmo
            if ($target['id'] == $adminCheck['admin']['id']) {
                $response['success'] = false;
                $response['message'] = 'Você não pode banir sua própria conta';
                echo json_encode($response);
                exit();
            }
            
            $ban_reason = !empty($data->reason) ? $data->reason : 'Banido por administrador';
            
            // Banir conta
            $query = "UPDATE accounts 
                     SET banned = 1, ban_reason = :reason 
                     WHERE id = :target_id";
            $stmt = $db->prepare($query);
            $stmt->bindParam(':reason', $ban_reason);
            $stmt->bindParam(':target_id', $data->target_user_id);
            
            if ($stmt->execute()) {
                require_once __DIR__ . '/../../helpers/admin_audit_helper.php';
                logAdminAudit(
                    $db,
                    (string)$data->admin_username,
                    'ban_account',
                    "user={$target['username']};reason={$ban_reason}",
                    'player',
                    (int)$target['id'],
                    isset($adminCheck['admin']['id']) ? (int)$adminCheck['admin']['id'] : null,
                    ['reason' => $ban_reason]
                );
                $response['success'] = true;
                $response['message'] = "Conta '{$target['username']}' foi banida com sucesso";
                $response['banned_user'] = [
                    'id' => $target['id'],
                    'username' => $target['username']
                ];
                $response['reason'] = $ban_reason;
            } else {
                $response['success'] = false;
                $response['message'] = 'Erro ao banir conta';
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

