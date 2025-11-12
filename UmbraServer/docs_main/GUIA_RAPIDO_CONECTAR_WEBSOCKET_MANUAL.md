# ⚡ **GUIA RÁPIDO: Conectar WebSocket Manualmente no Blueprint**

## 🎯 **PROBLEMA:**

O `BeginPlay` do C++ executa antes do personagem estar selecionado, então o WebSocket não conecta.

---

## ✅ **SOLUÇÃO: Chamar `ConnectWebSocketManual` do Blueprint**

**Adicionei uma função `ConnectWebSocketManual()` que pode ser chamada quando o personagem estiver pronto!**

---

## 📋 **IMPLEMENTAÇÃO RÁPIDA:**

### **OPÇÃO 1: Sobrescrever BeginPlay no Blueprint (RECOMENDADO)**

**No novo `BP_NetMovementClient`:**

1. **Event Graph** → **Botão direito** → **Event BeginPlay**
2. **Conecte** ao `Parent: BeginPlay` (chama o C++ primeiro)
3. **Adicione retry:**

```
[Event BeginPlay] (do Blueprint)
  ↓
[Parent: BeginPlay] ← CHAMA O C++ PRIMEIRO
  ↓
[Delay: 1.0] ← Aguardar personagem estar pronto
  ↓
[Get Game Instance] → [Cast to UmbraGameInstance] → [HasActiveCharacter]
  ↓
[Branch: HasActiveCharacter?]
  ├─ True:
  │    ↓
  │  [Call Function: ConnectWebSocketManual] ← FUNÇÃO C++
  │    ↓
  │  [Print String: "[BP_NEW] ✅ WebSocket conectado manualmente!"]
  └─ False:
       ↓
     [Print String: "[BP_NEW] ⚠️ Personagem não está pronto, tentando novamente..."]
     ↓
     [Delay: 2.0] ← Aguardar mais
     ↓
     [Call Function: ConnectWebSocketManual] ← Tentar novamente
```

---

### **OPÇÃO 2: Custom Event com Retry**

**No novo `BP_NetMovementClient`:**

1. **My Blueprint** → **Functions** → **+ Function**
2. **Nome:** `TryConnectWebSocket`
3. **Implemente:**

```
[Function: TryConnectWebSocket]
  ↓
[Get Game Instance] → [Cast to UmbraGameInstance] → [HasActiveCharacter]
  ↓
[Branch: HasActiveCharacter?]
  ├─ True:
  │    ↓
  │  [Call Function: ConnectWebSocketManual] ← FUNÇÃO C++
  │    ↓
  │  [Print String: "[BP_NEW] ✅ WebSocket conectado!"]
  └─ False:
       ↓
     [Print String: "[BP_NEW] ⚠️ Personagem não está pronto, retry em 1s..."]
     ↓
     [Delay: 1.0]
     ↓
     [Call Function: TryConnectWebSocket] ← RECURSIVO (retry)
```

**E chame no `BeginPlay`:**

```
[Event BeginPlay] (do Blueprint)
  ↓
[Parent: BeginPlay] ← CHAMA O C++ PRIMEIRO
  ↓
[Delay: 0.5] ← Aguardar inicialização
  ↓
[Call Function: TryConnectWebSocket] ← TENTAR CONECTAR
```

---

## 🧪 **TESTE:**

1. **Compile** o projeto
2. **Abra** o novo `BP_NetMovementClient` no Blueprint Editor
3. **Implemente** uma das opções acima
4. **Execute** o jogo
5. **Verifique os logs:**

**Deve aparecer:**
```
[NetMovementClient] ========== BeginPlay INICIADO! ==========
[NetMovementClient] ✅ GameInstance encontrado!
[NetMovementClient] NetMode: 3 (Client)
[NetMovementClient] ✅ Executando no Client!
[NetMovementClient] HasActiveCharacter: 0 (ou 1)
[NetMovementClient] ⚠️ Nenhum personagem selecionado ainda!
[BP_NEW] ConnectWebSocketManual chamado!
[NetMovementClient] ConnectWebSocketManual chamado!
[NetMovementClient] ✅ MyPlayerId setado: 1
[NetMovementClient] ✅ Criando e conectando WebSocket manualmente...
[NetMovementClient] WebSocket Connected!
```

---

## 🎯 **RESUMO:**

**Problema:** `BeginPlay` do C++ executa antes do personagem estar selecionado.

**Solução:**
1. ✅ Sobrescreva `BeginPlay` no Blueprint
2. ✅ Chame `Parent: BeginPlay` primeiro
3. ✅ Aguarde o personagem estar pronto
4. ✅ Chame `ConnectWebSocketManual()` quando pronto

**Com isso, o WebSocket deve conectar corretamente!**

