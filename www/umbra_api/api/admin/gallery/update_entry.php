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
    
    // Verificar se há algo para atualizar
    if (empty($data['title']) && empty($data['description']) && !isset($data['tags'])) {
        $response['message'] = 'Nenhum campo para atualizar fornecido';
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
        
        // Preparar updates
        $updates = [];
        if (!empty($data['title'])) {
            $updates['title'] = $data['title'];
        }
        if (isset($data['description'])) {
            $updates['description'] = $data['description'];
        }
        if (isset($data['tags']) && is_array($data['tags'])) {
            $updates['tags'] = $data['tags'];
        }
        
        // Atualizar entrada
        $updatedData = updateEntryInGallery($galleryData, $data['category'], $data['filename'], $updates);
        if ($updatedData === false) {
            $response['message'] = 'Erro ao atualizar entrada no gallery.json';
            echo json_encode($response);
            exit;
        }
        
        // Salvar alterações
        if (!writeGalleryJSON($updatedData)) {
            $response['message'] = 'Erro ao salvar gallery.json';
            echo json_encode($response);
            exit;
        }
        
        // Buscar entrada atualizada para retornar
        $updatedEntry = findEntryInGallery($updatedData, $data['category'], $data['filename']);
        
        $response['success'] = true;
        $response['message'] = 'Entrada atualizada com sucesso';
        $response['data'] = [
            'entry' => $updatedEntry['data'],
            'category' => $data['category'],
            'backup' => $backupFile,
            'updatedFields' => array_keys($updates)
        ];
        
    } catch (Exception $e) {
        $response['message'] = 'Erro: ' . $e->getMessage();
        error_log("Erro em update_entry.php: " . $e->getMessage());
    }
} else {
    $response['message'] = 'Método não permitido';
}

echo json_encode($response, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
?>
