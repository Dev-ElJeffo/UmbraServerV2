<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'message' => 'Method Not Allowed']);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?? [];
require_once __DIR__ . '/require_admin_auth.php';
require_once __DIR__ . '/../../helpers/item_visual_helper.php';
requireAdminAuth($data);

$itemId = (int)($data['item_id'] ?? 0);
if ($itemId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'item_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();

    $stmt = $pdo->prepare('SELECT item_id FROM item_templates WHERE item_id = :id LIMIT 1');
    $stmt->execute([':id' => $itemId]);
    if (!$stmt->fetch(PDO::FETCH_ASSOC)) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Item não encontrado'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $validItemTypes = ['weapon', 'armor', 'consumable', 'material', 'quest', 'misc'];
    $validRarities = ['common', 'uncommon', 'rare', 'epic', 'legendary'];
    $validEquipmentSlots = ['none', 'head', 'chest', 'legs', 'feet', 'hands', 'main_hand', 'off_hand', 'ring', 'amulet', 'necklace', 'earring', 'bracelet', 'mount'];
    $validItemCategories = ['equipment', 'consumable', 'material', 'upgrade', 'quest', 'misc'];

    $sets = [];
    $params = [':item_id' => $itemId];

    $fieldMap = [
        'item_name' => 'item_name',
        'item_description' => 'item_description',
        'item_type' => 'item_type',
        'item_subtype' => 'item_subtype',
        'icon_path' => 'icon_path',
        'skeletal_mesh_path' => 'skeletal_mesh_path',
        'max_stack_size' => 'max_stack_size',
        'equipment_slot' => 'equipment_slot',
        'required_level' => 'required_level',
        'rarity' => 'rarity',
        'value' => 'value',
        'weight' => 'weight',
        'use_cooldown_ms' => 'use_cooldown_ms',
        'can_be_refined' => 'can_be_refined',
        'tradeable' => 'tradeable',
        'item_category' => 'item_category',
    ];

    foreach ($fieldMap as $payloadField => $dbField) {
        if (!array_key_exists($payloadField, $data)) {
            continue;
        }

        $value = $data[$payloadField];
        if ($payloadField === 'item_type' && !in_array($value, $validItemTypes, true)) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'item_type inválido'], JSON_UNESCAPED_UNICODE);
            exit;
        }
        if ($payloadField === 'rarity' && !in_array($value, $validRarities, true)) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'rarity inválido'], JSON_UNESCAPED_UNICODE);
            exit;
        }
        if ($payloadField === 'equipment_slot' && !in_array($value, $validEquipmentSlots, true)) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'equipment_slot inválido'], JSON_UNESCAPED_UNICODE);
            exit;
        }
        if ($payloadField === 'item_category' && !in_array($value, $validItemCategories, true)) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'item_category inválido'], JSON_UNESCAPED_UNICODE);
            exit;
        }

        if (in_array($payloadField, ['max_stack_size', 'required_level', 'value', 'use_cooldown_ms'], true)) {
            $value = max(0, (int)$value);
        } elseif ($payloadField === 'weight') {
            $value = max(0, (float)$value);
        } elseif (in_array($payloadField, ['can_be_refined', 'tradeable'], true)) {
            $value = $value ? 1 : 0;
        } elseif ($payloadField === 'item_name') {
            $value = trim((string)$value);
        } elseif (is_string($value)) {
            $value = trim($value);
        }

        $param = ':' . $payloadField;
        $sets[] = "{$dbField} = {$param}";
        $params[$param] = $value;
    }

    if (array_key_exists('stats', $data)) {
        $stats = is_array($data['stats']) ? $data['stats'] : [];
        if (isset($stats['physical_attack']) && !isset($stats['attack'])) {
            $stats['attack'] = $stats['physical_attack'];
        }
        if (isset($stats['physical_defense']) && !isset($stats['defense'])) {
            $stats['defense'] = $stats['physical_defense'];
        }

        $statsClean = [];
        foreach ($stats as $key => $value) {
            if ($value !== null && $value !== '' && $value !== 0 && $value !== 0.0) {
                $statsClean[$key] = $value;
            }
        }

        $sets[] = 'stats_json = :stats_json';
        $params[':stats_json'] = !empty($statsClean)
            ? json_encode($statsClean, JSON_UNESCAPED_UNICODE | JSON_NUMERIC_CHECK)
            : null;
    }

    if (array_key_exists('visual_meshes_json', $data) && item_templates_has_visual_meshes_json($pdo)) {
        $visualValidation = validate_item_visual_meshes_for_storage($data['visual_meshes_json']);
        if (!$visualValidation['ok']) {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => $visualValidation['error'] ?? 'visual_meshes_json inválido'], JSON_UNESCAPED_UNICODE);
            exit;
        }
        $sets[] = 'visual_meshes_json = :visual_meshes_json';
        $params[':visual_meshes_json'] = $visualValidation['json'];
    }

    if (empty($sets)) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Nenhum campo para atualizar'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $sql = 'UPDATE item_templates SET ' . implode(', ', $sets) . ' WHERE item_id = :item_id';
    $upd = $pdo->prepare($sql);
    $upd->execute($params);

    echo json_encode([
        'success' => true,
        'message' => 'Item atualizado com sucesso',
        'item_id' => $itemId,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/update_item] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
