<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST');

include_once '../config/database.php';

// Carregar JWT helper para gerar tokens JWT reais
$jwt_helper_paths = [
    __DIR__ . '/common/jwt_helper.php',
    __DIR__ . '/../common/jwt_helper.php',
    'C:/wamp64/www/umbra_api/api/common/jwt_helper.php',
];

$jwt_loaded = false;
foreach ($jwt_helper_paths as $path) {
    if (file_exists($path)) {
        require_once $path;
        $jwt_loaded = true;
        break;
    }
}

if (!$jwt_loaded) {
    // Se não conseguir carregar JWT helper, logar erro mas continuar (fallback para token antigo)
    error_log("JWT Helper não encontrado em login.php - usando token temporário");
}

$response = array();

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents("php://input"));
    
    if (!empty($data->username) && !empty($data->password)) {
        try {
            $database = new Database();
            $db = $database->connect();
            
            // Buscar conta
            $query = "SELECT id, username, email, password_hash, banned, ban_reason, isadmin 
                     FROM accounts WHERE username = :username";
            $stmt = $db->prepare($query);
            $stmt->bindParam(':username', $data->username);
            $stmt->execute();
            
            if ($stmt->rowCount() > 0) {
                $account = $stmt->fetch(PDO::FETCH_ASSOC);
                
                // Verificar ban
                if ($account['banned']) {
                    $response['success'] = false;
                    $response['message'] = 'Conta banida: ' . $account['ban_reason'];
                }
                // Verificar senha
                else if (password_verify($data->password, $account['password_hash'])) {
                    // Atualizar last_login
                    $update = "UPDATE accounts SET last_login_at = NOW() WHERE id = :id";
                    $stmt = $db->prepare($update);
                    $stmt->bindParam(':id', $account['id']);
                    $stmt->execute();
                    
                    // Buscar personagens
                    $query = "SELECT id, character_name, level, current_zone,
                 health, max_health, mana, max_mana, 
                 stamina, max_stamina, strength, dexterity, 
                 intelligence, vitality
         FROM players WHERE account_id = :account_id";
                    $stmt = $db->prepare($query);
                    $stmt->bindParam(':account_id', $account['id']);
                    $stmt->execute();
                    $players = $stmt->fetchAll(PDO::FETCH_ASSOC);
                    foreach ($players as &$player) {
                        $player['player_id'] = (int)$player['id'];
                        unset($player['id']);
                        $player['stats'] = [
                            'health' => (int)$player['health'],
                            'max_health' => (int)$player['max_health'],
                            'mana' => (int)$player['mana'],
                            'max_mana' => (int)$player['max_mana'],
                            'stamina' => (int)$player['stamina'],
                            'max_stamina' => (int)$player['max_stamina'],
                            'strength' => (int)$player['strength'],
                            'dexterity' => (int)$player['dexterity'],
                            'intelligence' => (int)$player['intelligence'],
                            'vitality' => (int)$player['vitality']
                        ];
                        unset($player['health']);
                        unset($player['max_health']);
                        unset($player['mana']);
                        unset($player['max_mana']);
                        unset($player['stamina']);
                        unset($player['max_stamina']);
                        unset($player['strength']);
                        unset($player['dexterity']);
                        unset($player['intelligence']);
                        unset($player['vitality']);
                    }
                    
                    $response['success'] = true;
                    $response['message'] = 'Login bem-sucedido!';
                    $response['account'] = array(
                        'id' => (int)$account['id'],     // Forçar inteiro
                        'username' => $account['username'],
                        'email' => $account['email'],
                        'isadmin' => (bool)$account['isadmin'] // Forçar booleano
                    );
                    $response['players'] = $players;
                    
                    // ✅ Gerar JWT token real (compatível com AuthServer C++)
                    if ($jwt_loaded && function_exists('generateJWT')) {
                        // Obter player_id do primeiro personagem (ou 0 se não houver)
                        $firstPlayerId = !empty($players) ? (int)$players[0]['player_id'] : 0;
                        
                        // Gerar JWT com 60 minutos de expiração (padrão)
                        $jwtToken = generateJWT(
                            (int)$account['id'],
                            $firstPlayerId,
                            $account['username'],
                            60 // 60 minutos
                        );
                        
                        $response['token'] = $jwtToken;
                        error_log("JWT token gerado para usuário: " . $account['username']);
                    } else {
                        // Fallback: token temporário (compatibilidade com sistemas antigos)
                        $response['token'] = base64_encode($account['username'] . ':' . time());
                        error_log("AVISO: JWT Helper não disponível, usando token temporário");
                    }
                } else {
                    $response['success'] = false;
                    $response['message'] = 'Senha incorreta';
                }
            } else {
                $response['success'] = false;
                $response['message'] = 'Username não encontrado';
            }
        } catch (Exception $e) {
            $response['success'] = false;
            $response['message'] = 'Erro: ' . $e->getMessage();
        }
    } else {
        $response['success'] = false;
        $response['message'] = 'Username e senha são obrigatórios';
    }
} else {
    $response['success'] = false;
    $response['message'] = 'Método não permitido. Use POST';
}

echo json_encode($response, JSON_PRETTY_PRINT);
?>

