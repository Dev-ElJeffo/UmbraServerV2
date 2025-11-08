# 🔧 **SOLUÇÃO: Usar OnWSClosed para CleanupRemoteActors**

## 🎯 **PROBLEMA:**

No PIE, fechar uma janela (X) fecha todas as janelas, então não é possível testar o `Event EndPlay` de um único cliente.

---

## ✅ **SOLUÇÃO: Usar OnWSClosed do WebSocket**

Quando um cliente desconecta do servidor WebSocket, o evento `OnWSClosed` dispara. Use esse evento para chamar `CleanupRemoteActors`.

---

## 📋 **IMPLEMENTAÇÃO:**

### **No `BP_NetMovementClient`:**

**Adicione um evento `OnWSClosed` (se ainda não existir):**

1. **No `BP_NetMovementClient`, localize o `WebSocketRef`**
2. **Adicione um evento `OnWSClosed`** (do `UmbraWSClient`)
3. **Conecte `CleanupRemoteActors` ao `OnWSClosed`:**

```
OnWSClosed (do WebSocket)
  ↓
Print String: "🔴 [OnWSClosed] WebSocket fechado, limpando remote actors..."
  ↓
CleanupRemoteActors
  ↓
Print String: "🔴 [OnWSClosed] CleanupRemoteActors executado!"
```

---

## 🧪 **COMO TESTAR:**

### **Método 1: Desconectar um Cliente do Servidor**

1. **Inicie o servidor C++ (`zone_server`)**
2. **Inicie 2 clientes PIE**
3. **Em um cliente, pare o servidor ou feche a conexão WebSocket manualmente**
4. **O `OnWSClosed` deve disparar e chamar `CleanupRemoteActors`**

### **Método 2: Simular Desconexão no Blueprint**

**Adicione um botão ou comando para fechar o WebSocket manualmente:**

```
[Botão ou Comando]
  ↓
Is Valid (WebSocketRef)
  ├─ then: Close (WebSocket) ← Isso dispara OnWSClosed
  └─ else: (não conectado)
```

### **Método 3: Usar Console Command**

**No cliente que quer desconectar:**

1. Pressione ` (til) para abrir o console
2. Digite: `disconnect` (se disponível)
3. Ou feche o WebSocket via Blueprint

---

## 📋 **ESTRUTURA COMPLETA RECOMENDADA:**

```
OnWSClosed (do WebSocket)
  ↓
Print String: "🔴 [OnWSClosed] WebSocket fechado!"
  ↓
CleanupRemoteActors
  ↓
Print String: "🔴 [OnWSClosed] Cleanup completo!"
```

**E no `Event EndPlay` (como backup):**

```
Event EndPlay
  ↓
Print String: "[Event EndPlay] EVENTO DISPARADO!"
  ↓
Is Valid (WebSocketRef)
  ├─ then: Close (WebSocket) ← Isso dispara OnWSClosed, que chama CleanupRemoteActors
  └─ else: CleanupRemoteActors ← Se WebSocket já foi fechado, limpa diretamente
```

---

## ⚠️ **IMPORTANTE:**

- **`OnWSClosed` dispara quando o WebSocket fecha** (desconexão do servidor)
- **`Event EndPlay` dispara quando o actor é destruído** (fechar PIE)
- **Use ambos** para garantir que `CleanupRemoteActors` sempre execute

---

## 🎯 **VANTAGENS:**

- **`OnWSClosed` é mais confiável** para desconexões reais
- **Funciona mesmo se `Event EndPlay` não disparar**
- **Testável sem fechar o PIE**

