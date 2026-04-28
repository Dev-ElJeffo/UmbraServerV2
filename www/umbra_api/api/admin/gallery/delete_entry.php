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
    
    // Validar dados obrigatórios
    if (empty($data['category']) || empty($data['filename'])) {
        $response['message'] = 'Campos obrigatórios faltando (category, filename)';
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
        
        // Validar categoria
        if (!isValidCategory($data['category'])) {
            $response['message'] = 'Categoria inválida. Use: ' . implode(', ', VALID_CATEGORIES);
            echo json_encode($response);
            exit;
        }
        
        // Ler gallery.json atual
        $galleryData = readGalleryJSON();
        if ($galleryData === false) {
            $response['message'] = 'Erro ao ler gallery.json';
            echo json_encode($response);
            exit;
        }
        
        // Verificar se entrada existe
        $existing = findEntryInGallery($galleryData, $data['category'], $data['filename']);
        if ($existing === false) {
            $response['message'] = 'Entrada não encontrada: ' . $data['filename'];
            echo json_encode($response);
            exit;
        }
        
        // Criar backup antes de modificar
        $backupFile = backupGalleryJSON();
        if ($backupFile === false) {
            $response['message'] = 'Erro ao criar backup do gallery.json';
            echo json_encode($response);
            exit;
        }
        
        // Verificar se deve deletar arquivo físico também
        $deletePhysicalFile = isset($data['deleteFile']) && $data['deleteFile'] === true;
        $fileDeleted = false;
        $fileDeleteError = null;
        
        if ($deletePhysicalFile) {
            $filePath = CONCEPTS_DIR . $data['category'] . '/' . $data['filename'];
            
            if (file_exists($filePath)) {
                if (unlink($filePath)) {
                    $fileDeleted = true;
                } else {
                    $fileDeleteError = 'Erro ao deletar arquivo físico';
                    error_log("Erro ao deletar arquivo: $filePath");
                }
            } else {
                $fileDeleteError = 'Arquivo físico não encontrado';
            }
        }
        
        // Salvar dados da entrada antes de deletar
        $deletedEntry = $existing['data'];
        
        // Remover entrada do JSON
        $updatedData = deleteEntryFromGallery($galleryData, $data['category'], $data['filename']);
        if ($updatedData === false) {
            $response['message'] = 'Erro ao remover entrada do gallery.json';
            echo json_encode($response);
            exit;
        }
        
        // Salvar alterações
        if (!writeGalleryJSON($updatedData)) {
            $response['message'] = 'Erro ao salvar gallery.json';
            echo json_encode($response);
            exit;
        }
        
        $response['success'] = true;
        $response['message'] = 'Entrada removida com sucesso';
        $response['data'] = [
            'deletedEntry' => $deletedEntry,
            'category' => $data['category'],
            'backup' => $backupFile,
            'physicalFileDeleted' => $fileDeleted,
            'fileDeleteError' => $fileDeleteError
        ];
        
        if ($fileDeleteError) {
            $response['message'] .= ' (Aviso: ' . $fileDeleteError . ')';
        }
        
    } catch (Exception $e) {
        $response['message'] = 'Erro: ' . $e->getMessage();
        error_log("Erro em delete_entry.php: " . $e->getMessage());
    }
} else {
    $response['message'] = 'Método não permitido';
}

echo json_encode($response, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
?>
