<?php
header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type');

require_once __DIR__ . '/gallery_helper.php';
require_once __DIR__ . '/../../../config/database.php';

$response = ['success' => false, 'message' => '', 'data' => null];

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    
    // Validar username (via POST normal, não JSON)
    if (empty($_POST['username'])) {
        $response['message'] = 'Nome de usuário não fornecido';
        echo json_encode($response);
        exit;
    }
    
    // Validar categoria
    if (empty($_POST['category'])) {
        $response['message'] = 'Categoria não fornecida';
        echo json_encode($response);
        exit;
    }
    
    // Validar se arquivo foi enviado
    if (!isset($_FILES['image']) || $_FILES['image']['error'] === UPLOAD_ERR_NO_FILE) {
        $response['message'] = 'Nenhum arquivo enviado';
        echo json_encode($response);
        exit;
    }
    
    // Verificar erros de upload
    if ($_FILES['image']['error'] !== UPLOAD_ERR_OK) {
        $errors = [
            UPLOAD_ERR_INI_SIZE => 'Arquivo excede tamanho máximo permitido pelo servidor',
            UPLOAD_ERR_FORM_SIZE => 'Arquivo excede tamanho máximo permitido',
            UPLOAD_ERR_PARTIAL => 'Upload parcial, tente novamente',
            UPLOAD_ERR_NO_TMP_DIR => 'Pasta temporária não encontrada',
            UPLOAD_ERR_CANT_WRITE => 'Erro ao escrever arquivo no disco',
            UPLOAD_ERR_EXTENSION => 'Upload bloqueado por extensão'
        ];
        $response['message'] = isset($errors[$_FILES['image']['error']]) 
            ? $errors[$_FILES['image']['error']] 
            : 'Erro desconhecido no upload';
        echo json_encode($response);
        exit;
    }
    
    try {
        // Conectar ao banco
        $db = getConnection();
        
        // Verificar se é admin
        $adminCheck = verifyGalleryAdmin($db, $_POST['username']);
        if (!$adminCheck['success']) {
            $response['message'] = $adminCheck['message'];
            echo json_encode($response);
            exit;
        }
        
        // Validar categoria
        if (!isValidCategory($_POST['category'])) {
            $response['message'] = 'Categoria inválida. Use: ' . implode(', ', VALID_CATEGORIES);
            echo json_encode($response);
            exit;
        }
        
        $category = $_POST['category'];
        $uploadFile = $_FILES['image'];
        
        // Validar tamanho (2MB máximo)
        $maxSize = 2 * 1024 * 1024; // 2MB em bytes
        if ($uploadFile['size'] > $maxSize) {
            $response['message'] = 'Arquivo muito grande. Tamanho máximo: 2MB';
            echo json_encode($response);
            exit;
        }
        
        // Validar tipo MIME real
        $finfo = finfo_open(FILEINFO_MIME_TYPE);
        $mimeType = finfo_file($finfo, $uploadFile['tmp_name']);
        finfo_close($finfo);
        
        if (!isValidMimeType($mimeType)) {
            $response['message'] = 'Tipo de arquivo inválido. Use apenas JPG, PNG ou WebP';
            echo json_encode($response);
            exit;
        }
        
        // Obter nome original e sanitizar
        $originalName = basename($uploadFile['name']);
        $sanitizedName = sanitizeFilename($originalName);
        
        // Validar extensão
        if (!isValidImageExtension($sanitizedName)) {
            $response['message'] = 'Extensão de arquivo inválida';
            echo json_encode($response);
            exit;
        }
        
        // Caminho de destino
        $targetDir = CONCEPTS_DIR . $category . '/';
        
        // Criar diretório se não existir
        if (!is_dir($targetDir)) {
            if (!mkdir($targetDir, 0755, true)) {
                $response['message'] = 'Erro ao criar diretório de destino';
                echo json_encode($response);
                exit;
            }
        }
        
        $targetPath = $targetDir . $sanitizedName;
        
        // Verificar se arquivo já existe
        $finalName = $sanitizedName;
        if (file_exists($targetPath)) {
            // Gerar nome único
            $ext = pathinfo($sanitizedName, PATHINFO_EXTENSION);
            $name = pathinfo($sanitizedName, PATHINFO_FILENAME);
            $counter = 1;
            
            while (file_exists($targetDir . $finalName)) {
                $finalName = $name . '_' . $counter . '.' . $ext;
                $counter++;
                
                if ($counter > 100) {
                    $response['message'] = 'Erro: muitos arquivos com nomes similares';
                    echo json_encode($response);
                    exit;
                }
            }
            
            $targetPath = $targetDir . $finalName;
        }
        
        // Mover arquivo
        if (!move_uploaded_file($uploadFile['tmp_name'], $targetPath)) {
            $response['message'] = 'Erro ao salvar arquivo';
            echo json_encode($response);
            exit;
        }
        
        // Definir permissões
        chmod($targetPath, 0644);
        
        $response['success'] = true;
        $response['message'] = 'Imagem enviada com sucesso';
        $response['data'] = [
            'filename' => $finalName,
            'originalName' => $originalName,
            'category' => $category,
            'size' => $uploadFile['size'],
            'mimeType' => $mimeType,
            'path' => 'assets/images/concepts/' . $category . '/' . $finalName
        ];
        
        // Se nome foi alterado, avisar
        if ($finalName !== $sanitizedName) {
            $response['message'] .= ' (Nome alterado para evitar conflito)';
        }
        
    } catch (Exception $e) {
        $response['message'] = 'Erro: ' . $e->getMessage();
        error_log("Erro em upload_image.php: " . $e->getMessage());
    }
} else {
    $response['message'] = 'Método não permitido';
}

echo json_encode($response, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
?>
