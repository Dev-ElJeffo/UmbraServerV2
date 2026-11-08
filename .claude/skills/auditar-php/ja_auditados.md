# Endpoints PHP já auditados

## 2026-11-08 — PR #24 (origem: branches do Cursor, auditoria parcial)

Só os endpoints apontados pelo Cursor; o restante de `api/**` **ainda não foi auditado**.

| Endpoint | Achado | Situação |
|---|---|---|
| `api/combat/apply_vitals.php` | IDOR no `player_id` (source) | Corrigido (403) |
| `api/combat/dot_apply.php` | IDOR no `source_player_id` | Corrigido |
| `api/combat/dot_remove.php` | IDOR (alvo/source) | Corrigido (dono do alvo **ou** do source) |
| `api/combat/respawn.php` | IDOR no `player_id` | Corrigido |
| `api/skills/set_skillbar.php` | IDOR no `player_id` | Corrigido |
| `api/admin/*` (ban, items, accounts, status) | Sem auth (alegado pelo Cursor) | Já protegido por `requireAdminAuth` |

**Em aberto (design):** `apply_vitals.php` aplica o `delta_health`/`delta_mana` calculado pelo cliente (combate legado V1).
