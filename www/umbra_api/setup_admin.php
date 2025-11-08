<?php
/**
 * Script de Setup para Sistema Admin
 * Execute uma vez: http://localhost/umbra_api/setup_admin.php
 */

header('Content-Type: text/html; charset=utf-8');

echo "<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <title>Setup Admin - UmbraEternum</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            max-width: 800px;
            margin: 50px auto;
            padding: 20px;
            background: #f5f5f5;
        }
        .container {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }
        h1 { color: #667eea; }
        .success { color: #4caf50; padding: 10px; background: #d4edda; border-radius: 5px; margin: 10px 0; }
        .error { color: #f44336; padding: 10px; background: #f8d7da; border-radius: 5px; margin: 10px 0; }
        .info { color: #2196f3; padding: 10px; background: #d1ecf1; border-radius: 5px; margin: 10px 0; }
        pre { background: #2d2d2d; color: #f8f8f2; padding: 15px; border-radius: 5px; overflow-x: auto; }
        .btn {
            display: inline-block;
            padding: 10px 20px;
            background: #667eea;
            color: white;
            text-decoration: none;
            border-radius: 5px;
            margin: 10px 5px;
        }
        .btn:hover { background: #5568d3; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>🔧 Setup Sistema Admin</h1>";

include_once 'config/database.php';

try {
    $database = new Database();
    $db = $database->connect();
    
    echo "<div class='success'>✅ Conexão com banco de dados OK!</div>";
    
    // Verificar se coluna isadmin existe
    echo "<h2>Verificando estrutura do banco...</h2>";
    
    $query = "SHOW COLUMNS FROM accounts LIKE 'isadmin'";
    $stmt = $db->query($query);
    $columnExists = $stmt->rowCount() > 0;
    
    if (!$columnExists) {
        echo "<div class='error'>❌ Coluna 'isadmin' NÃO existe!</div>";
        echo "<div class='info'>Tentando criar coluna...</div>";
        
        try {
            // Adicionar coluna
            $db->exec("ALTER TABLE accounts ADD COLUMN isadmin TINYINT(1) NOT NULL DEFAULT 0 AFTER banned");
            echo "<div class='success'>✅ Coluna 'isadmin' criada!</div>";
            
            // Criar índice
            $db->exec("CREATE INDEX idx_isadmin ON accounts(isadmin)");
            echo "<div class='success'>✅ Índice criado!</div>";
            
            // Tornar primeira conta admin
            $db->exec("UPDATE accounts SET isadmin = 1 WHERE id = 1");
            echo "<div class='success'>✅ Primeira conta agora é admin!</div>";
            
        } catch (Exception $e) {
            echo "<div class='error'>❌ Erro ao criar coluna: " . $e->getMessage() . "</div>";
        }
    } else {
        echo "<div class='success'>✅ Coluna 'isadmin' já existe!</div>";
    }
    
    // Verificar estrutura atual
    echo "<h2>Estrutura Atual da Tabela 'accounts':</h2>";
    $query = "DESCRIBE accounts";
    $stmt = $db->query($query);
    $columns = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    echo "<pre>";
    foreach ($columns as $col) {
        echo str_pad($col['Field'], 20) . " | " . 
             str_pad($col['Type'], 15) . " | " . 
             str_pad($col['Null'], 5) . " | " . 
             $col['Default'] . "\n";
    }
    echo "</pre>";
    
    // Verificar contas admin
    echo "<h2>Contas de Administrador:</h2>";
    $query = "SELECT id, username, email, isadmin FROM accounts WHERE isadmin = 1";
    $stmt = $db->query($query);
    $admins = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    if (count($admins) > 0) {
        echo "<pre>";
        foreach ($admins as $admin) {
            echo "ID: {$admin['id']} | Username: {$admin['username']} | Email: {$admin['email']}\n";
        }
        echo "</pre>";
        echo "<div class='success'>✅ Total de admins: " . count($admins) . "</div>";
    } else {
        echo "<div class='error'>❌ Nenhum admin encontrado!</div>";
        echo "<div class='info'>Tornando primeira conta admin...</div>";
        
        try {
            $db->exec("UPDATE accounts SET isadmin = 1 WHERE id = 1");
            echo "<div class='success'>✅ Primeira conta agora é admin!</div>";
        } catch (Exception $e) {
            echo "<div class='error'>❌ Erro: " . $e->getMessage() . "</div>";
        }
    }
    
    // Estatísticas
    echo "<h2>Estatísticas:</h2>";
    $query = "SELECT 
        COUNT(*) as total,
        SUM(CASE WHEN isadmin = 1 THEN 1 ELSE 0 END) as admins,
        SUM(CASE WHEN isadmin = 0 THEN 1 ELSE 0 END) as users,
        SUM(CASE WHEN banned = 1 THEN 1 ELSE 0 END) as banned
        FROM accounts";
    $stmt = $db->query($query);
    $stats = $stmt->fetch(PDO::FETCH_ASSOC);
    
    echo "<pre>";
    echo "Total de contas: {$stats['total']}\n";
    echo "Administradores: {$stats['admins']}\n";
    echo "Usuários:        {$stats['users']}\n";
    echo "Banidos:         {$stats['banned']}\n";
    echo "</pre>";
    
    echo "<div class='success'>
        <strong>✅ Setup Completo!</strong><br>
        O sistema admin está configurado corretamente.
    </div>";
    
    echo "<a href='admin.html' class='btn'>👑 Ir para Painel Admin</a>";
    echo "<a href='dashboard.html' class='btn'>📊 Dashboard</a>";
    echo "<a href='index.php' class='btn'>🏠 Home</a>";
    
} catch (Exception $e) {
    echo "<div class='error'>❌ Erro de conexão: " . $e->getMessage() . "</div>";
}

echo "
    </div>
</body>
</html>";
?>

