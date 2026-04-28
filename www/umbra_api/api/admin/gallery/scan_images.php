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
    
    // Validar username (autenticação básica via JSON)
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
        
        // Ler gallery.json atual
        $galleryData = readGalleryJSON();
        if ($galleryData === false) {
            $response['message'] = 'Erro ao ler gallery.json';
            echo json_encode($response);
            exit;
        }
        
        // Escanear cada categoria
        $categories = [];
        
        foreach (VALID_CATEGORIES as $category) {
            $categoryPath = CONCEPTS_DIR . $category . '/';
            
            // Verificar se diretório existe
            if (!is_dir($categoryPath)) {
                $categories[$category] = [
                    'available' => [],
                    'inGallery' => [],
                    'orphans' => [],
                    'missing' => [],
                    'total' => 0
                ];
                continue;
            }
            
            // Escanear arquivos na pasta
            $files = scandir($categoryPath);
            $availableImages = [];
            
            foreach ($files as $file) {
                // Pular . e ..
                if ($file === '.' || $file === '..') {
                    continue;
                }
                
                // Verificar se é imagem válida
                if (isValidImageExtension($file)) {
                    $availableImages[] = $file;
                }
            }
            
            // Listar imagens que já estão no gallery.json
            $inGallery = [];
            $missingFiles = [];
            
            if (isset($galleryData['categories'][$category]['images'])) {
                foreach ($galleryData['categories'][$category]['images'] as $entry) {
                    $inGallery[] = $entry['filename'];
                    
                    // Verificar se arquivo físico existe
                    if (!in_array($entry['filename'], $availableImages)) {
                        $missingFiles[] = $entry['filename'];
                    }
                }
            }
            
            // Identificar imagens órfãs (disponíveis mas não no JSON)
            $orphanImages = array_diff($availableImages, $inGallery);
            
            $categories[$category] = [
                'available' => $availableImages,
                'inGallery' => $inGallery,
                'orphans' => array_values($orphanImages),
                'missing' => $missingFiles,
                'total' => count($availableImages),
                'inGalleryCount' => count($inGallery),
                'orphansCount' => count($orphanImages),
                'missingCount' => count($missingFiles)
            ];
        }
        
        // Calcular estatísticas globais
        $totalAvailable = 0;
        $totalInGallery = 0;
        $totalOrphans = 0;
        $totalMissing = 0;
        
        foreach ($categories as $cat) {
            $totalAvailable += $cat['total'];
            $totalInGallery += $cat['inGalleryCount'];
            $totalOrphans += $cat['orphansCount'];
            $totalMissing += $cat['missingCount'];
        }
        
        $response['success'] = true;
        $response['message'] = 'Escaneamento concluído';
        $response['data'] = [
            'categories' => $categories,
            'stats' => [
                'totalAvailable' => $totalAvailable,
                'totalInGallery' => $totalInGallery,
                'totalOrphans' => $totalOrphans,
                'totalMissing' => $totalMissing,
                'isSynced' => ($totalOrphans === 0 && $totalMissing === 0)
            ]
        ];
        
    } catch (Exception $e) {
        $response['message'] = 'Erro: ' . $e->getMessage();
        error_log("Erro em scan_images.php: " . $e->getMessage());
    }
} else {
    $response['message'] = 'Método não permitido';
}

echo json_encode($response, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
?>
