> ⚠️ **DOCUMENTO OBSOLETO — NÃO SEGUIR (2026-06-20)**
>
> Este guia descreve uma arquitetura de **takeover C++** (`bCppOwnsRemoteStateUpdates`, dedupe, destroy adiado, `ReassertExclusiveWebSocketBinaryHandler`) que **causou a regressão** de duplicata de remote actor, fantasma e spam `Remote Actor Ref NOT valid`. Essa abordagem foi **revertida** e **não existe mais** no código.
>
> **Estado atual correto:** o **Blueprint `BP_NetMovementClient2` é dono** do spawn/movimento dos remote actors via `ProcessNextFrame`. O C++ apenas: roteia mensagens não-movimento (chat/NPC/combate), expõe `RemoteStates` e helpers **sob demanda** (`PrepareProcessNextFrame`/`GetValidRemoteActorForPlayerID`), e **não muta** `RemoteActors`/`RemoteActorIds` no caminho de movimento (MsgType 2).
>
> **Não reintroduzir** nada deste documento sem flag explícita e teste A/B em 2 Standalone. Mantido apenas como histórico.

---

# BP_NetMovementClient — higiene pós-fix C++ (UE 5.6.1) [HISTÓRICO/OBSOLETO]

O C++ em `ANetMovementClient` assume StateUpdate remoto (`bCppOwnsRemoteStateUpdates=true` por padrão), faz dedupe, destroy adiado e **reasserta handler WS exclusivo** (`ReassertExclusiveWebSocketBinaryHandler`) após `OnWSConnected` e a cada 0,25 s.

## 0. Handler WebSocket exclusivo (C++ já faz — BP opcional)

O C++ limpa `WebSocketRef->OnRawMessage` e registra **somente** `HandleWebSocketBinaryMessage`. Isso impede que `ProcessNextFrame` do BP rode em paralelo com refs stale.

**Recomendado no BP (EventGraph / OnWSConnected):**

1. **Remover** o nó `Add Delegate` de `OnRawMessage` → `OnWSBinaryMessage`.
2. O BP **não** deve escutar bytes binários diretamente; o C++ repassa só mensagens não-movimento via `OnWSBinaryMessage` quando `bCppOwnsRemoteStateUpdates=false`.

Com `bCppOwnsRemoteStateUpdates=true` (default), MsgType 2 remoto **nunca** chega ao BP.

## 1. Desabilitar spawn/update remoto no ProcessNextFrame

1. Abra `BP_NetMovementClient` (pai: `NetMovementClient`).
2. No início de `ProcessNextFrame`, adicione `Prepare Process Next Frame` (categoria Net|RemoteActors).
3. Envolva spawn/update remoto com `Branch`: `NOT bCppOwnsRemoteStateUpdates` — ou remova o caminho inteiro (recomendado).
4. Antes de `Set Actor Location`, use **`Get Valid Remote Actor For Player ID`** em vez de `RemoteActorRef` cacheado; só prossiga se `Is Valid`.

## 2. Corrigir Array_Add de RemoteActorIds (se ainda existir)

Conforme [`PROBLEMA_CRITICO_ARRAY_ADD_REMOTEACTORIDS.md`](PROBLEMA_CRITICO_ARRAY_ADD_REMOTEACTORIDS.md):

- Pin **Target** de `Array_Add (RemoteActorIds)` → `Get RemoteActorIds` (não `RemoteActors`).
- Remover `Array_Add` do ramo `then` (actor já existe) — ver [`GUIA_PRATICO_CORRECAO_MULTIPLOS_SPAWNS_E_MOVIMENTO.md`](GUIA_PRATICO_CORRECAO_MULTIPLOS_SPAWNS_E_MOVIMENTO.md).

## 3. Event Tick e interpolação

Com `bCppOwnsRemoteStateUpdates=true`, o **Tick C++** interpola `RemoteStates` e aplica `SetActorLocationAndRotation`.

- **Desabilite** o **ReceiveTick** do BP que itera `RemoteStates` + `Set Actor Rotation` (evita dupla aplicação e refs inválidas).
- Mantenha no BP apenas lógica visual secundária se necessário.

## 4. Remover spam "Remote Actor Ref NOT valid"

No `For Each` sobre `RemoteActors` em `ProcessNextFrame`:

- Substitua o `Print String` vermelho por `Prepare Process Next Frame` + `Get Valid Remote Actor For Player ID`, **ou** remova o loop se C++ processa StateUpdate.

## 5. Compilar

1. Build C++ (`Compile UmbraEternumUE` ou Ctrl+Alt+F11).
2. Compile e salve `BP_NetMovementClient` (remover AddDelegate se ainda existir).

## Teste rápido (2 Standalone)

- Zero erros `"BP_RemotePlayer_C_N não é válido"` em `ProcessNextFrame`.
- 1 remote actor por PlayerID; nameplates e widget de alvo corretos.
- Movimento simétrico entre os dois clients.
