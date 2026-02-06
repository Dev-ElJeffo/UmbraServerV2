# 🔧 GUIA: Registro Automático de Remote Actor (V2)

## 🎯 PROBLEMA

Quando um client conecta após outro client já estar logado, o segundo client não vê o nameplate do primeiro client, mesmo que o servidor esteja enviando corretamente o `PlayerInfoUpdate` e `StateUpdate`.

## ✅ SOLUÇÃO

O Blueprint **DEVE** chamar `RegisterRemoteActorInGameInstance` após spawnar um remote actor e adicioná-lo aos arrays.

---

## 📋 ONDE ADICIONAR

No Blueprint `BP_NetMovementClient2`, na função `ProcessNextFrame`, **IMEDIATAMENTE APÓS** adicionar o actor aos arrays:

```
[ProcessNextFrame]
    │
    ├──► [ParseStateUpdateFrame] → OutPlayerId, OutLocation, OutYawDegrees
    │
    ├──► [Array_Find (RemoteActorIds, OutPlayerId)] → FoundIndex
    │
    ├──► [Branch: FoundIndex >= 0?]
    │       │
    │       ├──► True: [Atualiza actor existente]
    │       │
    │       └──► False: [Novo actor - SPAWN]
    │               │
    │               ├──► [SpawnActorFromClass] → SpawnedActor
    │               │       Class: BP_RemotePlayer
    │               │       Transform: (OutLocation, OutYawDegrees)
    │               │
    │               ├──► [Add Item to Array]
    │               │       Array: RemoteActorIds
    │               │       Item: OutPlayerId
    │               │
    │               ├──► [Add Item to Array]
    │               │       Array: RemoteActors
    │               │       Item: SpawnedActor
    │               │
    │               └──► ⭐⭐ [AQUI ADICIONE O REGISTRO] ⭐⭐
    │                       │
    │                       └──► [Register Remote Actor In Game Instance]
    │                               Target: self (BP_NetMovementClient2)
    │                               PlayerID: OutPlayerId
    │                               RemoteActor: SpawnedActor
```

---

## 🔧 PASSO A PASSO

### **PASSO 1: Localizar o Spawn do Actor**

1. Abra o Blueprint `BP_NetMovementClient2`
2. Encontre a função `ProcessNextFrame`
3. Localize o nó `Spawn Actor from Class` que cria `BP_RemotePlayer`
4. Encontre onde `Add Item to Array` é chamado para `RemoteActors`

### **PASSO 2: Adicionar Register Remote Actor In Game Instance**

**IMEDIATAMENTE APÓS `Add Item to Array` para `RemoteActors`:**

1. **Adicione `Register Remote Actor In Game Instance`:**
   - Busque por `Register Remote Actor In Game Instance`
   - **Target:** Conecte ao `self` (BP_NetMovementClient2)
   - **PlayerID:** Conecte ao `OutPlayerId` (do `ParseStateUpdateFrame`)
   - **RemoteActor:** Conecte ao `SpawnedActor` (do `Spawn Actor from Class`)

**IMPORTANTE:** Esta função:
- Registra o actor no `RemotePlayerActorsMap` do GameInstance
- Aplica automaticamente dados pendentes de nameplate se houver
- Garante que o nameplate seja atualizado corretamente

---

## ✅ RESULTADO ESPERADO

Após adicionar o registro:

1. **Todos os remote actors serão registrados** quando spawnados
2. **Dados pendentes de nameplate serão aplicados automaticamente**
3. **Nameplates aparecerão corretamente** mesmo em login sequencial
4. **Logs mostrarão:** `✅✅✅ Remote actor registrado com sucesso no GameInstance!`

---

## 🔍 VERIFICAÇÃO

Após adicionar, verifique os logs do cliente:

```
[NetMovementClient] ✅ Registrando remote actor no GameInstance: PlayerID=1, Actor=BP_RemotePlayer_C_0
[UmbraGameInstance] ✅✅✅ Actor remoto registrado: PlayerID 1, Actor: BP_RemotePlayer_C_0 (Total no Map: 1) ✅✅✅
[NetMovementClient] ✅✅✅ Remote actor registrado com sucesso no GameInstance! PlayerID=1 ✅✅✅
```

Se houver dados pendentes:
```
[UmbraGameInstance] 🔄🔄🔄 Dados de nameplate pendentes encontrados para PlayerID 1! Aplicando agora... 🔄🔄🔄
[UmbraGameInstance] ✅✅✅ Dados pendentes aplicados diretamente ao widget! PlayerID=1 ✅✅✅
```

---

## ⚠️ NOTA IMPORTANTE

Esta função **DEVE** ser chamada **APENAS** quando um novo actor é spawnado (não quando um actor existente é atualizado).

Certifique-se de que está no ramo `False` do `Branch` que verifica se o actor já existe.
