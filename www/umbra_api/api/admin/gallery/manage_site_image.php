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
    
    if (empty($data['username']) || empty($data['action'])) {
        $response['message'] = 'Dados obrigatórios faltando';
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
        
        $action = $data['action'];
        $category = $data['category'] ?? null;
        $subcategory = $data['subcategory'] ?? null;
        $filename = $data['filename'] ?? null;
        
        if (!$category || !isValidSiteCategory($category)) {
            $response['message'] = 'Categoria inválida';
            echo json_encode($response);
            exit;
        }
        
        $siteData = readSiteImagesJSON();
        if (!$siteData) {
            $response['message'] = 'Erro ao ler site_images.json';
            echo json_encode($response);
            exit;
        }
        
        // Backup antes de modificar
        $backupFile = backupSiteImagesJSON();
        
        switch ($action) {
            case 'add':
                if (empty($data['title']) || empty($filename)) {
                    $response['message'] = 'Título e filename são obrigatórios';
                    echo json_encode($response);
                    exit;
                }
                
                $entry = [
                    'filename' => $filename,
                    'title' => $data['title'],
                    'description' => $data['description'] ?? '',
                    'usage' => $data['usage'] ?? '',
                    'tags' => $data['tags'] ?? []
                ];
                
                // Adicionar classType se for categoria classes
                if ($category === 'classes' && !empty($data['classType'])) {
                    $entry['classType'] = $data['classType'];
                }
                
                $updatedData = addSiteImageEntry($siteData, $category, $subcategory, $entry);
                if (!$updatedData) {
                    $response['message'] = 'Erro ao adicionar entrada';
                    echo json_encode($response);
                    exit;
                }
                
                if (!writeSiteImagesJSON($updatedData)) {
                    $response['message'] = 'Erro ao salvar site_images.json';
                    echo json_encode($response);
                    exit;
                }
                
                $response['success'] = true;
                $response['message'] = 'Entrada adicionada com sucesso';
                $response['data'] = ['entry' => $entry, 'backup' => $backupFile];
                break;
                
            case 'update':
                if (empty($filename)) {
                    $response['message'] = 'Filename é obrigatório';
                    echo json_encode($response);
                    exit;
                }
                
                $updates = [];
                if (isset($data['title'])) $updates['title'] = $data['title'];
                if (isset($data['description'])) $updates['description'] = $data['description'];
                if (isset($data['usage'])) $updates['usage'] = $data['usage'];
                if (isset($data['tags'])) $updates['tags'] = $data['tags'];
                if (isset($data['classType'])) $updates['classType'] = $data['classType'];
                
                $updatedData = updateSiteImageEntry($siteData, $category, $subcategory, $filename, $updates);
                if (!$updatedData) {
                    $response['message'] = 'Erro ao atualizar entrada';
                    echo json_encode($response);
                    exit;
                }
                
                if (!writeSiteImagesJSON($updatedData)) {
                    $response['message'] = 'Erro ao salvar site_images.json';
                    echo json_encode($response);
                    exit;
                }
                
                $response['success'] = true;
                $response['message'] = 'Entrada atualizada com sucesso';
                $response['data'] = ['updates' => $updates, 'backup' => $backupFile];
                break;
                
            case 'delete':
                if (empty($filename)) {
                    $response['message'] = 'Filename é obrigatório';
                    echo json_encode($response);
                    exit;
                }
                
                $deleteFile = isset($data['deleteFile']) && $data['deleteFile'] === true;
                $fileDeleted = false;
                
                if ($deleteFile) {
                    $fileDeleted = deleteSiteImageFile($category, $subcategory, $filename);
                }
                
                $updatedData = deleteSiteImageEntry($siteData, $category, $subcategory, $filename);
                if (!$updatedData) {
                    $response['message'] = 'Erro ao deletar entrada';
                    echo json_encode($response);
                    exit;
                }
                
                if (!writeSiteImagesJSON($updatedData)) {
                    $response['message'] = 'Erro ao salvar site_images.json';
                    echo json_encode($response);
                    exit;
                }
                
                $response['success'] = true;
                $response['message'] = 'Entrada deletada com sucesso';
                $response['data'] = [
                    'filename' => $filename,
                    'fileDeleted' => $fileDeleted,
                    'backup' => $backupFile
                ];
                break;
                
            default:
                $response['message'] = 'Ação inválida';
                break;
        }
        
    } catch (Exception $e) {
        $response['message'] = 'Erro: ' . $e->getMessage();
        error_log("Erro em manage_site_image.php: " . $e->getMessage());
    }
} else {
    $response['message'] = 'Método não permitido';
}

echo json_encode($response, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
?>
