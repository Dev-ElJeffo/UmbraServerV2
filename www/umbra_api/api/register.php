<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type');

include_once '../config/database.php';

$response = array();

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Receber dados JSON
    $data = json_decode(file_get_contents("php://input"));
    
    if (!empty($data->username) && !empty($data->email) && !empty($data->password)) {
        try {
            $database = new Database();
            $db = $database->connect();
            
            // Verificar se username já existe
            $query = "SELECT id FROM accounts WHERE username = :username";
            $stmt = $db->prepare($query);
            $stmt->bindParam(':username', $data->username);
            $stmt->execute();
            
            if ($stmt->rowCount() > 0) {
                $response['success'] = false;
                $response['message'] = 'Username já existe';
            } else {
                // Hash da senha
                $password_hash = password_hash($data->password, PASSWORD_BCRYPT);
                $salt = bin2hex(random_bytes(16));
                
                // Inserir conta
                $query = "INSERT INTO accounts (username, email, password_hash, salt) 
                         VALUES (:username, :email, :password_hash, :salt)";
                $stmt = $db->prepare($query);
                $stmt->bindParam(':username', $data->username);
                $stmt->bindParam(':email', $data->email);
                $stmt->bindParam(':password_hash', $password_hash);
                $stmt->bindParam(':salt', $salt);
                
                if ($stmt->execute()) {
                    $account_id = $db->lastInsertId();
                    
                    $response['success'] = true;
                    $response['message'] = 'Conta criada com sucesso!';
                    $response['account_id'] = $account_id;
                    $response['username'] = $data->username;
                } else {
                    $response['success'] = false;
                    $response['message'] = 'Erro ao criar conta';
                }
            }
        } catch (Exception $e) {
            $response['success'] = false;
            $response['message'] = 'Erro: ' . $e->getMessage();
        }
    } else {
        $response['success'] = false;
        $response['message'] = 'Dados incompletos (username, email, password)';
    }
} else {
    $response['success'] = false;
    $response['message'] = 'Método não permitido. Use POST';
}

echo json_encode($response, JSON_PRETTY_PRINT);
?>

