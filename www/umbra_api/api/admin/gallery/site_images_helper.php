<?php
// Helper unificado para gerenciamento de TODAS as imagens do site

// Caminhos
define('IMAGES_BASE_DIR', __DIR__ . '/../../../assets/images/');
define('SITE_IMAGES_JSON', IMAGES_BASE_DIR . 'site_images.json');
define('SITE_IMAGES_BACKUP_DIR', IMAGES_BASE_DIR . 'backups/');

// Categorias de site_images.json (não-concepts)
define('SITE_CATEGORIES', ['backgrounds', 'classes', 'lore', 'screenshots', 'logos', 'icons']);

// Subcategorias de icons
define('ICONS_SUBCATEGORIES', ['classes', 'ui', 'social']);

/**
 * Lê o site_images.json
 * @return array|false
 */
function readSiteImagesJSON() {
    if (!file_exists(SITE_IMAGES_JSON)) {
        error_log("site_images.json não encontrado");
        return false;
    }
    
    $content = file_get_contents(SITE_IMAGES_JSON);
    if ($content === false) {
        return false;
    }
    
    $data = json_decode($content, true);
    if (json_last_error() !== JSON_ERROR_NONE) {
        error_log("Erro ao decodificar site_images.json: " . json_last_error_msg());
        return false;
    }
    
    return $data;
}

/**
 * Escreve dados no site_images.json
 * @param array $data
 * @return bool
 */
function writeSiteImagesJSON($data) {
    $data['lastUpdated'] = date('Y-m-d');
    
    $json = json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
    if ($json === false) {
        return false;
    }
    
    $result = file_put_contents(SITE_IMAGES_JSON, $json);
    return $result !== false;
}

/**
 * Cria backup do site_images.json
 * @return string|false
 */
function backupSiteImagesJSON() {
    if (!file_exists(SITE_IMAGES_JSON)) {
        return false;
    }
    
    if (!is_dir(SITE_IMAGES_BACKUP_DIR)) {
        mkdir(SITE_IMAGES_BACKUP_DIR, 0755, true);
    }
    
    $timestamp = date('Y-m-d_H-i-s');
    $backupFile = SITE_IMAGES_BACKUP_DIR . "site_images_backup_{$timestamp}.json";
    
    if (!copy(SITE_IMAGES_JSON, $backupFile)) {
        return false;
    }
    
    // Limpar backups antigos (manter 10)
    $files = glob(SITE_IMAGES_BACKUP_DIR . 'site_images_backup_*.json');
    if (count($files) > 10) {
        usort($files, function($a, $b) {
            return filemtime($a) - filemtime($b);
        });
        $toDelete = count($files) - 10;
        for ($i = 0; $i < $toDelete; $i++) {
            unlink($files[$i]);
        }
    }
    
    return basename($backupFile);
}

/**
 * Valida categoria de site
 * @param string $category
 * @return bool
 */
function isValidSiteCategory($category) {
    return in_array($category, SITE_CATEGORIES);
}

/**
 * Obtém caminho físico para uma categoria
 * @param string $category
 * @param string $subcategory (opcional)
 * @return string
 */
function getCategoryPath($category, $subcategory = null) {
    $basePath = IMAGES_BASE_DIR . $category . '/';
    
    if ($subcategory && $category === 'icons') {
        $basePath .= $subcategory . '/';
    }
    
    return $basePath;
}

/**
 * Escaneia diretório de imagens de uma categoria
 * @param string $category
 * @param string $subcategory (opcional)
 * @return array
 */
function scanCategoryImages($category, $subcategory = null) {
    $path = getCategoryPath($category, $subcategory);
    
    if (!is_dir($path)) {
        return [];
    }
    
    $files = scandir($path);
    $images = [];
    
    foreach ($files as $file) {
        if ($file === '.' || $file === '..') {
            continue;
        }
        
        $ext = strtolower(pathinfo($file, PATHINFO_EXTENSION));
        $validExts = ['jpg', 'jpeg', 'png', 'webp', 'svg', 'ico'];
        
        if (in_array($ext, $validExts)) {
            $images[] = $file;
        }
    }
    
    return $images;
}

/**
 * Escaneia TODAS as categorias e retorna estatísticas
 * @return array
 */
function scanAllSiteImages() {
    $siteData = readSiteImagesJSON();
    if (!$siteData) {
        return false;
    }
    
    $result = [];
    
    foreach (SITE_CATEGORIES as $category) {
        if ($category === 'icons') {
            // Icons tem subcategorias
            $result[$category] = [];
            foreach (ICONS_SUBCATEGORIES as $sub) {
                $available = scanCategoryImages($category, $sub);
                $inJSON = [];
                
                if (isset($siteData['categories'][$category]['subcategories'][$sub]['images'])) {
                    foreach ($siteData['categories'][$category]['subcategories'][$sub]['images'] as $img) {
                        $inJSON[] = $img['filename'];
                    }
                }
                
                $result[$category][$sub] = [
                    'available' => $available,
                    'inJSON' => $inJSON,
                    'orphans' => array_diff($available, $inJSON),
                    'missing' => array_diff($inJSON, $available)
                ];
            }
        } elseif ($category === 'concepts') {
            // Concepts já tem seu próprio sistema, pular
            continue;
        } else {
            // Categorias simples
            $available = scanCategoryImages($category);
            $inJSON = [];
            
            if (isset($siteData['categories'][$category]['images'])) {
                foreach ($siteData['categories'][$category]['images'] as $img) {
                    $inJSON[] = $img['filename'];
                }
            }
            
            $result[$category] = [
                'available' => $available,
                'inJSON' => $inJSON,
                'orphans' => array_diff($available, $inJSON),
                'missing' => array_diff($inJSON, $available)
            ];
        }
    }
    
    return $result;
}

/**
 * Adiciona entrada ao site_images.json
 * @param array $siteData
 * @param string $category
 * @param string $subcategory (opcional)
 * @param array $entry
 * @return array|false
 */
function addSiteImageEntry($siteData, $category, $subcategory, $entry) {
    if (!isValidSiteCategory($category)) {
        return false;
    }
    
    if (empty($entry['filename']) || empty($entry['title'])) {
        return false;
    }
    
    $newEntry = [
        'filename' => $entry['filename'],
        'title' => $entry['title'],
        'description' => isset($entry['description']) ? $entry['description'] : '',
        'usage' => isset($entry['usage']) ? $entry['usage'] : '',
        'tags' => isset($entry['tags']) && is_array($entry['tags']) ? $entry['tags'] : []
    ];
    
    // Adicionar classType para categoria "classes"
    if ($category === 'classes' && isset($entry['classType'])) {
        $newEntry['classType'] = $entry['classType'];
    }
    
    if ($category === 'icons' && $subcategory) {
        if (!in_array($subcategory, ICONS_SUBCATEGORIES)) {
            return false;
        }
        $siteData['categories'][$category]['subcategories'][$subcategory]['images'][] = $newEntry;
    } else {
        $siteData['categories'][$category]['images'][] = $newEntry;
    }
    
    return $siteData;
}

/**
 * Atualiza entrada no site_images.json
 * @param array $siteData
 * @param string $category
 * @param string $subcategory (opcional)
 * @param string $filename
 * @param array $updates
 * @return array|false
 */
function updateSiteImageEntry($siteData, $category, $subcategory, $filename, $updates) {
    if (!isValidSiteCategory($category)) {
        return false;
    }
    
    if ($category === 'icons' && $subcategory) {
        $images = &$siteData['categories'][$category]['subcategories'][$subcategory]['images'];
    } else {
        $images = &$siteData['categories'][$category]['images'];
    }
    
    foreach ($images as &$image) {
        if ($image['filename'] === $filename) {
            if (isset($updates['title'])) $image['title'] = $updates['title'];
            if (isset($updates['description'])) $image['description'] = $updates['description'];
            if (isset($updates['usage'])) $image['usage'] = $updates['usage'];
            if (isset($updates['tags'])) $image['tags'] = $updates['tags'];
            if (isset($updates['classType'])) $image['classType'] = $updates['classType'];
            return $siteData;
        }
    }
    
    return false;
}

/**
 * Remove entrada do site_images.json
 * @param array $siteData
 * @param string $category
 * @param string $subcategory (opcional)
 * @param string $filename
 * @return array|false
 */
function deleteSiteImageEntry($siteData, $category, $subcategory, $filename) {
    if (!isValidSiteCategory($category)) {
        return false;
    }
    
    if ($category === 'icons' && $subcategory) {
        $images = &$siteData['categories'][$category]['subcategories'][$subcategory]['images'];
    } else {
        $images = &$siteData['categories'][$category]['images'];
    }
    
    foreach ($images as $index => $image) {
        if ($image['filename'] === $filename) {
            array_splice($images, $index, 1);
            return $siteData;
        }
    }
    
    return false;
}

/**
 * Deleta arquivo físico
 * @param string $category
 * @param string $subcategory (opcional)
 * @param string $filename
 * @return bool
 */
function deleteSiteImageFile($category, $subcategory, $filename) {
    $path = getCategoryPath($category, $subcategory) . $filename;
    
    if (file_exists($path)) {
        return unlink($path);
    }
    
    return false;
}
?>
