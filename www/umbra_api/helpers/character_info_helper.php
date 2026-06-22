<?php
/**
 * Helper para montar o payload "character" usado por get_character_info e get_public_info.
 * Retorna a mesma estrutura em ambos os casos para o cliente UE parsear igual.
 *
 * @param PDO $pdo
 * @param int $player_id
 * @param array $options ['create_stat_points_if_missing' => bool] Se true, insere linha em player_stat_points quando não existir (apenas para próprio personagem).
 * @return array|null Array 'character' ou null se jogador não existir.
 */
require_once __DIR__ . '/stat_key_mapping.php';

/**
 * Lista passivas aprendidas e se health_below_percent está ativa (debug).
 *
 * @return array<int, array<string, mixed>>
 */
function collect_passive_debug(PDO $pdo, int $player_id, int $health_pct): array
{
    $passives = [];
    $passive_stmt = $pdo->prepare("
        SELECT s.skill_id, s.skill_name, s.effects_json
        FROM player_skills ps
        INNER JOIN skills s ON ps.skill_id = s.skill_id
        WHERE ps.player_id = :player_id AND s.type_id = 2
    ");
    $passive_stmt->execute(['player_id' => $player_id]);
    while ($prow = $passive_stmt->fetch(PDO::FETCH_ASSOC)) {
        $effects_raw = (string)($prow['effects_json'] ?? '');
        if ($effects_raw === '' || $effects_raw === 'null') {
            continue;
        }
        $effects = json_decode($effects_raw, true);
        if (!is_array($effects)) {
            continue;
        }
        foreach ($effects as $item) {
            if (!is_array($item)) {
                continue;
            }
            $cond = $item['conditions_json'] ?? [];
            if (!is_array($cond)) {
                continue;
            }
            $threshold = (int)($cond['health_below_percent'] ?? 0);
            if ($threshold <= 0) {
                continue;
            }
            $passives[] = [
                'skill_id' => (int)$prow['skill_id'],
                'skill_name' => (string)($prow['skill_name'] ?? ''),
                'condition' => 'health_below_percent:' . $threshold,
                'condition_met' => $health_pct < $threshold,
                'health_percent' => $health_pct,
                'target_stat' => (string)($item['target_stat'] ?? ''),
                'value_flat' => (int)($item['value_flat'] ?? $item['value'] ?? 0),
                'value_percent' => (int)($item['value_percent'] ?? 0),
            ];
        }
    }
    return $passives;
}

function get_character_info_data(PDO $pdo, int $player_id, array $options = []): ?array
{
    $create_stat_points_if_missing = $options['create_stat_points_if_missing'] ?? false;
    $include_debug = !empty($options['include_debug']);

    // 1. Buscar informações básicas do personagem
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
                        c.base_critical_resistance,
                        c.base_double_attack_resistance,
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
        return null;
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

    if (!$stat_points) {
        if ($create_stat_points_if_missing) {
            $insert_stat_points = $pdo->prepare("
                INSERT INTO player_stat_points 
                (player_id, unspent_points, strength_points, dexterity_points, 
                 intelligence_points, vitality_points, luck_points)
                VALUES (:player_id, 0, 0, 0, 0, 0, 0)
            ");
            $insert_stat_points->execute(['player_id' => $player_id]);
        }
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
                        pi.refinement_level,
                        pi.refinement_bonus_stats,
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

    $base_phys_atk = (int)($player['base_physical_attack'] ?? 0);
    $base_mag_atk = (int)($player['base_magic_attack'] ?? 0);
    $base_phys_def = (int)($player['base_physical_defense'] ?? 0);
    $base_mag_def = (int)($player['base_magic_defense'] ?? 0);
    $base_accuracy = (int)($player['base_accuracy'] ?? 0);
    $base_dodge = (int)($player['base_dodge'] ?? 0);
    $base_critical = (int)($player['base_critical'] ?? 0);
    $base_movement = (int)($player['base_movement'] ?? 0);
    $base_resistance = (int)($player['base_critical_resistance'] ?? 0);
    $base_double_atk = (int)($player['base_double_attack_rate'] ?? 0);

    $level = (int)$player['level'];
    $base_strength = (int)($player['base_strength'] ?? 10);
    $base_dexterity = (int)($player['base_dexterity'] ?? 10);
    $base_intelligence = (int)($player['base_intelligence'] ?? 10);
    $base_vitality = (int)($player['base_vitality'] ?? 10);
    $base_luck = (int)($player['base_luck'] ?? 10);

    $total_strength = $base_strength + (int)$stat_points['strength_points'];
    $total_dexterity = $base_dexterity + (int)$stat_points['dexterity_points'];
    $total_intelligence = $base_intelligence + (int)$stat_points['intelligence_points'];
    $total_vitality = $base_vitality + (int)$stat_points['vitality_points'];
    $total_luck = $base_luck + (int)$stat_points['luck_points'];

    $level_hp_bonus = $level * 20;
    $level_mp_bonus = $level * 20;
    $level_phys_atk = $level * 5;
    $level_mag_atk = $level * 5;
    $level_phys_def = $level * 3;
    $level_mag_def = $level * 3;

    $total_stats = [
        'strength' => $total_strength,
        'dexterity' => $total_dexterity,
        'intelligence' => $total_intelligence,
        'vitality' => $total_vitality,
        'luck' => $total_luck,
        'health_bonus' => 0,
        'mana_bonus' => 0,
        'defense' => $base_phys_def + $level_phys_def,
        'magic_defense' => $base_mag_def + $level_mag_def,
        'attack' => $base_phys_atk + $level_phys_atk,
        'magic_attack' => $base_mag_atk + $level_mag_atk,
        'accuracy' => $base_accuracy,
        'dodge' => $base_dodge,
        'critical' => $base_critical,
        'movement' => $base_movement,
        'resistance' => $base_resistance,
        'double_attack_resistance' => 0,
        'double_attack_rate' => $base_double_atk
    ];

    foreach ($equipped_items as $item) {
        $equipment_slot = $item['equipment_slot'];
        $stats = [];
        $refinement_level = (int)($item['refinement_level'] ?? 0);
        $refinement_bonus_stats = [];
        if (!empty($item['stats_json']) && trim($item['stats_json']) !== '' && trim($item['stats_json']) !== 'null') {
            $decoded_stats = json_decode($item['stats_json'], true);
            if (json_last_error() === JSON_ERROR_NONE && is_array($decoded_stats)) {
                if (empty($decoded_stats)) {
                    $stats = [];
                } elseif (array_keys($decoded_stats) !== range(0, count($decoded_stats) - 1)) {
                    $stats = $decoded_stats;
                } else {
                    $stats = [];
                }
            } else {
                $stats = [];
            }
        } else {
            $stats = [];
        }

        if (!empty($item['refinement_bonus_stats']) && trim((string)$item['refinement_bonus_stats']) !== '' && trim((string)$item['refinement_bonus_stats']) !== 'null') {
            $decoded_bonus = json_decode($item['refinement_bonus_stats'], true);
            if (json_last_error() === JSON_ERROR_NONE && is_array($decoded_bonus)) {
                $refinement_bonus_stats = $decoded_bonus;
            }
        }

        // Aplicar bônus de refinação sobre os stats do item equipado
        // As chaves esperadas seguem o contrato de refine_item.php (attack, accuracy, critical, etc.).
        if (!empty($refinement_bonus_stats)) {
            foreach ($refinement_bonus_stats as $bonus_key => $bonus_value) {
                $bonus_int = (int)$bonus_value;
                if (!isset($stats[$bonus_key])) {
                    $stats[$bonus_key] = 0;
                }
                $stats[$bonus_key] += $bonus_int;
            }
        }

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
            'refinement_level' => $refinement_level,
            'refinement_bonus_stats' => $refinement_bonus_stats,
            'stats' => $stats
        ];

        if (isset($stats['strength'])) $total_stats['strength'] += (int)$stats['strength'];
        if (isset($stats['dexterity'])) $total_stats['dexterity'] += (int)$stats['dexterity'];
        if (isset($stats['intelligence'])) $total_stats['intelligence'] += (int)$stats['intelligence'];
        if (isset($stats['vitality'])) $total_stats['vitality'] += (int)$stats['vitality'];
        if (isset($stats['agility'])) $total_stats['agility'] = ($total_stats['agility'] ?? 0) + (int)$stats['agility'];
        if (isset($stats['constitution'])) $total_stats['constitution'] = ($total_stats['constitution'] ?? 0) + (int)$stats['constitution'];
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
        if (isset($stats['critical_resistance'])) {
            $total_stats['resistance'] += (int)$stats['critical_resistance'];
        } elseif (isset($stats['resistance'])) {
            $total_stats['resistance'] += (int)$stats['resistance'];
        }
        if (isset($stats['double_attack_resistance'])) {
            $total_stats['double_attack_resistance'] += (int)$stats['double_attack_resistance'];
        }
        if (isset($stats['double_attack_rate'])) $total_stats['double_attack_rate'] += (int)$stats['double_attack_rate'];
    }

    // Buffs temporários de poções (player_item_buffs)
    $now_ms_buff = (int)round(microtime(true) * 1000);
    $buffs_stmt = $pdo->prepare("
        SELECT buff_key, bonus_value
        FROM player_item_buffs
        WHERE player_id = :player_id AND expires_at_ms > :now_ms
    ");
    $buffs_stmt->execute(['player_id' => $player_id, 'now_ms' => $now_ms_buff]);
    while ($buff_row = $buffs_stmt->fetch(PDO::FETCH_ASSOC)) {
        $buff_key = (string)($buff_row['buff_key'] ?? '');
        if (strlen($buff_key) < 6 || substr($buff_key, -5) !== '_buff') {
            continue;
        }
        $stat_base = substr($buff_key, 0, -5);
        $bonus = (int)($buff_row['bonus_value'] ?? 0);
        if ($bonus <= 0) {
            continue;
        }
        if (isset($total_stats[$stat_base])) {
            $total_stats[$stat_base] += $bonus;
        }
    }

    // Buffs/debuffs de skills (active_buffs) — flat + percent, espelhando CharacterStateLoader.
    $skill_percent_mods = [];
    $skill_buffs_stmt = $pdo->prepare("
        SELECT buff_type, current_stacks, COALESCE(snapshot_json, '') AS snapshot_json
        FROM active_buffs
        WHERE target_player_id = :player_id
          AND (expires_at > NOW(3) OR is_permanent = 1)
    ");
    $skill_buffs_stmt->execute(['player_id' => $player_id]);
    while ($sb = $skill_buffs_stmt->fetch(PDO::FETCH_ASSOC)) {
        $snap_raw = (string)($sb['snapshot_json'] ?? '');
        if ($snap_raw === '' || $snap_raw === 'null') {
            continue;
        }
        $snap = json_decode($snap_raw, true);
        if (!is_array($snap) || !empty($snap['reaction_armed'])) {
            continue;
        }
        $stacks = max(1, (int)($sb['current_stacks'] ?? 1));
        $stat_key = map_target_stat_to_canonical((string)($snap['target_stat'] ?? ''));
        $flat = (int)($snap['value_flat'] ?? 0) * $stacks;
        $pct = (int)($snap['value_percent'] ?? 0) * $stacks;
        if ($stat_key !== '' && $flat !== 0) {
            apply_flat_to_totals($stat_key, $flat, $total_stats);
        }
        if ($stat_key !== '' && $pct !== 0) {
            $skill_percent_mods[] = [
                'key' => $stat_key,
                'percent' => $pct,
                'apply_to_totals' => is_totals_percent_key($stat_key),
            ];
        }
    }

    // Passivas condicionais (health_below_percent).
    $current_health = (int)($player['health'] ?? 0);
    $base_health_for_pct = (int)($player['base_health'] ?? $player['max_health'] ?? 100);
    $health_pct = ($current_health > 0 && $base_health_for_pct > 0)
        ? (int)floor($current_health * 100 / $base_health_for_pct)
        : 100;
    $passive_stmt = $pdo->prepare("
        SELECT s.effects_json
        FROM player_skills ps
        INNER JOIN skills s ON ps.skill_id = s.skill_id
        WHERE ps.player_id = :player_id AND s.type_id = 2
    ");
    $passive_stmt->execute(['player_id' => $player_id]);
    while ($prow = $passive_stmt->fetch(PDO::FETCH_ASSOC)) {
        $effects_raw = (string)($prow['effects_json'] ?? '');
        if ($effects_raw === '' || $effects_raw === 'null') {
            continue;
        }
        $effects = json_decode($effects_raw, true);
        if (!is_array($effects)) {
            continue;
        }
        foreach ($effects as $item) {
            if (!is_array($item)) {
                continue;
            }
            $cond = $item['conditions_json'] ?? [];
            if (!is_array($cond)) {
                continue;
            }
            $threshold = (int)($cond['health_below_percent'] ?? 0);
            if ($threshold <= 0 || $health_pct >= $threshold) {
                continue;
            }
            $stat_key = map_target_stat_to_canonical((string)($item['target_stat'] ?? ''));
            $flat = (int)($item['value_flat'] ?? $item['value'] ?? 0);
            $pct = (int)($item['value_percent'] ?? 0);
            if ($stat_key !== '' && $flat !== 0) {
                apply_flat_to_totals($stat_key, $flat, $total_stats);
            }
            if ($stat_key !== '' && $pct !== 0) {
                $skill_percent_mods[] = [
                    'key' => $stat_key,
                    'percent' => $pct,
                    'apply_to_totals' => is_totals_percent_key($stat_key),
                ];
            }
        }
    }

    $strength_phys_atk_bonus = floor($total_stats['strength'] / 5) * 2;
    $strength_crit_atk_bonus = floor($total_stats['strength'] / 10);
    $strength_double_atk_bonus = floor($total_stats['strength'] / 10);
    $dexterity_accuracy_bonus = floor($total_stats['dexterity'] / 5);
    $dexterity_phys_atk_bonus = floor($total_stats['dexterity'] / 10);
    $dexterity_dodge_bonus = floor($total_stats['dexterity'] / 10);
    $intelligence_mag_atk_bonus = floor($total_stats['intelligence'] / 5) * 2;
    $intelligence_crit_atk_bonus = floor($total_stats['intelligence'] / 10);
    $intelligence_mana_bonus_total = floor($total_stats['intelligence'] / 10) * 30;
    $vitality_crit_res_bonus = floor($total_stats['vitality'] / 5);
    $vitality_double_res_bonus = floor($total_stats['vitality'] / 10);
    $vitality_hp_bonus_total = floor($total_stats['vitality'] / 10) * 30;

    $total_stats['attack'] += $strength_phys_atk_bonus + $dexterity_phys_atk_bonus;
    $total_stats['magic_attack'] += $intelligence_mag_atk_bonus;
    $total_stats['accuracy'] += $dexterity_accuracy_bonus;
    $total_stats['dodge'] += $dexterity_dodge_bonus;
    $total_stats['critical'] += $strength_crit_atk_bonus + $intelligence_crit_atk_bonus;
    $total_stats['resistance'] += $vitality_crit_res_bonus;
    $total_stats['double_attack_resistance'] += $vitality_double_res_bonus;
    $total_stats['double_attack_rate'] += $strength_double_atk_bonus;
    $total_stats['health_bonus'] += $vitality_hp_bonus_total;
    $total_stats['mana_bonus'] += $intelligence_mana_bonus_total;

    foreach ($skill_percent_mods as $pm) {
        if (!empty($pm['apply_to_totals'])) {
            apply_percent_to_totals($pm['key'], (int)$pm['percent'], $total_stats);
        }
    }

    $base_health = (int)($player['base_health'] ?? $player['max_health']);
    $base_mana = (int)($player['base_mana'] ?? $player['max_mana']);
    $final_max_health = $base_health + $level_hp_bonus + $total_stats['health_bonus'];
    $final_max_mana = $base_mana + $level_mp_bonus + $total_stats['mana_bonus'];

    $combat_stats = [
        'physical_attack' => $total_stats['attack'],
        'magic_attack' => $total_stats['magic_attack'],
        'physical_defense' => $total_stats['defense'],
        'magic_defense' => $total_stats['magic_defense'],
        'accuracy' => $total_stats['accuracy'],
        'dodge' => $total_stats['dodge'],
        'critical' => $total_stats['critical'],
        'movement' => $total_stats['movement'],
        'critical_resistance' => $total_stats['resistance'],
        'double_attack_rate' => $total_stats['double_attack_rate'],
        'double_attack_resistance' => $total_stats['double_attack_resistance'],
    ];
    foreach ($skill_percent_mods as $pm) {
        if (empty($pm['apply_to_totals'])) {
            apply_percent_to_combat_stats($pm['key'], (int)$pm['percent'], $combat_stats);
        }
    }

    $current_level = (int)$player['level'];
    $current_exp = (int)$player['experience'];

    $level_query = "SELECT exp_for_next_level, exp_required FROM player_levels WHERE level_number = :level";
    $level_stmt = $pdo->prepare($level_query);
    $level_stmt->execute(['level' => $current_level]);
    $level_data = $level_stmt->fetch(PDO::FETCH_ASSOC);

    if ($level_data) {
        $exp_for_next_level = (int)$level_data['exp_for_next_level'];
        $exp_required_for_current = (int)$level_data['exp_required'];
        $exp_in_current_level = $current_exp - $exp_required_for_current;
        $exp_progress = $exp_for_next_level > 0 ? ($exp_in_current_level / $exp_for_next_level) * 100 : 0;
        if ($exp_progress < 0) $exp_progress = 0;
        if ($exp_progress > 100) $exp_progress = 100;
    } else {
        $exp_for_next_level = $current_level * 1000;
        $exp_progress = $exp_for_next_level > 0 ? ($current_exp / $exp_for_next_level) * 100 : 0;
    }

    $character_payload = [
        'player_id' => (int)$player['id'],
        'character_name' => $player['character_name'],
        'level' => $current_level,
        'experience' => $current_exp,
        'exp_for_next_level' => $exp_for_next_level ?? ($current_level * 1000),
        'exp_progress_percent' => round($exp_progress ?? 0, 2),
        'current_zone' => $player['current_zone'],
        'position' => [
            'x' => (float)$player['pos_x'],
            'y' => (float)$player['pos_y'],
            'z' => (float)$player['pos_z']
        ],
        'class' => [
            'class_id' => (int)($player['class_id'] ?: 0),
            'class_name' => $player['class_name'] ?: ''
        ],
        'faction' => [
            'faction_id' => (int)($player['faction_id'] ?: 0),
            'faction_name' => $player['faction_name'] ?: ''
        ],
        'guild' => [
            'guild_id' => (int)($player['current_guild_id'] ?: 0),
            'guild_name' => $player['guild_name'] ?: ''
        ],
        'title' => [
            'title_id' => (int)($player['equipped_title_id'] ?: 0),
            'title_name' => $player['title_name'] ?: ''
        ],
        'selected_class' => (int)($player['selected_class'] ?: 0),
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
            'combat' => $combat_stats
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
    ];

    if ($include_debug) {
        $character_payload['debug'] = [
            'health_percent' => $health_pct,
            'passives' => collect_passive_debug($pdo, $player_id, $health_pct),
        ];
    }

    return $character_payload;
}
