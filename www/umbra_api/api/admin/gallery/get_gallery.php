<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type');

require_once __DIR__ . '/gallery_helper.php';
require_once __DIR__ . '/../../../config/database.php';

$response = ['success' => false, 'message' => '', 'data' => null];

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents("php://input"), true);
    
    // Validar username
    if (empty($data['username'])) {
        $response['message'] = 'Nome de usuário não fornecido';
        echo json_encode($response);
        exit;
    }
    
    try {
        // Conectar ao banco
        $db = getConnection();
        
        // Verificar se é admin
        $adminCheck = verifyGalleryAdmin($db, $data['username']);
        if (!$adminCheck['success']) {
            $response['message'] = $adminCheck['message'];
            echo json_encode($response);
            exit;
        }
        
        // Ler gallery.json
        $galleryData = readGalleryJSON();
        if ($galleryData === false) {
            $response['message'] = 'Erro ao ler gallery.json. Verifique se o arquivo existe e tem permissões corretas.';
            echo json_encode($response);
            exit;
        }
        
        // Adicionar informações extras sobre cada imagem
        foreach ($galleryData['categories'] as $category => &$catData) {
            if (isset($catData['images'])) {
                foreach ($catData['images'] as &$image) {
                    // Verificar se arquivo físico existe
                    $image['fileExists'] = imageFileExists($category, $image['filename']);
                    
                    // Adicionar caminho relativo para preview
                    $image['previewPath'] = '../../../assets/images/concepts/' . $category . '/' . $image['filename'];
                }
            }
        }
        
        // Contar imagens por categoria
        $stats = [];
        foreach ($galleryData['categories'] as $category => $catData) {
            $stats[$category] = isset($catData['images']) ? count($catData['images']) : 0;
        }
        
        $response['success'] = true;
        $response['message'] = 'Gallery.json carregado com sucesso';
        $response['data'] = [
            'gallery' => $galleryData,
            'stats' => $stats,
            'totalImages' => array_sum($stats)
        ];
        
    } catch (Exception $e) {
        $response['message'] = 'Erro: ' . $e->getMessage();
        error_log("Erro em get_gallery.php: " . $e->getMessage());
    }
} else {
    $response['message'] = 'Método não permitido';
}

echo json_encode($response, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
?>
