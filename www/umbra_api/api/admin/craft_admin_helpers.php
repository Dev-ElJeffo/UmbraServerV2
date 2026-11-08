<?php
require_once __DIR__ . '/../../helpers/craft_helper.php';

function craft_admin_validate_payload(array $data, bool $requireId): array
{
    if ($requireId) {
        $id = (int)($data['recipe_id'] ?? 0);
        if ($id <= 0) {
            throw new InvalidArgumentException('recipe_id obrigatório');
        }
    }

    $key = trim((string)($data['recipe_key'] ?? ''));
    $name = trim((string)($data['display_name'] ?? ''));
    if ($key === '' || strlen($key) > 64) {
        throw new InvalidArgumentException('recipe_key obrigatório (máx 64)');
    }
    if ($name === '' || strlen($name) > 128) {
        throw new InvalidArgumentException('display_name obrigatório (máx 128)');
    }

    $category = craftNormalizeCategory($data['craft_category'] ?? '');
    $resultId = (int)($data['result_item_template_id'] ?? 0);
    if ($resultId <= 0) {
        throw new InvalidArgumentException('result_item_template_id obrigatório');
    }

    $resultQty = max(1, (int)($data['result_quantity'] ?? 1));
    $gold = max(0, (int)($data['gold_cost'] ?? 0));
    $minLevel = max(1, (int)($data['min_level'] ?? 1));
    $recipeItem = isset($data['recipe_item_template_id']) && $data['recipe_item_template_id'] !== '' && $data['recipe_item_template_id'] !== null
        ? (int)$data['recipe_item_template_id']
        : null;
    if ($recipeItem !== null && $recipeItem <= 0) {
        $recipeItem = null;
    }

    $allowed = [];
    if (isset($data['allowed_class_ids']) && is_array($data['allowed_class_ids'])) {
        $allowed = array_values(array_unique(array_map('intval', $data['allowed_class_ids'])));
        $allowed = array_values(array_filter($allowed, static fn($v) => $v > 0));
    }
    $isActive = isset($data['is_active']) ? (int)((bool)$data['is_active']) : 1;
    $sort = (int)($data['sort_order'] ?? 0);

    $ingredients = [];
    if (!isset($data['ingredients']) || !is_array($data['ingredients'])) {
        throw new InvalidArgumentException('ingredients[] obrigatório (1 a 5)');
    }
    $seenSlots = [];
    foreach ($data['ingredients'] as $ing) {
        if (!is_array($ing)) {
            continue;
        }
        $slot = (int)($ing['slot_index'] ?? -1);
        $tpl = (int)($ing['item_template_id'] ?? 0);
        $qty = max(1, (int)($ing['quantity'] ?? 1));
        if ($slot < 0 || $slot > 4) {
            throw new InvalidArgumentException('slot_index deve ser 0-4');
        }
        if ($tpl <= 0) {
            throw new InvalidArgumentException('ingredient item_template_id inválido');
        }
        if (isset($seenSlots[$slot])) {
            throw new InvalidArgumentException('slot_index duplicado: ' . $slot);
        }
        $seenSlots[$slot] = true;
        $ingredients[] = [
            'slot_index' => $slot,
            'item_template_id' => $tpl,
            'quantity' => $qty,
        ];
    }
    if (count($ingredients) < 1 || count($ingredients) > 5) {
        throw new InvalidArgumentException('Informe de 1 a 5 ingredientes');
    }
    usort($ingredients, static fn($a, $b) => $a['slot_index'] <=> $b['slot_index']);

    $fields = [
        'recipe_key' => $key,
        'display_name' => $name,
        'craft_category' => $category,
        'result_item_template_id' => $resultId,
        'result_quantity' => $resultQty,
        'gold_cost' => $gold,
        'min_level' => $minLevel,
        'recipe_item_template_id' => $recipeItem,
        'allowed_class_ids' => count($allowed) ? json_encode($allowed) : null,
        'is_active' => $isActive,
        'sort_order' => $sort,
    ];
    if ($requireId) {
        $fields['recipe_id'] = (int)$data['recipe_id'];
    }
    return ['fields' => $fields, 'ingredients' => $ingredients];
}

function craft_admin_replace_ingredients(PDO $pdo, int $recipeId, array $ingredients): void
{
    $pdo->prepare('DELETE FROM craft_recipe_ingredients WHERE recipe_id = ?')->execute([$recipeId]);
    $ins = $pdo->prepare('
        INSERT INTO craft_recipe_ingredients (recipe_id, slot_index, item_template_id, quantity)
        VALUES (?, ?, ?, ?)
    ');
    foreach ($ingredients as $ing) {
        $ins->execute([$recipeId, $ing['slot_index'], $ing['item_template_id'], $ing['quantity']]);
    }
}

function craft_admin_format_full(PDO $pdo, array $row): array
{
    $row['recipe_id'] = (int)$row['recipe_id'];
    $row['result_item_template_id'] = (int)$row['result_item_template_id'];
    $row['result_quantity'] = (int)$row['result_quantity'];
    $row['gold_cost'] = (int)$row['gold_cost'];
    $row['min_level'] = (int)$row['min_level'];
    $row['recipe_item_template_id'] = $row['recipe_item_template_id'] !== null ? (int)$row['recipe_item_template_id'] : null;
    $row['allowed_class_ids'] = craftDecodeAllowedClasses($row['allowed_class_ids'] ?? null);
    $row['is_active'] = (bool)(int)$row['is_active'];
    $row['sort_order'] = (int)$row['sort_order'];
    $row['ingredients'] = craftLoadIngredients($pdo, (int)$row['recipe_id']);
    return $row;
}
