<?php
/**
 * GET /api/inventory/get_item_templates.php
 * Obtém todos os templates de itens disponíveis no jogo
 * 
 * Query params opcionais:
 * - type: filtrar por tipo (weapon, armor, consumable, etc.)
 * - rarity: filtrar por raridade (common, uncommon, rare, epic, legendary)
 * - search: buscar por nome (case-insensitive)
 * 
 * Retorna:
 * - Lista de templates de itens
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';

// Parâmetros de filtro opcionais
$type = $_GET['type'] ?? null;
$rarity = $_GET['rarity'] ?? null;
$search = $_GET['search'] ?? null;
$idsParam = $_GET['ids'] ?? null;

try {
    $pdo = getConnection();
    
    // Construir query base
    $query = "SELECT * FROM item_templates WHERE 1=1";
    $params = [];
    
    // Aplicar filtros
    if ($type) {
        $query .= " AND item_type = :type";
        $params['type'] = $type;
    }
    
    if ($rarity) {
        $query .= " AND rarity = :rarity";
        $params['rarity'] = $rarity;
    }
    
    if ($search) {
        $query .= " AND (item_name LIKE :search OR item_description LIKE :search)";
        $params['search'] = "%$search%";
    }

    if ($idsParam !== null && $idsParam !== '') {
        $idList = [];
        foreach (explode(',', (string)$idsParam) as $raw) {
            $id = (int)trim($raw);
            if ($id > 0) {
                $idList[$id] = $id;
            }
        }
        if (!empty($idList)) {
            $placeholders = [];
            $i = 0;
            foreach ($idList as $id) {
                $key = ':id' . $i;
                $placeholders[] = $key;
                $params[$key] = $id;
                $i++;
            }
            $query .= ' AND item_id IN (' . implode(',', $placeholders) . ')';
        }
    }
    
    $query .= " ORDER BY rarity DESC, item_name ASC";
    
    $stmt = $pdo->prepare($query);
    $stmt->execute($params);
    $templates = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Processar JSON fields
    foreach ($templates as &$template) {
        // Decodificar stats_json
        if ($template['stats_json']) {
            $template['stats'] = json_decode($template['stats_json'], true);
            unset($template['stats_json']);
        } else {
            $template['stats'] = [];
        }
        
        // Converter valores numéricos
        $template['item_id'] = (int)$template['item_id'];
        // Alias para o parser do cliente UE (ParseItemTemplate usa item_template_id)
        $template['item_template_id'] = $template['item_id'];
        $template['max_stack_size'] = (int)$template['max_stack_size'];
        $template['required_level'] = (int)$template['required_level'];
        $template['value'] = (int)$template['value'];
        $template['weight'] = (float)$template['weight'];
    }
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'message' => 'Templates carregados com sucesso',
        'templates' => $templates,
        'total' => count($templates),
        'filters_applied' => [
            'type' => $type,
            'rarity' => $rarity,
            'search' => $search
        ]
    ], JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
    
} catch (PDOException $e) {
    error_log("Erro ao obter templates: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao carregar templates',
        'error' => $e->getMessage()
    ]);
}
?>

