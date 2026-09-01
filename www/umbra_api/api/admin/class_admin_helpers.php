<?php
/**
 * Helpers CRUD admin da tabela classes.
 */

function class_has_anim_set_column(PDO $pdo): bool
{
    static $cached = null;
    if ($cached !== null) {
        return $cached;
    }
    try {
        $stmt = $pdo->query("SHOW COLUMNS FROM classes LIKE 'anim_set_json'");
        $cached = $stmt && $stmt->rowCount() > 0;
    } catch (Throwable $e) {
        $cached = false;
    }
    return $cached;
}

function class_has_mesh_path_columns(PDO $pdo): bool
{
    static $cached = null;
    if ($cached !== null) {
        return $cached;
    }
    try {
        $stmt = $pdo->query("SHOW COLUMNS FROM classes LIKE 'skeletal_mesh_path'");
        $cached = $stmt && $stmt->rowCount() > 0;
    } catch (Throwable $e) {
        $cached = false;
    }
    return $cached;
}

function class_modular_mesh_columns(): array
{
    return [
        'torso_mesh_path',
        'arms_mesh_path',
        'legs_mesh_path',
        'feet_mesh_path',
        'main_hand_mesh_path',
        'off_hand_mesh_path',
    ];
}

function class_legacy_hand_mesh_columns(): array
{
    return ['left_hand_mesh_path', 'right_hand_mesh_path'];
}

function class_table_columns(PDO $pdo): array
{
    static $cache = [];
    $key = spl_object_hash($pdo);
    if (isset($cache[$key])) {
        return $cache[$key];
    }
    $cols = [];
    try {
        $stmt = $pdo->query('SHOW COLUMNS FROM classes');
        if ($stmt) {
            while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
                if (!empty($row['Field'])) {
                    $cols[(string)$row['Field']] = true;
                }
            }
        }
    } catch (Throwable $e) {
        // ignore
    }
    $cache[$key] = $cols;
    return $cols;
}

function class_column_exists(PDO $pdo, string $column): bool
{
    $cols = class_table_columns($pdo);
    return isset($cols[$column]);
}

/** Colunas modulares que existem de fato no schema atual (inclui legacy left/right). */
function class_modular_mesh_columns_for_db(PDO $pdo): array
{
    $cols = [];
    foreach (class_modular_mesh_columns() as $col) {
        if (class_column_exists($pdo, $col)) {
            $cols[] = $col;
        }
    }
    foreach (class_legacy_hand_mesh_columns() as $legacy) {
        if (class_column_exists($pdo, $legacy)) {
            $cols[] = $legacy;
        }
    }
    return $cols;
}

function class_has_modular_mesh_columns(PDO $pdo): bool
{
    return count(class_modular_mesh_columns_for_db($pdo)) > 0;
}

function class_modular_mesh_select_sql(PDO $pdo): string
{
    $cols = class_modular_mesh_columns_for_db($pdo);
    if (empty($cols)) {
        return '';
    }
    return ', ' . implode(', ', $cols);
}

function class_apply_modular_mesh_payload(PDO $pdo, array $data, array &$fields, bool $forUpdate): void
{
    $available = array_flip(class_modular_mesh_columns_for_db($pdo));
    foreach (class_modular_mesh_columns() as $meshCol) {
        if ($forUpdate && !array_key_exists($meshCol, $data)) {
            continue;
        }
        if (!$forUpdate && !array_key_exists($meshCol, $data)) {
            continue;
        }
        $val = $data[$meshCol] ?? null;
        $normalized = ($val === null || $val === '') ? null : trim((string)$val);
        if (isset($available[$meshCol])) {
            $fields[$meshCol] = $normalized;
            continue;
        }
        if ($meshCol === 'arms_mesh_path' && $normalized !== null) {
            if (isset($available['left_hand_mesh_path'])) {
                $fields['left_hand_mesh_path'] = $normalized;
            }
            if (isset($available['right_hand_mesh_path'])) {
                $fields['right_hand_mesh_path'] = $normalized;
            }
        }
    }
}

function class_encode_anim_json($value): ?string
{
    if ($value === null || $value === '') {
        return null;
    }
    if (is_array($value) || is_object($value)) {
        $json = json_encode($value, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
        return $json === false ? null : $json;
    }
    $raw = trim((string)$value);
    if ($raw === '' || strtolower($raw) === 'null') {
        return null;
    }
    json_decode($raw, true);
    return json_last_error() === JSON_ERROR_NONE ? $raw : null;
}

function class_payload_fields(array $data, bool $forUpdate, bool $includeAnim, ?PDO $pdo = null): array
{
    $fields = [];
    $map = [
        'class_name' => 'string',
        'class_description' => 'string',
        'base_strength' => 'int',
        'base_dexterity' => 'int',
        'base_intelligence' => 'int',
        'base_vitality' => 'int',
        'base_luck' => 'int',
        'base_health' => 'int',
        'base_mana' => 'int',
        'base_stamina' => 'int',
        'base_physical_attack' => 'int',
        'base_magic_attack' => 'int',
        'base_physical_defense' => 'int',
        'base_magic_defense' => 'int',
        'base_accuracy' => 'int',
        'base_dodge' => 'int',
        'base_critical' => 'int',
        'base_movement' => 'int',
        'base_critical_resistance' => 'int',
        'base_double_attack_resistance' => 'int',
        'base_double_attack_rate' => 'int',
    ];
    foreach ($map as $col => $type) {
        if ($forUpdate && !array_key_exists($col, $data)) {
            continue;
        }
        if ($type === 'int') {
            $fields[$col] = (int)($data[$col] ?? 0);
        } else {
            $val = $data[$col] ?? '';
            $fields[$col] = $val === null ? null : (string)$val;
        }
    }
    if ($includeAnim && (!$forUpdate || array_key_exists('anim_set_json', $data))) {
        $fields['anim_set_json'] = class_encode_anim_json($data['anim_set_json'] ?? null);
    }
    foreach (['skeletal_mesh_path', 'anim_blueprint_path'] as $meshCol) {
        if ($forUpdate && !array_key_exists($meshCol, $data)) {
            continue;
        }
        if (!$forUpdate && !array_key_exists($meshCol, $data)) {
            continue;
        }
        $val = $data[$meshCol] ?? null;
        $fields[$meshCol] = ($val === null || $val === '') ? null : trim((string)$val);
    }
    if ($pdo instanceof PDO) {
        class_apply_modular_mesh_payload($pdo, $data, $fields, $forUpdate);
    } else {
        foreach (class_modular_mesh_columns() as $meshCol) {
            if ($forUpdate && !array_key_exists($meshCol, $data)) {
                continue;
            }
            if (!$forUpdate && !array_key_exists($meshCol, $data)) {
                continue;
            }
            $val = $data[$meshCol] ?? null;
            $fields[$meshCol] = ($val === null || $val === '') ? null : trim((string)$val);
        }
    }
    if (!$forUpdate) {
        $name = trim((string)($fields['class_name'] ?? ''));
        if ($name === '') {
            throw new InvalidArgumentException('class_name é obrigatório');
        }
        $fields['class_name'] = $name;
    }
    return $fields;
}
