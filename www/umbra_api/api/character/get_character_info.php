<?php
/**
 * POST /api/character/get_character_info.php
 * Obtém todas as informações do personagem incluindo stats, equipamentos e atributos calculados
 * 
 * Headers requeridos:
 * - Authorization: Bearer {jwt_token}
 * 
 * Body (JSON):
 * {
 *   "token": "jwt_token"
 * }
 * 
 * Retorna:
 * - Informações completas do personagem
 * - Stats base e calculados (com equipamentos)
 * - Lista de equipamentos por slot
 * - EXP atual e necessário para próximo nível
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

$player_id = $validation['payload']['player_id'] ?? null;
if (!$player_id) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Player ID não encontrado no token']);
    exit;
}

try {
    $pdo = getConnection();
    
    // 1. Buscar informações básicas do personagem
    // Incluir JOINs com classes, factions, guilds, titles
    // Incluir TODOS os stats base da classe
    $player_query = "SELECT 
                        p.id,
                        p.account_id,
                        p.character_name,
                        p.level,
                        p.experience,
                        p.current_zone,
                        p.pos_x,
                        p.pos_y,
                        p.pos_z,
                        p.health,
                        p.max_health,
                        p.mana,
                        p.max_mana,
                        p.stamina,
                        p.max_stamina,
                        p.strength,
                        p.dexterity,
                        p.intelligence,
                        p.vitality,
                        COALESCE(p.luck, 10) as luck,
                        p.class_id,
                        p.faction_id,
                        p.current_guild_id,
                        p.equipped_title_id,
                        p.selected_class,
                        p.pvp,
                        p.chaos,
                        p.honor,
                        p.created_at,
                        p.last_played_at,
                        c.class_name,
                        c.base_strength,
                        c.base_dexterity,
                        c.base_intelligence,
                        c.base_vitality,
                        c.base_luck,
                        c.base_health,
                        c.base_mana,
                        c.base_stamina,
                        c.base_physical_attack,
                        c.base_magic_attack,
                        c.base_physical_defense,
                        c.base_magic_defense,
                        c.base_accuracy,
                        c.base_dodge,
                        c.base_critical,
                        c.base_movement,
                        c.base_resistance,
                        c.base_double_attack_rate,
                        f.faction_name,
                        g.guild_name,
                        t.title_name
                    FROM players p
                    LEFT JOIN classes c ON p.class_id = c.class_id
                    LEFT JOIN factions f ON p.faction_id = f.faction_id
                    LEFT JOIN guilds g ON p.current_guild_id = g.guild_id
                    LEFT JOIN titles t ON p.equipped_title_id = t.title_id
                    WHERE p.id = :player_id";
    $player_stmt = $pdo->prepare($player_query);
    $player_stmt->execute(['player_id' => $player_id]);
    $player = $player_stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$player) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Personagem não encontrado']);
        exit;
    }
    
    // 2. Buscar pontos de atributos não distribuídos
    $stat_points_query = "SELECT 
                            unspent_points,
                            strength_points,
                            dexterity_points,
                            intelligence_points,
                            vitality_points,
                            luck_points
                          FROM player_stat_points
                          WHERE player_id = :player_id";
    $stat_points_stmt = $pdo->prepare($stat_points_query);
    $stat_points_stmt->execute(['player_id' => $player_id]);
    $stat_points = $stat_points_stmt->fetch(PDO::FETCH_ASSOC);
    
    // Se não existir registro, criar com valores padrão
    if (!$stat_points) {
        $insert_stat_points = $pdo->prepare("
            INSERT INTO player_stat_points 
            (player_id, unspent_points, strength_points, dexterity_points, 
             intelligence_points, vitality_points, luck_points)
            VALUES (:player_id, 0, 0, 0, 0, 0, 0)
        ");
        $insert_stat_points->execute(['player_id' => $player_id]);
        
        $stat_points = [
            'unspent_points' => 0,
            'strength_points' => 0,
            'dexterity_points' => 0,
            'intelligence_points' => 0,
            'vitality_points' => 0,
            'luck_points' => 0
        ];
    }
    
    // 3. Buscar todos os itens equipados
    $equipped_query = "SELECT 
                        pi.inventory_id,
                        pi.item_template_id,
                        pi.quantity,
                        pi.durability,
                        pi.custom_properties,
                        it.item_name,
                        it.item_description,
                        it.item_type,
                        it.item_subtype,
                        it.icon_path,
                        it.equipment_slot,
                        it.required_level,
                        it.stats_json,
                        it.rarity,
                        it.value,
                        it.weight
                      FROM player_inventory pi
                      INNER JOIN item_templates it ON pi.item_template_id = it.item_id
                      WHERE pi.player_id = :player_id
                        AND pi.is_equipped = TRUE
                      ORDER BY it.equipment_slot ASC";
    $equipped_stmt = $pdo->prepare($equipped_query);
    $equipped_stmt->execute(['player_id' => $player_id]);
    $equipped_items = $equipped_stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // 4. Processar itens equipados e calcular stats totais
    $equipped_by_slot = [];
    
    // Obter stats base da classe (ou 0 se não tiver classe)
    $base_phys_atk = (int)($player['base_physical_attack'] ?? 0);
    $base_mag_atk = (int)($player['base_magic_attack'] ?? 0);
    $base_phys_def = (int)($player['base_physical_defense'] ?? 0);
    $base_mag_def = (int)($player['base_magic_defense'] ?? 0);
    $base_accuracy = (int)($player['base_accuracy'] ?? 0);
    $base_dodge = (int)($player['base_dodge'] ?? 0);
    $base_critical = (int)($player['base_critical'] ?? 0);
    $base_movement = (int)($player['base_movement'] ?? 0);
    $base_resistance = (int)($player['base_resistance'] ?? 0);
    $base_double_atk = (int)($player['base_double_attack_rate'] ?? 0);
    
    // Obter nível e atributos totais (base da classe + pontos distribuídos)
    $level = (int)$player['level'];
    $base_strength = (int)($player['base_strength'] ?? 10);
    $base_dexterity = (int)($player['base_dexterity'] ?? 10);
    $base_intelligence = (int)($player['base_intelligence'] ?? 10);
    $base_vitality = (int)($player['base_vitality'] ?? 10);
    $base_luck = (int)($player['base_luck'] ?? 10);
    
    // Atributos totais = base da classe + pontos distribuídos
    $total_strength = $base_strength + (int)$stat_points['strength_points'];
    $total_dexterity = $base_dexterity + (int)$stat_points['dexterity_points'];
    $total_intelligence = $base_intelligence + (int)$stat_points['intelligence_points'];
    $total_vitality = $base_vitality + (int)$stat_points['vitality_points'];
    $total_luck = $base_luck + (int)$stat_points['luck_points'];
    
    // Ganhos por nível
    $level_hp_bonus = $level * 20;
    $level_mp_bonus = $level * 20;
    $level_phys_atk = $level * 5;
    $level_mag_atk = $level * 5;
    $level_phys_def = $level * 3;
    $level_mag_def = $level * 3;
    
    // Bônus de atributos (conforme fórmulas)
    // Strength: cada 5 = 2 Phys Atk, cada 10 = 1 Crit Atk e 1 Double Atk
    $strength_phys_atk = floor($total_strength / 5) * 2;
    $strength_crit_atk = floor($total_strength / 10);
    $strength_double_atk = floor($total_strength / 10);
    
    // Dexterity: cada 5 = 1 Accuracy, cada 10 = 1 Phys Atk e 1 Dodge
    $dexterity_accuracy = floor($total_dexterity / 5);
    $dexterity_phys_atk = floor($total_dexterity / 10);
    $dexterity_dodge = floor($total_dexterity / 10);
    
    // Intelligence: cada 5 = 2 Mag Atk, cada 10 = 1 Crit Atk e 30 Mana Bonus
    $intelligence_mag_atk = floor($total_intelligence / 5) * 2;
    $intelligence_crit_atk = floor($total_intelligence / 10);
    $intelligence_mana_bonus = floor($total_intelligence / 10) * 30;
    
    // Vitality: cada 5 = 1 Crit Res, cada 10 = 1 Double Res e 30 HP Bonus
    $vitality_crit_res = floor($total_vitality / 5);
    $vitality_double_res = floor($total_vitality / 10);
    $vitality_hp_bonus = floor($total_vitality / 10) * 30;
    
    // Inicializar stats totais com base da classe + ganhos por nível + bônus de atributos
    $total_stats = [
        'strength' => $total_strength,
        'dexterity' => $total_dexterity,
        'intelligence' => $total_intelligence,
        'vitality' => $total_vitality,
        'luck' => $total_luck,
        'health_bonus' => $vitality_hp_bonus,
        'mana_bonus' => $intelligence_mana_bonus,
        'defense' => $base_phys_def + $level_phys_def,
        'magic_defense' => $base_mag_def + $level_mag_def,
        'attack' => $base_phys_atk + $level_phys_atk + $strength_phys_atk + $dexterity_phys_atk,
        'magic_attack' => $base_mag_atk + $level_mag_atk + $intelligence_mag_atk,
        'accuracy' => $base_accuracy + $dexterity_accuracy,
        'dodge' => $base_dodge + $dexterity_dodge,
        'critical' => $base_critical + $strength_crit_atk + $intelligence_crit_atk,
        'movement' => $base_movement,
        'resistance' => $base_resistance + $vitality_crit_res,
        'double_attack_rate' => $base_double_atk + $strength_double_atk
    ];
    
    foreach ($equipped_items as $item) {
        $equipment_slot = $item['equipment_slot'];
        $stats = [];
        
        if (!empty($item['stats_json'])) {
            $stats = json_decode($item['stats_json'], true) ?: [];
        }
        
        // Adicionar item ao array por slot
        $equipped_by_slot[$equipment_slot] = [
            'inventory_id' => (int)$item['inventory_id'],
            'item_template_id' => (int)$item['item_template_id'],
            'item_name' => $item['item_name'],
            'item_description' => $item['item_description'],
            'item_type' => $item['item_type'],
            'item_subtype' => $item['item_subtype'],
            'icon_path' => $item['icon_path'],
            'equipment_slot' => $equipment_slot,
            'required_level' => (int)$item['required_level'],
            'durability' => (float)$item['durability'],
            'rarity' => $item['rarity'],
            'value' => (int)$item['value'],
            'weight' => (float)$item['weight'],
            'stats' => $stats
        ];
        
        // Somar stats dos equipamentos
        if (isset($stats['strength'])) $total_stats['strength'] += (int)$stats['strength'];
        if (isset($stats['dexterity'])) $total_stats['dexterity'] += (int)$stats['dexterity'];
        if (isset($stats['intelligence'])) $total_stats['intelligence'] += (int)$stats['intelligence'];
        if (isset($stats['vitality'])) $total_stats['vitality'] += (int)$stats['vitality'];
        if (isset($stats['agility'])) $total_stats['agility'] += (int)$stats['agility'];
        if (isset($stats['constitution'])) $total_stats['constitution'] += (int)$stats['constitution'];
        if (isset($stats['luck'])) $total_stats['luck'] += (int)$stats['luck'];
        if (isset($stats['health_bonus'])) $total_stats['health_bonus'] += (int)$stats['health_bonus'];
        if (isset($stats['mana_bonus'])) $total_stats['mana_bonus'] += (int)$stats['mana_bonus'];
        if (isset($stats['defense'])) $total_stats['defense'] += (int)$stats['defense'];
        if (isset($stats['magic_defense'])) $total_stats['magic_defense'] += (int)$stats['magic_defense'];
        if (isset($stats['attack'])) $total_stats['attack'] += (int)$stats['attack'];
        if (isset($stats['magic_attack'])) $total_stats['magic_attack'] += (int)$stats['magic_attack'];
        if (isset($stats['accuracy'])) $total_stats['accuracy'] += (int)$stats['accuracy'];
        if (isset($stats['dodge'])) $total_stats['dodge'] += (int)$stats['dodge'];
        if (isset($stats['critical'])) $total_stats['critical'] += (int)$stats['critical'];
        if (isset($stats['movement'])) $total_stats['movement'] += (int)$stats['movement'];
        if (isset($stats['resistance'])) $total_stats['resistance'] += (int)$stats['resistance'];
        if (isset($stats['double_attack_rate'])) $total_stats['double_attack_rate'] += (int)$stats['double_attack_rate'];
    }
    
    // 5. Calcular stats finais (base da classe + nível + bônus de atributos + equipamentos)
    $base_health = (int)($player['base_health'] ?? $player['max_health']);
    $base_mana = (int)($player['base_mana'] ?? $player['max_mana']);
    $final_max_health = $base_health + $level_hp_bonus + $total_stats['health_bonus'];
    $final_max_mana = $base_mana + $level_mp_bonus + $total_stats['mana_bonus'];
    
    // 6. Buscar informações de EXP do nível atual
    $current_level = (int)$player['level'];
    $current_exp = (int)$player['experience'];
    
    // Buscar EXP necessário para próximo nível da tabela player_levels
    $level_query = "SELECT 
                        exp_for_next_level,
                        exp_required
                    FROM player_levels
                    WHERE level_number = :level";
    $level_stmt = $pdo->prepare($level_query);
    $level_stmt->execute(['level' => $current_level]);
    $level_data = $level_stmt->fetch(PDO::FETCH_ASSOC);
    
    if ($level_data) {
        $exp_for_next_level = (int)$level_data['exp_for_next_level'];
        $exp_required_for_current = (int)$level_data['exp_required'];
        
        // Calcular progresso: (EXP atual - EXP necessário para nível atual) / EXP para próximo nível
        $exp_in_current_level = $current_exp - $exp_required_for_current;
        $exp_progress = $exp_for_next_level > 0 ? ($exp_in_current_level / $exp_for_next_level) * 100 : 0;
        
        // Limitar entre 0 e 100
        if ($exp_progress < 0) $exp_progress = 0;
        if ($exp_progress > 100) $exp_progress = 100;
    } else {
        // Fallback: usar fórmula simples se nível não existir na tabela
        $exp_for_next_level = $current_level * 1000;
        $exp_progress = $exp_for_next_level > 0 ? ($current_exp / $exp_for_next_level) * 100 : 0;
    }
    
    // 6. Montar resposta completa
    http_response_code(200);
    echo json_encode([
        'success' => true,
        'character' => [
            'player_id' => (int)$player['id'],
            'character_name' => $player['character_name'],
            'level' => $current_level,
            'experience' => $current_exp,
            'exp_for_next_level' => $exp_for_next_level,
            'exp_progress_percent' => round($exp_progress, 2),
            'current_zone' => $player['current_zone'],
            'position' => [
                'x' => (float)$player['pos_x'],
                'y' => (float)$player['pos_y'],
                'z' => (float)$player['pos_z']
            ],
            'class' => [
                'class_id' => (int)$player['class_id'] ?: 0,
                'class_name' => $player['class_name'] ?: ''
            ],
            'faction' => [
                'faction_id' => (int)$player['faction_id'] ?: 0,
                'faction_name' => $player['faction_name'] ?: ''
            ],
            'guild' => [
                'guild_id' => (int)$player['current_guild_id'] ?: 0,
                'guild_name' => $player['guild_name'] ?: ''
            ],
            'title' => [
                'title_id' => (int)$player['equipped_title_id'] ?: 0,
                'title_name' => $player['title_name'] ?: ''
            ],
            'selected_class' => (int)$player['selected_class'] ?: 0,
            'pvp_stats' => [
                'pvp' => (int)$player['pvp'],
                'chaos' => (int)$player['chaos'],
                'honor' => (int)$player['honor']
            ],
            'stats' => [
                'base' => [
                    'strength' => $base_strength,
                    'dexterity' => $base_dexterity,
                    'intelligence' => $base_intelligence,
                    'vitality' => $base_vitality,
                    'luck' => $base_luck
                ],
                'total' => [
                    'strength' => $total_stats['strength'],
                    'dexterity' => $total_stats['dexterity'],
                    'intelligence' => $total_stats['intelligence'],
                    'vitality' => $total_stats['vitality'],
                    'luck' => $total_stats['luck']
                ],
                'health' => [
                    'current' => (int)$player['health'],
                    'max_base' => $base_health,
                    'max_total' => $final_max_health,
                    'bonus' => $total_stats['health_bonus']
                ],
                'mana' => [
                    'current' => (int)$player['mana'],
                    'max_base' => $base_mana,
                    'max_total' => $final_max_mana,
                    'bonus' => $total_stats['mana_bonus']
                ],
                'stamina' => [
                    'current' => (int)$player['stamina'],
                    'max' => (int)$player['max_stamina']
                ],
                'combat' => [
                    'physical_attack' => $total_stats['attack'],
                    'magic_attack' => $total_stats['magic_attack'],
                    'physical_defense' => $total_stats['defense'],
                    'magic_defense' => $total_stats['magic_defense'],
                    'accuracy' => $total_stats['accuracy'],
                    'dodge' => $total_stats['dodge'],
                    'critical' => $total_stats['critical'],
                    'movement' => $total_stats['movement'],
                    'resistance' => $total_stats['resistance'],
                    'double_attack_rate' => $total_stats['double_attack_rate']
                ]
            ],
            'equipped_items' => $equipped_by_slot,
            'stat_points' => [
                'unspent_points' => (int)$stat_points['unspent_points'],
                'strength_points' => (int)$stat_points['strength_points'],
                'dexterity_points' => (int)$stat_points['dexterity_points'],
                'intelligence_points' => (int)$stat_points['intelligence_points'],
                'vitality_points' => (int)$stat_points['vitality_points'],
                'luck_points' => (int)$stat_points['luck_points']
            ],
            'created_at' => $player['created_at'],
            'last_played_at' => $player['last_played_at']
        ]
    ]);
    
} catch (PDOException $e) {
    error_log("Erro ao obter informações do personagem: " . $e->getMessage());
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erro ao obter informações do personagem',
        'error' => $e->getMessage()
    ]);
}
?>

