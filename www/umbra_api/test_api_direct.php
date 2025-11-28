<?php
/**
 * Teste Direto da API de Inventário
 * Este arquivo testa diretamente a API sem JavaScript
 */

// Token JWT (substitua pelo seu token real)
$token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhY2NvdW50X2lkIjo0LCJwbGF5ZXJfaWQiOjEsInVzZXJuYW1lIjoiamVmZm8iLCJpYXQiOjE3NjMxNDMyMzEsImV4cCI6MTc2MzE0NjgzMX0.2zYxTjRWLckbX4krPJT73M6kT86DPcJ7f-NfoTHj5LI";

// URL da API
$api_url = "http://localhost/umbra_api/api/inventory/get_inventory.php";

// Configurar contexto da requisição
$options = [
    'http' => [
        'method' => 'GET',
        'header' => "Authorization: Bearer $token\r\n" .
                   "Content-Type: application/json\r\n"
    ]
];

$context = stream_context_create($options);

// Fazer requisição
echo "<h1>Teste Direto da API de Inventário</h1>";
echo "<h2>Endpoint: GET /api/inventory/get_inventory.php</h2>";
echo "<h3>Token usado:</h3>";
echo "<pre>" . substr($token, 0, 50) . "...</pre>";

echo "<h3>Requisição:</h3>";
echo "<pre>";
print_r($options);
echo "</pre>";

echo "<h3>Resposta:</h3>";
$response = @file_get_contents($api_url, false, $context);

if ($response === FALSE) {
    echo "<p style='color: red; font-weight: bold;'>❌ ERRO: Não foi possível fazer a requisição!</p>";
    echo "<p>Verifique se o servidor web está rodando e a URL está correta.</p>";
} else {
    echo "<pre style='background: #f0f0f0; padding: 15px; border-radius: 5px;'>";
    echo htmlspecialchars($response);
    echo "</pre>";
    
    // Tentar decodificar JSON
    $data = json_decode($response, true);
    if ($data) {
        echo "<h3>JSON Decodificado:</h3>";
        echo "<pre style='background: #e8f5e9; padding: 15px; border-radius: 5px;'>";
        print_r($data);
        echo "</pre>";
        
        if (isset($data['success']) && $data['success']) {
            echo "<p style='color: green; font-weight: bold;'>✅ SUCESSO! API está funcionando corretamente!</p>";
        } else {
            echo "<p style='color: orange; font-weight: bold;'>⚠️ API retornou, mas com erro:</p>";
            echo "<p>" . htmlspecialchars($data['message'] ?? 'Erro desconhecido') . "</p>";
        }
    } else {
        echo "<p style='color: red; font-weight: bold;'>❌ ERRO: Resposta não é JSON válido!</p>";
    }
}

echo "<hr>";
echo "<h2>Teste do Endpoint de Templates (Sem Autenticação)</h2>";

$templates_url = "http://localhost/umbra_api/api/inventory/get_item_templates.php";
$templates_response = @file_get_contents($templates_url);

if ($templates_response === FALSE) {
    echo "<p style='color: red; font-weight: bold;'>❌ ERRO: Não foi possível acessar o endpoint de templates!</p>";
} else {
    $templates_data = json_decode($templates_response, true);
    if ($templates_data && isset($templates_data['success']) && $templates_data['success']) {
        echo "<p style='color: green; font-weight: bold;'>✅ SUCESSO! " . $templates_data['total'] . " templates encontrados!</p>";
        echo "<table border='1' cellpadding='5' style='border-collapse: collapse;'>";
        echo "<tr><th>ID</th><th>Nome</th><th>Tipo</th><th>Raridade</th><th>Valor</th></tr>";
        foreach ($templates_data['templates'] as $template) {
            echo "<tr>";
            echo "<td>" . $template['item_id'] . "</td>";
            echo "<td>" . htmlspecialchars($template['item_name']) . "</td>";
            echo "<td>" . $template['item_type'] . "</td>";
            echo "<td>" . $template['rarity'] . "</td>";
            echo "<td>" . $template['value'] . " 🪙</td>";
            echo "</tr>";
        }
        echo "</table>";
    } else {
        echo "<p style='color: red; font-weight: bold;'>❌ ERRO ao acessar templates!</p>";
        echo "<pre>" . htmlspecialchars($templates_response) . "</pre>";
    }
}
?>

