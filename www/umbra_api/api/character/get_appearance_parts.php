<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../admin/appearance_part_admin_helpers.php';

try {
    $pdo = getConnection();
    if (!$pdo) {
        throw new RuntimeException('Falha na conexão');
    }
    $stmt = $pdo->query(
        "SELECT appearance_part_id, part_type, part_id, mesh_path, attach_socket, is_enabled
         FROM player_appearance_parts
         WHERE is_enabled = 1
         ORDER BY part_type ASC, part_id ASC"
    );
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    $parts = [];
    foreach ($rows as $row) {
        $parts[] = appearance_part_row_normalize($row);
    }
    echo json_encode([
        'success' => true,
        'parts' => $parts,
        'total' => count($parts),
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[get_appearance_parts] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao buscar partes de aparência'], JSON_UNESCAPED_UNICODE);
}
