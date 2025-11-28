<?php
/**
 * Script para corrigir todos os endpoints de inventário
 * Substitui validateJWTRequest() por validateJWTRequest($data, $_SERVER)
 */

$files = [
    'api/inventory/add_item.php',
    'api/inventory/remove_item.php',
    'api/inventory/move_item.php',
    'api/inventory/equip_item.php',
];

$old_pattern = '/\/\/ Validar JWT e obter player_id\s+\$jwt_data = validateJWTRequest\(\);/s';
$new_code = '// Obter dados da requisição
$json = file_get_contents(\'php://input\');
$data = json_decode($json, true) ?: [];

// Validar JWT e obter player_id
$validation = validateJWTRequest($data, $_SERVER);';

$old_jwt_check = '/if \(\!?\$jwt_data\) \{[^}]+\}/s';
$new_jwt_check = 'if (!$validation[\'valid\']) {
    http_response_code(401);
    echo json_encode([\'success\' => false, \'message\' => $validation[\'error\'] ?? \'Token inválido ou expirado\']);
    exit;
}';

$old_player_id = '/\$player_id = \$jwt_data\[\'player_id\'\] \?\? null;/';
$new_player_id = '$player_id = $validation[\'payload\'][\'player_id\'] ?? null;';

foreach ($files as $file) {
    if (!file_exists($file)) {
        echo "❌ Arquivo não encontrado: $file\n";
        continue;
    }
    
    $content = file_get_contents($file);
    
    // Aplicar substituições
    $content = preg_replace($old_pattern, $new_code, $content);
    $content = preg_replace($old_jwt_check, $new_jwt_check, $content);
    $content = preg_replace($old_player_id, $new_player_id, $content);
    
    file_put_contents($file, $content);
    echo "✅ Corrigido: $file\n";
}

echo "\n🎉 Todos os endpoints foram corrigidos!\n";
echo "\nAgora teste: http://localhost/umbra_api/test_inventory.php\n";
?>

