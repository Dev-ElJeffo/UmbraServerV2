<?php
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

$data = json_decode(file_get_contents('php://input'), true) ?? [];
require_once __DIR__ . '/require_admin_auth.php';
requireAdminAuth($data);

$playerId = (int)($data['player_id'] ?? 0);
if ($playerId <= 0) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'player_id é obrigatório'], JSON_UNESCAPED_UNICODE);
    exit;
}

try {
    $pdo = getConnection();

    $playerStmt = $pdo->prepare("
        SELECT
            p.id AS player_id,
            p.account_id,
            a.username AS account_username,
            p.character_name,
            p.level,
            p.pos_x,
            p.pos_y,
            p.pos_z,
            p.current_zone,
            p.health,
            p.max_health,
            p.mana,
            p.max_mana,
            p.gold,
            p.last_played_at,
            c.class_name,
            g.guild_name
        FROM players p
        INNER JOIN accounts a ON a.id = p.account_id
        LEFT JOIN classes c ON c.class_id = p.class_id
        LEFT JOIN guilds g ON g.guild_id = p.current_guild_id
        WHERE p.id = :player_id
        LIMIT 1
    ");
    $playerStmt->execute([':player_id' => $playerId]);
    $player = $playerStmt->fetch(PDO::FETCH_ASSOC);
    if (!$player) {
        http_response_code(404);
        echo json_encode(['success' => false, 'message' => 'Player não encontrado'], JSON_UNESCAPED_UNICODE);
        exit;
    }

    $partyStmt = $pdo->prepare("
        SELECT pm.party_id, p2.character_name
        FROM party_members pm
        INNER JOIN players p2 ON p2.id = pm.player_id
        WHERE pm.party_id = (
            SELECT party_id FROM party_members WHERE player_id = :player_id LIMIT 1
        )
        ORDER BY p2.character_name
    ");
    $partyStmt->execute([':player_id' => $playerId]);
    $partyRows = $partyStmt->fetchAll(PDO::FETCH_ASSOC);

    $inventoryStmt = $pdo->prepare("
        SELECT pi.slot_index, pi.item_template_id, it.item_name, pi.quantity
        FROM player_inventory pi
        INNER JOIN item_templates it ON it.item_id = pi.item_template_id
        WHERE pi.player_id = :player_id
        ORDER BY pi.slot_index ASC
        LIMIT 40
    ");
    $inventoryStmt->execute([':player_id' => $playerId]);
    $inventoryRows = $inventoryStmt->fetchAll(PDO::FETCH_ASSOC);

    $storageStmt = $pdo->prepare("
        SELECT s.slot_index, pi.item_template_id, it.item_name, pi.quantity
        FROM player_storage s
        INNER JOIN player_inventory pi ON pi.inventory_id = s.inventory_id
        INNER JOIN item_templates it ON it.item_id = pi.item_template_id
        WHERE s.player_id = :player_id
        ORDER BY s.slot_index ASC
        LIMIT 40
    ");
    $storageStmt->execute([':player_id' => $playerId]);
    $storageRows = $storageStmt->fetchAll(PDO::FETCH_ASSOC);

    $questStmt = $pdo->prepare("
        SELECT pq.quest_id, q.title AS quest_title, pq.status
        FROM player_quests pq
        INNER JOIN quests q ON q.quest_id = pq.quest_id
        WHERE pq.player_id = :player_id
        ORDER BY COALESCE(pq.completed_at, pq.accepted_at) DESC
        LIMIT 20
    ");
    $questStmt->execute([':player_id' => $playerId]);
    $questRows = $questStmt->fetchAll(PDO::FETCH_ASSOC);

    $auctionStmt = $pdo->prepare("
        SELECT listing_id, price_gold, status
        FROM auction_listings
        WHERE seller_player_id = :player_id
        ORDER BY created_at DESC
        LIMIT 10
    ");
    $auctionStmt->execute([':player_id' => $playerId]);
    $auctionRows = $auctionStmt->fetchAll(PDO::FETCH_ASSOC);

    $shopStmt = $pdo->prepare("
        SELECT shop_id, shop_name, status
        FROM personal_shops
        WHERE seller_player_id = :player_id
        ORDER BY opened_at DESC
        LIMIT 10
    ");
    $shopStmt->execute([':player_id' => $playerId]);
    $shopRows = $shopStmt->fetchAll(PDO::FETCH_ASSOC);

    $partyLabel = empty($partyRows) ? 'Sem party' : ('Party #' . (int)$partyRows[0]['party_id'] . ' (' . count($partyRows) . ' membros)');
    $economy = [];
    foreach ($auctionRows as $row) {
        $economy[] = [
            'kind' => 'auction',
            'label' => 'Leilão #' . (int)$row['listing_id'] . ' | ' . $row['status'] . ' | ' . (int)$row['price_gold'] . ' gold',
        ];
    }
    foreach ($shopRows as $row) {
        $economy[] = [
            'kind' => 'shop',
            'label' => 'Loja #' . (int)$row['shop_id'] . ' | ' . $row['status'] . ' | ' . $row['shop_name'],
        ];
    }

    $inventoryLabels = [];
    foreach ($inventoryRows as $row) {
        $slot = (int)$row['slot_index'];
        $name = (string)$row['item_name'];
        $qty = (int)$row['quantity'];
        $tpl = (int)$row['item_template_id'];
        $inventoryLabels[] = [
            'location' => 'INV',
            'slot_index' => $slot,
            'item_template_id' => $tpl,
            'item_name' => $name,
            'quantity' => $qty,
            'slot_label' => 'INV[' . $slot . '] ' . $name . ' x' . $qty,
        ];
    }
    foreach ($storageRows as $row) {
        $slot = (int)$row['slot_index'];
        $name = (string)$row['item_name'];
        $qty = (int)$row['quantity'];
        $tpl = (int)$row['item_template_id'];
        $inventoryLabels[] = [
            'location' => 'STO',
            'slot_index' => $slot,
            'item_template_id' => $tpl,
            'item_name' => $name,
            'quantity' => $qty,
            'slot_label' => 'STO[' . $slot . '] ' . $name . ' x' . $qty,
        ];
    }

    $questLabels = [];
    foreach ($questRows as $row) {
        $title = (string)$row['quest_title'];
        $status = (string)$row['status'];
        $qid = (int)$row['quest_id'];
        $questLabels[] = [
            'quest_id' => $qid,
            'quest_title' => $title,
            'status' => $status,
            'label' => '[' . $status . '] ' . $title,
        ];
    }

    $partyLabels = [];
    foreach ($partyRows as $row) {
        $partyLabels[] = ['kind' => 'party', 'label' => 'Party #' . (int)$row['party_id'] . ' | ' . $row['character_name']];
    }

    $friendLabels = [];
    try {
        $friendStmt = $pdo->prepare("
            SELECT p.id AS friend_player_id, p.character_name
            FROM friends f
            INNER JOIN players p ON p.id = IF(f.player1_id = :pid, f.player2_id, f.player1_id)
            WHERE f.player1_id = :pid OR f.player2_id = :pid
            ORDER BY p.character_name
            LIMIT 50
        ");
        $friendStmt->execute([':pid' => $playerId]);
        foreach ($friendStmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
            $friendLabels[] = [
                'kind' => 'friend',
                'label' => 'Amigo #' . (int)$row['friend_player_id'] . ' | ' . $row['character_name'],
                'friend_player_id' => (int)$row['friend_player_id'],
                'character_name' => $row['character_name'],
            ];
        }
    } catch (Throwable $fe) {
        error_log('[admin/player_inspector] friends: ' . $fe->getMessage());
    }

    echo json_encode([
        'success' => true,
        'player' => [
            'player_id' => (int)$player['player_id'],
            'account_id' => (int)$player['account_id'],
            'account_username' => $player['account_username'],
            'character_name' => $player['character_name'],
            'level' => (int)$player['level'],
            'pos_x' => (float)$player['pos_x'],
            'pos_y' => (float)$player['pos_y'],
            'pos_z' => (float)$player['pos_z'],
            'zone_id' => preg_replace('/\\D+/', '', (string)$player['current_zone']) ?: 0,
            'health' => (int)$player['health'],
            'max_health' => (int)$player['max_health'],
            'mana' => (int)$player['mana'],
            'max_mana' => (int)$player['max_mana'],
            'gold' => (int)$player['gold'],
            'class_name' => $player['class_name'] ?? '',
            'guild_name' => $player['guild_name'] ?? '',
            'party_label' => $partyLabel,
            'online' => !empty($player['last_played_at']),
        ],
        'summary' => [
            'active_quest_count' => count(array_filter($questRows, static fn($q) => in_array($q['status'], ['active', 'ready'], true))),
            'completed_quest_count' => count(array_filter($questRows, static fn($q) => $q['status'] === 'completed')),
            'inventory_count' => count($inventoryRows),
            'storage_count' => count($storageRows),
            'active_auction_count' => count(array_filter($auctionRows, static fn($r) => $r['status'] === 'active')),
            'open_shop_count' => count(array_filter($shopRows, static fn($r) => $r['status'] === 'open')),
            'friend_count' => count($friendLabels),
        ],
        'inventory_items' => $inventoryLabels,
        'quests' => $questLabels,
        'party_members' => $partyLabels,
        'friends' => $friendLabels,
        'economy' => $economy,
    ], JSON_UNESCAPED_UNICODE);
} catch (Throwable $e) {
    error_log('[admin/player_inspector] ' . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno'], JSON_UNESCAPED_UNICODE);
}
