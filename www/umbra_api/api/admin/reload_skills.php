<?php
/**
 * Marca intenção de reload. O UmbraManager deve enviar o comando admin
 * TCP `reload_skills` para cada zone autenticada (SkillService::reloadSkills).
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

echo json_encode([
    'success' => true,
    'message' => 'Chame reload_skills nas zones via AdminHub',
    'zone_command' => 'reload_skills',
], JSON_UNESCAPED_UNICODE);
