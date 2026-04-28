<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type');

require_once __DIR__ . '/site_images_helper.php';
require_once __DIR__ . '/../../../config/database.php';
require_once __DIR__ . '/gallery_helper.php';

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
        
        $adminCheck = verifyGalleryAdmin($db, $data['username']);
        if (!$adminCheck['success']) {
            $response['message'] = $adminCheck['message'];
            echo json_encode($response);
            exit;
        }
        
        $siteData = readSiteImagesJSON();
        if ($siteData === false) {
            $response['message'] = 'Erro ao ler site_images.json';
            echo json_encode($response);
            exit;
        }
        
        // Adicionar info de file exists
        foreach ($siteData['categories'] as $category => &$catData) {
            if ($category === 'concepts') continue;
            
            if ($category === 'icons') {
                foreach ($catData['subcategories'] as $subcat => &$subData) {
                    if (isset($subData['images'])) {
                        foreach ($subData['images'] as &$image) {
                            $path = getCategoryPath($category, $subcat) . $image['filename'];
                            $image['fileExists'] = file_exists($path);
                            $image['previewPath'] = '../../../assets/images/' . $category . '/' . $subcat . '/' . $image['filename'];
                        }
                    }
                }
            } else {
                if (isset($catData['images'])) {
                    foreach ($catData['images'] as &$image) {
                        $path = getCategoryPath($category) . $image['filename'];
                        $image['fileExists'] = file_exists($path);
                        $image['previewPath'] = '../../../assets/images/' . $category . '/' . $image['filename'];
                    }
                }
            }
        }
        
        $response['success'] = true;
        $response['message'] = 'Site images carregadas com sucesso';
        $response['data'] = $siteData;
        
    } catch (Exception $e) {
        $response['message'] = 'Erro: ' . $e->getMessage();
        error_log("Erro em get_site_images.php: " . $e->getMessage());
    }
} else {
    $response['message'] = 'Método não permitido';
}

echo json_encode($response, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
?>
