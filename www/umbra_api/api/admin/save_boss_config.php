<?php
/**
 * POST /api/admin/save_boss_config.php — substitui a config de chefe de um template (etapa 5b).
 * Body: npc_template_id, is_boss (0 = remove tudo), config{enrage_seconds, enrage_damage_mult, enrage_text,
 *       reset_on_leash, show_boss_bar}, phases[{phase_order, name, hp_threshold_pct, damage_mult, speed_mult,
 *       attack_cooldown_mult, invulnerable_ms, announce_text, transition_anim_path, transition_vfx_path,
 *       skills[{npc_skill_id, skill_rank, weight, cooldown_override_ms}], adds[{npc_template_id, count, radius}]}].
 * Regras: fase 1 = 100%; limiares estritamente decrescentes; até 10 fases. Depois: reload_boss_configs.
 */
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

require_once __DIR__ . '/require_admin_auth.php';
$data = admin_decode_json_body();
requireAdminAuth($data);

function boss_fail(string $msg): void
{
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => $msg], JSON_UNESCAPED_UNICODE);
    exit;
}

/** Soft path vazio ou /Game/...Asset.Asset. */
function boss_soft_path($value, string $field): string
{
    $v = trim((string)$value);
    if ($v === '') {
        return '';
    }
    if (!preg_match('#^/Game/[A-Za-z0-9_/\-]+\.[A-Za-z0-9_\-]+$#', $v) || mb_strlen($v) > 255) {
        boss_fail("{$field}: use o soft path completo (/Game/.../Asset.Asset)");
    }
    return $v;
}

$templateId = (int)($data['npc_template_id'] ?? 0);
if ($templateId <= 0) {
    boss_fail('npc_template_id é obrigatório');
}
$isBoss = ((int)($data['is_boss'] ?? 1)) !== 0;
$cfgIn = is_array($data['config'] ?? null) ? $data['config'] : [];
$phasesIn = is_array($data['phases'] ?? null) ? array_values($data['phases']) : [];

$config = [
    'enrage_seconds' => max(0, min(3600, (int)($cfgIn['enrage_seconds'] ?? 0))),
    'enrage_damage_mult' => max(1.0, min(100.0, (float)($cfgIn['enrage_damage_mult'] ?? 2.0))),
    'enrage_text' => mb_substr(trim((string)($cfgIn['enrage_text'] ?? '')), 0, 200),
    'reset_on_leash' => ((int)($cfgIn['reset_on_leash'] ?? 1)) !== 0 ? 1 : 0,
    'show_boss_bar' => ((int)($cfgIn['show_boss_bar'] ?? 1)) !== 0 ? 1 : 0,
];

$phases = [];
if ($isBoss) {
    if (count($phasesIn) > 10) {
        boss_fail('No máximo 10 fases');
    }
    usort($phasesIn, static fn($a, $b) => ((int)($a['phase_order'] ?? 0)) <=> ((int)($b['phase_order'] ?? 0)));
    $prevPct = 101;
    foreach ($phasesIn as $i => $p) {
        $order = $i + 1;  // renumera 1..N na ordem recebida
        $pct = $order === 1 ? 100 : (int)($p['hp_threshold_pct'] ?? 0);
        if ($order > 1 && ($pct < 1 || $pct > 99)) {
            boss_fail("Fase {$order}: limiar de vida entre 1 e 99%");
        }
        if ($pct >= $prevPct) {
            boss_fail("Fase {$order}: o limiar precisa ser menor que o da fase anterior");
        }
        $prevPct = $pct;
        $skills = [];
        foreach ((array)($p['skills'] ?? []) as $s) {
            $sid = (int)($s['npc_skill_id'] ?? 0);
            if ($sid <= 0) {
                continue;
            }
            $skills[] = [
                'id' => $sid,
                'rank' => max(1, min(255, (int)($s['skill_rank'] ?? 1))),
                'weight' => max(0, (int)($s['weight'] ?? 100)),
                'cd' => max(0, (int)($s['cooldown_override_ms'] ?? 0)),
            ];
        }
        $adds = [];
        foreach ((array)($p['adds'] ?? []) as $a) {
            $tid = (int)($a['npc_template_id'] ?? 0);
            if ($tid <= 0) {
                continue;
            }
            if ($tid === $templateId) {
                boss_fail("Fase {$order}: o chefe não pode chamar ele mesmo como add");
            }
            $adds[] = [
                'tpl' => $tid,
                'count' => max(1, min(20, (int)($a['count'] ?? 1))),
                'radius' => max(50.0, min(3000.0, (float)($a['radius'] ?? 400))),
            ];
        }
        $phases[] = [
            'order' => $order,
            'name' => mb_substr(trim((string)($p['name'] ?? '')), 0, 64),
            'pct' => $pct,
            'dmg' => max(0.1, min(100.0, (float)($p['damage_mult'] ?? 1.0))),
            'speed' => max(0.2, min(5.0, (float)($p['speed_mult'] ?? 1.0))),
            'cd' => max(0.2, min(5.0, (float)($p['attack_cooldown_mult'] ?? 1.0))),
            'invuln' => max(0, min(30000, (int)($p['invulnerable_ms'] ?? 0))),
            'announce' => mb_substr(trim((string)($p['announce_text'] ?? '')), 0, 200),
            'anim' => boss_soft_path($p['transition_anim_path'] ?? '', "Fase {$order} animação"),
            'vfx' => boss_soft_path($p['transition_vfx_path'] ?? '', "Fase {$order} VFX"),
            'skills' => $skills,
            'adds' => $adds,
        ];
    }
}

try {
    $pdo = getConnection();
    $chk = $pdo->prepare('SELECT npc_template_id FROM npc_templates WHERE npc_template_id = ?');
    $chk->execute([$templateId]);
    if (!$chk->fetchColumn()) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Template não encontrado'], JSON_UNESCAPED_UNICODE);
        exit;
    }
    // Skills e templates de add precisam existir.
    $skillIds = [];
    $addIds = [];
    foreach ($phases as $p) {
        foreach ($p['skills'] as $s) {
            $skillIds[$s['id']] = true;
        }
        foreach ($p['adds'] as $a) {
            $addIds[$a['tpl']] = true;
        }
    }
    foreach ([['npc_skills', 'npc_skill_id', array_keys($skillIds), 'Skill'],
              ['npc_templates', 'npc_template_id', array_keys($addIds), 'Template de add']] as [$table, $col, $ids, $label]) {
        if (!$ids) {
            continue;
        }
        $ph = implode(',', array_fill(0, count($ids), '?'));
        $q = $pdo->prepare("SELECT {$col} FROM {$table} WHERE {$col} IN ({$ph})");
        $q->execute($ids);
        $missing = array_diff($ids, array_map('intval', $q->fetchAll(PDO::FETCH_COLUMN)));
        if ($missing) {
            boss_fail("{$label} inexistente: " . implode(', ', $missing));
        }
    }

    $pdo->beginTransaction();
    $old = $pdo->prepare('SELECT phase_id FROM npc_boss_phases WHERE npc_template_id = ?');
    $old->execute([$templateId]);
    $oldIds = array_map('intval', $old->fetchAll(PDO::FETCH_COLUMN));
    if ($oldIds) {
        $ph = implode(',', array_fill(0, count($oldIds), '?'));
        $pdo->prepare("DELETE FROM npc_boss_phase_skills WHERE phase_id IN ({$ph})")->execute($oldIds);
        $pdo->prepare("DELETE FROM npc_boss_phase_adds WHERE phase_id IN ({$ph})")->execute($oldIds);
    }
    $pdo->prepare('DELETE FROM npc_boss_phases WHERE npc_template_id = ?')->execute([$templateId]);
    $pdo->prepare('DELETE FROM npc_boss_configs WHERE npc_template_id = ?')->execute([$templateId]);

    if ($isBoss) {
        $pdo->prepare(
            'INSERT INTO npc_boss_configs
               (npc_template_id, enrage_seconds, enrage_damage_mult, enrage_text, reset_on_leash, show_boss_bar)
             VALUES (?, ?, ?, ?, ?, ?)'
        )->execute([$templateId, $config['enrage_seconds'], $config['enrage_damage_mult'], $config['enrage_text'],
            $config['reset_on_leash'], $config['show_boss_bar']]);
        $insPhase = $pdo->prepare(
            'INSERT INTO npc_boss_phases
               (npc_template_id, phase_order, name, hp_threshold_pct, damage_mult, speed_mult, attack_cooldown_mult,
                invulnerable_ms, announce_text, transition_anim_path, transition_vfx_path)
             VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)'
        );
        $insSkill = $pdo->prepare(
            'INSERT INTO npc_boss_phase_skills (phase_id, npc_skill_id, skill_rank, weight, cooldown_override_ms)
             VALUES (?, ?, ?, ?, ?)'
        );
        $insAdd = $pdo->prepare('INSERT INTO npc_boss_phase_adds (phase_id, npc_template_id, count, radius) VALUES (?, ?, ?, ?)');
        foreach ($phases as $p) {
            $insPhase->execute([$templateId, $p['order'], $p['name'], $p['pct'], $p['dmg'], $p['speed'], $p['cd'],
                $p['invuln'], $p['announce'], $p['anim'], $p['vfx']]);
            $phaseId = (int)$pdo->lastInsertId();
            foreach ($p['skills'] as $s) {
                $insSkill->execute([$phaseId, $s['id'], $s['rank'], $s['weight'], $s['cd']]);
            }
            foreach ($p['adds'] as $a) {
                $insAdd->execute([$phaseId, $a['tpl'], $a['count'], $a['radius']]);
            }
        }
    }
    $pdo->commit();
    auditAdminWrite('save_boss_config', "npc_template_id={$templateId} is_boss=" . ($isBoss ? 1 : 0)
        . ' fases=' . count($phases), 'npc_template', $templateId);
    echo json_encode(['success' => true, 'message' => $isBoss ? 'Chefe salvo' : 'Config de chefe removida',
        'phases' => count($phases)], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    if (isset($pdo) && $pdo->inTransaction()) {
        $pdo->rollBack();
    }
    error_log('[admin/save_boss_config] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
