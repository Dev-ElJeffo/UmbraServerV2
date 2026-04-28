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
    if (empty($data['category']) || empty($data['filename']) || empty($data['title'])) {
        $response['message'] = 'Campos obrigatórios faltando (category, filename, title)';
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
        
        // Validar extensão do arquivo
        if (!isValidImageExtension($data['filename'])) {
            $response['message'] = 'Extensão de arquivo inválida. Use: ' . implode(', ', ALLOWED_IMAGE_EXTENSIONS);
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
        
        // Verificar se já existe entrada com mesmo filename
        $existing = findEntryInGallery($galleryData, $data['category'], $data['filename']);
        if ($existing !== false) {
            $response['message'] = 'Já existe uma entrada com este filename na categoria ' . $data['category'];
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
        
        // Preparar entrada
        $entry = [
            'filename' => $data['filename'],
            'title' => $data['title'],
            'description' => isset($data['description']) ? $data['description'] : '',
            'tags' => isset($data['tags']) && is_array($data['tags']) ? $data['tags'] : []
        ];
        
        // Adicionar entrada
        $updatedData = addEntryToGallery($galleryData, $data['category'], $entry);
        if ($updatedData === false) {
            $response['message'] = 'Erro ao adicionar entrada ao gallery.json';
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
        $response['message'] = 'Entrada adicionada com sucesso';
        $response['data'] = [
            'entry' => $entry,
            'category' => $data['category'],
            'backup' => $backupFile
        ];
        
    } catch (Exception $e) {
        $response['message'] = 'Erro: ' . $e->getMessage();
        error_log("Erro em add_entry.php: " . $e->getMessage());
    }
} else {
    $response['message'] = 'Método não permitido';
}

echo json_encode($response, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
?>
