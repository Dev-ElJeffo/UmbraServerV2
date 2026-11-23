<?php
/**
 * Missões de guilda (etapa 3b): listar, resgatar e creditar eventos do PHP (craft/refino).
 * O sorteio do período é a procedure sp_guild_missions_ensure (a mesma que a zone usa).
 * Eventos de combate/loot/consumível são creditados pela zone (GuildMissionService).
 */

require_once __DIR__ . '/quest_helper.php';

/** Guilda do jogador (0 = sem guilda). */
function gmPlayerGuildId(PDO $pdo, int $player_id): int
{
    $stmt = $pdo->prepare('SELECT guild_id FROM guild_members WHERE player_id = ? LIMIT 1');
    $stmt->execute([$player_id]);
    return (int)($stmt->fetchColumn() ?: 0);
}

/** Sorteia as missões do período. NÃO chamar dentro de transação (a procedure faz commit). */
function gmEnsure(PDO $pdo, int $guild_id): void
{
    if ($guild_id <= 0) {
        return;
    }
    $stmt = $pdo->query('CALL sp_guild_missions_ensure(' . (int)$guild_id . ')');
    if ($stmt) {
        do {
            // drena os resultados do CALL
        } while ($stmt->nextRowset());
        $stmt->closeCursor();
    }
}

/** Filtro SQL do período atual (diária de hoje + semanal desta semana). */
function gmCurrentPeriodSql(string $alias = 'gm'): string
{
    return "(({$alias}.period = 'daily' AND {$alias}.period_key = CURDATE())
            OR ({$alias}.period = 'weekly' AND {$alias}.period_key = DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY)))";
}

/** Lista as missões do período da guilda do jogador, com a contribuição dele. */
function gmList(PDO $pdo, int $player_id): array
{
    $guild_id = gmPlayerGuildId($pdo, $player_id);
    if ($guild_id <= 0) {
        return ['ok' => false, 'message' => 'Você não está em uma guilda.'];
    }
    gmEnsure($pdo, $guild_id);
    $period = gmCurrentPeriodSql('gm');
    $stmt = $pdo->prepare("
        SELECT gm.guild_mission_id, gm.period, gm.progress, gm.required_total, gm.status,
               t.mission_key, t.title, t.description, t.objective_type, t.reward_guild_xp, t.reward_gold,
               t.reward_item_id, t.reward_item_qty, t.min_contribution,
               it.item_name AS reward_item_name, it.icon_path AS reward_item_icon,
               COALESCE(c.amount, 0) AS my_contribution, COALESCE(c.reward_claimed, 0) AS reward_claimed
        FROM guild_missions gm
        INNER JOIN guild_mission_templates t ON t.mission_id = gm.mission_id
        LEFT JOIN item_templates it ON it.item_id = t.reward_item_id
        LEFT JOIN guild_mission_contributions c ON c.guild_mission_id = gm.guild_mission_id AND c.player_id = ?
        WHERE gm.guild_id = ? AND {$period}
        ORDER BY FIELD(gm.period, 'daily', 'weekly'), gm.guild_mission_id
    ");
    $stmt->execute([$player_id, $guild_id]);
    $now = time();
    $dailyEnd = strtotime('tomorrow', $now);
    $weeklyEnd = strtotime('next monday', strtotime('today', $now));
    $missions = [];
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $r) {
        $completed = $r['status'] === 'completed';
        $mine = (int)$r['my_contribution'];
        $claimed = (int)$r['reward_claimed'] === 1;
        $minContrib = max(1, (int)$r['min_contribution']);
        $missions[] = [
            'guild_mission_id' => (int)$r['guild_mission_id'],
            'period' => $r['period'],
            'mission_key' => $r['mission_key'],
            'title' => $r['title'],
            'description' => $r['description'],
            'objective_type' => $r['objective_type'],
            'progress' => (int)$r['progress'],
            'required_total' => (int)$r['required_total'],
            'status' => $r['status'],
            'reward_guild_xp' => (int)$r['reward_guild_xp'],
            'reward_gold' => (int)$r['reward_gold'],
            'reward_item_id' => (int)($r['reward_item_id'] ?? 0),
            'reward_item_qty' => (int)$r['reward_item_qty'],
            'reward_item_name' => (string)($r['reward_item_name'] ?? ''),
            'reward_item_icon' => (string)($r['reward_item_icon'] ?? ''),
            'min_contribution' => $minContrib,
            'my_contribution' => $mine,
            'reward_claimed' => $claimed,
            'can_claim' => $completed && !$claimed && $mine >= $minContrib,
            'ends_at' => $r['period'] === 'daily' ? $dailyEnd : $weeklyEnd,
        ];
    }
    return ['ok' => true, 'guild_id' => $guild_id, 'missions' => $missions];
}

/** Resgate da recompensa individual (ouro/item) de uma missão concluída. */
function gmClaim(PDO $pdo, int $player_id, int $guild_mission_id): array
{
    $guild_id = gmPlayerGuildId($pdo, $player_id);
    if ($guild_id <= 0) {
        return ['ok' => false, 'message' => 'Você não está em uma guilda.'];
    }
    try {
        $pdo->beginTransaction();
        $stmt = $pdo->prepare("
            SELECT gm.status, gm.guild_id, t.reward_gold, t.reward_item_id, t.reward_item_qty, t.min_contribution,
                   c.amount, c.reward_claimed
            FROM guild_missions gm
            INNER JOIN guild_mission_templates t ON t.mission_id = gm.mission_id
            INNER JOIN guild_mission_contributions c ON c.guild_mission_id = gm.guild_mission_id AND c.player_id = ?
            WHERE gm.guild_mission_id = ?
            FOR UPDATE
        ");
        $stmt->execute([$player_id, $guild_mission_id]);
        $row = $stmt->fetch(PDO::FETCH_ASSOC);
        if (!$row || (int)$row['guild_id'] !== $guild_id) {
            throw new RuntimeException('Você não contribuiu para esta missão.');
        }
        if ($row['status'] !== 'completed') {
            throw new RuntimeException('A missão ainda não foi concluída.');
        }
        if ((int)$row['reward_claimed'] === 1) {
            throw new RuntimeException('Recompensa já resgatada.');
        }
        if ((int)$row['amount'] < max(1, (int)$row['min_contribution'])) {
            throw new RuntimeException('Contribuição insuficiente: mínimo ' . (int)$row['min_contribution'] . '.');
        }
        $gold = (int)$row['reward_gold'];
        if ($gold > 0) {
            $pdo->prepare('UPDATE players SET gold = gold + ? WHERE id = ?')->execute([$gold, $player_id]);
        }
        $itemId = (int)($row['reward_item_id'] ?? 0);
        if ($itemId > 0 && !questGrantItemToPlayer($pdo, $player_id, $itemId, max(1, (int)$row['reward_item_qty']))) {
            throw new RuntimeException('Inventário cheio: libere espaço para resgatar.');
        }
        $pdo->prepare('UPDATE guild_mission_contributions SET reward_claimed = 1 WHERE guild_mission_id = ? AND player_id = ?')
            ->execute([$guild_mission_id, $player_id]);
        $pdo->commit();
        $g = $pdo->prepare('SELECT gold FROM players WHERE id = ?');
        $g->execute([$player_id]);
        return [
            'ok' => true,
            'reward_gold' => $gold,
            'reward_item_id' => $itemId,
            'reward_item_qty' => $itemId > 0 ? max(1, (int)$row['reward_item_qty']) : 0,
            'new_gold' => (int)$g->fetchColumn(),
        ];
    } catch (Throwable $e) {
        if ($pdo->inTransaction()) {
            $pdo->rollBack();
        }
        if ($e instanceof RuntimeException) {
            return ['ok' => false, 'message' => $e->getMessage()];
        }
        error_log('[guild/claim_guild_mission_reward] ' . $e->getMessage());
        return ['ok' => false, 'message' => 'Erro interno'];
    }
}

/**
 * Credita craft_item / refine_item nas missões do período (chamado pelo craft/refino, pode estar em transação).
 * Só missões já sorteadas contam (o sorteio acontece ao abrir a janela ou no 1º evento na zone).
 */
function gmCreditEvent(PDO $pdo, int $player_id, string $type, int $key, int $amount = 1): void
{
    if (!in_array($type, ['craft_item', 'refine_item'], true) || $player_id <= 0 || $amount <= 0) {
        return;
    }
    $guild_id = gmPlayerGuildId($pdo, $player_id);
    if ($guild_id <= 0) {
        return;
    }
    $period = gmCurrentPeriodSql('gm');
    $stmt = $pdo->prepare("
        SELECT gm.guild_mission_id, t.params_json
        FROM guild_missions gm
        INNER JOIN guild_mission_templates t ON t.mission_id = gm.mission_id
        WHERE gm.guild_id = ? AND gm.status = 'active' AND t.objective_type = ? AND {$period}
    ");
    $stmt->execute([$guild_id, $type]);
    foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $row) {
        $params = questDecodeParams($row['params_json'] ?? null);
        if ($type === 'craft_item') {
            $need = (int)($params['item_template_id'] ?? 0);
            if ($need > 0 && $need !== $key) {
                continue;
            }
        } elseif ((int)($params['min_refine_level'] ?? 1) > $key) {
            continue;
        }
        $gmId = (int)$row['guild_mission_id'];
        $pdo->prepare("UPDATE guild_missions SET progress = LEAST(required_total, progress + ?) WHERE guild_mission_id = ? AND status = 'active'")
            ->execute([$amount, $gmId]);
        $pdo->prepare('
            INSERT INTO guild_mission_contributions (guild_mission_id, player_id, amount) VALUES (?, ?, ?)
            ON DUPLICATE KEY UPDATE amount = amount + VALUES(amount)
        ')->execute([$gmId, $player_id, $amount]);
        gmCompleteIfReady($pdo, $gmId, $guild_id);
    }
}

/** Mesma conclusão da zone: status + EXP uma vez (xp_awarded) + nível pela curva do PHP. */
function gmCompleteIfReady(PDO $pdo, int $guild_mission_id, int $guild_id): void
{
    $pdo->prepare("
        UPDATE guild_missions SET status = 'completed', completed_at = NOW()
        WHERE guild_mission_id = ? AND status = 'active' AND progress >= required_total
    ")->execute([$guild_mission_id]);
    $pdo->prepare("
        UPDATE guilds g
        INNER JOIN guild_missions gm ON gm.guild_id = g.guild_id
        INNER JOIN guild_mission_templates t ON t.mission_id = gm.mission_id
        SET g.guild_xp = g.guild_xp + t.reward_guild_xp, gm.xp_awarded = 1
        WHERE gm.guild_mission_id = ? AND gm.xp_awarded = 0 AND gm.status = 'completed'
    ")->execute([$guild_mission_id]);
    $pdo->prepare('
        UPDATE guilds SET guild_level = GREATEST(1, FLOOR(SQRT(guild_xp / 1000)) + 1), ranking_score = guild_xp
        WHERE guild_id = ?
    ')->execute([$guild_id]);
}
