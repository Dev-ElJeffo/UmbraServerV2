<?php
/**
 * Helpers para NPC vendedor (catálogo, compra, venda)
 */

const NPC_VENDOR_PROXIMITY_MARGIN = 50.0;

/**
 * Primeiro slot livre na bolsa (0-49), igual loja pessoal.
 */
function npcVendorFindFreeSlot(PDO $pdo, int $player_id): ?int
{
    $stmt = $pdo->prepare("
        SELECT slot_index FROM player_inventory
        WHERE player_id = ? AND slot_index >= 0 AND slot_index < 50 AND auction_listing_id IS NULL
    ");
    $stmt->execute([$player_id]);
    $occupied = $stmt->fetchAll(PDO::FETCH_COLUMN);
    for ($s = 0; $s < 50; $s++) {
        if (!in_array($s, $occupied, true)) {
            return $s;
        }
    }
    return null;
}

/**
 * Extrai posição opcional enviada pelo cliente (pos_x/y/z).
 *
 * @return array{pos_x:float,pos_y:float,pos_z:float}|null
 */
function npcVendorExtractClientPos(array $data): ?array
{
    if (!isset($data['pos_x'], $data['pos_y'], $data['pos_z'])) {
        return null;
    }
    if (!is_numeric($data['pos_x']) || !is_numeric($data['pos_y']) || !is_numeric($data['pos_z'])) {
        return null;
    }
    return [
        'pos_x' => (float)$data['pos_x'],
        'pos_y' => (float)$data['pos_y'],
        'pos_z' => (float)$data['pos_z'],
    ];
}

/**
 * Carrega instância + template + vendedor e valida proximidade do jogador.
 *
 * @param array{pos_x:float,pos_y:float,pos_z:float}|null $client_pos
 * @return array{ok:bool,message?:string,instance?:array,vendor?:array}
 */
function npcVendorLoadContext(PDO $pdo, int $player_id, int $npc_instance_id, bool $require_vendor = false, ?array $client_pos = null): array
{
    if ($npc_instance_id <= 0 || $player_id <= 0) {
        return ['ok' => false, 'message' => 'Parâmetros inválidos.'];
    }

    $stmt = $pdo->prepare("
        SELECT
            ni.npc_instance_id,
            ni.zone_id,
            ni.pos_x,
            ni.pos_y,
            ni.pos_z,
            ni.is_dead,
            nt.npc_template_id,
            nt.npc_name,
            nt.is_attackable,
            nt.interaction_radius,
            nt.has_vendor,
            nt.has_quest_dialog,
            nt.dialog_title,
            nt.dialog_text,
            nv.vendor_id,
            nv.vendor_display_name,
            nv.sell_rate_percent
        FROM npc_instances ni
        INNER JOIN npc_templates nt ON nt.npc_template_id = ni.npc_template_id
        LEFT JOIN npc_vendors nv ON nv.npc_template_id = nt.npc_template_id
        WHERE ni.npc_instance_id = ?
        LIMIT 1
    ");
    $stmt->execute([$npc_instance_id]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    if (!$row) {
        return ['ok' => false, 'message' => 'NPC não encontrado.'];
    }
    if (!empty($row['is_dead'])) {
        return ['ok' => false, 'message' => 'NPC indisponível.'];
    }
    if ($require_vendor && empty($row['has_vendor'])) {
        return ['ok' => false, 'message' => 'Este NPC não é vendedor.'];
    }
    if ($require_vendor && empty($row['vendor_id'])) {
        return ['ok' => false, 'message' => 'Vendedor não configurado para este NPC.'];
    }

    $pstmt = $pdo->prepare("
        SELECT id, current_zone, pos_x, pos_y, pos_z
        FROM players
        WHERE id = ?
        LIMIT 1
    ");
    $pstmt->execute([$player_id]);
    $player = $pstmt->fetch(PDO::FETCH_ASSOC);
    if (!$player) {
        return ['ok' => false, 'message' => 'Jogador não encontrado.'];
    }

    $playerZone = (int)($player['current_zone'] ?? 0);
    $npcZone = (int)($row['zone_id'] ?? 0);
    if ($playerZone > 0 && $npcZone > 0 && $playerZone !== $npcZone) {
        return ['ok' => false, 'message' => 'NPC está em outra zona.'];
    }

    $playerPosX = (float)$player['pos_x'];
    $playerPosY = (float)$player['pos_y'];
    $playerPosZ = (float)$player['pos_z'];
    if ($client_pos !== null) {
        $playerPosX = $client_pos['pos_x'];
        $playerPosY = $client_pos['pos_y'];
        $playerPosZ = $client_pos['pos_z'];
    }

    $dx = $playerPosX - (float)$row['pos_x'];
    $dy = $playerPosY - (float)$row['pos_y'];
    $dz = $playerPosZ - (float)$row['pos_z'];
    $dist = sqrt($dx * $dx + $dy * $dy + $dz * $dz);
    $maxDist = (float)$row['interaction_radius'] + NPC_VENDOR_PROXIMITY_MARGIN;
    if ($dist > $maxDist) {
        return ['ok' => false, 'message' => 'Você está longe demais do NPC.'];
    }

    return [
        'ok' => true,
        'instance' => $row,
        'vendor' => [
            'vendor_id' => (int)($row['vendor_id'] ?? 0),
            'vendor_display_name' => $row['vendor_display_name'] ?? $row['npc_name'],
            'sell_rate_percent' => (int)($row['sell_rate_percent'] ?? 50),
        ],
        'distance' => $dist,
    ];
}

/**
 * Preço de venda ao NPC (% do value do template).
 */
function npcVendorCalcSellUnitPrice(int $item_value, int $sell_rate_percent): int
{
    if ($item_value <= 0) {
        return 0;
    }
    $rate = max(0, min(100, $sell_rate_percent));
    $price = (int)floor($item_value * $rate / 100);
    return max(1, $price);
}

/**
 * Monta entrada de stock para JSON da API.
 */
function npcVendorFormatStockRow(array $stockRow, array $templateRow): array
{
    $stats = [];
    if (!empty($templateRow['stats_json'])) {
        $decoded = json_decode($templateRow['stats_json'], true);
        $stats = is_array($decoded) ? $decoded : [];
    } elseif (!empty($templateRow['stats']) && is_array($templateRow['stats'])) {
        $stats = $templateRow['stats'];
    }

    return [
        'stock_id' => (int)$stockRow['stock_id'],
        'item_template_id' => (int)$stockRow['item_template_id'],
        'item_name' => $templateRow['item_name'] ?? '',
        'item_description' => $templateRow['item_description'] ?? '',
        'icon_path' => $templateRow['icon_path'] ?? '',
        'item_type' => $templateRow['item_type'] ?? '',
        'item_subtype' => $templateRow['item_subtype'] ?? '',
        'equipment_slot' => $templateRow['equipment_slot'] ?? 'none',
        'required_level' => (int)($templateRow['required_level'] ?? 0),
        'rarity' => isset($templateRow['rarity']) && $templateRow['rarity'] !== '' && $templateRow['rarity'] !== null
            ? (is_numeric($templateRow['rarity'])
                ? (int)$templateRow['rarity']
                : strtolower(trim((string)$templateRow['rarity'])))
            : 'common',
        'max_stack_size' => (int)($templateRow['max_stack_size'] ?? 1),
        'buy_price_gold' => (int)$stockRow['buy_price_gold'],
        'stock_qty' => (int)$stockRow['stock_qty'],
        'max_buy_per_tx' => (int)$stockRow['max_buy_per_tx'],
        'sort_order' => (int)$stockRow['sort_order'],
        'value' => (int)($templateRow['value'] ?? 0),
        'weight' => isset($templateRow['weight']) ? (float)$templateRow['weight'] : 0.0,
        'can_be_refined' => ((int)($templateRow['can_be_refined'] ?? 0)) === 1,
        'tradeable' => !isset($templateRow['tradeable']) ? true : (((int)$templateRow['tradeable']) === 1),
        'stats' => $stats,
    ];
}
