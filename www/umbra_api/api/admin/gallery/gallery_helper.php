<?php
// Helper functions para gerenciamento do gallery.json

// Caminhos
define('GALLERY_JSON_PATH', __DIR__ . '/../../../assets/images/concepts/gallery.json');
define('GALLERY_BACKUP_DIR', __DIR__ . '/../../../assets/images/concepts/backups/');
define('CONCEPTS_DIR', __DIR__ . '/../../../assets/images/concepts/');

// Categorias válidas
define('VALID_CATEGORIES', ['characters', 'environments', 'creatures', 'weapons']);

// Extensões de imagem permitidas
define('ALLOWED_IMAGE_EXTENSIONS', ['jpg', 'jpeg', 'png', 'webp']);
define('ALLOWED_MIME_TYPES', ['image/jpeg', 'image/png', 'image/webp']);

/**
 * Lê o conteúdo do gallery.json
 * @return array|false Array com dados do JSON ou false em caso de erro
 */
function readGalleryJSON() {
    if (!file_exists(GALLERY_JSON_PATH)) {
        error_log("gallery.json não encontrado em: " . GALLERY_JSON_PATH);
        return false;
    }
    
    $content = file_get_contents(GALLERY_JSON_PATH);
    if ($content === false) {
        error_log("Erro ao ler gallery.json");
        return false;
    }
    
    $data = json_decode($content, true);
    if (json_last_error() !== JSON_ERROR_NONE) {
        error_log("Erro ao decodificar gallery.json: " . json_last_error_msg());
        return false;
    }
    
    return $data;
}

/**
 * Escreve dados no gallery.json
 * @param array $data Dados a serem escritos
 * @return bool True se sucesso, false caso contrário
 */
function writeGalleryJSON($data) {
    // Validar estrutura básica
    if (!isset($data['version']) || !isset($data['categories'])) {
        error_log("Estrutura inválida do gallery.json");
        return false;
    }
    
    // Atualizar lastUpdated
    $data['lastUpdated'] = date('Y-m-d');
    
    // Codificar com formatação bonita
    $json = json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
    if ($json === false) {
        error_log("Erro ao codificar JSON: " . json_last_error_msg());
        return false;
    }
    
    // Escrever arquivo
    $result = file_put_contents(GALLERY_JSON_PATH, $json);
    if ($result === false) {
        error_log("Erro ao escrever gallery.json");
        return false;
    }
    
    return true;
}

/**
 * Cria backup do gallery.json atual
 * @return string|false Nome do arquivo de backup ou false em caso de erro
 */
function backupGalleryJSON() {
    if (!file_exists(GALLERY_JSON_PATH)) {
        error_log("gallery.json não encontrado para backup");
        return false;
    }
    
    // Criar diretório de backup se não existir
    if (!is_dir(GALLERY_BACKUP_DIR)) {
        if (!mkdir(GALLERY_BACKUP_DIR, 0755, true)) {
            error_log("Erro ao criar diretório de backups");
            return false;
        }
    }
    
    // Nome do arquivo de backup com timestamp
    $timestamp = date('Y-m-d_H-i-s');
    $backupFile = GALLERY_BACKUP_DIR . "gallery_backup_{$timestamp}.json";
    
    // Copiar arquivo
    if (!copy(GALLERY_JSON_PATH, $backupFile)) {
        error_log("Erro ao criar backup");
        return false;
    }
    
    // Limpar backups antigos (manter apenas últimos 10)
    cleanOldBackups();
    
    return basename($backupFile);
}

/**
 * Remove backups antigos, mantendo apenas os 10 mais recentes
 */
function cleanOldBackups() {
    if (!is_dir(GALLERY_BACKUP_DIR)) {
        return;
    }
    
    $files = glob(GALLERY_BACKUP_DIR . 'gallery_backup_*.json');
    if (count($files) <= 10) {
        return;
    }
    
    // Ordenar por data de modificação (mais antigos primeiro)
    usort($files, function($a, $b) {
        return filemtime($a) - filemtime($b);
    });
    
    // Deletar os mais antigos
    $toDelete = count($files) - 10;
    for ($i = 0; $i < $toDelete; $i++) {
        unlink($files[$i]);
    }
}

/**
 * Lista todos os backups disponíveis
 * @return array Array com informações dos backups
 */
function listBackups() {
    if (!is_dir(GALLERY_BACKUP_DIR)) {
        return [];
    }
    
    $files = glob(GALLERY_BACKUP_DIR . 'gallery_backup_*.json');
    $backups = [];
    
    foreach ($files as $file) {
        $backups[] = [
            'filename' => basename($file),
            'date' => date('d/m/Y H:i:s', filemtime($file)),
            'size' => filesize($file)
        ];
    }
    
    // Ordenar por data (mais recentes primeiro)
    usort($backups, function($a, $b) {
        return strcmp($b['filename'], $a['filename']);
    });
    
    return $backups;
}

/**
 * Valida se uma categoria é válida
 * @param string $category Nome da categoria
 * @return bool True se válida, false caso contrário
 */
function isValidCategory($category) {
    return in_array($category, VALID_CATEGORIES);
}

/**
 * Valida se uma extensão de imagem é permitida
 * @param string $filename Nome do arquivo
 * @return bool True se válida, false caso contrário
 */
function isValidImageExtension($filename) {
    $ext = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
    return in_array($ext, ALLOWED_IMAGE_EXTENSIONS);
}

/**
 * Valida se um tipo MIME é permitido
 * @param string $mimeType Tipo MIME
 * @return bool True se válido, false caso contrário
 */
function isValidMimeType($mimeType) {
    return in_array($mimeType, ALLOWED_MIME_TYPES);
}

/**
 * Sanitiza nome de arquivo (remove caracteres especiais)
 * @param string $filename Nome do arquivo
 * @return string Nome sanitizado
 */
function sanitizeFilename($filename) {
    // Separar nome e extensão
    $ext = pathinfo($filename, PATHINFO_EXTENSION);
    $name = pathinfo($filename, PATHINFO_FILENAME);
    
    // Remover caracteres especiais, manter apenas letras, números, underscore e hífen
    $name = preg_replace('/[^a-zA-Z0-9_-]/', '_', $name);
    
    // Remover underscores/hífens duplicados
    $name = preg_replace('/[_-]+/', '_', $name);
    
    // Converter para minúsculas
    $name = strtolower($name);
    
    return $name . '.' . strtolower($ext);
}

/**
 * Verifica se um arquivo de imagem existe fisicamente
 * @param string $category Categoria da imagem
 * @param string $filename Nome do arquivo
 * @return bool True se existe, false caso contrário
 */
function imageFileExists($category, $filename) {
    $path = CONCEPTS_DIR . $category . '/' . $filename;
    return file_exists($path);
}

/**
 * Busca uma entrada no gallery.json
 * @param array $galleryData Dados do gallery.json
 * @param string $category Categoria
 * @param string $filename Nome do arquivo
 * @return array|false Array com dados da entrada ou false se não encontrado
 */
function findEntryInGallery($galleryData, $category, $filename) {
    if (!isset($galleryData['categories'][$category]['images'])) {
        return false;
    }
    
    foreach ($galleryData['categories'][$category]['images'] as $index => $image) {
        if ($image['filename'] === $filename) {
            return ['index' => $index, 'data' => $image];
        }
    }
    
    return false;
}

/**
 * Adiciona uma entrada ao gallery.json
 * @param array $galleryData Dados do gallery.json
 * @param string $category Categoria
 * @param array $entry Dados da entrada (filename, title, description, tags)
 * @return array|false Dados atualizados ou false em caso de erro
 */
function addEntryToGallery($galleryData, $category, $entry) {
    // Validar categoria
    if (!isValidCategory($category)) {
        error_log("Categoria inválida: $category");
        return false;
    }
    
    // Validar campos obrigatórios
    if (empty($entry['filename']) || empty($entry['title'])) {
        error_log("Campos obrigatórios faltando (filename ou title)");
        return false;
    }
    
    // Validar extensão
    if (!isValidImageExtension($entry['filename'])) {
        error_log("Extensão de arquivo inválida: " . $entry['filename']);
        return false;
    }
    
    // Verificar se já existe
    if (findEntryInGallery($galleryData, $category, $entry['filename']) !== false) {
        error_log("Entrada já existe: " . $entry['filename']);
        return false;
    }
    
    // Preparar entrada
    $newEntry = [
        'filename' => $entry['filename'],
        'title' => $entry['title'],
        'description' => isset($entry['description']) ? $entry['description'] : '',
        'tags' => isset($entry['tags']) && is_array($entry['tags']) ? $entry['tags'] : []
    ];
    
    // Adicionar ao array
    $galleryData['categories'][$category]['images'][] = $newEntry;
    
    return $galleryData;
}

/**
 * Atualiza uma entrada no gallery.json
 * @param array $galleryData Dados do gallery.json
 * @param string $category Categoria
 * @param string $filename Nome do arquivo
 * @param array $updates Dados a atualizar
 * @return array|false Dados atualizados ou false em caso de erro
 */
function updateEntryInGallery($galleryData, $category, $filename, $updates) {
    // Validar categoria
    if (!isValidCategory($category)) {
        error_log("Categoria inválida: $category");
        return false;
    }
    
    // Buscar entrada
    $found = findEntryInGallery($galleryData, $category, $filename);
    if ($found === false) {
        error_log("Entrada não encontrada: $filename");
        return false;
    }
    
    // Atualizar campos permitidos
    if (isset($updates['title'])) {
        $galleryData['categories'][$category]['images'][$found['index']]['title'] = $updates['title'];
    }
    if (isset($updates['description'])) {
        $galleryData['categories'][$category]['images'][$found['index']]['description'] = $updates['description'];
    }
    if (isset($updates['tags']) && is_array($updates['tags'])) {
        $galleryData['categories'][$category]['images'][$found['index']]['tags'] = $updates['tags'];
    }
    
    return $galleryData;
}

/**
 * Remove uma entrada do gallery.json
 * @param array $galleryData Dados do gallery.json
 * @param string $category Categoria
 * @param string $filename Nome do arquivo
 * @return array|false Dados atualizados ou false em caso de erro
 */
function deleteEntryFromGallery($galleryData, $category, $filename) {
    // Validar categoria
    if (!isValidCategory($category)) {
        error_log("Categoria inválida: $category");
        return false;
    }
    
    // Buscar entrada
    $found = findEntryInGallery($galleryData, $category, $filename);
    if ($found === false) {
        error_log("Entrada não encontrada: $filename");
        return false;
    }
    
    // Remover do array
    array_splice($galleryData['categories'][$category]['images'], $found['index'], 1);
    
    return $galleryData;
}

/**
 * Valida credenciais de admin
 * @param PDO $db Conexão com banco de dados
 * @param string $username Nome de usuário
 * @return array Array com resultado da validação
 */
function verifyGalleryAdmin($db, $username) {
    try {
        $query = "SELECT id, username, email, isadmin, banned 
                 FROM accounts WHERE username = :username";
        $stmt = $db->prepare($query);
        $stmt->bindParam(':username', $username);
        $stmt->execute();
        
        if ($stmt->rowCount() > 0) {
            $account = $stmt->fetch(PDO::FETCH_ASSOC);
            
            if ($account['banned']) {
                return ['success' => false, 'message' => 'Conta banida'];
            }
            
            if ($account['isadmin'] != 1) {
                return ['success' => false, 'message' => 'Acesso negado. Apenas administradores'];
            }
            
            return [
                'success' => true,
                'admin' => [
                    'id' => $account['id'],
                    'username' => $account['username'],
                    'email' => $account['email']
                ]
            ];
        } else {
            return ['success' => false, 'message' => 'Usuário não encontrado'];
        }
    } catch (Exception $e) {
        return ['success' => false, 'message' => 'Erro: ' . $e->getMessage()];
    }
}
?>
