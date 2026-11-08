<?php
/**
 * Helper: restrição de armas por classe (allowed_class_ids + subtypes canônicos).
 */

/** Subtypes canônicos de arma (main_hand nesta fase). */
function item_weapon_canonical_subtypes(): array
{
    return ['axe', 'cestus', 'scythe', 'dagger', 'sword', 'staff'];
}

/**
 * Mapa subtype -> class_id default (para UI do Manager).
 * 1 Barbarian, 2 Templar, 3 DarkMage, 4 Cleric/Alchemist, 5 Assassin, 6 Monk/Martial
 */
function item_weapon_subtype_default_class_map(): array
{
    return [
        'axe' => 1,
        'sword' => 2,
        'scythe' => 3,
        'staff' => 4,
        'dagger' => 5,
        'cestus' => 6,
    ];
}

function item_weapon_class_display_labels(): array
{
    return [
        1 => 'Barbarian',
        2 => 'Templar',
        3 => 'Dark Mage',
        4 => 'Alchemist (Cleric)',
        5 => 'Assassin',
        6 => 'Martial (Monk)',
    ];
}

function item_templates_has_allowed_class_ids(PDO $pdo): bool
{
    static $cached = null;
    if ($cached !== null) {
        return $cached;
    }
    try {
        $chk = $pdo->query("SHOW COLUMNS FROM item_templates LIKE 'allowed_class_ids'");
        $cached = $chk && $chk->rowCount() > 0;
    } catch (Exception $e) {
        $cached = false;
    }
    return $cached;
}

/**
 * Fragmento SQL para SELECT (ex.: ", it.allowed_class_ids") ou string vazia se a coluna não existir.
 */
function item_templates_allowed_class_select_sql(PDO $pdo, string $alias = 'it'): string
{
    if (!item_templates_has_allowed_class_ids($pdo)) {
        return '';
    }
    $alias = preg_replace('/[^a-zA-Z0-9_]/', '', $alias) ?: 'it';
    return ", {$alias}.allowed_class_ids";
}

/**
 * Anexa allowed_class_ids (null|int[]) e allow_all_classes ao array de resposta JSON.
 *
 * @param array<string,mixed> $row
 * @param mixed $rawAllowed Valor bruto da coluna JSON (ou já parseado); se null, usa $row['allowed_class_ids']
 */
function append_allowed_class_fields(array &$row, $rawAllowed = null): void
{
    if ($rawAllowed === null && array_key_exists('allowed_class_ids', $row)) {
        $rawAllowed = $row['allowed_class_ids'];
    }
    $parsed = parse_allowed_class_ids($rawAllowed);
    $row['allowed_class_ids'] = $parsed;
    $row['allow_all_classes'] = ($parsed === null);
}

/**
 * Normaliza allowed_class_ids para armazenamento.
 * null ou [] = todas as classes (armazena JSON "[]").
 *
 * @param mixed $input
 * @return array{ok:bool, json:?string, ids:array<int,int>, error:?string}
 */
function normalize_allowed_class_ids_for_storage($input): array
{
    if ($input === null || $input === '' || $input === false) {
        return ['ok' => true, 'json' => null, 'ids' => [], 'error' => null];
    }

    if (is_string($input)) {
        $decoded = json_decode($input, true);
        if (json_last_error() !== JSON_ERROR_NONE || !is_array($decoded)) {
            return ['ok' => false, 'json' => null, 'ids' => [], 'error' => 'allowed_class_ids inválido'];
        }
        $input = $decoded;
    }

    if (!is_array($input)) {
        return ['ok' => false, 'json' => null, 'ids' => [], 'error' => 'allowed_class_ids deve ser array ou null'];
    }

    $ids = [];
    foreach ($input as $v) {
        $id = (int)$v;
        if ($id > 0) {
            $ids[$id] = $id;
        }
    }
    $ids = array_values($ids);
    sort($ids);

    if (count($ids) === 0) {
        // Todas as classes: null no DB (compatível com itens legados)
        return ['ok' => true, 'json' => null, 'ids' => [], 'error' => null];
    }

    return [
        'ok' => true,
        'json' => json_encode($ids, JSON_UNESCAPED_UNICODE | JSON_NUMERIC_CHECK),
        'ids' => $ids,
        'error' => null,
    ];
}

/**
 * @param mixed $raw Coluna JSON/string/array/null
 * @return array<int,int>|null null = todas as classes; array não vazio = restrição
 */
function parse_allowed_class_ids($raw): ?array
{
    if ($raw === null || $raw === '') {
        return null;
    }
    if (is_string($raw)) {
        $decoded = json_decode($raw, true);
        if (json_last_error() !== JSON_ERROR_NONE || !is_array($decoded)) {
            return null;
        }
        $raw = $decoded;
    }
    if (!is_array($raw)) {
        return null;
    }
    $ids = [];
    foreach ($raw as $v) {
        $id = (int)$v;
        if ($id > 0) {
            $ids[$id] = $id;
        }
    }
    $ids = array_values($ids);
    if (count($ids) === 0) {
        return null; // [] = todas
    }
    return $ids;
}

/**
 * Pode o personagem com $classId equipar este template?
 * Restringe weapon e armor com equipment_slot != none.
 * null/[] em allowed_class_ids = todas as classes.
 *
 * @param array<string,mixed> $template
 */
function item_can_be_equipped_by_class(array $template, int $classId): bool
{
    $itemType = isset($template['item_type']) ? (string)$template['item_type'] : '';
    $slot = isset($template['equipment_slot']) ? (string)$template['equipment_slot'] : '';

    if ($itemType !== 'weapon' && $itemType !== 'armor') {
        return true;
    }
    if ($slot === '' || $slot === 'none') {
        return true;
    }

    $allowed = parse_allowed_class_ids($template['allowed_class_ids'] ?? null);
    if ($allowed === null) {
        return true;
    }
    if ($classId <= 0) {
        return false;
    }
    return in_array($classId, $allowed, true);
}
