<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');

include_once '../config/database.php';

$response = array();

try {
    $database = new Database();
    $db = $database->connect();
    
    if ($db) {
        // Testar query
        $query = "SELECT VERSION() as version, DATABASE() as db_name";
        $stmt = $db->prepare($query);
        $stmt->execute();
        $result = $stmt->fetch(PDO::FETCH_ASSOC);
        
        // Contar registros
        $query = "SELECT 
            (SELECT COUNT(*) FROM accounts) as total_accounts,
            (SELECT COUNT(*) FROM players) as total_players,
            (SELECT version FROM schema_version LIMIT 1) as schema_version";
        $stmt = $db->prepare($query);
        $stmt->execute();
        $counts = $stmt->fetch(PDO::FETCH_ASSOC);
        
        $response['success'] = true;
        $response['message'] = 'Conexão bem-sucedida!';
        $response['mysql_version'] = $result['version'];
        $response['database'] = $result['db_name'];
        $response['schema_version'] = $counts['schema_version'];
        $response['stats'] = array(
            'accounts' => (int)$counts['total_accounts'],
            'players' => (int)$counts['total_players']
        );
    }
} catch (Exception $e) {
    $response['success'] = false;
    $response['message'] = 'Erro: ' . $e->getMessage();
}

echo json_encode($response, JSON_PRETTY_PRINT);
?>

