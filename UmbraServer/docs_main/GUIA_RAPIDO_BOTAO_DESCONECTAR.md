# 🔧 **GUIA RÁPIDO: Botão para Desconectar WebSocket (Teste)**

## 🎯 **OBJETIVO:**

Adicionar um botão simples na tela que desconecta o WebSocket, permitindo testar se os actors remotos são destruídos.

---

## ⚡ **SOLUÇÃO MAIS RÁPIDA: Widget HUD Simples**

### **PASSO 1: Criar Widget**

1. **Content Browser** → Botão Direito → **User Interface** → **Widget Blueprint**
2. **Nome:** `WBP_DisconnectButton`

### **PASSO 2: Designer (Layout)**

```
Canvas Panel (Root)
  └─ Button: "BTN_Disconnect"
      └─ Text: "🔴 DESCONECTAR"
      └─ Position: Top-Right (X=1600, Y=50)
      └─ Size: Width=200, Height=50
      └─ Background: Vermelho (#e74c3c)
```

**Configurar Button:**
- **Is Variable:** ✓ TRUE
- **Variable Name:** `BTN_Disconnect`

### **PASSO 3: Event Graph - Lógica do Botão**

**Adicione no Event Graph:**

```
[BTN_Disconnect] → OnClicked
  ↓
[Get All Actors of Class: BP_NetMovementClient]
  ↓
[Get Array Length]
  ↓
[Branch: Length > 0?]
  ├─ True:
  │   ├─ [Get Array Item] (Index: 0)
  │   ├─ [Cast to BP_NetMovementClient]
  │   ├─ [Get Variable: WebSocketRef] (do cast)
  │   ├─ [Branch: Is Valid (WebSocketRef)?]
  │   │   ├─ True:
  │   │   │   ├─ [Print String: "🔴 Desconectando WebSocket..."]
  │   │   │   ├─ [Call Function: Close] (Target: WebSocketRef)
  │   │   │   └─ [Print String: "✅ WebSocket fechado!"]
  │   │   └─ False:
  │   │       └─ [Print String: "⚠️ WebSocketRef inválido!"]
  │   └─ False:
  │       └─ [Print String: "⚠️ BP_NetMovementClient não encontrado!"]
```

**DETALHES:**
- **Get All Actors of Class:** Procure por `Get All Actors of Class`, classe: `BP_NetMovementClient`
- **Get Array Item:** Conecte o array retornado, índice: `0`
- **Cast to BP_NetMovementClient:** Conecte o `Array Item` ao `Object` do Cast
- **Get Variable:** No `BP_NetMovementClient`, procure pela variável `WebSocketRef`
- **Close:** No `WebSocketRef`, procure pela função `Close` (deve estar em `UmbraWSClient`)

### **PASSO 4: Adicionar Widget ao Viewport**

**No `BP_NetMovementClient`, Event Graph, `BeginPlay`:**

```
[Event BeginPlay]
  ↓
[... código existente ...]
  ↓
[Delay: 1.0] (após conexão WebSocket)
  ↓
[Create Widget: WBP_DisconnectButton]
  ↓
[Add to Viewport]
  ↓
[Set Viewport Mode: Viewport]
```

**OU no `BP_ThirdPersonCharacter`, Event Graph, `BeginPlay`:**

```
[Event BeginPlay]
  ↓
[Delay: 2.0] (aguardar inicialização completa)
  ↓
[Create Widget: WBP_DisconnectButton]
  ↓
[Add to Viewport]
```

---

## 🧪 **TESTE RÁPIDO:**

1. **Compile e execute o projeto**
2. **Inicie 2 clientes PIE**
3. **No Cliente 1:**
   - Clique no botão "🔴 DESCONECTAR" (canto superior direito)
   - Verifique os logs: `"🔴 Desconectando WebSocket..."` e `"✅ WebSocket fechado!"`
4. **No Cliente 2:**
   - O actor remoto do Cliente 1 deve desaparecer
   - Verifique os logs: `"PlayerDisconnected processado"`

---

## 🔍 **SE O BOTÃO NÃO APARECER:**

1. **Verifique se o Widget foi criado:**
   - Adicione `Print String: "Widget criado!"` após `Create Widget`
   - Verifique se o log aparece

2. **Verifique Z-Order:**
   - No Designer do Widget, selecione o Canvas Panel
   - Aumente o `Z-Order` para um valor alto (ex: 1000)

3. **Verifique se está no Viewport:**
   - Adicione `Print String: "Widget adicionado ao viewport!"` após `Add to Viewport`
   - Verifique se o log aparece

---

## 🔍 **SE O WEBSOCKET NÃO DESCONECTAR:**

1. **Verifique se `WebSocketRef` é válido:**
   - Adicione `Is Valid` check antes de `Close()`
   - Se inválido, verifique se o WebSocket foi criado corretamente no `OnWSConnected`

2. **Verifique se `Close()` está disponível:**
   - Certifique-se de que `WebSocketRef` é do tipo `UmbraWSClient`
   - A função `Close()` deve estar visível no Blueprint

3. **Verifique logs do servidor:**
   - O servidor C++ deve detectar a desconexão
   - Deve aparecer: `"Client X disconnected"` e `"Broadcasted PlayerDisconnected message"`

---

## ✅ **CHECKLIST MÍNIMO:**

- [ ] Widget `WBP_DisconnectButton` criado
- [ ] Botão `BTN_Disconnect` configurado
- [ ] Event Graph: `OnClicked` implementado
- [ ] Widget adicionado ao Viewport (no `BeginPlay`)
- [ ] Testado: Botão aparece no jogo
- [ ] Testado: Clicar desconecta o WebSocket
- [ ] Testado: Actors remotos são destruídos em outros clientes

---

## 📝 **NOTAS:**

- **Este botão é apenas para TESTE.** Em produção, remova ou esconda o botão.
- **O `OnWSClosed` deve estar conectado ao `CleanupRemoteActors`** no `BP_NetMovementClient`.
- **O servidor deve estar enviando `PlayerDisconnected`** quando um cliente desconecta.

---

**✅ Guia rápido para adicionar botão de desconexão!**


