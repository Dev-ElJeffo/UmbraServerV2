<?php
/**
 * DIAGNÓSTICO COMPLETO DO SISTEMA DE INVENTÁRIO
 * Este script verifica TUDO para encontrar o problema
 */

error_reporting(E_ALL);
ini_set('display_errors', 1);

echo "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Diagnóstico Completo</title>";
echo "<style>
    body { font-family: Arial, sans-serif; padding: 20px; background: #f5f5f5; }
    .section { background: white; padding: 20px; margin: 10px 0; border-radius: 5px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
    .success { color: green; font-weight: bold; }
    .error { color: red; font-weight: bold; }
    .warning { color: orange; font-weight: bold; }
    pre { background: #f0f0f0; padding: 10px; border-radius: 3px; overflow-x: auto; }
    h2 { color: #333; border-bottom: 2px solid #4CAF50; padding-bottom: 5px; }
</style></head><body>";

echo "<h1>🔍 Diagnóstico Completo do Sistema de Inventário</h1>";

// ============================================
// 1. VERIFICAR ARQUIVOS ESSENCIAIS
// ============================================
echo "<div class='section'>";
echo "<h2>1️⃣ Verificação de Arquivos Essenciais</h2>";

$arquivos_essenciais = [
    'config/database.php' => __DIR__ . '/config/database.php',
    'helpers/jwt_helper.php' => __DIR__ . '/helpers/jwt_helper.php',
    'vendor/autoload.php' => __DIR__ . '/vendor/autoload.php',
    'api/inventory/get_inventory.php' => __DIR__ . '/api/inventory/get_inventory.php',
    'api/inventory/add_item.php' => __DIR__ . '/api/inventory/add_item.php',
    'api/inventory/get_item_templates.php' => __DIR__ . '/api/inventory/get_item_templates.php',
];

foreach ($arquivos_essenciais as $nome => $caminho) {
    if (file_exists($caminho)) {
        echo "✅ <span class='success'>$nome</span> - Existe<br>";
    } else {
        echo "❌ <span class='error'>$nome</span> - <strong>NÃO ENCONTRADO!</strong><br>";
        echo "   Caminho: <code>$caminho</code><br>";
    }
}
echo "</div>";

// ============================================
// 2. VERIFICAR COMPOSER E BIBLIOTECAS
// ============================================
echo "<div class='section'>";
echo "<h2>2️⃣ Verificação do Composer e Bibliotecas</h2>";

if (file_exists(__DIR__ . '/vendor/autoload.php')) {
    echo "✅ <span class='success'>vendor/autoload.php existe</span><br>";
    
    require_once __DIR__ . '/vendor/autoload.php';
    
    // Verificar se a biblioteca JWT está instalada
    if (class_exists('Firebase\JWT\JWT')) {
        echo "✅ <span class='success'>Biblioteca Firebase JWT instalada</span><br>";
        echo "   Classe: <code>Firebase\\JWT\\JWT</code><br>";
    } else {
        echo "❌ <span class='error'>Biblioteca Firebase JWT NÃO ENCONTRADA!</span><br>";
        echo "   <strong>SOLUÇÃO:</strong> Execute no diretório umbra_api:<br>";
        echo "   <pre>composer require firebase/php-jwt</pre>";
    }
} else {
    echo "❌ <span class='error'>vendor/autoload.php NÃO ENCONTRADO!</span><br>";
    echo "   <strong>SOLUÇÃO:</strong> Execute no diretório umbra_api:<br>";
    echo "   <pre>composer install</pre>";
    echo "   Ou se não tiver composer.json:<br>";
    echo "   <pre>composer init\ncomposer require firebase/php-jwt</pre>";
}
echo "</div>";

// ============================================
// 3. TESTAR CONEXÃO COM BANCO DE DADOS
// ============================================
echo "<div class='section'>";
echo "<h2>3️⃣ Teste de Conexão com Banco de Dados</h2>";

if (file_exists(__DIR__ . '/config/database.php')) {
    try {
        require_once __DIR__ . '/config/database.php';
        
        if (isset($conn) && $conn instanceof mysqli) {
            if ($conn->connect_error) {
                echo "❌ <span class='error'>Erro de conexão: " . $conn->connect_error . "</span><br>";
            } else {
                echo "✅ <span class='success'>Conectado ao MySQL</span><br>";
                echo "   Host: " . $conn->host_info . "<br>";
                
                // Testar query
                $result = $conn->query("SELECT COUNT(*) as total FROM item_templates");
                if ($result) {
                    $row = $result->fetch_assoc();
                    echo "✅ <span class='success'>Tabela item_templates: " . $row['total'] . " itens</span><br>";
                } else {
                    echo "❌ <span class='error'>Erro ao consultar item_templates: " . $conn->error . "</span><br>";
                }
                
                $result2 = $conn->query("SELECT COUNT(*) as total FROM player_inventory");
                if ($result2) {
                    $row2 = $result2->fetch_assoc();
                    echo "✅ <span class='success'>Tabela player_inventory: " . $row2['total'] . " itens</span><br>";
                } else {
                    echo "❌ <span class='error'>Erro ao consultar player_inventory: " . $conn->error . "</span><br>";
                }
            }
        } else {
            echo "❌ <span class='error'>Variável \$conn não está definida ou não é mysqli</span><br>";
        }
    } catch (Exception $e) {
        echo "❌ <span class='error'>Exceção: " . $e->getMessage() . "</span><br>";
    }
} else {
    echo "❌ <span class='error'>config/database.php não encontrado</span><br>";
}
echo "</div>";

// ============================================
// 4. TESTAR JWT HELPER
// ============================================
echo "<div class='section'>";
echo "<h2>4️⃣ Teste do JWT Helper</h2>";

if (file_exists(__DIR__ . '/helpers/jwt_helper.php')) {
    echo "✅ <span class='success'>jwt_helper.php existe</span><br>";
    
    try {
        require_once __DIR__ . '/helpers/jwt_helper.php';
        echo "✅ <span class='success'>jwt_helper.php carregado sem erros</span><br>";
        
        // Verificar se as funções existem
        $funcoes = ['getJWTFromHeader', 'validateJWT', 'validateJWTRequest', 'generateJWT'];
        foreach ($funcoes as $func) {
            if (function_exists($func)) {
                echo "✅ <span class='success'>Função $func() existe</span><br>";
            } else {
                echo "❌ <span class='error'>Função $func() NÃO ENCONTRADA</span><br>";
            }
        }
        
        // Testar geração de token
        if (function_exists('generateJWT')) {
            $test_token = generateJWT(['test' => 'value', 'player_id' => 999], 1);
            if ($test_token) {
                echo "✅ <span class='success'>Token de teste gerado com sucesso</span><br>";
                echo "   Token: <code>" . substr($test_token, 0, 50) . "...</code><br>";
                
                // Testar validação
                if (function_exists('validateJWT')) {
                    $decoded = validateJWT($test_token);
                    if ($decoded && isset($decoded['player_id']) && $decoded['player_id'] == 999) {
                        echo "✅ <span class='success'>Token validado com sucesso</span><br>";
                        echo "   Dados: <pre>" . print_r($decoded, true) . "</pre>";
                    } else {
                        echo "❌ <span class='error'>Falha ao validar token de teste</span><br>";
                    }
                }
            } else {
                echo "❌ <span class='error'>Falha ao gerar token de teste</span><br>";
            }
        }
        
    } catch (Exception $e) {
        echo "❌ <span class='error'>Erro ao carregar jwt_helper.php: " . $e->getMessage() . "</span><br>";
        echo "<pre>" . $e->getTraceAsString() . "</pre>";
    }
} else {
    echo "❌ <span class='error'>jwt_helper.php não encontrado</span><br>";
}
echo "</div>";

// ============================================
// 5. TESTAR ENDPOINT GET_ITEM_TEMPLATES (SEM AUTH)
// ============================================
echo "<div class='section'>";
echo "<h2>5️⃣ Teste do Endpoint get_item_templates.php (Sem Autenticação)</h2>";

if (file_exists(__DIR__ . '/api/inventory/get_item_templates.php')) {
    echo "✅ <span class='success'>get_item_templates.php existe</span><br>";
    
    try {
        // Capturar output
        ob_start();
        include __DIR__ . '/api/inventory/get_item_templates.php';
        $output = ob_get_clean();
        
        echo "<strong>Output do endpoint:</strong><br>";
        echo "<pre>" . htmlspecialchars($output) . "</pre>";
        
        // Tentar decodificar JSON
        $data = json_decode($output, true);
        if ($data) {
            if (isset($data['success']) && $data['success']) {
                echo "✅ <span class='success'>Endpoint retornou JSON válido</span><br>";
                echo "   Total de templates: " . ($data['total'] ?? 0) . "<br>";
            } else {
                echo "⚠️ <span class='warning'>Endpoint retornou JSON, mas com erro</span><br>";
                echo "   Mensagem: " . ($data['message'] ?? 'Desconhecida') . "<br>";
            }
        } else {
            echo "❌ <span class='error'>Endpoint NÃO retornou JSON válido</span><br>";
            echo "   Erro JSON: " . json_last_error_msg() . "<br>";
        }
        
    } catch (Exception $e) {
        echo "❌ <span class='error'>Exceção ao executar endpoint: " . $e->getMessage() . "</span><br>";
    }
} else {
    echo "❌ <span class='error'>get_item_templates.php não encontrado</span><br>";
}
echo "</div>";

// ============================================
// 6. TESTAR ENDPOINT GET_INVENTORY (COM AUTH)
// ============================================
echo "<div class='section'>";
echo "<h2>6️⃣ Teste do Endpoint get_inventory.php (Com Autenticação)</h2>";

if (file_exists(__DIR__ . '/api/inventory/get_inventory.php')) {
    echo "✅ <span class='success'>get_inventory.php existe</span><br>";
    
    // Simular header Authorization
    $test_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhY2NvdW50X2lkIjo0LCJwbGF5ZXJfaWQiOjEsInVzZXJuYW1lIjoiamVmZm8iLCJpYXQiOjE3NjMxNDMyMzEsImV4cCI6MTc2MzE0NjgzMX0.2zYxTjRWLckbX4krPJT73M6kT86DPcJ7f-NfoTHj5LI";
    
    $_SERVER['HTTP_AUTHORIZATION'] = "Bearer $test_token";
    
    echo "   Token de teste: <code>" . substr($test_token, 0, 40) . "...</code><br>";
    
    try {
        // Capturar output
        ob_start();
        include __DIR__ . '/api/inventory/get_inventory.php';
        $output = ob_get_clean();
        
        echo "<strong>Output do endpoint:</strong><br>";
        echo "<pre>" . htmlspecialchars($output) . "</pre>";
        
        // Tentar decodificar JSON
        $data = json_decode($output, true);
        if ($data) {
            if (isset($data['success']) && $data['success']) {
                echo "✅ <span class='success'>Endpoint retornou JSON válido</span><br>";
                echo "   Player ID: " . ($data['player']['player_id'] ?? 'N/A') . "<br>";
                echo "   Total de itens: " . ($data['total_items'] ?? 0) . "<br>";
            } else {
                echo "⚠️ <span class='warning'>Endpoint retornou JSON, mas com erro</span><br>";
                echo "   Mensagem: " . ($data['message'] ?? 'Desconhecida') . "<br>";
            }
        } else {
            echo "❌ <span class='error'>Endpoint NÃO retornou JSON válido</span><br>";
            echo "   Erro JSON: " . json_last_error_msg() . "<br>";
        }
        
    } catch (Exception $e) {
        echo "❌ <span class='error'>Exceção ao executar endpoint: " . $e->getMessage() . "</span><br>";
        echo "<pre>" . $e->getTraceAsString() . "</pre>";
    }
} else {
    echo "❌ <span class='error'>get_inventory.php não encontrado</span><br>";
}
echo "</div>";

// ============================================
// 7. INFORMAÇÕES DO SISTEMA
// ============================================
echo "<div class='section'>";
echo "<h2>7️⃣ Informações do Sistema</h2>";
echo "PHP Version: <strong>" . phpversion() . "</strong><br>";
echo "Server Software: <strong>" . ($_SERVER['SERVER_SOFTWARE'] ?? 'Desconhecido') . "</strong><br>";
echo "Document Root: <strong>" . ($_SERVER['DOCUMENT_ROOT'] ?? 'Desconhecido') . "</strong><br>";
echo "Script Filename: <strong>" . __FILE__ . "</strong><br>";
echo "Current Directory: <strong>" . __DIR__ . "</strong><br>";
echo "</div>";

// ============================================
// 8. LOGS DE ERROS PHP
// ============================================
echo "<div class='section'>";
echo "<h2>8️⃣ Logs de Erros PHP</h2>";

$php_error_log = ini_get('error_log');
echo "Arquivo de log: <code>" . ($php_error_log ?: 'Padrão do sistema') . "</code><br>";

// Tentar ler últimas linhas do log do Apache/PHP
$possible_logs = [
    'C:\wamp64\logs\php_error.log',
    'C:\wamp64\logs\apache_error.log',
    __DIR__ . '/error.log',
];

foreach ($possible_logs as $log_file) {
    if (file_exists($log_file)) {
        echo "<br><strong>Últimas 20 linhas de: $log_file</strong><br>";
        $lines = file($log_file);
        $last_lines = array_slice($lines, -20);
        echo "<pre>" . htmlspecialchars(implode('', $last_lines)) . "</pre>";
        break;
    }
}
echo "</div>";

echo "<hr>";
echo "<h2>🎯 CONCLUSÃO E PRÓXIMOS PASSOS</h2>";
echo "<p>Analise os resultados acima. Os erros em <span class='error'>VERMELHO</span> precisam ser corrigidos primeiro.</p>";
echo "<p><strong>Prioridade de correção:</strong></p>";
echo "<ol>";
echo "<li>Instalar Composer e biblioteca JWT (se necessário)</li>";
echo "<li>Verificar conexão com banco de dados</li>";
echo "<li>Corrigir erros no jwt_helper.php</li>";
echo "<li>Testar endpoints individualmente</li>";
echo "</ol>";

echo "</body></html>";
?>

