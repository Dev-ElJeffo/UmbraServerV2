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
            
            // Informações do MySQL
            $query = "SELECT VERSION() as mysql_version";
            $stmt = $db->query($query);
            $mysql_info = $stmt->fetch(PDO::FETCH_ASSOC);
            
            // Estatísticas do banco
            $query = "SELECT 
                (SELECT COUNT(*) FROM accounts) as total_accounts,
                (SELECT COUNT(*) FROM accounts WHERE isadmin = 1) as admin_accounts,
                (SELECT COUNT(*) FROM accounts WHERE banned = 1) as banned_accounts,
                (SELECT COUNT(*) FROM players) as total_players,
                (SELECT version FROM schema_version LIMIT 1) as schema_version";
            $stmt = $db->query($query);
            $db_stats = $stmt->fetch(PDO::FETCH_ASSOC);
            
            // Última atividade
            $query = "SELECT 
                MAX(created_at) as last_registration,
                MAX(last_login_at) as last_login
                FROM accounts";
            $stmt = $db->query($query);
            $activity = $stmt->fetch(PDO::FETCH_ASSOC);
            
            // Verificar se servidor C++ está rodando
            $cpp_server_status = false;
            $cpp_ports = [8080, 8081, 9000];
            $cpp_services = [];
            
            foreach ($cpp_ports as $port) {
                $connection = @fsockopen('localhost', $port, $errno, $errstr, 1);
                $status = ($connection !== false);
                if ($connection) {
                    fclose($connection);
                }
                
                // Compatível com PHP 7.4+
                if ($port == 8080) {
                    $service_name = 'Auth Server';
                } elseif ($port == 8081) {
                    $service_name = 'World Server';
                } elseif ($port == 9000) {
                    $service_name = 'Gateway';
                } else {
                    $service_name = 'Unknown';
                }
                
                $cpp_services[] = [
                    'name' => $service_name,
                    'port' => $port,
                    'status' => $status ? 'online' : 'offline'
                ];
                
                if ($status) $cpp_server_status = true;
            }
            
            // PHP Info
            $php_info = [
                'version' => phpversion(),
                'memory_limit' => ini_get('memory_limit'),
                'max_execution_time' => ini_get('max_execution_time'),
                'upload_max_filesize' => ini_get('upload_max_filesize')
            ];
            
            // Server time
            $server_time = date('Y-m-d H:i:s');
            $uptime = sys_getloadavg(); // Linux only, retorna false no Windows
            
            $response['success'] = true;
            $response['server'] = [
                'status' => 'online',
                'time' => $server_time,
                'php' => $php_info,
                'mysql' => [
                    'version' => $mysql_info['mysql_version'],
                    'status' => 'online'
                ],
                'cpp_server' => [
                    'status' => $cpp_server_status ? 'online' : 'offline',
                    'services' => $cpp_services
                ]
            ];
            $response['database'] = [
                'stats' => $db_stats,
                'activity' => $activity
            ];
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

