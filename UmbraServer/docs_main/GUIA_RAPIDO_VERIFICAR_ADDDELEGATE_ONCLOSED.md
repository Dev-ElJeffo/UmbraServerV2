# 🚀 **GUIA RÁPIDO: Verificar se AddDelegate OnClosed Está Sendo Chamado**

## 🎯 **PROBLEMA:**

`OnWSClosed` não está disparando, mas `OnWSConnected` e `OnWSBinaryMessage` funcionam.

**Causa provável:** O `AddDelegate` para `OnClosed` não está sendo chamado em `OnWSConnected`.

---

## ✅ **SOLUÇÃO RÁPIDA:**

### **PASSO 1: Localizar `OnWSConnected` no Blueprint**

**No `BP_NetMovementClient`, localize o evento `OnWSConnected`:**

1. **Procure por `OnWSConnected` no Event Graph**
2. **Encontre onde o WebSocket é criado/conectado**

---

### **PASSO 2: Verificar se `AddDelegate` para `OnClosed` Existe**

**Após criar/conectar o WebSocket, você DEVE ter:**

```
[OnWSConnected]
  ↓
[Create WebSocket] ou [Connect WebSocket]
  ↓
[Set Variable: WebSocketRef = (WebSocket criado)]
  ↓
[Add Delegate: OnClosed] ← DEVE ESTAR AQUI!
  ├─ Target: WebSocketRef
  ├─ Delegate: OnClosed
  └─ Function: OnWSClosed (custom event)
```

**Se NÃO encontrar `Add Delegate: OnClosed`, você precisa adicionar!**

---

### **PASSO 3: Adicionar `AddDelegate` para `OnClosed` (se não existir)**

**No `BP_NetMovementClient`, no evento `OnWSConnected`:**

1. **Localize onde `WebSocketRef` é definido (após criar o WebSocket)**
2. **Botão direito** → **"Get Variable: WebSocketRef"**
3. **Arraste do pin de saída** → Procure por **"OnClosed"** ou **"Get OnClosed"**
4. **Botão direito** → **"AddDelegate"** (ou "Bind Event to OnClosed")
5. **Conecte:**
   - **Target:** `WebSocketRef` (pin de saída)
   - **Delegate:** `OnClosed` (pin do WebSocketRef)
   - **Function:** `OnWSClosed` (custom event - arraste do Event Graph)
6. **Conecte a execução:** Do `OnWSConnected` → Para o `AddDelegate`

**Estrutura final:**

```
[OnWSConnected]
  ↓
[Create WebSocket] ou [Connect WebSocket]
  ↓
[Set Variable: WebSocketRef = (WebSocket criado)]
  ↓
[Is Valid (WebSocketRef)?]
  ├─ then:
  │    ↓
  │  [Get Variable: WebSocketRef]
  │    ↓
  │  [Get OnClosed] (ou similar)
  │    ↓
  │  [AddDelegate]
  │    ├─ Target: WebSocketRef
  │    ├─ Delegate: OnClosed
  │    └─ Function: OnWSClosed
  │    ↓
  │  [Print String: "🔴 [OnWSConnected] Delegate OnClosed conectado!"] ← ADICIONAR LOG
  └─ else:
       [Print String: "⚠️ [OnWSConnected] WebSocketRef inválido!"]
```

---

### **PASSO 4: Adicionar Log de Confirmação**

**Após o `AddDelegate`, adicione um log:**

1. **Botão direito** → **"Print String"**
2. **InString:** `"🔴 [OnWSConnected] Delegate OnClosed conectado com sucesso!"`
3. **Conecte a execução:** Do `AddDelegate.then` → Para o `Print String.execute`

---

### **PASSO 5: Adicionar Log no Início do `OnWSClosed`**

**No `BP_NetMovementClient`, no evento `OnWSClosed`:**

1. **Localize o primeiro nó do `OnWSClosed` custom event**
2. **Adicione um log ANTES de qualquer outra coisa:**

```
[OnWSClosed Custom Event]
  ↓
[Print String: "[DEBUG] OnWSClosed Custom Event DISPARADO!"] ← PRIMEIRO LOG
  ↓
[Print String: "[OnWSClosed] EVENTO DISPARADO!"]
  ↓
[... resto do código ...]
```

---

## 🧪 **TESTE:**

### **TESTE 1: Verificar se `AddDelegate` Está Sendo Chamado**

**Execute o jogo e conecte:**

**Você DEVE ver no log:**
```
[OnWSConnected] EVENTO DISPARADO!
🔴 [OnWSConnected] Delegate OnClosed conectado com sucesso!
```

**Se NÃO ver o segundo log, o `AddDelegate` não está sendo chamado!**

### **TESTE 2: Verificar se `OnWSClosed` Está Sendo Disparado**

**Feche o WebSocket (via F9 ou fechando o jogo):**

**Você DEVE ver no log:**
```
[DEBUG] OnWSClosed Custom Event DISPARADO!
[OnWSClosed] EVENTO DISPARADO!
```

**Se NÃO ver nenhum log, o delegate não está sendo disparado!**

---

## 🔍 **INTERPRETAÇÃO DOS RESULTADOS:**

### **CENÁRIO 1: Log `"[OnWSConnected] Delegate OnClosed conectado!"` NÃO aparece**

**Problema:** `AddDelegate` não está sendo chamado.

**Solução:**
- Verifique se o `AddDelegate` está conectado ao fluxo de execução de `OnWSConnected`
- Verifique se `WebSocketRef` é válido quando `AddDelegate` é chamado

### **CENÁRIO 2: Log `"[OnWSConnected] Delegate OnClosed conectado!"` aparece, mas `"[DEBUG] OnWSClosed Custom Event DISPARADO!"` NÃO aparece**

**Problema:** O delegate está conectado, mas não está sendo disparado.

**Possíveis causas:**
- O WebSocket não está sendo fechado explicitamente (chamando `Close()`)
- O delegate está sendo desconectado antes do WebSocket fechar
- O `OnWSClosed` custom event não está configurado corretamente

**Solução:**
- Teste fechando o WebSocket manualmente (F9) para verificar se `OnWSClosed` dispara
- Verifique se `OnWSClosed` é uma **Custom Event** (não uma função normal)

### **CENÁRIO 3: Ambos os logs aparecem**

**Problema:** O delegate está funcionando, mas `RemoveRemoteActor` não está sendo executado.

**Solução:**
- Verifique as conexões de execução em `OnWSClosed`
- Verifique se `RemoveRemoteActor` está sendo chamado corretamente

---

## 📋 **CHECKLIST:**

- [ ] `AddDelegate` para `OnClosed` existe em `OnWSConnected`?
- [ ] `AddDelegate.execute` está conectado ao fluxo de execução de `OnWSConnected`?
- [ ] `AddDelegate.Target` está conectado a `WebSocketRef`?
- [ ] `AddDelegate.Delegate` está conectado a `OnClosed` (do WebSocket)?
- [ ] `AddDelegate.Function` está conectado a `OnWSClosed` (custom event)?
- [ ] Log `"[OnWSConnected] Delegate OnClosed conectado!"` foi adicionado?
- [ ] Log `"[DEBUG] OnWSClosed Custom Event DISPARADO!"` foi adicionado no início de `OnWSClosed`?
- [ ] `OnWSClosed` é uma **Custom Event** (não uma função normal)?

---

## 🎯 **RESUMO:**

**Se `OnWSConnected` e `OnWSBinaryMessage` funcionam, mas `OnWSClosed` não:**

1. ✅ Verifique se `AddDelegate` para `OnClosed` está sendo chamado em `OnWSConnected`
2. ✅ Adicione logs para confirmar que o delegate está sendo conectado
3. ✅ Adicione logs no início do `OnWSClosed` para confirmar que está sendo disparado
4. ✅ Teste fechando o WebSocket manualmente (F9) para verificar se `OnWSClosed` dispara

**Com esses passos, você identificará exatamente onde está o problema!**

