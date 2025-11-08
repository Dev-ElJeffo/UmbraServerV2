<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST');

include_once '../../config/database.php';
include_once 'verify_admin.php';

$response = array();

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents("php://input"));
    
    if (!empty($data->admin_username)) {
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
            
            // Buscar todas as contas
            $query = "SELECT 
                id, 
                username, 
                email, 
                banned, 
                ban_reason,
                isadmin,
                created_at, 
                last_login_at,
                (SELECT COUNT(*) FROM players WHERE account_id = accounts.id) as player_count
                FROM accounts 
                ORDER BY created_at DESC";
            
            $stmt = $db->prepare($query);
            $stmt->execute();
            $accounts = $stmt->fetchAll(PDO::FETCH_ASSOC);
            
            // Estatísticas
            $stats = [
                'total' => count($accounts),
                'admins' => 0,
                'banned' => 0,
                'active' => 0
            ];
            
            foreach ($accounts as $account) {
                if ($account['isadmin']) $stats['admins']++;
                if ($account['banned']) $stats['banned']++;
                if (!$account['banned']) $stats['active']++;
            }
            
            $response['success'] = true;
            $response['accounts'] = $accounts;
            $response['stats'] = $stats;
            $response['admin'] = $adminCheck['admin'];
            
        } catch (Exception $e) {
            $response['success'] = false;
            $response['message'] = 'Erro: ' . $e->getMessage();
        }
    } else {
        $response['success'] = false;
        $response['message'] = 'admin_username é obrigatório';
    }
} else {
    $response['success'] = false;
    $response['message'] = 'Método não permitido. Use POST';
}

echo json_encode($response, JSON_PRETTY_PRINT);
?>

