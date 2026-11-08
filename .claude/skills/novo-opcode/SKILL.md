---
name: novo-opcode
description: Cria um opcode novo, ou um campo trailing num opcode existente, no protocolo WS binário da zone (servidor C++ e cliente UE 5.6.1), seguindo as regras do CLAUDE.md §4. Cobre o enum, o encode/decode, o branch no handleInboundBinary (só enfileira), o worker, o forward no NetMovementClient, o handler e delegate no GameInstance e a seção Protocolo do guia. Usar quando a feature exigir mensagem nova cliente↔zone ou campo novo em mensagem existente.
---

# Opcode novo / campo novo na zone

Regras: `CLAUDE.md` §4 e §5. Pacote **little-endian**, opcode no 1º byte, frames de movimento 25B/34B intocados.

## 0) Plano (antes de editar)

Definir e apresentar ao usuário para o OK:
- **Direção:** Cliente→Servidor, Servidor→Cliente ou ambos.
- **Campos:** nome, tipo (`u8/u16/u32/i32/f32`, string `u16`+UTF-8 com limite) e significado.
- **Quem processa no servidor** (`CombatCoreEngine`, `LootService`, `QuestProgressService`...) e **quem recebe** (sender, AOI, party, todos).
- Arquivos dos dois lados (lista abaixo) e o guia.

## 1) Alocar o valor

```bash
grep -nE '= [0-9]+,' src/zone/MovementProtocol.hpp | tail -5
```
O próximo opcode é o último do `enum MovementMsgType` + 1 (era **118**, `CombatControlState`, em 2026-11-08). Conferir também se o UE não usa o valor (`grep -n "MsgType == <N>"` em `NetMovementClient.cpp` e `UmbraGameInstance.cpp`). Depois, atualizar o "último opcode" no `CLAUDE.md` §4 (é arquivo fora do código: incluir no plano).

## 2) Servidor: `src/zone/MovementProtocol.hpp`

- Valor no enum com comentário da direção (`// Cliente -> Servidor: ...`).
- `struct <Nome>Payload` com defaults.
- `encode<Nome>` / `decode<Nome>`, seguindo `encodeCombatControlState` como referência. Strings via `appendStringField(out, s, maxBytes)` / `readStringField(data, off, out, maxBytes)`.
- **Decoder tolerante:** valida o tamanho mínimo e lê os campos trailing só se `off < data.size()` (frame antigo → 0 / string vazia).
- **Campo novo em opcode existente:** sempre no **fim**; nunca reordenar nem mudar tipo.

## 3) Servidor: dispatch e processamento

Em `src/zone/MovementServer.hpp`, `handleInboundBinary` (roda no **tick**; o recv só enfileira):

```cpp
if (msgType == MovementMsgType::<Nome>) {
  <Nome>Payload payload;
  if (decode<Nome>(data, payload)) {
    uint32_t senderPlayerId = 0;
    {
      std::lock_guard<std::mutex> lock(mu_);
      auto it = clientIdToPlayerId_.find(cid);
      if (it != clientIdToPlayerId_.end()) senderPlayerId = it->second;
    }
    // Nunca confiar em IDs/valores do payload: o sender vem do cid.
    if (senderPlayerId > 0 && combatCoreEngine_) combatCoreEngine_->enqueue<Nome>(senderPlayerId, payload);
  }
  return;
}
```

- **Proibido** no branch: DB (`db_->`, `executePrepared*`), I/O e manter `mu_` travado ao chamar o engine.
- Se precisar de DB: novo `CombatJobKind` no `CombatCoreEngine` (`enqueue<Nome>` + `process<Nome>` no worker, com fallback síncrono quando `!workerRunning_` e dedup na fila se fizer sentido). O padrão de referência é `enqueueForeignVitals`/`processForeignVitals`.
- Escrita: `enqueueDbWrite`. Broadcast Servidor→Cliente: método público do `MovementServer` que trava `mu_`, monta o `Outbox` e chama `flushOutbox` depois de soltar o lock.
- Validar no servidor tudo que o cliente pode forjar: posse, alcance (`CombatRange.hpp`), cooldown e rate-limit por `cid`.

## 4) Cliente UE

1. `Network/NetMovementClient.cpp`, `HandleWebSocketBinaryMessage`: incluir `|| MsgType == <N>` na lista `bIsSocialMessage` (perto de `MsgType == 117 || MsgType == 118`). Sem isso, o GameInstance nunca recebe a mensagem.
2. `Core/UmbraGameInstance.cpp`, `ProcessSocialWebSocketMessage`: branch `if (MsgType == <N>)` com o parse little-endian (mesmo layout do decoder, tolerante a frame curto) e o comentário citando o opcode.
3. `Core/UmbraGameInstance.h`: `DECLARE_DYNAMIC_MULTICAST_DELEGATE_*` (ex.: `FOnCombatControlStateUpdated`) + `UPROPERTY(BlueprintAssignable)` só se o BP precisar escutar para algo cosmético. Senão, o componente C++ faz o bind via `AddDynamic` com o handler `UFUNCTION()`.
4. Envio Cliente→Servidor: função `Send<Nome>ViaWebSocket` no GameInstance (padrão de `SendForeignVitalsNotifyViaWebSocket`), checando a conexão antes de enviar.
5. O componente ou widget reage em C++. Nenhuma lógica no Event Graph.

Não compilar o editor: listar os arquivos UE para o usuário compilar.

## 5) Guia

Na seção **"4) Protocolo"** do guia da feature (`docs_main/GUIA_<TEMA>_UE561.md`):

| Campo | Tipo | Observação |
|---|---|---|
| `campo` | `u32` | trailing; frame antigo → 0 |

com a direção, quem recebe, o arquivo do encoder e o do handler UE. Ao QA, acrescentar "frame antigo / campo vazio → sem crash".

## 6) Validação

`/validar` (build `zone_server`, lint e checagem de DB no tick). Teste: PIE com ≥ 2 clients, conferindo que o remoto recebe (quando houver broadcast) e que um frame forjado (outro `playerId` no payload) é ignorado.
