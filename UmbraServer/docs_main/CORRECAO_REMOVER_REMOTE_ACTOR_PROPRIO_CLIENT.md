# 🔧 **CORREÇÃO: Remover Remote Actor do Próprio Client ao Desconectar**

## 🎯 **PROBLEMA:**

**Situação:**
- Segundo client (ID 19) tem um remote actor spawnado no mundo
- Quando o client fecha, o remote actor dele **NÃO está sendo removido**
- O remote actor permanece no mundo mesmo após o client desconectar

**Causa:**
- O `OnWSClosed` não está removendo o remote actor do próprio client
- O `EndPlay` não está removendo o remote actor do próprio client
- O servidor pode não estar enviando a mensagem `PlayerDisconnected` quando o client fecha

---

## ✅ **SOLUÇÃO COMPLETA:**

### **PARTE 1: Remover Remote Actor no OnWSClosed**

**No `BP_NetMovementClient`, no evento `OnWSClosed`:**

```
[OnWSClosed]
  ↓
[Print String: "🔴 [OnWSClosed] WebSocket fechado, removendo remote actors..."]
  ↓
[Get Variable: MyPlayerId]
  ↓
[Print String: "🔴 [OnWSClosed] Removendo remote actor do próprio client (ID: [MyPlayerId])"]
  ↓
[Call Function: RemoveRemoteActor] (Input: MyPlayerId)
  ↓
[Call Function: CleanupRemoteActors]
  ↓
[Print String: "🔴 [OnWSClosed] Limpeza completa!"]
```

**IMPORTANTE:** Remover o remote actor do próprio client **ANTES** de chamar `CleanupRemoteActors` (que remove os remote actors dos outros players).

---

### **PARTE 2: Remover Remote Actor no EndPlay**

**No `BP_NetMovementClient`, no evento `Event EndPlay`:**

```
[Event EndPlay]
  ↓
[Print String: "🔴 [EndPlay] Evento disparado, removendo remote actors..."]
  ↓
[Get Variable: MyPlayerId]
  ↓
[Print String: "🔴 [EndPlay] Removendo remote actor do próprio client (ID: [MyPlayerId])"]
  ↓
[Call Function: RemoveRemoteActor] (Input: MyPlayerId)
  ↓
[Branch: Is Valid (GetFirstPlayerPawnHelper)?]
  ├─ True: [Save Player Position]
  └─ False: (não fazer nada)
  ↓
[Call Function: CleanupRemoteActors]
  ↓
[Branch: Is Valid (WebSocketRef)?]
  ├─ True: [Close WebSocket]
  └─ False: (não fazer nada)
  ↓
[Print String: "🔴 [EndPlay] Limpeza completa!"]
```

**IMPORTANTE:** Remover o remote actor do próprio client **ANTES** de chamar `CleanupRemoteActors`.

---

### **PARTE 3: Verificar se RemoveRemoteActor Funciona Corretamente**

**No `BP_NetMovementClient`, verificar a função `RemoveRemoteActor`:**

**Estrutura correta:**
```
RemoveRemoteActor (Custom Event)
  Input: PlayerID
  ↓
[Print String: "🔴 [RemoveRemoteActor] Removendo player: [PlayerID]"]
  ↓
[Array_Find] (RemoteActorIds, PlayerID)
  → FoundIndex
  ↓
[Greater or Equal] (FoundIndex >= 0?)
  → bFound
  ↓
[Branch: bFound?]
  ├─ True:
  │   ├─ [Get Array Item] (RemoteActors, FoundIndex)
  │   │   → Actor
  │   ├─ [Is Valid: Actor?]
  │   │   ├─ True:
  │   │   │   ├─ [Destroy Actor: Actor]
  │   │   │   └─ [Print String: "🔴 [RemoveRemoteActor] Actor destruído!"]
  │   │   └─ False:
  │   │       └─ [Print String: "⚠️ [RemoveRemoteActor] Actor inválido!"]
  │   ├─ [Remove Array Item] (RemoteActorIds, FoundIndex)
  │   ├─ [Remove Array Item] (RemoteActors, FoundIndex)
  │   └─ [Print String: "🔴 [RemoveRemoteActor] Player removido dos arrays!"]
  └─ False:
      └─ [Print String: "⚠️ [RemoveRemoteActor] Player não encontrado nos arrays!"]
```

---

## 🔍 **VERIFICAÇÃO ADICIONAL: Servidor Enviando PlayerDisconnected**

**Verificar se o servidor está enviando a mensagem `PlayerDisconnected` quando um client desconecta:**

**No servidor C++ (`MovementServer.hpp` ou similar):**

Quando um client desconecta, o servidor deve:
1. **Enviar mensagem `PlayerDisconnected`** para todos os outros clients
2. **Remover o client da lista de clients conectados**

**Se o servidor não estiver enviando, os outros clients não saberão que o player desconectou.**

---

## 📋 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: Adicionar Remoção no OnWSClosed**

**No `BP_NetMovementClient`, localize o evento `OnWSClosed`:**

1. **Se não existir, criar:**
   - Botão direito → **"Event"** → **"OnWSClosed"** (do `UmbraWSClient`)
   - Conectar ao `WebSocketRef`

2. **Adicionar lógica:**
   ```
   [OnWSClosed]
     ↓
   [Get Variable: MyPlayerId]
     ↓
   [Call Function: RemoveRemoteActor] (Input: MyPlayerId)
     ↓
   [Call Function: CleanupRemoteActors]
   ```

### **PASSO 2: Adicionar Remoção no EndPlay**

**No `BP_NetMovementClient`, localize o evento `Event EndPlay`:**

1. **Adicionar no início:**
   ```
   [Event EndPlay]
     ↓
   [Get Variable: MyPlayerId]
     ↓
   [Call Function: RemoveRemoteActor] (Input: MyPlayerId)
     ↓
   [Resto da lógica existente...]
   ```

### **PASSO 3: Verificar RemoveRemoteActor**

**No `BP_NetMovementClient`, verificar a função `RemoveRemoteActor`:**

1. **Verificar se existe**
2. **Verificar se está correta** (ver estrutura acima)
3. **Se não existir ou estiver incorreta, corrigir**

---

## 🧪 **TESTE:**

1. **Compile** o Blueprint
2. **Execute** o jogo com 2 clients
3. **No Client 1**, verifique se há um remote actor do Client 2
4. **No Client 2**, pressione F9 para fechar o WebSocket
5. **Verifique os logs:**
   ```
   🔴 [OnWSClosed] WebSocket fechado, removendo remote actors...
   🔴 [OnWSClosed] Removendo remote actor do próprio client (ID: 19)
   🔴 [RemoveRemoteActor] Removendo player: 19
   🔴 [RemoveRemoteActor] Actor destruído!
   🔴 [RemoveRemoteActor] Player removido dos arrays!
   🔴 [OnWSClosed] Limpeza completa!
   ```
6. **No Client 1**, verifique se o remote actor do Client 2 foi removido

---

## ⚠️ **IMPORTANTE:**

**Ordem de execução:**
1. **Primeiro:** Remover o remote actor do próprio client (`RemoveRemoteActor` com `MyPlayerId`)
2. **Depois:** Remover os remote actors dos outros players (`CleanupRemoteActors`)

**Isso garante que:**
- O próprio client remove seu remote actor antes de limpar os outros
- Os outros clients também recebem a mensagem `PlayerDisconnected` do servidor (se implementado)

---

**✅ Com essa correção, o remote actor do próprio client será removido quando ele desconectar!**

