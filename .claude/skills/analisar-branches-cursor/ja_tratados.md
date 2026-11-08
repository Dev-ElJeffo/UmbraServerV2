# Achados do Cursor já tratados

Antes de analisar, conferir se o achado já está aqui. Mesmo assim, se ele reaparecer, checar rapidamente se houve regressão no código atual.

## Rodada 2026-11-08 — PR #24 (`fix/cursor-findings`, merge `8b9d5ac`)

14 branches analisadas e apagadas (`critical-bug-investigation-*` ×12, `critical-correctness-bugs-*` ×2).

| Achado | Veredito | Onde ficou |
|---|---|---|
| Opcode 88 (`ForeignVitalsNotify`) confia no HP/MP e no `sourcePlayerId` do cliente | Corrigido | `CombatCoreEngine::enqueueForeignVitals/processForeignVitals`, `MovementServer::broadcastForeignVitals` |
| IDOR: `apply_vitals`, `dot_apply`, `dot_remove`, `respawn` (combat) | Corrigido | `assertPlayerBelongsToAccount` (`helpers/personal_shop_helper.php`) |
| IDOR: `set_skillbar.php` | Corrigido | idem |
| `getLastInsertId` global (race entre inserts) | Corrigido | Overload `executePreparedInsert(..., outInsertId)`; `AccountDAO`/`PlayerDAO` |
| Teto de HP = HP atual em `ZoneCombatService::applyVitalsInDb` (HoT travado) | Código morto, removido | `tickActiveDots`/`applyVitalsInDb` apagados |
| Mana stale / cooldown antes do débito no skill cast | Obsoleto | Write-behind + `patchCachedMana` |
| Max HP sem buff no dano PvP (`applyPlayerDamage`) | Obsoleto | Já usa `buffedStats.maxHealth` |
| `std::max(0LL, ...)` em `SkillTypes.hpp` (build Linux) | Obsoleto | Já corrigido |
| `admin_secret` nas APIs admin | Obsoleto | Admin usa `requireAdminAuth` (JWT `umbra_manager`) |
| `use_skill.php` "indefinido" (branch `b004`) | **Não verificado** | Reavaliar na próxima rodada se reaparecer |

**Em aberto (conhecido, fora de escopo):** `apply_vitals.php` ainda aceita o dano calculado pelo cliente (design legado do V1). Se o Cursor apontar isso, é a mesma questão.
