<?php
require_once __DIR__ . '/secrets.php';

$__umbraDb = umbra_db_settings();
if (!defined('DB_HOST')) define('DB_HOST', $__umbraDb['host']);
if (!defined('DB_PORT')) define('DB_PORT', $__umbraDb['port']);
if (!defined('DB_NAME')) define('DB_NAME', $__umbraDb['name']);
if (!defined('DB_USER')) define('DB_USER', $__umbraDb['user']);
if (!defined('DB_PASS')) define('DB_PASS', $__umbraDb['pass']);

/**
 * Função para obter conexão PDO
 * Retorna instância PDO configurada
 */
function getConnection() {
    try {
        $dsn = "mysql:host=" . DB_HOST . ";port=" . DB_PORT . ";dbname=" . DB_NAME . ";charset=utf8mb4";
        $pdo = new PDO($dsn, DB_USER, DB_PASS);
        $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        $pdo->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);
        $pdo->setAttribute(PDO::ATTR_EMULATE_PREPARES, false);
        return $pdo;
    } catch (PDOException $e) {
        // Em produção, não mostre detalhes do erro
        error_log("Database Connection Error: " . $e->getMessage());
        return null;
    }
}

// Classe Database (mantida para compatibilidade)
class Database {
    private $conn;
    
    public function connect() {
        $this->conn = null;
        
        try {
            $this->conn = new PDO(
                "mysql:host=" . DB_HOST . 
                ";port=" . DB_PORT . 
                ";dbname=" . DB_NAME,
                DB_USER,
                DB_PASS
            );
            $this->conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            $this->conn->exec("set names utf8mb4");
        } catch(PDOException $e) {
            error_log('Database Connection Error: ' . $e->getMessage());
        }
        
        return $this->conn;
    }
}
?>

