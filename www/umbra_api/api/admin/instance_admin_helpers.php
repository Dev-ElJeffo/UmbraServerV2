<?php
/**
 * Instâncias (etapa 4): campos editáveis de instance_templates e colunas novas de npc_instances.
 * Depois de salvar: admin TCP reload_instances (o Manager faz sozinho).
 */

function instance_template_columns(): array
{
    $cols = [
        'code', 'name', 'description', 'zone_id', 'map_path', 'origin_x', 'origin_y', 'origin_z', 'slot_spacing',
        'entry_x', 'entry_y', 'entry_z', 'entry_yaw', 'min_level', 'max_players', 'party_required',
        'time_limit_minutes', 'empty_close_seconds', 'max_copies', 'is_active',
    ];
    return array_merge($cols, instance_template_dungeon_columns());
}

/** Etapa 5 (add_dungeons.sql): regras de dungeon do molde. */
function instance_template_dungeon_columns(): array
{
    return ['is_dungeon', 'completion_mode', 'reward_exp', 'reward_gold', 'lockout_reset', 'completion_close_seconds'];
}

/** True se instance_templates já tem as colunas de dungeon. */
function instance_template_has_dungeon_columns(PDO $pdo): bool
{
    static $has = null;
    if ($has === null) {
        $stmt = $pdo->query("SHOW COLUMNS FROM instance_templates LIKE 'is_dungeon'");
        $has = $stmt && $stmt->fetch() !== false;
    }
    return $has;
}

/** True se npc_instances já tem dungeon_role. */
function npc_instance_has_dungeon_role(PDO $pdo): bool
{
    static $has = null;
    if ($has === null) {
        $stmt = $pdo->query("SHOW COLUMNS FROM npc_instances LIKE 'dungeon_role'");
        $has = $stmt && $stmt->fetch() !== false;
    }
    return $has;
}

/** '' / 'none' = NULL (inimigo comum); trash, boss, final_boss. */
function npc_instance_dungeon_role($value): ?string
{
    $v = strtolower(trim((string)$value));
    if ($v === '' || $v === 'none' || $v === 'null') {
        return null;
    }
    if (!in_array($v, ['trash', 'boss', 'final_boss'], true)) {
        throw new InvalidArgumentException('dungeon_role: use trash, boss, final_boss ou vazio');
    }
    return $v;
}

/** Normaliza um campo; lança InvalidArgumentException (pt-BR) se inválido. */
function instance_template_normalize_field(string $field, $value)
{
    switch ($field) {
        case 'code':
            $v = strtoupper(trim((string)$value));
            if (!preg_match('/^[A-Z0-9_]{2,64}$/', $v)) {
                throw new InvalidArgumentException('code: use 2-64 letras maiúsculas, números ou _');
            }
            return $v;
        case 'name':
            $v = trim((string)$value);
            if ($v === '' || mb_strlen($v) > 120) {
                throw new InvalidArgumentException('name: obrigatório, até 120 caracteres');
            }
            return $v;
        case 'description':
            return mb_substr(trim((string)$value), 0, 500);
        case 'map_path':
            $v = trim((string)$value);
            // Soft path completo do level: /Game/.../Lvl_X.Lvl_X
            if (!preg_match('#^/Game/[A-Za-z0-9_/\-]+\.[A-Za-z0-9_\-]+$#', $v) || mb_strlen($v) > 255) {
                throw new InvalidArgumentException('map_path: use o soft path do level, ex.: /Game/Maps/Instances/Lvl_X.Lvl_X');
            }
            return $v;
        case 'origin_x':
        case 'origin_y':
        case 'origin_z':
            $v = (float)$value;
            if (abs($v) > 1000000) {
                throw new InvalidArgumentException("{$field}: fique dentro de ±1.000.000");
            }
            return $v;
        case 'slot_spacing':
            $v = (float)$value;
            return $v < 30000 ? 150000.0 : min($v, 1000000.0);
        case 'entry_x':
        case 'entry_y':
        case 'entry_z':
        case 'entry_yaw':
            return (float)$value;
        case 'zone_id':
            return max(0, (int)$value);
        case 'min_level':
            return max(1, (int)$value);
        case 'max_players':
            $v = (int)$value;
            return $v <= 0 ? 5 : min(40, $v);
        case 'party_required':
        case 'is_active':
            return ((int)$value) !== 0 ? 1 : 0;
        case 'time_limit_minutes':
            return max(0, min(1440, (int)$value));
        case 'empty_close_seconds':
            $v = (int)$value;
            return $v < 10 ? 120 : min(3600, $v);
        case 'max_copies':
            $v = (int)$value;
            return $v <= 0 ? 12 : min(200, $v);
        case 'is_dungeon':
            return ((int)$value) !== 0 ? 1 : 0;
        case 'completion_mode':
            $v = strtolower(trim((string)$value));
            if (!in_array($v, ['final_boss', 'all_bosses'], true)) {
                throw new InvalidArgumentException('completion_mode: final_boss ou all_bosses');
            }
            return $v;
        case 'reward_exp':
        case 'reward_gold':
            return max(0, (int)$value);
        case 'lockout_reset':
            $v = strtolower(trim((string)$value));
            if (!in_array($v, ['none', 'daily', 'weekly'], true)) {
                throw new InvalidArgumentException('lockout_reset: none, daily ou weekly');
            }
            return $v;
        case 'completion_close_seconds':
            $v = (int)$value;
            return $v < 10 ? 300 : min(3600, $v);
    }
    throw new InvalidArgumentException("Campo desconhecido: {$field}");
}

/** True se npc_instances já tem instance_template_id/portal_instance_template_id (create_instances.sql). */
function npc_instance_has_instance_columns(PDO $pdo): bool
{
    static $has = null;
    if ($has === null) {
        $stmt = $pdo->query("SHOW COLUMNS FROM npc_instances LIKE 'portal_instance_template_id'");
        $has = $stmt && $stmt->fetch() !== false;
    }
    return $has;
}

/** 0/vazio = NULL (mundo aberto / não é portal). */
function npc_instance_template_ref($value): ?int
{
    $v = (int)$value;
    return $v > 0 ? $v : null;
}
