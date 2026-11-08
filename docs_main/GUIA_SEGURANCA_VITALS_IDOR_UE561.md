# GUIA — Segurança de vitals (opcode 88) e IDOR nas APIs de combate (UE 5.6.1)

Correções de segurança vindas da análise das branches `cursor/*`, reescritas sobre o código atual. Não há mudança no Editor, no Manager nem no SQL. O que é preciso fazer: **sincronizar o PHP com o WAMP**, **reiniciar a zone** e rodar o QA.

## Objetivo

- **Opcode 88 (`ForeignVitalsNotify`):** a zone não confia mais no HP/MP enviado pelo cliente. Quem envia o frame vira o `source`, e o combat worker relê HP/MP do alvo no DB antes do rebroadcast. Um cliente adulterado não consegue mais mostrar HP falso nem a "morte" de outro player para a AOI ou a party.
- **IDOR no PHP:** os endpoints abaixo passam a exigir que o personagem informado pertença à conta do JWT. Caso contrário, respondem **403**.
- **`getLastInsertId` global:** `AccountDAO` e `PlayerDAO` passam a ler o ID do próprio insert, sem risco de trocar o ID quando dois inserts ocorrem ao mesmo tempo.
- **Limpeza:** foi removido o código morto `ZoneCombatService::tickActiveDots`/`applyVitalsInDb`, que tinha o bug de teto de HP no HoT e não era chamado. O DOT/HOT continua rodando em memória no `CombatCoreEngine`.
- **O que não muda:** o formato binário do opcode 88 (o cliente UE não precisa ser recompilado) e os opcodes 87/96/98.

## Dados / Manager

Nada a fazer. Não há coluna nova nem tela nova.

---

## 0) Pré-requisitos

1. **Sincronizar o PHP com o WAMP** (as APIs rodam em `C:\wamp64\www\umbra_api`):
   ```bat
   scripts_main\sync_umbra_api_to_wamp.bat
   ```
   Arquivos alterados:

   | Arquivo no repo | Destino no WAMP |
   |-----------------|-----------------|
   | `www/umbra_api/api/combat/apply_vitals.php` | `C:\wamp64\www\umbra_api\api\combat\apply_vitals.php` |
   | `www/umbra_api/api/combat/dot_apply.php` | `C:\wamp64\www\umbra_api\api\combat\dot_apply.php` |
   | `www/umbra_api/api/combat/dot_remove.php` | `C:\wamp64\www\umbra_api\api\combat\dot_remove.php` |
   | `www/umbra_api/api/combat/respawn.php` | `C:\wamp64\www\umbra_api\api\combat\respawn.php` |
   | `www/umbra_api/api/skills/set_skillbar.php` | `C:\wamp64\www\umbra_api\api\skills\set_skillbar.php` |

   Todos usam `helpers/personal_shop_helper.php` (`assertPlayerBelongsToAccount`), que já existe no WAMP. Confira se ele está lá.
2. Recompilar e reiniciar o `zone_server` e o `auth_server`/`umbra_server`:
   ```bat
   cd D:\UmbraServerV2\build
   cmake --build . --config Release --target zone_server auth_server umbra_server
   ```
3. **UE:** não precisa recompilar.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Handler do 88 | `src/zone/MovementServer.hpp` (`handleInboundBinary`) | Resolve o sender pelo `cid` e só enfileira. Não faz DB no tick nem no recv |
| `enqueueForeignVitals` | `src/zone/CombatCoreEngine.cpp` | Rejeita `sourcePlayerId` divergente do sender e deduplica por (sender, alvo) na fila |
| `processForeignVitals` | `src/zone/CombatCoreEngine.cpp` | Lê `health`/`mana` no DB, pega o max buffado do `CharacterStateLoader`, faz clamp, atualiza o cache e chama `broadcastForeignVitals` |
| `broadcastForeignVitals` | `src/zone/MovementServer.hpp` | Calcula o delta pelo último HP conhecido (o hint do cliente só vale sem HP anterior) e emite 87, mais o CombatEvent/morte quando aplicável |
| `executePreparedInsert(..., outInsertId)` | `src/database/MySQLConnector.{hpp,cpp}` | Devolve o `mysql_stmt_insert_id` do próprio statement |

> **Não** adicione lógica no Event Graph.

---

## 4) Protocolo

### Opcode 88 (`ForeignVitalsNotify`): formato inalterado

| Campo | Antes | Agora |
|-------|-------|-------|
| `playerId` (alvo) | usado | usado (alvo do rebroadcast) |
| `currentHealth`/`maxHealth`/`currentMana`/`maxMana` | repassados como vieram | **ignorados**: relidos no DB e no cache |
| `sourcePlayerId` | aceito do payload | precisa ser `0` ou igual ao sender; senão o frame é descartado |
| `reason` | repassado | repassado (cosmético) |
| `deltaAppliedHealth` | usado quando delta = 0 | só usado sem HP anterior em memória, com clamp em ±maxHealth |

Servidor: `src/zone/MovementServer.hpp`, `src/zone/CombatCoreEngine.cpp`. Cliente (sem alteração): `SendForeignVitalsNotifyViaWebSocket` em `UmbraGameInstance.cpp`.

### APIs PHP: nova regra de autorização

| Endpoint | Personagem validado contra o `account_id` do JWT |
|----------|------------------------------------------------|
| `api/combat/apply_vitals.php` | `player_id` (source) |
| `api/combat/dot_apply.php` | `source_player_id` |
| `api/combat/dot_remove.php` | `target_player_id` **ou** `source_player_id` |
| `api/combat/respawn.php` | `player_id` |
| `api/skills/set_skillbar.php` | `player_id` |

Falha → `403 {"success": false, "message": "Sem permissão para este personagem"}`. Token sem `account_id` → `401`.

---

## 6) QA (PIE)

PIE com **Number of Players ≥ 2** (contas diferentes).

| Cenário | Esperado |
|---------|----------|
| A causa dano em B por um caminho que usa `apply_vitals.php` | HP de B cai igual em A, em B e na party |
| A cura a si mesmo (HoT/heal via PHP) | HP sobe e B vê o mesmo valor |
| Skillbar: arrastar skill e trocar keybind | Salva normalmente (sem 403) |
| Morrer e clicar em respawn | Respawn normal |
| `curl` em `set_skillbar.php` com o token de A e o `player_id` de B | **403** |
| `curl` em `apply_vitals.php` com o token de A e `player_id` = B | **403** |
| Frame 88 com `sourcePlayerId` de outro player | Descartado, sem broadcast |
| Criar conta/personagem pelo `auth_server` | ID correto no log `Created account/player ... (ID: N)` |

Teste rápido do 403 (troque token e IDs):

```bat
curl -X POST http://localhost/umbra_api/api/skills/set_skillbar.php -H "Content-Type: application/json" -H "Authorization: Bearer <TOKEN_DA_CONTA_A>" -d "{\"player_id\": <ID_DE_B>, \"slot_index\": 0, \"keybind\": \"1\", \"keybind_only\": true}"
```

Logs esperados na zone (só em caso de spoof):
- `"[CombatCoreEngine] ForeignVitalsNotify rejeitado: sender=..."`
- `"ForeignVitalsNotify ignorado: client ... sem player associado"`

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| 403 em uso legítimo (skillbar/respawn) | O cliente enviou o `player_id` de outro personagem da sessão, ou o JWT é de outra conta | Conferir no Output Log o `player_id` enviado e o personagem selecionado |
| `500` / "Call to undefined function assertPlayerBelongsToAccount" | `helpers/personal_shop_helper.php` ausente no WAMP | Rodar o `sync_umbra_api_to_wamp.bat` de novo |
| HP do alvo não atualiza nos outros clients após dano via PHP | Zone sem DB (modo só movimento) ou alvo inexistente | Ver o log `ForeignVitalsNotify: alvo ... não encontrado` / conexão MySQL |
| Floating text sem número em dano via PHP | Sem HP anterior em memória e hint 0 | Esperado no 1º hit. Os seguintes usam o delta real |

## 8) Referência de código

- Servidor: `src/zone/MovementServer.hpp` (`broadcastForeignVitals`, handler 88), `src/zone/CombatCoreEngine.cpp` (`enqueueForeignVitals`, `processForeignVitals`), `src/zone/ZoneCombatService.{hpp,cpp}`
- Banco: `src/database/MySQLConnector.{hpp,cpp}`, `AccountDAO.cpp`, `PlayerDAO.cpp`
- PHP: `www/umbra_api/api/combat/{apply_vitals,dot_apply,dot_remove,respawn}.php`, `www/umbra_api/api/skills/set_skillbar.php`
- Cliente (sem mudança): `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp` (`SendForeignVitalsNotifyViaWebSocket`)

**Fim do guia.**
