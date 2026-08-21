<?php
/**
 * Correio in-game — helpers (até 5 anexos inventory-slot).
 */

require_once __DIR__ . '/npc_vendor_helper.php';
require_once __DIR__ . '/stat_key_mapping.php';
require_once __DIR__ . '/enchant_helper.php';

const MAIL_MAX_ATTACHMENTS = 5;
const MAIL_SUBJECT_MAX = 128;
const MAIL_BODY_MAX = 4000;

function mailEnsureTables(PDO $pdo): void
{
    $pdo->exec("
        CREATE TABLE IF NOT EXISTS mail_messages (
          id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
          sender_player_id BIGINT UNSIGNED NULL,
          recipient_player_id BIGINT UNSIGNED NOT NULL,
          subject VARCHAR(128) NOT NULL DEFAULT '',
          body TEXT NOT NULL,
          is_read TINYINT(1) NOT NULL DEFAULT 0,
          attachment_count TINYINT UNSIGNED NOT NULL DEFAULT 0,
          created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
          expires_at TIMESTAMP NULL DEFAULT NULL,
          deleted_by_recipient TINYINT(1) NOT NULL DEFAULT 0,
          PRIMARY KEY (id),
          INDEX idx_mail_recipient (recipient_player_id, deleted_by_recipient, created_at),
          INDEX idx_mail_sender (sender_player_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
    ");
    $pdo->exec("
        CREATE TABLE IF NOT EXISTS mail_attachments (
          id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
          mail_id BIGINT UNSIGNED NOT NULL,
          slot_index TINYINT UNSIGNED NOT NULL,
          item_template_id INT NOT NULL,
          quantity INT NOT NULL DEFAULT 1,
          refinement_level TINYINT NOT NULL DEFAULT 0,
          durability FLOAT NOT NULL DEFAULT 100.0,
          bonus_stats_json JSON NULL,
          claimed TINYINT(1) NOT NULL DEFAULT 0,
          claimed_at TIMESTAMP NULL DEFAULT NULL,
          PRIMARY KEY (id),
          UNIQUE KEY uq_mail_slot (mail_id, slot_index),
          INDEX idx_mail_attach_mail (mail_id, claimed)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
    ");
    try {
        $col = $pdo->query("SHOW COLUMNS FROM mail_attachments LIKE 'enchantments_json'")->fetch();
        if (!$col) {
            $pdo->exec('ALTER TABLE mail_attachments ADD COLUMN enchantments_json JSON NULL');
        }
    } catch (PDOException $e) {
        // Tabela pode não existir ainda em instalações antigas.
    }
}

/**
 * Normaliza attachments do request. Retorna [ok, error|null, rows[]].
 * @param mixed $raw
 * @return array{0:bool,1:?string,2:list<array<string,mixed>>}
 */
function mailNormalizeAttachments($raw): array
{
    if ($raw === null || $raw === '') {
        return [true, null, []];
    }
    if (!is_array($raw)) {
        return [false, 'attachments deve ser array', []];
    }
    if (count($raw) > MAIL_MAX_ATTACHMENTS) {
        return [false, 'Máximo de ' . MAIL_MAX_ATTACHMENTS . ' anexos', []];
    }
    $used = [];
    $out = [];
    foreach ($raw as $a) {
        if (!is_array($a)) {
            return [false, 'Anexo inválido', []];
        }
        $slot = isset($a['slot_index']) ? (int)$a['slot_index'] : -1;
        $itemId = (int)($a['item_template_id'] ?? 0);
        $qty = (int)($a['quantity'] ?? 1);
        $ref = (int)($a['refinement_level'] ?? 0);
        $dur = isset($a['durability']) ? (float)$a['durability'] : 100.0;
        $bonus = $a['bonus_stats_json'] ?? ($a['bonus_stats'] ?? null);
        $ench = $a['enchantments_json'] ?? ($a['enchantments'] ?? null);
        if (is_array($ench)) {
            $ench = enchant_encode_list($ench);
        }
        if ($slot < 0 || $slot > 4) {
            return [false, 'slot_index deve ser 0–4', []];
        }
        if (isset($used[$slot])) {
            return [false, 'slot_index duplicado: ' . $slot, []];
        }
        if ($itemId <= 0 || $qty < 1) {
            return [false, 'item_template_id e quantity obrigatórios', []];
        }
        $used[$slot] = true;
        $bonusJson = null;
        if (is_array($bonus)) {
            $bonusJson = json_encode($bonus, JSON_UNESCAPED_UNICODE);
        } elseif (is_string($bonus) && $bonus !== '') {
            $bonusJson = $bonus;
        }
        $out[] = [
            'slot_index' => $slot,
            'item_template_id' => $itemId,
            'quantity' => $qty,
            'refinement_level' => max(0, min(12, $ref)),
            'durability' => $dur,
            'bonus_stats_json' => $bonusJson,
            'enchantments_json' => is_string($ench) ? $ench : null,
        ];
    }
    return [true, null, $out];
}

/**
 * @param list<array<string,mixed>> $attachments
 */
function mailInsertMessage(
    PDO $pdo,
    ?int $senderPlayerId,
    int $recipientPlayerId,
    string $subject,
    string $body,
    array $attachments,
    ?string $expiresAt = null
): int {
    $subject = mb_substr(trim($subject), 0, MAIL_SUBJECT_MAX);
    $body = mb_substr($body, 0, MAIL_BODY_MAX);
    $count = count($attachments);
    $st = $pdo->prepare('
        INSERT INTO mail_messages
          (sender_player_id, recipient_player_id, subject, body, attachment_count, expires_at)
        VALUES
          (:sid, :rid, :subj, :body, :ac, :exp)
    ');
    $st->execute([
        ':sid' => $senderPlayerId,
        ':rid' => $recipientPlayerId,
        ':subj' => $subject,
        ':body' => $body,
        ':ac' => $count,
        ':exp' => $expiresAt,
    ]);
    $mailId = (int)$pdo->lastInsertId();
    if ($count > 0) {
        $ins = $pdo->prepare('
            INSERT INTO mail_attachments
              (mail_id, slot_index, item_template_id, quantity, refinement_level, durability, bonus_stats_json, enchantments_json)
            VALUES
              (:mid, :slot, :iid, :qty, :ref, :dur, :bonus, :ench)
        ');
        foreach ($attachments as $a) {
            $ins->execute([
                ':mid' => $mailId,
                ':slot' => $a['slot_index'],
                ':iid' => $a['item_template_id'],
                ':qty' => $a['quantity'],
                ':ref' => $a['refinement_level'],
                ':dur' => $a['durability'],
                ':bonus' => $a['bonus_stats_json'],
                ':ench' => $a['enchantments_json'] ?? null,
            ]);
        }
    }
    return $mailId;
}

/**
 * @return list<array<string,mixed>>
 */
function mailFetchAttachments(PDO $pdo, int $mailId): array
{
    $st = $pdo->prepare('
        SELECT ma.id, ma.slot_index, ma.item_template_id, ma.quantity, ma.refinement_level,
               ma.durability, ma.bonus_stats_json, ma.enchantments_json, ma.claimed, ma.claimed_at,
               it.item_name, it.icon_path, it.item_type, it.rarity, it.max_stack_size
        FROM mail_attachments ma
        LEFT JOIN item_templates it ON it.item_id = ma.item_template_id
        WHERE ma.mail_id = ?
        ORDER BY ma.slot_index ASC
    ');
    $st->execute([$mailId]);
    $rows = [];
    while ($r = $st->fetch(PDO::FETCH_ASSOC)) {
        $rows[] = [
            'attachment_id' => (int)$r['id'],
            'slot_index' => (int)$r['slot_index'],
            'item_template_id' => (int)$r['item_template_id'],
            'quantity' => (int)$r['quantity'],
            'refinement_level' => (int)$r['refinement_level'],
            'durability' => (float)$r['durability'],
            'bonus_stats_json' => $r['bonus_stats_json'],
            'enchantments_json' => $r['enchantments_json'] ?? null,
            'enchantments' => enchant_parse_list($r['enchantments_json'] ?? null),
            'claimed' => (int)$r['claimed'] === 1,
            'claimed_at' => $r['claimed_at'],
            'item_name' => $r['item_name'] ?? '',
            'icon_path' => $r['icon_path'] ?? '',
            'item_type' => $r['item_type'] ?? '',
            'rarity' => $r['rarity'] ?? '',
            'max_stack_size' => isset($r['max_stack_size']) ? (int)$r['max_stack_size'] : 1,
        ];
    }
    return $rows;
}

function mailGrantAttachmentToInventory(PDO $pdo, int $playerId, array $att): bool
{
    $itemId = (int)$att['item_template_id'];
    $qty = (int)$att['quantity'];
    $ref = (int)($att['refinement_level'] ?? 0);
    $dur = (float)($att['durability'] ?? 100.0);
    $bonus = $att['bonus_stats_json'] ?? null;
    $ench = $att['enchantments_json'] ?? null;
    if (is_array($ench)) {
        $ench = enchant_encode_list($ench);
    }

    $tpl = $pdo->prepare('SELECT item_id, max_stack_size FROM item_templates WHERE item_id = ? LIMIT 1');
    $tpl->execute([$itemId]);
    $template = $tpl->fetch(PDO::FETCH_ASSOC);
    if (!$template) {
        return false;
    }
    $remaining = $qty;
    $maxStack = max(1, (int)$template['max_stack_size']);
    // Itens refinados / com bonus: não stackar — um insert por chunk com stats
    $forceUnique = $ref > 0 || ($bonus !== null && $bonus !== '') || ($ench !== null && $ench !== '');
    while ($remaining > 0) {
        $chunk = $forceUnique ? $remaining : min($remaining, $maxStack);
        if (!$forceUnique && $maxStack > 1) {
            $stack = $pdo->prepare('
                SELECT inventory_id, quantity FROM player_inventory
                WHERE player_id = ? AND item_template_id = ? AND is_equipped = 0
                  AND auction_listing_id IS NULL AND slot_index BETWEEN 0 AND 49
                  AND COALESCE(refinement_level, 0) = 0
                  AND quantity < ?
                ORDER BY slot_index ASC LIMIT 1
                FOR UPDATE
            ');
            $stack->execute([$playerId, $itemId, $maxStack]);
            $row = $stack->fetch(PDO::FETCH_ASSOC);
            if ($row) {
                $add = min($chunk, $maxStack - (int)$row['quantity']);
                $pdo->prepare('UPDATE player_inventory SET quantity = quantity + ? WHERE inventory_id = ?')
                    ->execute([$add, (int)$row['inventory_id']]);
                $remaining -= $add;
                continue;
            }
        }
        $slot = npcVendorFindFreeSlot($pdo, $playerId);
        if ($slot === null) {
            return false;
        }
        $ins = $pdo->prepare('
            INSERT INTO player_inventory
              (player_id, item_template_id, quantity, slot_index, is_equipped, durability, refinement_level, refinement_bonus_stats, enchantments_json)
            VALUES
              (?, ?, ?, ?, 0, ?, ?, ?, ?)
        ');
        $ins->execute([$playerId, $itemId, $chunk, $slot, $dur, $ref, $bonus, $ench]);
        if ($ench === null || $ench === '') {
            enchant_apply_roll_to_inventory_id($pdo, (int)$pdo->lastInsertId());
        }
        $remaining -= $chunk;
        if ($forceUnique) {
            break;
        }
    }
    return $remaining <= 0;
}

/**
 * Remove itens do inventário do remetente para anexar (player send).
 * Espera attachments com inventory_id opcional; senão debita por template+qty.
 * @param list<array<string,mixed>> $attachments
 * @return array{0:bool,1:?string,2:list<array<string,mixed>>} anexos enriquecidos para insert
 */
function mailDebitSenderInventory(PDO $pdo, int $senderId, array $attachments): array
{
    $enriched = [];
    foreach ($attachments as $a) {
        $invId = isset($a['inventory_id']) ? (int)$a['inventory_id'] : 0;
        $qty = (int)$a['quantity'];
        if ($invId > 0) {
            $st = $pdo->prepare('
                SELECT inventory_id, item_template_id, quantity, refinement_level, durability, refinement_bonus_stats, enchantments_json
                FROM player_inventory
                WHERE inventory_id = ? AND player_id = ? AND is_equipped = 0
                  AND auction_listing_id IS NULL AND slot_index BETWEEN 0 AND 49
                FOR UPDATE
            ');
            $st->execute([$invId, $senderId]);
            $row = $st->fetch(PDO::FETCH_ASSOC);
            if (!$row) {
                return [false, 'Item de inventário não encontrado: ' . $invId, []];
            }
            $have = (int)$row['quantity'];
            if ($have < $qty) {
                return [false, 'Quantidade insuficiente no inventário', []];
            }
            if ($have === $qty) {
                $pdo->prepare('DELETE FROM player_inventory WHERE inventory_id = ?')->execute([$invId]);
            } else {
                $pdo->prepare('UPDATE player_inventory SET quantity = quantity - ? WHERE inventory_id = ?')
                    ->execute([$qty, $invId]);
            }
            $enriched[] = [
                'slot_index' => (int)$a['slot_index'],
                'item_template_id' => (int)$row['item_template_id'],
                'quantity' => $qty,
                'refinement_level' => (int)($row['refinement_level'] ?? 0),
                'durability' => (float)($row['durability'] ?? 100.0),
                'bonus_stats_json' => $row['refinement_bonus_stats'] ?? null,
                'enchantments_json' => $row['enchantments_json'] ?? null,
            ];
        } else {
            $itemId = (int)$a['item_template_id'];
            $need = $qty;
            $st = $pdo->prepare('
                SELECT inventory_id, quantity, refinement_level, durability, refinement_bonus_stats, enchantments_json
                FROM player_inventory
                WHERE player_id = ? AND item_template_id = ? AND is_equipped = 0
                  AND auction_listing_id IS NULL AND slot_index BETWEEN 0 AND 49
                  AND COALESCE(refinement_level, 0) = ?
                ORDER BY slot_index ASC
                FOR UPDATE
            ');
            $refWant = (int)($a['refinement_level'] ?? 0);
            $st->execute([$senderId, $itemId, $refWant]);
            $rows = $st->fetchAll(PDO::FETCH_ASSOC);
            $taken = 0;
            $dur = (float)($a['durability'] ?? 100.0);
            $bonus = $a['bonus_stats_json'] ?? null;
            $ench = $a['enchantments_json'] ?? null;
            foreach ($rows as $row) {
                if ($taken >= $need) {
                    break;
                }
                $have = (int)$row['quantity'];
                $take = min($have, $need - $taken);
                if ($take >= $have) {
                    $pdo->prepare('DELETE FROM player_inventory WHERE inventory_id = ?')->execute([(int)$row['inventory_id']]);
                } else {
                    $pdo->prepare('UPDATE player_inventory SET quantity = quantity - ? WHERE inventory_id = ?')
                        ->execute([$take, (int)$row['inventory_id']]);
                }
                $dur = (float)($row['durability'] ?? $dur);
                $bonus = $row['refinement_bonus_stats'] ?? $bonus;
                $ench = $row['enchantments_json'] ?? $ench;
                $taken += $take;
            }
            if ($taken < $need) {
                return [false, "Itens insuficientes (template {$itemId})", []];
            }
            $enriched[] = [
                'slot_index' => (int)$a['slot_index'],
                'item_template_id' => $itemId,
                'quantity' => $qty,
                'refinement_level' => $refWant,
                'durability' => $dur,
                'bonus_stats_json' => $bonus,
                'enchantments_json' => $ench,
            ];
        }
    }
    return [true, null, $enriched];
}

/**
 * Best-effort: grava fila de notify (Zone/Manager consome via notify_mail).
 * Também tenta HTTP local opcional se config existir.
 */
function mailQueueNotify(PDO $pdo, int $recipientPlayerId, int $mailId, string $fromName, string $subject): void
{
    try {
        $pdo->exec("
            CREATE TABLE IF NOT EXISTS mail_notify_queue (
              id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
              recipient_player_id BIGINT UNSIGNED NOT NULL,
              mail_id BIGINT UNSIGNED NOT NULL,
              from_name VARCHAR(64) NOT NULL DEFAULT '',
              subject VARCHAR(128) NOT NULL DEFAULT '',
              created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
              consumed TINYINT(1) NOT NULL DEFAULT 0,
              PRIMARY KEY (id),
              INDEX idx_mail_notify_pending (consumed, recipient_player_id)
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
        ");
        $pdo->prepare('
            INSERT INTO mail_notify_queue (recipient_player_id, mail_id, from_name, subject)
            VALUES (?, ?, ?, ?)
        ')->execute([
            $recipientPlayerId,
            $mailId,
            mb_substr($fromName, 0, 64),
            mb_substr($subject, 0, 128),
        ]);
    } catch (Throwable $e) {
        error_log('[mailQueueNotify] ' . $e->getMessage());
    }
}
