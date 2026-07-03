<?php
/**
 * Dependências comuns para api/npc_vendor/*.php
 */
$umbraApiRoot = dirname(__DIR__, 2);

$db = $umbraApiRoot . DIRECTORY_SEPARATOR . 'config' . DIRECTORY_SEPARATOR . 'database.php';
$jwt = $umbraApiRoot . DIRECTORY_SEPARATOR . 'helpers' . DIRECTORY_SEPARATOR . 'jwt_helper.php';
$psh = $umbraApiRoot . DIRECTORY_SEPARATOR . 'helpers' . DIRECTORY_SEPARATOR . 'personal_shop_helper.php';
$nvh = $umbraApiRoot . DIRECTORY_SEPARATOR . 'helpers' . DIRECTORY_SEPARATOR . 'npc_vendor_helper.php';

foreach ([$db => 'config/database.php', $jwt => 'helpers/jwt_helper.php', $psh => 'helpers/personal_shop_helper.php', $nvh => 'helpers/npc_vendor_helper.php'] as $path => $label) {
    if (!is_readable($path)) {
        header('Content-Type: application/json; charset=utf-8');
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Servidor incompleto: ' . $label . ' não encontrado.']);
        exit;
    }
}

require_once $db;
require_once $jwt;
require_once $psh;
require_once $nvh;
