<?php
/**
 * Umbra Eternum - API de Skills
 * Endpoint: set_skillbar.php
 * Método: POST
 *
 * Permite ao jogador configurar um slot da barra de skills (skill OU consumível).
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

require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../helpers/jwt_helper.php';

try {
    $data = json_decode(file_get_contents('php://input'), true);

    $jwtResult = validateJWTRequest($data, $_SERVER);
    if (!$jwtResult['valid']) {
        http_response_code(401);
        echo json_encode(['success' => false, 'message' => $jwtResult['error']]);
        exit;
    }

    $playerId = null;
    if (isset($data['player_id']) && is_numeric($data['player_id'])) {
        $playerId = (int)$data['player_id'];
    } else {
        $playerId = $jwtResult['payload']['player_id'] ?? null;
    }

    $slotIndex = $data['slot_index'] ?? null;
    $keybind = $data['keybind'] ?? null;

    $rawSkillId = $data['skill_id'] ?? null;
    if ($rawSkillId === null || $rawSkillId === '' || (int)$rawSkillId <= 0) {
        $skillId = null;
    } else {
        $skillId = (int)$rawSkillId;
    }

    $rawItemTemplateId = $data['item_template_id'] ?? null;
    if ($rawItemTemplateId === null || $rawItemTemplateId === '' || (int)$rawItemTemplateId <= 0) {
        $itemTemplateId = null;
    } else {
        $itemTemplateId = (int)$rawItemTemplateId;
    }

    if (!$playerId) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'player_id não encontrado']);
        exit;
    }

    if ($slotIndex === null || $slotIndex < 0 || $slotIndex >= 20) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'slot_index inválido (0-19)']);
        exit;
    }

    $accountId = $jwtResult['payload']['account_id'] ?? null;
    if (!$accountId) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'account_id não encontrado no token']);
        exit;
    }

    $pdo = getConnection();
    $ownStmt = $pdo->prepare('SELECT id FROM players WHERE id = :player_id AND account_id = :account_id');
    $ownStmt->execute([':player_id' => $playerId, ':account_id' => (int)$accountId]);
    if (!$ownStmt->fetch(PDO::FETCH_ASSOC)) {
        http_response_code(403);
        echo json_encode(['success' => false, 'message' => 'Acesso negado: personagem não pertence à sua conta']);
        exit;
    }

    $keybindOnly = !empty($data['keybind_only']);

    if ($keybindOnly) {
        $keybindValue = ($keybind === null || $keybind === '') ? null : (string)$keybind;

        if (!empty($keybindValue)) {
            $stmtClear = $pdo->prepare('
                UPDATE player_skillbar
                SET keybind = \'\', updated_at = NOW()
                WHERE player_id = :player_id
                  AND slot_index != :slot_index
                  AND keybind = :keybind
            ');
            $stmtClear->execute([
                ':player_id' => $playerId,
                ':slot_index' => $slotIndex,
                ':keybind' => $keybindValue,
            ]);
        }

        $stmt = $pdo->prepare('
            INSERT INTO player_skillbar (player_id, slot_index, keybind)
            VALUES (:player_id, :slot_index, :keybind)
            ON DUPLICATE KEY UPDATE keybind = VALUES(keybind), updated_at = NOW()
        ');
        $stmt->execute([
            ':player_id' => $playerId,
            ':slot_index' => $slotIndex,
            ':keybind' => $keybindValue
        ]);

        echo json_encode([
            'success' => true,
            'message' => "Atalho do slot {$slotIndex} atualizado",
            'data' => [
                'slot_index' => (int)$slotIndex,
                'keybind' => $keybindValue
            ]
        ], JSON_UNESCAPED_UNICODE);
        exit;
    }

    if ($skillId !== null && $itemTemplateId !== null) {
        http_response_code(400);
        echo json_encode(['success' => false, 'message' => 'Informe apenas skill_id ou item_template_id, não ambos']);
        exit;
    }

    if ($skillId !== null && $skillId > 0) {
        $stmt = $pdo->prepare("
            SELECT ps.skill_id, s.skill_name
            FROM player_skills ps
            JOIN skills s ON ps.skill_id = s.skill_id
            WHERE ps.player_id = :player_id AND ps.skill_id = :skill_id
        ");
        $stmt->execute([':player_id' => $playerId, ':skill_id' => $skillId]);
        $playerSkill = $stmt->fetch(PDO::FETCH_ASSOC);

        if (!$playerSkill) {
            http_response_code(403);
            echo json_encode(['success' => false, 'message' => 'Você não aprendeu esta skill']);
            exit;
        }
        $itemTemplateId = null;
    } elseif ($itemTemplateId !== null && $itemTemplateId > 0) {
        $stmt = $pdo->prepare("
            SELECT item_id, item_name, item_type
            FROM item_templates
            WHERE item_id = :item_id
        ");
        $stmt->execute([':item_id' => $itemTemplateId]);
        $template = $stmt->fetch(PDO::FETCH_ASSOC);

        if (!$template) {
            http_response_code(404);
            echo json_encode(['success' => false, 'message' => 'Item template não encontrado']);
            exit;
        }
        if ($template['item_type'] !== 'consumable') {
            http_response_code(400);
            echo json_encode(['success' => false, 'message' => 'Apenas consumíveis podem ser colocados na skillbar']);
            exit;
        }
        $skillId = null;
    } else {
        $skillId = null;
        $itemTemplateId = null;
    }

    $stmt = $pdo->prepare("
        INSERT INTO player_skillbar (player_id, slot_index, skill_id, item_template_id, keybind)
        VALUES (:player_id, :slot_index, :skill_id, :item_template_id, :keybind)
        ON DUPLICATE KEY UPDATE
            skill_id = VALUES(skill_id),
            item_template_id = VALUES(item_template_id),
            keybind = VALUES(keybind),
            updated_at = NOW()
    ");
    $stmt->execute([
        ':player_id' => $playerId,
        ':slot_index' => $slotIndex,
        ':skill_id' => $skillId,
        ':item_template_id' => $itemTemplateId,
        ':keybind' => $keybind
    ]);

    if ($skillId) {
        $message = "Skill atribuída ao slot {$slotIndex}";
    } elseif ($itemTemplateId) {
        $message = "Consumível atribuído ao slot {$slotIndex}";
    } else {
        $message = "Slot {$slotIndex} limpo";
    }

    echo json_encode([
        'success' => true,
        'message' => $message,
        'data' => [
            'slot_index' => (int)$slotIndex,
            'skill_id' => $skillId ? (int)$skillId : null,
            'item_template_id' => $itemTemplateId ? (int)$itemTemplateId : null,
            'keybind' => $keybind
        ]
    ], JSON_UNESCAPED_UNICODE);

} catch (PDOException $e) {
    error_log("Erro em set_skillbar: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
} catch (Exception $e) {
    error_log("Erro em set_skillbar: " . $e->getMessage());
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor']);
}
