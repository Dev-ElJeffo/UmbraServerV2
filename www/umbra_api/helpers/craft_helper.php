<?php
/**
 * Helpers do sistema de crafting (receitas, learn, craft).
 */

require_once __DIR__ . '/../config/database.php';
require_once __DIR__ . '/item_weapon_class_helper.php';

function craftValidCategories(): array
{
    return ['material', 'consumable', 'equipment'];
}

function craftNormalizeCategory(?string $cat): string
{
    $c = strtolower(trim((string)$cat));
    if (!in_array($c, craftValidCategories(), true)) {
        throw new InvalidArgumentException('craft_category inválido (material|consumable|equipment)');
    }
    return $c;
}

function craftDecodeAllowedClasses($raw): array
{
    if ($raw === null || $raw === '') {
        return [];
    }
    if (is_array($raw)) {
        return array_values(array_map('intval', $raw));
    }
    $decoded = json_decode((string)$raw, true);
    if (!is_array($decoded)) {
        return [];
    }
    return array_values(array_map('intval', $decoded));
}

function craftPlayerMeetsClass(array $allowedClassIds, int $playerClassId): bool
{
    if (count($allowedClassIds) === 0) {
        return true;
    }
    return in_array($playerClassId, $allowedClassIds, true);
}

function craftCountPlayerItem(PDO $pdo, int $playerId, int $itemTemplateId): int
{
    $stmt = $pdo->prepare('
        SELECT COALESCE(SUM(quantity), 0)
        FROM player_inventory
        WHERE player_id = ? AND item_template_id = ?
          AND auction_listing_id IS NULL
          AND slot_index >= 0 AND slot_index < 50
          AND (is_equipped = 0 OR is_equipped IS NULL OR is_equipped = FALSE)
    ');
    $stmt->execute([$playerId, $itemTemplateId]);
    return (int)$stmt->fetchColumn();
}

/** Remove qty de stacks na bag (0-49), não equipados. */
function craftRemoveItemsByTemplate(PDO $pdo, int $playerId, int $itemTemplateId, int $required): bool
{
    if ($itemTemplateId <= 0 || $required <= 0) {
        return true;
    }
    if (craftCountPlayerItem($pdo, $playerId, $itemTemplateId) < $required) {
        return false;
    }
    $remaining = $required;
    $stmt = $pdo->prepare('
        SELECT inventory_id, quantity FROM player_inventory
        WHERE player_id = ? AND item_template_id = ?
          AND auction_listing_id IS NULL
          AND slot_index >= 0 AND slot_index < 50
          AND (is_equipped = 0 OR is_equipped IS NULL OR is_equipped = FALSE)
        ORDER BY slot_index ASC
    ');
    $stmt->execute([$playerId, $itemTemplateId]);
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        if ($remaining <= 0) {
            break;
        }
        $invId = (int)$row['inventory_id'];
        $qty = (int)$row['quantity'];
        if ($qty <= $remaining) {
            $pdo->prepare('DELETE FROM player_inventory WHERE inventory_id = ?')->execute([$invId]);
            $remaining -= $qty;
        } else {
            $pdo->prepare('UPDATE player_inventory SET quantity = quantity - ? WHERE inventory_id = ?')
                ->execute([$remaining, $invId]);
            $remaining = 0;
        }
    }
    return $remaining <= 0;
}

/**
 * Consome qty de um inventory_id específico (deve pertencer ao player e ao template).
 */
function craftConsumeInventoryStack(PDO $pdo, int $playerId, int $inventoryId, int $itemTemplateId, int $qty): bool
{
    if ($inventoryId <= 0 || $qty <= 0) {
        return false;
    }
    $stmt = $pdo->prepare('
        SELECT inventory_id, quantity, item_template_id, slot_index, is_equipped
        FROM player_inventory
        WHERE inventory_id = ? AND player_id = ? AND auction_listing_id IS NULL
        FOR UPDATE
    ');
    $stmt->execute([$inventoryId, $playerId]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    if (!$row) {
        return false;
    }
    if ((int)$row['item_template_id'] !== $itemTemplateId) {
        return false;
    }
    $slot = (int)$row['slot_index'];
    if ($slot < 0 || $slot >= 50) {
        return false;
    }
    if (!empty($row['is_equipped'])) {
        return false;
    }
    $have = (int)$row['quantity'];
    if ($have < $qty) {
        return false;
    }
    if ($have === $qty) {
        $pdo->prepare('DELETE FROM player_inventory WHERE inventory_id = ?')->execute([$inventoryId]);
    } else {
        $pdo->prepare('UPDATE player_inventory SET quantity = quantity - ? WHERE inventory_id = ?')
            ->execute([$qty, $inventoryId]);
    }
    return true;
}

function craftLoadIngredients(PDO $pdo, int $recipeId): array
{
    $allowedSelect = item_templates_allowed_class_select_sql($pdo, 'it');
    $stmt = $pdo->prepare("
        SELECT i.ingredient_id, i.slot_index, i.item_template_id, i.quantity,
               it.item_name, it.icon_path, it.item_description, it.item_type, it.item_subtype,
               it.equipment_slot, it.rarity, it.required_level, it.max_stack_size, it.value, it.weight,
               it.can_be_refined, it.tradeable, it.stats_json{$allowedSelect}
        FROM craft_recipe_ingredients i
        JOIN item_templates it ON it.item_id = i.item_template_id
        WHERE i.recipe_id = ?
        ORDER BY i.slot_index ASC
    ");
    $stmt->execute([$recipeId]);
    $out = [];
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        $item = [
            'ingredient_id' => (int)$row['ingredient_id'],
            'slot_index' => (int)$row['slot_index'],
            'item_template_id' => (int)$row['item_template_id'],
            'quantity' => (int)$row['quantity'],
            'item_name' => $row['item_name'] ?? '',
            'icon_path' => $row['icon_path'] ?? '',
        ];
        craftAppendCatalogFields($item, $row);
        $out[] = $item;
    }
    return $out;
}

/**
 * Campos de catálogo para ParseItemTemplate no cliente (tooltip preview).
 * $row deve ter colunas de item_templates (com ou sem prefixo $prefix, ex. "result_").
 */
function craftAppendCatalogFields(array &$out, array $row, string $prefix = ''): void
{
    $get = static function (string $key) use ($row, $prefix) {
        $prefixed = $prefix . $key;
        if (array_key_exists($prefixed, $row)) {
            return $row[$prefixed];
        }
        if ($prefix === '' && array_key_exists($key, $row)) {
            return $row[$key];
        }
        return null;
    };

    $desc = $get('item_description');
    if ($desc !== null) {
        $out['item_description'] = (string)$desc;
    }
    $type = $get('item_type');
    if ($type !== null && $type !== '') {
        $out['item_type'] = $type;
    }
    $subtype = $get('item_subtype');
    if ($subtype !== null) {
        $out['item_subtype'] = (string)$subtype;
    }
    $equip = $get('equipment_slot');
    if ($equip !== null && $equip !== '') {
        $out['equipment_slot'] = $equip;
    }
    $rarity = $get('rarity');
    if ($rarity !== null && $rarity !== '') {
        $out['rarity'] = $rarity;
    }
    $reqLevel = $get('required_level');
    if ($reqLevel !== null) {
        $out['required_level'] = (int)$reqLevel;
    }
    $maxStack = $get('max_stack_size');
    if ($maxStack !== null) {
        $out['max_stack_size'] = (int)$maxStack;
    } elseif (array_key_exists('result_max_stack', $row) && $prefix === 'result_') {
        $out['max_stack_size'] = (int)$row['result_max_stack'];
    }
    $value = $get('value');
    if ($value !== null) {
        $out['value'] = (int)$value;
    }
    $weight = $get('weight');
    if ($weight !== null) {
        $out['weight'] = (float)$weight;
    }
    $refine = $get('can_be_refined');
    if ($refine !== null) {
        $out['can_be_refined'] = (bool)(int)$refine;
    }
    $trade = $get('tradeable');
    if ($trade !== null) {
        $out['tradeable'] = (bool)(int)$trade;
    }

    $statsJson = $get('stats_json');
    if ($statsJson !== null && $statsJson !== '') {
        $decoded = is_array($statsJson) ? $statsJson : json_decode((string)$statsJson, true);
        $out['stats'] = is_array($decoded) ? $decoded : new stdClass();
    } elseif ($statsJson !== null) {
        $out['stats'] = new stdClass();
    }

    $allowedRaw = $get('allowed_class_ids');
    if ($allowedRaw !== null || array_key_exists($prefix . 'allowed_class_ids', $row)
        || ($prefix === '' && array_key_exists('allowed_class_ids', $row))) {
        append_allowed_class_fields($out, $allowedRaw);
    }
}

function craftFormatResultItem(array $row): array
{
    $tplId = (int)($row['result_item_template_id'] ?? 0);
    $out = [
        'item_template_id' => $tplId,
        'item_name' => $row['result_item_name'] ?? '',
        'icon_path' => $row['result_icon_path'] ?? '',
    ];
    if ($tplId > 0) {
        craftAppendCatalogFields($out, $row, 'result_');
    }
    return $out;
}

function craftFormatRecipeRow(PDO $pdo, array $row, int $playerId = 0, int $playerLevel = 1, int $playerClassId = 0, ?array $knownSet = null): array
{
    $recipeId = (int)$row['recipe_id'];
    $category = (string)$row['craft_category'];
    $recipeItemId = isset($row['recipe_item_template_id']) && $row['recipe_item_template_id'] !== null
        ? (int)$row['recipe_item_template_id']
        : 0;
    $allowed = craftDecodeAllowedClasses($row['allowed_class_ids'] ?? null);
    $isDefault = $recipeItemId <= 0;
    $isKnown = false;
    $hasRecipeItem = false;
    $recipeItemCount = 0;

    if ($playerId > 0 && $recipeItemId > 0) {
        $recipeItemCount = craftCountPlayerItem($pdo, $playerId, $recipeItemId);
        $hasRecipeItem = $recipeItemCount > 0;
    }
    if ($playerId > 0 && !$isDefault && in_array($category, ['material', 'consumable'], true)) {
        if ($knownSet !== null) {
            $isKnown = isset($knownSet[$recipeId]);
        } else {
            $k = $pdo->prepare('SELECT 1 FROM player_known_craft_recipes WHERE player_id = ? AND recipe_id = ? LIMIT 1');
            $k->execute([$playerId, $recipeId]);
            $isKnown = (bool)$k->fetchColumn();
        }
    }

    $visible = false;
    $canLearn = false;
    $canCraft = false;
    if ($isDefault) {
        $visible = true;
        $canCraft = true;
    } elseif ($category === 'equipment') {
        $visible = $hasRecipeItem;
        $canCraft = $hasRecipeItem;
    } else {
        // material / consumable learnable
        if ($isKnown) {
            $visible = true;
            $canCraft = true;
        } elseif ($hasRecipeItem) {
            $visible = true;
            $canLearn = true;
            $canCraft = false;
        }
    }

    if ($playerLevel < (int)$row['min_level'] || !craftPlayerMeetsClass($allowed, $playerClassId)) {
        $canCraft = false;
        $canLearn = false;
    }

    $ingredients = craftLoadIngredients($pdo, $recipeId);

    return [
        'recipe_id' => $recipeId,
        'recipe_key' => $row['recipe_key'] ?? '',
        'display_name' => $row['display_name'] ?? '',
        'craft_category' => $category,
        'result_item_template_id' => (int)$row['result_item_template_id'],
        'result_item_name' => $row['result_item_name'] ?? '',
        'result_icon_path' => $row['result_icon_path'] ?? '',
        'result_quantity' => (int)($row['result_quantity'] ?? 1),
        'result_item' => craftFormatResultItem($row),
        'gold_cost' => (int)($row['gold_cost'] ?? 0),
        'min_level' => (int)($row['min_level'] ?? 1),
        'recipe_item_template_id' => $recipeItemId,
        'recipe_item_name' => $row['recipe_item_name'] ?? '',
        'allowed_class_ids' => $allowed,
        'is_active' => (bool)(int)($row['is_active'] ?? 1),
        'sort_order' => (int)($row['sort_order'] ?? 0),
        'is_default' => $isDefault,
        'is_known' => $isKnown,
        'has_recipe_item' => $hasRecipeItem,
        'recipe_item_count' => $recipeItemCount,
        'is_visible' => $visible,
        'can_learn' => $canLearn,
        'can_craft' => $canCraft,
        'ingredients' => $ingredients,
    ];
}

function craftLoadPlayerKnownSet(PDO $pdo, int $playerId): array
{
    $stmt = $pdo->prepare('SELECT recipe_id FROM player_known_craft_recipes WHERE player_id = ?');
    $stmt->execute([$playerId]);
    $set = [];
    foreach ($stmt->fetchAll(PDO::FETCH_COLUMN) as $id) {
        $set[(int)$id] = true;
    }
    return $set;
}

function craftFetchRecipe(PDO $pdo, int $recipeId): ?array
{
    $resultAllowed = item_templates_has_allowed_class_ids($pdo)
        ? ', it.allowed_class_ids AS result_allowed_class_ids'
        : '';
    $stmt = $pdo->prepare("
        SELECT r.*,
               it.item_name AS result_item_name,
               it.icon_path AS result_icon_path,
               it.max_stack_size AS result_max_stack,
               it.item_description AS result_item_description,
               it.item_type AS result_item_type,
               it.item_subtype AS result_item_subtype,
               it.equipment_slot AS result_equipment_slot,
               it.rarity AS result_rarity,
               it.required_level AS result_required_level,
               it.value AS result_value,
               it.weight AS result_weight,
               it.can_be_refined AS result_can_be_refined,
               it.tradeable AS result_tradeable,
               it.stats_json AS result_stats_json{$resultAllowed},
               rit.item_name AS recipe_item_name
        FROM craft_recipes r
        JOIN item_templates it ON it.item_id = r.result_item_template_id
        LEFT JOIN item_templates rit ON rit.item_id = r.recipe_item_template_id
        WHERE r.recipe_id = ?
        LIMIT 1
    ");
    $stmt->execute([$recipeId]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    return $row ?: null;
}

/**
 * Verifica se cabe result_qty * craft_count do template na bag (stack ou slot livre).
 */
function craftCanFitResult(PDO $pdo, int $playerId, int $resultTemplateId, int $totalQty, int $maxStack): bool
{
    if ($totalQty <= 0) {
        return true;
    }
    $maxStack = max(1, $maxStack);

    // Espaço em stacks existentes
    $stmt = $pdo->prepare('
        SELECT quantity FROM player_inventory
        WHERE player_id = ? AND item_template_id = ?
          AND auction_listing_id IS NULL
          AND slot_index >= 0 AND slot_index < 50
    ');
    $stmt->execute([$playerId, $resultTemplateId]);
    $remaining = $totalQty;
    foreach ($stmt->fetchAll(PDO::FETCH_COLUMN) as $q) {
        $free = $maxStack - (int)$q;
        if ($free > 0) {
            $remaining -= min($free, $remaining);
        }
        if ($remaining <= 0) {
            return true;
        }
    }

    // Slots livres
    $occ = $pdo->prepare('
        SELECT slot_index FROM player_inventory
        WHERE player_id = ? AND slot_index >= 0 AND slot_index < 50 AND auction_listing_id IS NULL
    ');
    $occ->execute([$playerId]);
    $used = [];
    foreach ($occ->fetchAll(PDO::FETCH_COLUMN) as $s) {
        $used[(int)$s] = true;
    }
    $freeSlots = 0;
    for ($i = 0; $i < 50; $i++) {
        if (!isset($used[$i])) {
            $freeSlots++;
        }
    }
    $needSlots = (int)ceil($remaining / $maxStack);
    return $freeSlots >= $needSlots;
}

/**
 * Insere resultado na bag (stack + novos slots). Assume espaço já validado.
 */
function craftGrantResult(PDO $pdo, int $playerId, int $resultTemplateId, int $totalQty, int $maxStack): void
{
    $maxStack = max(1, $maxStack);
    $remaining = $totalQty;

    $stmt = $pdo->prepare('
        SELECT inventory_id, quantity FROM player_inventory
        WHERE player_id = ? AND item_template_id = ?
          AND auction_listing_id IS NULL
          AND slot_index >= 0 AND slot_index < 50
        ORDER BY slot_index ASC
        FOR UPDATE
    ');
    $stmt->execute([$playerId, $resultTemplateId]);
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        if ($remaining <= 0) {
            break;
        }
        $free = $maxStack - (int)$row['quantity'];
        if ($free <= 0) {
            continue;
        }
        $add = min($free, $remaining);
        $pdo->prepare('UPDATE player_inventory SET quantity = quantity + ? WHERE inventory_id = ?')
            ->execute([$add, (int)$row['inventory_id']]);
        $remaining -= $add;
    }

    while ($remaining > 0) {
        $occ = $pdo->prepare('
            SELECT slot_index FROM player_inventory
            WHERE player_id = ? AND slot_index >= 0 AND slot_index < 50 AND auction_listing_id IS NULL
        ');
        $occ->execute([$playerId]);
        $used = [];
        foreach ($occ->fetchAll(PDO::FETCH_COLUMN) as $s) {
            $used[(int)$s] = true;
        }
        $slot = -1;
        for ($i = 0; $i < 50; $i++) {
            if (!isset($used[$i])) {
                $slot = $i;
                break;
            }
        }
        if ($slot < 0) {
            throw new RuntimeException('Inventário cheio ao conceder resultado do craft');
        }
        $add = min($maxStack, $remaining);
        $pdo->prepare('
            INSERT INTO player_inventory (player_id, item_template_id, quantity, slot_index, is_equipped, durability)
            VALUES (?, ?, ?, ?, 0, 100.0)
        ')->execute([$playerId, $resultTemplateId, $add, $slot]);
        $remaining -= $add;
    }
}

function craftPlayerLevelAndClass(PDO $pdo, int $playerId): array
{
    $stmt = $pdo->prepare('SELECT level, class_id, gold FROM players WHERE id = ? LIMIT 1');
    $stmt->execute([$playerId]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    if (!$row) {
        throw new RuntimeException('Player não encontrado');
    }
    return [
        'level' => (int)($row['level'] ?? 1),
        'class_id' => (int)($row['class_id'] ?? 0),
        'gold' => (int)($row['gold'] ?? 0),
    ];
}

/**
 * @param array $slotBindings map slot_index => inventory_id (opcional; se vazio, remove por template)
 */
function craftExecute(PDO $pdo, int $playerId, int $recipeId, int $craftCount, array $slotBindings = []): array
{
    if ($craftCount < 1) {
        throw new InvalidArgumentException('craft_count deve ser >= 1');
    }
    if ($craftCount > 99) {
        throw new InvalidArgumentException('craft_count máximo é 99');
    }

    $recipe = craftFetchRecipe($pdo, $recipeId);
    if (!$recipe || !(int)$recipe['is_active']) {
        throw new InvalidArgumentException('Receita inválida ou inativa');
    }

    $info = craftPlayerLevelAndClass($pdo, $playerId);
    $formatted = craftFormatRecipeRow($pdo, $recipe, $playerId, $info['level'], $info['class_id']);
    if (!$formatted['can_craft']) {
        throw new InvalidArgumentException('Você não pode craftar esta receita agora');
    }

    $ingredients = $formatted['ingredients'];
    if (count($ingredients) === 0) {
        throw new InvalidArgumentException('Receita sem ingredientes');
    }

    $goldCostTotal = (int)$recipe['gold_cost'] * $craftCount;
    if ($info['gold'] < $goldCostTotal) {
        throw new InvalidArgumentException('Ouro insuficiente');
    }

    $resultTemplateId = (int)$recipe['result_item_template_id'];
    $resultPerCraft = max(1, (int)$recipe['result_quantity']);
    $totalResult = $resultPerCraft * $craftCount;
    $maxStack = max(1, (int)($recipe['result_max_stack'] ?? 1));

    if (!craftCanFitResult($pdo, $playerId, $resultTemplateId, $totalResult, $maxStack)) {
        throw new InvalidArgumentException('Inventário cheio');
    }

    // Validar insumos
    foreach ($ingredients as $ing) {
        $need = (int)$ing['quantity'] * $craftCount;
        $slotIdx = (int)$ing['slot_index'];
        $tpl = (int)$ing['item_template_id'];
        if (isset($slotBindings[$slotIdx]) && (int)$slotBindings[$slotIdx] > 0) {
            $invId = (int)$slotBindings[$slotIdx];
            $chk = $pdo->prepare('
                SELECT quantity, item_template_id FROM player_inventory
                WHERE inventory_id = ? AND player_id = ? AND auction_listing_id IS NULL
            ');
            $chk->execute([$invId, $playerId]);
            $stack = $chk->fetch(PDO::FETCH_ASSOC);
            if (!$stack || (int)$stack['item_template_id'] !== $tpl || (int)$stack['quantity'] < $need) {
                throw new InvalidArgumentException('Ingrediente insuficiente no slot ' . $slotIdx);
            }
        } elseif (craftCountPlayerItem($pdo, $playerId, $tpl) < $need) {
            throw new InvalidArgumentException('Ingrediente insuficiente: ' . ($ing['item_name'] ?? $tpl));
        }
    }

    $category = (string)$recipe['craft_category'];
    $recipeItemId = isset($recipe['recipe_item_template_id']) ? (int)$recipe['recipe_item_template_id'] : 0;
    if ($category === 'equipment' && $recipeItemId > 0) {
        if (craftCountPlayerItem($pdo, $playerId, $recipeItemId) < $craftCount) {
            throw new InvalidArgumentException('Receitas insuficientes para craftar essa quantidade');
        }
    }

    // Consumir
    foreach ($ingredients as $ing) {
        $need = (int)$ing['quantity'] * $craftCount;
        $slotIdx = (int)$ing['slot_index'];
        $tpl = (int)$ing['item_template_id'];
        if (isset($slotBindings[$slotIdx]) && (int)$slotBindings[$slotIdx] > 0) {
            if (!craftConsumeInventoryStack($pdo, $playerId, (int)$slotBindings[$slotIdx], $tpl, $need)) {
                throw new RuntimeException('Falha ao consumir ingrediente do slot ' . $slotIdx);
            }
        } else {
            if (!craftRemoveItemsByTemplate($pdo, $playerId, $tpl, $need)) {
                throw new RuntimeException('Falha ao consumir ingrediente ' . $tpl);
            }
        }
    }

    if ($category === 'equipment' && $recipeItemId > 0) {
        if (!craftRemoveItemsByTemplate($pdo, $playerId, $recipeItemId, $craftCount)) {
            throw new RuntimeException('Falha ao consumir item-receita');
        }
    }

    $goldStmt = $pdo->prepare('UPDATE players SET gold = gold - ? WHERE id = ? AND gold >= ?');
    $goldStmt->execute([$goldCostTotal, $playerId, $goldCostTotal]);
    if ($goldStmt->rowCount() < 1) {
        throw new RuntimeException('Falha ao debitar ouro');
    }
    $g2 = $pdo->prepare('SELECT gold FROM players WHERE id = ?');
    $g2->execute([$playerId]);
    $newGold = (int)$g2->fetchColumn();

    craftGrantResult($pdo, $playerId, $resultTemplateId, $totalResult, $maxStack);

    return [
        'recipe_id' => $recipeId,
        'craft_count' => $craftCount,
        'result_item_template_id' => $resultTemplateId,
        'result_quantity' => $totalResult,
        'gold_spent' => $goldCostTotal,
        'new_gold' => $newGold,
        'recipe_items_consumed' => ($category === 'equipment' && $recipeItemId > 0) ? $craftCount : 0,
    ];
}

function craftLearnRecipe(PDO $pdo, int $playerId, int $recipeId): array
{
    $recipe = craftFetchRecipe($pdo, $recipeId);
    if (!$recipe || !(int)$recipe['is_active']) {
        throw new InvalidArgumentException('Receita inválida ou inativa');
    }
    $category = (string)$recipe['craft_category'];
    if (!in_array($category, ['material', 'consumable'], true)) {
        throw new InvalidArgumentException('Somente material/consumable podem ser aprendidos permanentemente');
    }
    $recipeItemId = isset($recipe['recipe_item_template_id']) ? (int)$recipe['recipe_item_template_id'] : 0;
    if ($recipeItemId <= 0) {
        throw new InvalidArgumentException('Esta receita é padrão e não precisa ser aprendida');
    }

    $info = craftPlayerLevelAndClass($pdo, $playerId);
    $allowed = craftDecodeAllowedClasses($recipe['allowed_class_ids'] ?? null);
    if ($info['level'] < (int)$recipe['min_level'] || !craftPlayerMeetsClass($allowed, $info['class_id'])) {
        throw new InvalidArgumentException('Nível ou classe insuficiente para aprender');
    }

    $chk = $pdo->prepare('SELECT 1 FROM player_known_craft_recipes WHERE player_id = ? AND recipe_id = ?');
    $chk->execute([$playerId, $recipeId]);
    if ($chk->fetchColumn()) {
        throw new InvalidArgumentException('Receita já aprendida');
    }

    if (craftCountPlayerItem($pdo, $playerId, $recipeItemId) < 1) {
        throw new InvalidArgumentException('Você precisa do item-receita no inventário');
    }
    if (!craftRemoveItemsByTemplate($pdo, $playerId, $recipeItemId, 1)) {
        throw new RuntimeException('Falha ao consumir item-receita');
    }

    $pdo->prepare('INSERT INTO player_known_craft_recipes (player_id, recipe_id) VALUES (?, ?)')
        ->execute([$playerId, $recipeId]);

    return [
        'recipe_id' => $recipeId,
        'display_name' => $recipe['display_name'] ?? '',
        'learned' => true,
    ];
}
