<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type');

require_once __DIR__ . '/site_images_helper.php';
require_once __DIR__ . '/../../../config/database.php';
require_once __DIR__ . '/gallery_helper.php'; // Para verifyGalleryAdmin

$response = ['success' => false, 'message' => '', 'data' => null];

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents("php://input"), true);
    
    if (empty($data['username'])) {
        $response['message'] = 'Nome de usuário não fornecido';
        echo json_encode($response);
        exit;
    }
    
    try {
        $db = getConnection();
        
        // Verificar se é admin
        $adminCheck = verifyGalleryAdmin($db, $data['username']);
        if (!$adminCheck['success']) {
            $response['message'] = $adminCheck['message'];
            echo json_encode($response);
            exit;
        }
        
        // Escanear todas as categorias
        $scanResult = scanAllSiteImages();
        if ($scanResult === false) {
            $response['message'] = 'Erro ao escanear imagens do site';
            echo json_encode($response);
            exit;
        }
        
        // Calcular estatísticas
        $totalAvailable = 0;
        $totalInJSON = 0;
        $totalOrphans = 0;
        $totalMissing = 0;
        
        foreach ($scanResult as $category => $data) {
            if ($category === 'icons') {
                foreach ($data as $subData) {
                    $totalAvailable += count($subData['available']);
                    $totalInJSON += count($subData['inJSON']);
                    $totalOrphans += count($subData['orphans']);
                    $totalMissing += count($subData['missing']);
                }
            } else {
                $totalAvailable += count($data['available']);
                $totalInJSON += count($data['inJSON']);
                $totalOrphans += count($data['orphans']);
                $totalMissing += count($data['missing']);
            }
        }
        
        $response['success'] = true;
        $response['message'] = 'Escaneamento concluído';
        $response['data'] = [
            'categories' => $scanResult,
            'stats' => [
                'totalAvailable' => $totalAvailable,
                'totalInJSON' => $totalInJSON,
                'totalOrphans' => $totalOrphans,
                'totalMissing' => $totalMissing
            ]
        ];
        
    } catch (Exception $e) {
        $response['message'] = 'Erro: ' . $e->getMessage();
        error_log("Erro em scan_site_images.php: " . $e->getMessage());
    }
} else {
    $response['message'] = 'Método não permitido';
}

echo json_encode($response, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
?>
