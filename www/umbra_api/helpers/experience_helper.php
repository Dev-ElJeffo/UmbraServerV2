<?php
/**
 * Helper espelhado do ExperienceService C++ (grant EXP + level-up).
 *
 * @param bool $manageTransaction Quando false, participa da transação PDO já aberta pelo chamador
 *                              (ex.: turn-in de quest). Evita rollback indevido por transação aninhada.
 *
 * @return array|null Resultado no formato ExperienceGrantResult ou null em falha.
 */
require_once __DIR__ . '/../config/database.php';

function umbra_grant_experience(PDO $pdo, int $player_id, int $amount, string $source, bool $manageTransaction = true): ?array
{
    if ($amount <= 0) {
        return null;
    }

    $startedHere = false;
    try {
        if ($manageTransaction) {
            if ($pdo->inTransaction()) {
                $manageTransaction = false;
            } else {
                $pdo->beginTransaction();
                $startedHere = true;
            }
        }

        $stmt = $pdo->prepare('SELECT level, experience FROM players WHERE id = :id FOR UPDATE');
        $stmt->execute([':id' => $player_id]);
        $player = $stmt->fetch(PDO::FETCH_ASSOC);
        if (!$player) {
            if ($startedHere && $pdo->inTransaction()) {
                $pdo->rollBack();
            }
            return null;
        }

        $old_level = (int)$player['level'];
        $experience = (int)$player['experience'] + $amount;

        $stat_stmt = $pdo->prepare('SELECT id FROM player_stat_points WHERE player_id = :id LIMIT 1');
        $stat_stmt->execute([':id' => $player_id]);
        if (!$stat_stmt->fetch()) {
            $pdo->prepare('INSERT INTO player_stat_points (player_id, unspent_points) VALUES (:id, 0)')
                ->execute([':id' => $player_id]);
        }

        $skill_stmt = $pdo->prepare('SELECT player_id FROM player_skill_points WHERE player_id = :id LIMIT 1');
        $skill_stmt->execute([':id' => $player_id]);
        if (!$skill_stmt->fetch()) {
            $total_skill = $old_level * 3;
            $pdo->prepare(
                'INSERT INTO player_skill_points (player_id, total_points_earned, points_spent, points_available)
                 VALUES (:id, :earned, 0, :avail)'
            )->execute([
                ':id' => $player_id,
                ':earned' => $total_skill,
                ':avail' => $total_skill,
            ]);
        }

        $skill_before_stmt = $pdo->prepare(
            'SELECT points_available FROM player_skill_points WHERE player_id = :id'
        );
        $skill_before_stmt->execute([':id' => $player_id]);
        $skill_before_row = $skill_before_stmt->fetch(PDO::FETCH_ASSOC);
        $skill_before = (int)($skill_before_row['points_available'] ?? 0);

        $new_level = $old_level;
        $stat_points_from_table = 0;
        $max_level = 50;

        while ($new_level < $max_level) {
            $lvl_stmt = $pdo->prepare(
                'SELECT exp_required, stat_points_gained FROM player_levels WHERE level_number = :lvl'
            );
            $lvl_stmt->execute([':lvl' => $new_level + 1]);
            $lvl_row = $lvl_stmt->fetch(PDO::FETCH_ASSOC);
            if (!$lvl_row) {
                break;
            }
            if ($experience < (int)$lvl_row['exp_required']) {
                break;
            }
            $stat_points_from_table += (int)$lvl_row['stat_points_gained'];
            $new_level++;
        }

        $progress_stmt = $pdo->prepare(
            'SELECT exp_required, exp_for_next_level FROM player_levels WHERE level_number = :lvl'
        );
        $progress_stmt->execute([':lvl' => $new_level]);
        $progress_row = $progress_stmt->fetch(PDO::FETCH_ASSOC);
        $exp_for_next = (int)($progress_row['exp_for_next_level'] ?? 1000);
        $exp_required_current = (int)($progress_row['exp_required'] ?? 0);
        $exp_in_level = max(0, $experience - $exp_required_current);
        $progress_percent = $exp_for_next > 0
            ? min(100.0, max(0.0, ($exp_in_level / $exp_for_next) * 100.0))
            : 0.0;

        $upd = $pdo->prepare(
            'UPDATE players SET level = :lvl, experience = :exp, next_level_exp = :next WHERE id = :id'
        );
        $upd->execute([
            ':lvl' => $new_level,
            ':exp' => $experience,
            ':next' => $exp_for_next,
            ':id' => $player_id,
        ]);

        if ($new_level > $old_level) {
            $total_skill = $new_level * 3;
            $pdo->prepare(
                'UPDATE player_skill_points SET total_points_earned = :earned, '
                . 'points_available = :earned - points_spent WHERE player_id = :id'
            )->execute([
                ':earned' => $total_skill,
                ':id' => $player_id,
            ]);
        }

        if ($startedHere) {
            $pdo->commit();
        }

        $skill_after_stmt = $pdo->prepare(
            'SELECT points_available FROM player_skill_points WHERE player_id = :id'
        );
        $skill_after_stmt->execute([':id' => $player_id]);
        $skill_after_row = $skill_after_stmt->fetch(PDO::FETCH_ASSOC);
        $skill_after = (int)($skill_after_row['points_available'] ?? $skill_before);

        $levels_gained = $new_level - $old_level;
        $stat_gained = $stat_points_from_table > 0
            ? $stat_points_from_table
            : $levels_gained * 10;

        return [
            'success' => true,
            'exp_granted' => $amount,
            'total_experience' => $experience,
            'exp_in_current_level' => (int)$exp_in_level,
            'old_level' => $old_level,
            'new_level' => $new_level,
            'levels_gained' => $levels_gained,
            'stat_points_gained' => $stat_gained,
            'skill_points_gained' => max(0, $skill_after - $skill_before),
            'skill_points_available' => $skill_after,
            'exp_for_next_level' => $exp_for_next,
            'exp_progress_percent' => round($progress_percent, 2),
            'source' => $source,
            'leveled_up' => $levels_gained > 0,
        ];
    } catch (Throwable $e) {
        if ($startedHere && $pdo->inTransaction()) {
            $pdo->rollBack();
        }
        error_log('[experience_helper] ' . $e->getMessage());
        return null;
    }
}
