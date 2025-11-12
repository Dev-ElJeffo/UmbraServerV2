# 🔧 **CORREÇÃO F9 SIMPLES: Usar MyPlayerId para Identificar o Client Correto**

## 🎯 **PROBLEMA:**

**O F9 está fechando o WebSocket de TODOS os clients porque:**
- `GetAllActorsOfClass` retorna TODOS os `BP_NetMovementClient`
- `Get Array Item` com Index 0 pega o PRIMEIRO, que pode ser de outro client

---

## ✅ **SOLUÇÃO SIMPLES: Comparar MyPlayerId**

**No `BP_ThirdPersonCharacter`, no evento `Action DisconnectWebsocket`:**

```
[Action DisconnectWebsocket Pressed]
  ↓
[Print String: "🔴 [F9] Desconectando WebSocket..."]
  ↓
[Get Game Instance]
  ↓
[Cast to Umbra Game Instance]
  ├─ Success:
  │   ├─ [Get Active Player ID] → Armazenar em variável temporária
  │   ├─ [Get All Actors of Class: BP_NetMovementClient]
  │   ├─ [Get Array Length]
  │   ├─ [Branch: Length > 0?]
  │   │   ├─ True:
  │   │   │   ├─ [Get Array Item] (Index: 0)
  │   │   │   ├─ [Get Variable: MyPlayerId]
  │   │   │   │   └─ Target: [Get Array Item]
  │   │   │   ├─ [Equal (Int Int)]
  │   │   │   │   ├─ A: [Get Variable: MyPlayerId]
  │   │   │   │   └─ B: [Get Active Player ID return value]
  │   │   │   ├─ [Branch: Equal?]
  │   │   │   │   ├─ True: (É O CORRETO!)
  │   │   │   │   │   ├─ [Get Variable: WebSocketRef]
  │   │   │   │   │   │   └─ Target: [Get Array Item]
  │   │   │   │   │   ├─ [Is Valid: WebSocketRef?]
  │   │   │   │   │   │   ├─ True:
  │   │   │   │   │   │   │   ├─ [Print String: "✅ [F9] WebSocketRef válido, fechando..."]
  │   │   │   │   │   │   │   ├─ [Call Function: Close]
  │   │   │   │   │   │   │   │   └─ Target: [WebSocketRef]
  │   │   │   │   │   │   │   └─ [Print String: "✅ [F9] WebSocket fechado com sucesso"]
  │   │   │   │   │   │   └─ False:
  │   │   │   │   │   │       └─ [Print String: "⚠️ [F9] WebSocketRef inválido"]
  │   │   │   │   └─ False: (NÃO É O CORRETO, tentar próximo)
  │   │   │   │       ├─ [Get Array Item] (Index: 1) ← TENTAR PRÓXIMO
  │   │   │   │       ├─ [Get Variable: MyPlayerId]
  │   │   │   │       │   └─ Target: [Get Array Item]
  │   │   │   │       ├─ [Equal (Int Int)]
  │   │   │   │       │   ├─ A: [Get Variable: MyPlayerId]
  │   │   │   │       │   └─ B: [Get Active Player ID return value]
  │   │   │   │       ├─ [Branch: Equal?]
  │   │   │   │       │   ├─ True: (É O CORRETO!)
  │   │   │   │       │   │   └─ [Mesma lógica acima]
  │   │   │   │       │   └─ False: (Tentar próximo...)
  │   │   │   │       └─ [Continuar para Index 2, 3, etc...]
  │   │   └─ False:
  │   │       └─ [Print String: "❌ [F9] BP_NetMovementClient não encontrado no level"]
  └─ Failed:
      └─ [Print String: "❌ [F9] Falha ao obter Game Instance"]
```

---

## ✅ **SOLUÇÃO AINDA MAIS SIMPLES: Usar Loop com Limite**

**Como normalmente há poucos clients (2-4), podemos tentar os primeiros índices:**

```
[Action DisconnectWebsocket Pressed]
  ↓
[Print String: "🔴 [F9] Desconectando WebSocket..."]
  ↓
[Get Game Instance]
  ↓
[Cast to Umbra Game Instance]
  ├─ Success:
  │   ├─ [Get Active Player ID] → Armazenar
  │   ├─ [Get All Actors of Class: BP_NetMovementClient]
  │   ├─ [Get Array Length]
  │   ├─ [Branch: Length > 0?]
  │   │   ├─ True:
  │   │   │   ├─ [Execution Sequence] (para tentar múltiplos índices)
  │   │   │   │   ├─ [Out 0]: Tentar Index 0
  │   │   │   │   ├─ [Out 1]: Tentar Index 1
  │   │   │   │   ├─ [Out 2]: Tentar Index 2
  │   │   │   │   └─ [Out 3]: Tentar Index 3
  │   │   │   │
  │   │   │   ├─ [SEQUENCE OUT 0]:
  │   │   │   │   ├─ [Get Array Item] (Index: 0)
  │   │   │   │   ├─ [Get Variable: MyPlayerId] (Target: Array Item)
  │   │   │   │   ├─ [Equal] (MyPlayerId == ActivePlayerID?)
  │   │   │   │   ├─ [Branch: Equal?]
  │   │   │   │   │   ├─ True:
  │   │   │   │   │   │   ├─ [Get Variable: WebSocketRef] (Target: Array Item)
  │   │   │   │   │   │   ├─ [Is Valid: WebSocketRef?]
  │   │   │   │   │   │   │   ├─ True:
  │   │   │   │   │   │   │   │   ├─ [Call Function: Close] (Target: WebSocketRef)
  │   │   │   │   │   │   │   │   └─ [Print String: "✅ [F9] WebSocket fechado"]
  │   │   │   │   │   │   │   └─ False: (tentar próximo)
  │   │   │   │   │   └─ False: (tentar próximo)
  │   │   │   │   │
  │   │   │   ├─ [SEQUENCE OUT 1]: (mesma lógica, Index: 1)
  │   │   │   ├─ [SEQUENCE OUT 2]: (mesma lógica, Index: 2)
  │   │   │   └─ [SEQUENCE OUT 3]: (mesma lógica, Index: 3)
  │   │   └─ False:
  │   │       └─ [Print String: "❌ [F9] BP_NetMovementClient não encontrado"]
  └─ Failed:
      └─ [Print String: "❌ [F9] Falha ao obter Game Instance"]
```

---

## 🎯 **IMPLEMENTAÇÃO PRÁTICA (VERSÃO SIMPLES):**

**Como normalmente há 1-4 clients, vamos tentar os primeiros 4 índices:**

### **PASSO 1: Criar Função "FindAndCloseMyWebSocket"**

**No `BP_ThirdPersonCharacter`, criar função:**

**Nome:** `FindAndCloseMyWebSocket`

**Implementação:**

```
[Function Entry: FindAndCloseMyWebSocket]
  ↓
[Get Game Instance]
  ↓
[Cast to Umbra Game Instance]
  ├─ Success:
  │   ├─ [Get Active Player ID] → Variável local: "MyActivePlayerID"
  │   ├─ [Get All Actors of Class: BP_NetMovementClient]
  │   ├─ [Get Array Length]
  │   ├─ [Branch: Length > 0?]
  │   │   ├─ True:
  │   │   │   ├─ [For Loop] (Index de 0 até Length-1)
  │   │   │   │   ├─ [Get Array Item] (Index: [Loop Index])
  │   │   │   │   ├─ [Get Variable: MyPlayerId]
  │   │   │   │   │   └─ Target: [Get Array Item]
  │   │   │   │   ├─ [Equal (Int Int)]
  │   │   │   │   │   ├─ A: [Get Variable: MyPlayerId]
  │   │   │   │   │   └─ B: [MyActivePlayerID]
  │   │   │   │   ├─ [Branch: Equal?]
  │   │   │   │   │   ├─ True: (ENCONTROU!)
  │   │   │   │   │   │   ├─ [Get Variable: WebSocketRef]
  │   │   │   │   │   │   │   └─ Target: [Get Array Item]
  │   │   │   │   │   │   ├─ [Is Valid: WebSocketRef?]
  │   │   │   │   │   │   │   ├─ True:
  │   │   │   │   │   │   │   │   ├─ [Call Function: Close]
  │   │   │   │   │   │   │   │   │   └─ Target: [WebSocketRef]
  │   │   │   │   │   │   │   │   └─ [Print String: "✅ [F9] WebSocket fechado"]
  │   │   │   │   │   │   │   └─ False:
  │   │   │   │   │   │   │       └─ [Print String: "⚠️ [F9] WebSocketRef inválido"]
  │   │   │   │   │   │   └─ [Return Node] (sair da função)
  │   │   │   │   │   └─ False: (continuar loop)
  │   │   │   └─ [Loop Completed] (não encontrou)
  │   │   │       └─ [Print String: "❌ [F9] BP_NetMovementClient com MyPlayerId correspondente não encontrado"]
  │   │   └─ False:
  │   │       └─ [Print String: "❌ [F9] BP_NetMovementClient não encontrado no level"]
  └─ Failed:
      └─ [Print String: "❌ [F9] Falha ao obter Game Instance"]
```

### **PASSO 2: Chamar a Função no Action DisconnectWebsocket**

```
[Action DisconnectWebsocket Pressed]
  ↓
[Print String: "🔴 [F9] Desconectando WebSocket..."]
  ↓
[Call Function: FindAndCloseMyWebSocket]
```

---

## 🧪 **TESTE:**

1. **Compile** o Blueprint
2. **Execute** o jogo com **2 clients**
3. **No Client 1**, pressione F9
4. **Verifique os logs:**
   ```
   🔴 [F9] Desconectando WebSocket...
   ✅ [F9] WebSocket fechado
   ```
5. **Verifique:**
   - ✅ Apenas o Client 1 desconecta
   - ✅ O Client 2 continua conectado
   - ✅ O servidor detecta a desconexão e envia `PlayerDisconnected`
   - ✅ O Client 2 remove o remote actor do Client 1

---

**✅ Esta solução garante que apenas o WebSocket do próprio client seja fechado!**

