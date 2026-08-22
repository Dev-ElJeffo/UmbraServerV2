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

function class_payload_fields(array $data, bool $forUpdate, bool $includeAnim): array
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
    if (!$forUpdate) {
        $name = trim((string)($fields['class_name'] ?? ''));
        if ($name === '') {
            throw new InvalidArgumentException('class_name é obrigatório');
        }
        $fields['class_name'] = $name;
    }
    return $fields;
}
