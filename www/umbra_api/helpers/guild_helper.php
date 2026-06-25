<?php

function guild_require_player_membership(PDO $pdo, int $playerId): ?array
{
    $stmt = $pdo->prepare(
        "SELECT gm.guild_id, gm.member_rank, g.owner_player_id, g.guild_leader_id, g.member_limit, g.member_count
         FROM guild_members gm
         INNER JOIN guilds g ON g.guild_id = gm.guild_id
         WHERE gm.player_id = :player_id
         LIMIT 1"
    );
    $stmt->execute(['player_id' => $playerId]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    return $row ?: null;
}

function guild_is_owner(array $membership, int $playerId): bool
{
    return (int)($membership['owner_player_id'] ?? 0) === $playerId;
}

function guild_is_master(array $membership, int $playerId): bool
{
    return (int)($membership['guild_leader_id'] ?? 0) === $playerId;
}

function guild_can_invite(array $membership, int $playerId): bool
{
    if (guild_is_owner($membership, $playerId) || guild_is_master($membership, $playerId)) {
        return true;
    }
    return (int)($membership['member_rank'] ?? 1) >= 2;
}

function guild_can_remove_or_manage_rank(array $membership, int $playerId): bool
{
    if (guild_is_owner($membership, $playerId) || guild_is_master($membership, $playerId)) {
        return true;
    }
    return (int)($membership['member_rank'] ?? 1) >= 3;
}

function guild_calculate_level_from_xp(int $guildXp): int
{
    if ($guildXp <= 0) {
        return 1;
    }

    // Curva simples e determinística: level^2 * 1000
    $level = (int)floor(sqrt($guildXp / 1000.0)) + 1;
    return max(1, $level);
}

function guild_normalize_rank(int $rank): int
{
    if ($rank < 1) {
        return 1;
    }
    if ($rank > 3) {
        return 3;
    }
    return $rank;
}

