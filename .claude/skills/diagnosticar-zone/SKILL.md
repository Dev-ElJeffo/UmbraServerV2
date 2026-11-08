---
name: diagnosticar-zone
description: Investiga problemas da zone_server (crash, desconexão em massa, lag e tick lento, deadlock, combate e vitals errados, fila cheia) a partir dos logs, da telemetria de tick lento e dos crash dumps, e correlaciona com opcode, player e código. Só lê e diagnostica; propõe a correção para aprovação. Usar quando o usuário relatar crash, travamento, lag, queda de clientes ou comportamento estranho da zone, ou pedir /diagnosticar-zone.
---

# Diagnóstico da zone

**Só leitura** até o usuário aprovar uma correção. Pedir ao usuário: horário aproximado, zone (`zoneId`), quantos clients e o que estavam fazendo.

## 1) Fontes

| Fonte | Onde |
|---|---|
| Log da zone | `D:\UmbraServerV2\logs\zone_server.log` (`Logger::initialize` em `src/zone/main_zone.cpp`) |
| Log do monolito (login) | `logs/umbra_server.log` (`config/server.json` → `logging.file`) |
| Log do cliente | `UmbraEternumUE/Saved/Logs/UmbraEternumUE.log` (Output Log do PIE) |
| Crash dumps | `D:\UmbraServerV2\crash_dumps\zone\*.dmp`. Habilitar com `scripts_main\enable_zone_crash_dumps.bat` (WER LocalDumps; precisa de admin) |
| Stress de PvP | `scripts_main\stress_zone_pvp_combat.ps1` |

Os logs podem ser grandes: usar `tail -n 2000` ou Grep com padrão, sem ler tudo.

## 2) Padrões de log úteis

```bash
cd /d/UmbraServerV2/logs
grep -nE "\[(error|critical)\]" zone_server.log | tail -40
grep -n "tick lento" zone_server.log | tail -20              # CombatCoreEngine e NpcAi
grep -nE "queue full|fila de combate cheia|dropando" zone_server.log | tail
grep -nE "EDEADLK|deadlock|resource deadlock" zone_server.log | tail
grep -nE "SessionAuth|revoke|DuplicateLogin|aguardando SessionAuthNotify" zone_server.log | tail
grep -nE "Execute failed|Prepare failed|pool exhausted|server has gone away|Lost connection" zone_server.log | tail
grep -nE "rejeitado|ignorado" zone_server.log | tail -30     # 88 forjado, skill rejeitada etc.
```

Telemetria de tick lento (commit `4debac1`, só loga com ≥ 30 ms):
- `[CombatCoreEngine] tick lento Xms cpu=Yms (despawn= respawn= hits= dots= ...)`: a fase com mais ms é o gargalo;
- `[NpcAi] tick lento Xms cpu=Yms (players=..., npcLoop=... [lockWait=...])`: `lockWait` alto indica contenção de `mu_`;
- **wall ≫ cpu**: a thread estava bloqueada (I/O, MySQL remoto ou lock), não calculando. **cpu ≈ wall**: custo de CPU (loop, AOI, NPCs demais).

## 3) Causas conhecidas (histórico do projeto)

| Sintoma | Causa típica | Onde olhar |
|---|---|---|
| Clientes caem em cascata | DB/auth no callback de recv ou no tick | `MovementServer.hpp` (`setBinaryCallback`, `handleInboundBinary`), `handleClientDisconnect` |
| Desconexão com EDEADLK | `mu_` travado ao chamar o `CombatCoreEngine`, que volta em `broadcastToAll` | Branches 96/98/88 do `handleInboundBinary` |
| Tick lento periódico | SELECT síncrono no caminho quente; MySQL remoto lento | `executePreparedScalar` no `CombatCoreEngine`; trocar por cache + `enqueueDbWrite` |
| "inbound queue full" | Tick não drena (lento) ou cliente fazendo spam | Rate-limit por `cid` (ex.: 108) |
| Zone sem combate/EXP/loot | MySQL falhou no boot: modo só movimento | Início do log (`MySQL`/`connect`) |
| HP/MP divergente entre clients | Cache do `CharacterStateLoader` stale ou write-behind pendente | `patchCached*`, `broadcastPlayerVitals`, opcode 87/88 |
| Conflito de porta | Admin da zone 2 = 9104 = `admin.gateway_port` | `CLAUDE.md` §8 |

## 4) Crash dump

Se houver `.dmp`: informar caminho, tamanho e data. Abrir no Visual Studio (**File → Open → .dmp → Debug with Native Only**) com os `.pdb` de `build/bin/Release`. Se o usuário colar a pilha, mapear os frames para os arquivos `src/`. Não commitar dumps.

## 5) Reproduzir (com OK)

- Subir uma zone isolada: `build\bin\Release\zone_server.exe <zoneId>` (WS em 8082+id, admin em 9102+id).
- Para combate/carga: `stress_zone_pvp_combat.ps1` (ler o script antes e confirmar os parâmetros com o usuário).
- Nível debug só durante a investigação (`config/server.json` → `logging.level`), e lembrar de voltar.

## 6) Relatório

1. Linha do tempo (horário → evento do log).
2. Causa mais provável com evidência (`arquivo:linha` do log e do código) e o grau de confiança.
3. Correção proposta (arquivos e abordagem) e como validar (`/validar`, PIE ≥ 2, stress).
4. Se a causa não fechar: o que coletar a mais (dump, nível debug, horário exato).
