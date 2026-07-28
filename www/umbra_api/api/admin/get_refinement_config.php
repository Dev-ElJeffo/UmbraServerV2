<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?: [];
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    $data['admin_username'] = $_GET['admin_username'] ?? $data['admin_username'] ?? null;
}

require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);
require_once __DIR__ . '/../refinement/get_refinement_config.php';
