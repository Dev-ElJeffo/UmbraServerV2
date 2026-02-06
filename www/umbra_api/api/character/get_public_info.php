<?php
/**
 * POST /api/character/get_public_info.php
 * Obtém informações públicas de um jogador (para inspeção)
 * 
 * Suporta GET (query params) e POST (JSON body)
 * - player_id: ID do jogador a inspecionar
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

// Ler dados do body JSON (POST) ou headers (GET)
$data = [];
$json = file_get_contents('php://input');
if (!empty($json)) {
    $data = json_decode($json, true) ?: [];
}

// Se não tem token no body, tentar do header
if (!isset($data['token'])) {
    $headers = getallheaders();
    if (isset($headers['Authorization'])) {
        $data['token'] = str_replace('Bearer ', '', $headers['Authorization']);
    }
}

$validation = validateJWTRequest($data, $_SERVER);
if (!$validation['valid']) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => $validation['error'] ?? 'Token inválido ou expirado']);
    exit;
}

// Obter player_id: primeiro do body JSON (POST), depois de query params (GET)
$target_player_id = null;
if (isset($data['player_id']) && is_numeric($data['player_id'])) {
    $target_player_id = intval($data['player_id']);
} else {
    $target_player_id = isset($_GET['player_id']) ? intval($_GET['player_id']) : null;
}

if (!$target_player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id é obrigatório']);
    exit;
}

try {
    $pdo = getConnection();
    
    // Buscar informações públicas do jogador
    $query = $pdo->prepare("
        SELECT 
            p.id as player_id,
            p.character_name,
            p.level,
            p.experience,
            p.health as current_health,
            p.max_health,
            p.mana as current_mana,
            p.max_mana,
            p.strength,
            p.dexterity,
            p.intelligence,
            p.vitality,
            COALESCE(p.luck, 10) as luck,
            p.pvp,
            p.chaos,
            p.honor,
            c.class_name,
            f.faction_name,
            g.guild_name,
            t.title_name
        FROM players p
        LEFT JOIN classes c ON p.class_id = c.class_id
        LEFT JOIN factions f ON p.faction_id = f.faction_id
        LEFT JOIN guilds g ON p.current_guild_id = g.guild_id
        LEFT JOIN titles t ON p.equipped_title_id = t.title_id
        WHERE p.id = :player_id
    ");
    $query->execute(['player_id' => $target_player_id]);
    $player = $query->fetch(PDO::FETCH_ASSOC);
    
    if (!$player) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Jogador não encontrado']);
        exit;
    }
    
    // Calcular valores totais de HP/MP (base + nível + atributos + equipamentos)
    // IMPORTANTE: Seguir a mesma lógica do get_character_info.php
    $level = (int)$player['level'];
    
    // Bônus por nível
    $level_hp_bonus = $level * 20;
    $level_mp_bonus = $level * 20;
    
    // 1. Iniciar com atributos base do player
    $total_vitality = (int)$player['vitality'];
    $total_intelligence = (int)$player['intelligence'];
    $total_health_bonus = 0;
    $total_mana_bonus = 0;
    
    // 2. Buscar e somar atributos e bônus dos equipamentos
    $equipped_stats_query = $pdo->prepare("
        SELECT 
            JSON_EXTRACT(it.stats_json, '$.vitality') as vitality,
            JSON_EXTRACT(it.stats_json, '$.intelligence') as intelligence,
            JSON_EXTRACT(it.stats_json, '$.health_bonus') as health_bonus,
            JSON_EXTRACT(it.stats_json, '$.mana_bonus') as mana_bonus
        FROM player_inventory pi
        INNER JOIN item_templates it ON pi.item_template_id = it.item_id
        WHERE pi.player_id = :player_id AND pi.is_equipped = TRUE
    ");
    $equipped_stats_query->execute(['player_id' => $target_player_id]);
    $equipped_items_stats = $equipped_stats_query->fetchAll(PDO::FETCH_ASSOC);
    
    // Somar atributos e bônus diretos dos equipamentos
    foreach ($equipped_items_stats as $item_stats) {
        if (!empty($item_stats['vitality'])) {
            $total_vitality += (int)$item_stats['vitality'];
        }
        if (!empty($item_stats['intelligence'])) {
            $total_intelligence += (int)$item_stats['intelligence'];
        }
        if (!empty($item_stats['health_bonus'])) {
            $total_health_bonus += (int)$item_stats['health_bonus'];
        }
        if (!empty($item_stats['mana_bonus'])) {
            $total_mana_bonus += (int)$item_stats['mana_bonus'];
        }
    }
    
    // 3. Calcular bônus de HP/MP baseados nos atributos TOTAIS (incluindo dos equipamentos)
    $vitality_hp_bonus = floor($total_vitality / 10) * 30;
    $intelligence_mana_bonus = floor($total_intelligence / 10) * 30;
    
    // 4. Somar os bônus calculados aos bônus diretos dos equipamentos
    $total_health_bonus += $vitality_hp_bonus;
    $total_mana_bonus += $intelligence_mana_bonus;
    
    // 5. Calcular valores finais
    $base_health = (int)$player['max_health'];
    $base_mana = (int)$player['max_mana'];
    $final_max_health = $base_health + $level_hp_bonus + $total_health_bonus;
    $final_max_mana = $base_mana + $level_mp_bonus + $total_mana_bonus;
    
    // Log de debug (apenas em desenvolvimento)
    error_log(sprintf(
        "[get_public_info] PlayerID %d: Base HP=%d, Level=%d (+%d), Vit Total=%d (+%d HP), Equip Direto (+%d), Total Bonus (+%d) = Final %d | Base MP=%d, Level=%d (+%d), Int Total=%d (+%d MP), Equip Direto (+%d), Total Bonus (+%d) = Final %d",
        $target_player_id,
        $base_health, $level, $level_hp_bonus, $total_vitality, $vitality_hp_bonus, ($total_health_bonus - $vitality_hp_bonus), $total_health_bonus, $final_max_health,
        $base_mana, $level, $level_mp_bonus, $total_intelligence, $intelligence_mana_bonus, ($total_mana_bonus - $intelligence_mana_bonus), $total_mana_bonus, $final_max_mana
    ));
    
    // Buscar itens equipados (apenas IDs e nomes, sem stats detalhados)
    $equipped_query = $pdo->prepare("
        SELECT 
            pi.inventory_id,
            pi.item_template_id,
            it.item_name,
            it.equipment_slot,
            it.item_type
        FROM player_inventory pi
        INNER JOIN item_templates it ON pi.item_template_id = it.item_id
        WHERE pi.player_id = :player_id AND pi.is_equipped = TRUE
        ORDER BY it.equipment_slot ASC
    ");
    $equipped_query->execute(['player_id' => $target_player_id]);
    $equipped_items = $equipped_query->fetchAll(PDO::FETCH_ASSOC);
    
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'player' => [
            'player_id' => (int)$player['player_id'],
            'character_name' => $player['character_name'],
            'level' => (int)$player['level'],
            'experience' => (int)$player['experience'],
            'current_health' => (int)$player['current_health'],
            'max_health' => $final_max_health,
            'current_mana' => (int)$player['current_mana'],
            'max_mana' => $final_max_mana,
            'strength' => (int)$player['strength'],
            'dexterity' => (int)$player['dexterity'],
            'intelligence' => (int)$player['intelligence'],
            'vitality' => (int)$player['vitality'],
            'luck' => (int)$player['luck'],
            'pvp' => (int)$player['pvp'],
            'chaos' => (int)$player['chaos'],
            'honor' => (int)$player['honor'],
            'class_name' => $player['class_name'] ?? '',
            'faction_name' => $player['faction_name'] ?? '',
            'guild_name' => $player['guild_name'] ?? '',
            'title_name' => $player['title_name'] ?? ''
        ],
        'equipped_items' => $equipped_items
    ], JSON_UNESCAPED_UNICODE);
    
} catch (PDOException $e) {
    error_log("Erro ao obter informações públicas: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao processar solicitação',
        'error' => $e->getMessage()
    ]);
}
?>
