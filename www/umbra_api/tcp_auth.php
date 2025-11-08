<?php
set_time_limit(0);
error_reporting(E_ALL);

$host = '127.0.0.1';
$port = 9000;

$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
socket_bind($socket, $host, $port);
socket_listen($socket);

echo "🚀 Umbra Gateway TCP Server rodando em $host:$port\n";

while (true) {
    $client = socket_accept($socket);
    if ($client === false) continue;

    $input = socket_read($client, 2048);
    if (!$input) continue;

    // Parse mensagem UMBRA|TIMESTAMP|MESSAGE
    $parts = explode('|', trim($input), 3);
    if (count($parts) < 3) continue;

    $type = $parts[2];
    
    if (strpos($type, 'VALIDATE|') === 0) {
        $token = substr($type, 9);
        $isValid = validateToken($token);
        
        $response = $isValid ? "VALIDATED|TRUE" : "VALIDATED|FALSE";
        socket_write($client, $response . "\n");
    }
    
    socket_close($client);
}

function validateToken($token) {
    include '../config/core.php';
    
    try {
        $decoded = JWT::decode($token, $jwt_secret, array('HS256'));
        return isset($decoded->exp) && $decoded->exp > time();
    } catch (Exception $e) {
        return false;
    }
}

socket_close($socket);
?>