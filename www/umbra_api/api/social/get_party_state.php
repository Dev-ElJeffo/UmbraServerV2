<?php
/**
 * POST /api/social/get_party_state.php
 * Obtém estado do grupo (membros com nome, level, HP, MP)
 * Parâmetros: token, party_id (ou obtém do jogador atual)
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

$json = file_get_contents('php://input');
$data = json_decode($json, true) ?: [];

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

$player_id = (int)($validation['payload']['player_id'] ?? 0);
$party_id = isset($data['party_id']) ? (int)$data['party_id'] : 0;

if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

try {
    $pdo = getConnection();

    // Se party_id não informado, buscar grupo do jogador
    if (!$party_id) {
        $find = $pdo->prepare("SELECT party_id FROM party_members WHERE player_id = ?");
        $find->execute([$player_id]);
        $row = $find->fetch(PDO::FETCH_ASSOC);
        $party_id = $row ? (int)$row['party_id'] : 0;
    }

    if (!$party_id) {
        http_response_code(200);
        echo json_encode([
            'success' => true,
            'in_party' => false,
            'party_id' => 0,
            'leader_id' => 0,
            'members' => []
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    // Verificar se o jogador pertence ao grupo
    $check = $pdo->prepare("SELECT 1 FROM party_members WHERE party_id = ? AND player_id = ?");
    $check->execute([$party_id, $player_id]);
    if (!$check->fetch()) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Você não pertence a este grupo']);
        exit;
    }

    // Buscar líder e membros
    $party_stmt = $pdo->prepare("SELECT leader_id FROM parties WHERE party_id = ?");
    $party_stmt->execute([$party_id]);
    $party = $party_stmt->fetch(PDO::FETCH_ASSOC);
    if (!$party) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Grupo não encontrado']);
        exit;
    }

    $leader_id = (int)$party['leader_id'];

    // Buscar membros com info (incluindo vitality/intelligence para cálculo de HP/MP total)
    $members_stmt = $pdo->prepare("
        SELECT 
            p.id as player_id,
            p.character_name,
            COALESCE(p.level, 1) as level,
            COALESCE(p.health, 100) as current_health,
            COALESCE(p.max_health, 100) as base_max_health,
            COALESCE(p.mana, 50) as current_mana,
            COALESCE(p.max_mana, 50) as base_max_mana,
            COALESCE(p.vitality, 10) as base_vitality,
            COALESCE(p.intelligence, 10) as base_intelligence,
            COALESCE(psp.vitality_points, 0) as vitality_points,
            COALESCE(psp.intelligence_points, 0) as intelligence_points,
            c.class_name
        FROM party_members pm
        INNER JOIN players p ON pm.player_id = p.id
        LEFT JOIN classes c ON p.class_id = c.class_id
        LEFT JOIN player_stat_points psp ON p.id = psp.player_id
        WHERE pm.party_id = ?
        ORDER BY pm.joined_at ASC
    ");
    $members_stmt->execute([$party_id]);
    $rows = $members_stmt->fetchAll(PDO::FETCH_ASSOC);

    $members = [];
    foreach ($rows as $r) {
        $mid = (int)$r['player_id'];
        $level = (int)$r['level'];
        $base_max_health = (int)$r['base_max_health'];
        $base_max_mana = (int)$r['base_max_mana'];
        $total_vitality = (int)$r['base_vitality'] + (int)$r['vitality_points'];
        $total_intelligence = (int)$r['base_intelligence'] + (int)$r['intelligence_points'];
        $total_health_bonus = 0;
        $total_mana_bonus = 0;

        // Bônus dos equipamentos
        $equip_stmt = $pdo->prepare("
            SELECT 
                JSON_EXTRACT(it.stats_json, '$.vitality') as vitality,
                JSON_EXTRACT(it.stats_json, '$.intelligence') as intelligence,
                JSON_EXTRACT(it.stats_json, '$.health_bonus') as health_bonus,
                JSON_EXTRACT(it.stats_json, '$.mana_bonus') as mana_bonus
            FROM player_inventory pi
            INNER JOIN item_templates it ON pi.item_template_id = it.item_id
            WHERE pi.player_id = ? AND pi.is_equipped = TRUE
        ");
        $equip_stmt->execute([$mid]);
        foreach ($equip_stmt->fetchAll(PDO::FETCH_ASSOC) as $es) {
            $total_vitality += (int)($es['vitality'] ?? 0);
            $total_intelligence += (int)($es['intelligence'] ?? 0);
            $total_health_bonus += (int)($es['health_bonus'] ?? 0);
            $total_mana_bonus += (int)($es['mana_bonus'] ?? 0);
        }

        $level_hp_bonus = $level * 20;
        $level_mp_bonus = $level * 20;
        $vitality_hp_bonus = (int)floor($total_vitality / 10) * 30;
        $intelligence_mana_bonus = (int)floor($total_intelligence / 10) * 30;
        $total_health_bonus += $vitality_hp_bonus;
        $total_mana_bonus += $intelligence_mana_bonus;

        $final_max_health = $base_max_health + $level_hp_bonus + $total_health_bonus;
        $final_max_mana = $base_max_mana + $level_mp_bonus + $total_mana_bonus;

        $members[] = [
            'player_id' => $mid,
            'character_name' => $r['character_name'] ?? '',
            'level' => $level,
            'current_health' => (int)$r['current_health'],
            'max_health' => $final_max_health,
            'current_mana' => (int)$r['current_mana'],
            'max_mana' => $final_max_mana,
            'class_name' => $r['class_name'] ?? '',
            'is_leader' => ($mid === $leader_id)
        ];
    }

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'in_party' => true,
        'party_id' => (int)$party_id,
        'leader_id' => $leader_id,
        'members' => $members
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    error_log("Erro ao obter estado do grupo: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
