<?php
/**
 * Dependências comuns para api/shop/*.php
 * Raiz da API = dirname(__DIR__, 2)  →  …/umbra_api/
 */
$umbraApiRoot = dirname(__DIR__, 2);

$db = $umbraApiRoot . DIRECTORY_SEPARATOR . 'config' . DIRECTORY_SEPARATOR . 'database.php';
$jwt = $umbraApiRoot . DIRECTORY_SEPARATOR . 'helpers' . DIRECTORY_SEPARATOR . 'jwt_helper.php';
$psh = $umbraApiRoot . DIRECTORY_SEPARATOR . 'helpers' . DIRECTORY_SEPARATOR . 'personal_shop_helper.php';
$ench = $umbraApiRoot . DIRECTORY_SEPARATOR . 'helpers' . DIRECTORY_SEPARATOR . 'enchant_helper.php';
$statMap = $umbraApiRoot . DIRECTORY_SEPARATOR . 'helpers' . DIRECTORY_SEPARATOR . 'stat_key_mapping.php';

if (!is_readable($db)) {
    header('Content-Type: application/json; charset=utf-8');
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Servidor incompleto: arquivo config/database.php não encontrado em ' . $db]);
    exit;
}
if (!is_readable($jwt)) {
    header('Content-Type: application/json; charset=utf-8');
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Servidor incompleto: helpers/jwt_helper.php não encontrado.']);
    exit;
}
if (!is_readable($psh)) {
    header('Content-Type: application/json; charset=utf-8');
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Servidor incompleto: copie o arquivo helpers/personal_shop_helper.php para ' . $psh . ' (repositório: www/umbra_api/helpers/personal_shop_helper.php).',
    ]);
    exit;
}

require_once $db;
require_once $jwt;
require_once $psh;
if (is_readable($statMap)) {
    require_once $statMap;
}
if (is_readable($ench)) {
    require_once $ench;
}
