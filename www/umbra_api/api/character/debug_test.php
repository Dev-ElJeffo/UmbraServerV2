<?php
/**
 * Script de Diagnóstico - Versão 2.0
 * Verifica conexão e estrutura do banco
 */

// Suprimir warnings
error_reporting(E_ERROR | E_PARSE);
ini_set('display_errors', '0');

header('Content-Type: application/json; charset=utf-8');

try {
    // Teste 1: Verificar caminhos possíveis
    $possible_paths = [
        __DIR__ . '/../config/database.php',
        __DIR__ . '/../../config/database.php',
        'C:/wamp64/www/umbra_api/config/database.php',
    ];
    
    $db_file = null;
    foreach ($possible_paths as $path) {
        if (file_exists($path)) {
            $db_file = $path;
            break;
        }
    }
    
    if (!$db_file) {
        echo json_encode([
            'error' => 'database.php não encontrado em nenhum caminho',
            'caminhos_testados' => $possible_paths,
            'dir_atual' => __DIR__,
            'dir_separador' => DIRECTORY_SEPARATOR
        ], JSON_PRETTY_PRINT);
        exit;
    }
    
    // Teste 2: Carregar database.php
    require_once $db_file;
    
    // Teste 3: Verificar se função existe
    if (!function_exists('getConnection')) {
        echo json_encode([
            'error' => 'Função getConnection não encontrada',
            'arquivo_carregado' => $db_file
        ], JSON_PRETTY_PRINT);
        exit;
    }
    
    // Teste 4: Conectar ao banco
    $pdo = getConnection();
    
    if (!$pdo) {
        echo json_encode([
            'error' => 'getConnection retornou null',
            'arquivo_carregado' => $db_file
        ], JSON_PRETTY_PRINT);
        exit;
    }
    
    // Teste 5: Verificar tabela players
    $stmt = $pdo->query("SHOW TABLES LIKE 'players'");
    $table_exists = $stmt->fetch() !== false;
    
    if (!$table_exists) {
        echo json_encode([
            'success' => false,
            'error' => 'Tabela players não existe',
            'database_file' => $db_file,
            'solucao' => 'Execute o script SQL de criação de tabelas (veja FIX_API_CHARACTER_JSON.md)'
        ], JSON_PRETTY_PRINT);
        exit;
    }
    
    // Teste 6: Verificar estrutura da tabela
    $stmt = $pdo->query("DESCRIBE players");
    $columns = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Teste 7: Contar personagens
    $stmt = $pdo->query("SELECT COUNT(*) as total FROM players");
    $total = $stmt->fetch(PDO::FETCH_ASSOC)['total'];
    
    // Teste 8: Verificar tabela accounts
    $stmt = $pdo->query("SHOW TABLES LIKE 'accounts'");
    $accounts_exists = $stmt->fetch() !== false;
    
    // Teste 9: Contar accounts
    $total_accounts = 0;
    if ($accounts_exists) {
        $stmt = $pdo->query("SELECT COUNT(*) as total FROM accounts");
        $total_accounts = $stmt->fetch(PDO::FETCH_ASSOC)['total'];
    }
    
    echo json_encode([
        'success' => true,
        'message' => '✅ Todos os testes passaram!',
        'database_file' => $db_file,
        'database_file_size' => filesize($db_file) . ' bytes',
        'connection' => 'OK',
        'table_players_exists' => true,
        'table_accounts_exists' => $accounts_exists,
        'total_characters' => (int)$total,
        'total_accounts' => (int)$total_accounts,
        'columns' => array_column($columns, 'Field'),
        'php_version' => phpversion(),
        'pdo_drivers' => PDO::getAvailableDrivers()
    ], JSON_PRETTY_PRINT);
    
} catch (PDOException $e) {
    echo json_encode([
        'error' => 'Erro de banco de dados',
        'message' => $e->getMessage(),
        'code' => $e->getCode(),
        'file' => $e->getFile(),
        'line' => $e->getLine()
    ], JSON_PRETTY_PRINT);
} catch (Exception $e) {
    echo json_encode([
        'error' => 'Erro geral',
        'message' => $e->getMessage(),
        'type' => get_class($e),
        'file' => $e->getFile(),
        'line' => $e->getLine(),
        'trace' => $e->getTraceAsString()
    ], JSON_PRETTY_PRINT);
}
