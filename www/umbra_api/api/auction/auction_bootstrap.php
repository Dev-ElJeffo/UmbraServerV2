<?php
/**
 * Dependências comuns para api/auction/*.php
 */
$umbraApiRoot = dirname(__DIR__, 2);

$db = $umbraApiRoot . DIRECTORY_SEPARATOR . 'config' . DIRECTORY_SEPARATOR . 'database.php';
$jwt = $umbraApiRoot . DIRECTORY_SEPARATOR . 'helpers' . DIRECTORY_SEPARATOR . 'jwt_helper.php';
$psh = $umbraApiRoot . DIRECTORY_SEPARATOR . 'helpers' . DIRECTORY_SEPARATOR . 'personal_shop_helper.php';
$ah = $umbraApiRoot . DIRECTORY_SEPARATOR . 'helpers' . DIRECTORY_SEPARATOR . 'auction_helper.php';

foreach ([$db, $jwt, $psh, $ah] as $path) {
    if (!is_readable($path)) {
        header('Content-Type: application/json; charset=utf-8');
        http_response_code(500);
        echo json_encode(['success' => false, 'message' => 'Servidor incompleto: arquivo ausente em ' . $path]);
        exit;
    }
}

require_once $db;
require_once $jwt;
require_once $psh;
require_once $ah;
