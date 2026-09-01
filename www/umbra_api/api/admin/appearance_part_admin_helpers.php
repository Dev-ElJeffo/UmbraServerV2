<?php
/**
 * Helpers CRUD admin / leitura pública de player_appearance_parts.
 */

function appearance_part_row_normalize(array $row): array
{
    return [
        'appearance_part_id' => (int)($row['appearance_part_id'] ?? 0),
        'part_type' => (string)($row['part_type'] ?? ''),
        'part_id' => (int)($row['part_id'] ?? 0),
        'mesh_path' => $row['mesh_path'] ?? null,
        'attach_socket' => (string)($row['attach_socket'] ?? 'head'),
        'is_enabled' => (int)($row['is_enabled'] ?? 1),
    ];
}

function appearance_part_allowed_types(): array
{
    return ['body', 'hair', 'head'];
}

function appearance_part_validate_payload(array $data, bool $forUpdate): array
{
    $fields = [];
    if (!$forUpdate || array_key_exists('part_type', $data)) {
        $type = strtolower(trim((string)($data['part_type'] ?? '')));
        if (!in_array($type, appearance_part_allowed_types(), true)) {
            throw new InvalidArgumentException('part_type inválido (body, hair, head)');
        }
        $fields['part_type'] = $type;
    }
    if (!$forUpdate || array_key_exists('part_id', $data)) {
        $partId = (int)($data['part_id'] ?? -1);
        if ($partId < 0) {
            throw new InvalidArgumentException('part_id deve ser >= 0');
        }
        $fields['part_id'] = $partId;
    }
    if (!$forUpdate || array_key_exists('mesh_path', $data)) {
        $path = $data['mesh_path'] ?? null;
        if ($path === null || $path === '') {
            $fields['mesh_path'] = null;
        } else {
            $fields['mesh_path'] = trim((string)$path);
        }
    }
    if (!$forUpdate || array_key_exists('attach_socket', $data)) {
        $socket = trim((string)($data['attach_socket'] ?? 'head'));
        if ($socket === '') {
            throw new InvalidArgumentException('attach_socket é obrigatório');
        }
        $fields['attach_socket'] = $socket;
    }
    if (!$forUpdate || array_key_exists('is_enabled', $data)) {
        $fields['is_enabled'] = !empty($data['is_enabled']) ? 1 : 0;
    }
    if (!$forUpdate) {
        if (!isset($fields['part_type'], $fields['part_id'])) {
            throw new InvalidArgumentException('part_type e part_id são obrigatórios');
        }
    }
    return $fields;
}
