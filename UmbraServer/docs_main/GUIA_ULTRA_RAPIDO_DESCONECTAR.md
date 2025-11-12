# ⚡ **GUIA ULTRA RÁPIDO: Botão Desconectar e Voltar ao Login**

## 🎯 **OBJETIVO:**

Botão que fecha o WebSocket, aguarda 1 segundo e volta para `WBP_Login2`.

---

## ✅ **SOLUÇÃO EM 5 PASSOS:**

### **PASSO 1: Abrir Widget**

1. Abra `WBP_TestDisconnect` (ou seu widget)
2. Vá para **Event Graph**

---

### **PASSO 2: Conectar OnClicked do Botão**

1. **Selecione o botão `BTN_Disconnect`** no Designer
2. **No painel Details**, encontre **OnClicked** → clique no **+**
3. Isso cria o evento no Event Graph

---

### **PASSO 3: Adicionar Nós (Nesta Ordem)**

**Conecte os nós nesta sequência:**

```
[OnClicked] (do botão)
  ↓
[Get Variable: NetMovementClientRef]
  ↓
[Get Variable: WebSocketRef]
  │   └─ Target: [NetMovementClientRef]
  ↓
[Call Function: Close]
  │   └─ Target: [WebSocketRef]
  ↓
[Delay: 1.0]
  ↓
[Open Level]
  │   └─ Level Name: "Lvl_Login2" (ou seu level de login)
  ↓
[Print String: "✅ Retornando ao login"]
```

---

### **PASSO 4: Configurar Open Level**

1. **Nó `Open Level`:**
   - **Level Name**: Digite `"Lvl_Login2"` (ou o nome do seu level)
   - **⚠️ IMPORTANTE:** Use o nome sem a extensão `.umap`

---

### **PASSO 5: Compilar e Testar**

1. **Compile** o Blueprint
2. **Execute** o jogo
3. **Clique** no botão
4. **Aguarde 1 segundo** → deve voltar ao login

---

## 🔧 **SE NÃO TIVER A VARIÁVEL NetMovementClientRef:**

**Adicione no widget:**

1. **Painel My Blueprint** → **+ Variable**
2. **Nome:** `NetMovementClientRef`
3. **Tipo:** `BP Net Movement Client` (Object Reference)

**E passe a referência quando criar o widget:**

```
[Create Widget: WBP_TestDisconnect]
  ↓
[Call Function: SetNetMovementClient]
  └─ NetMovementClient: [NetMovementClientRef do Character]
```

---

## 📋 **FLUXO COMPLETO (COM VALIDAÇÕES):**

**Se quiser adicionar validações (recomendado):**

```
[OnClicked]
  ↓
[Get Variable: NetMovementClientRef]
  ↓
[Is Valid: NetMovementClientRef?]
  ├─ True:
  │   ├─ [Get Variable: WebSocketRef]
  │   │   └─ Target: [NetMovementClientRef]
  │   ├─ [Is Valid: WebSocketRef?]
  │   │   ├─ True:
  │   │   │   ├─ [Call Function: Close]
  │   │   │   │   └─ Target: [WebSocketRef]
  │   │   │   ├─ [Delay: 1.0]
  │   │   │   ├─ [Open Level]
  │   │   │   │   └─ Level Name: "Lvl_Login2"
  │   │   │   └─ [Print String: "✅ Retornando ao login"]
  │   │   └─ False:
  │   │       └─ [Print String: "⚠️ WebSocketRef inválido"]
  └─ False:
      └─ [Print String: "⚠️ NetMovementClientRef inválido"]
```

---

## ⚡ **VERSÃO MÍNIMA (Sem Validações):**

**Se quiser a versão mais simples possível:**

```
[OnClicked]
  ↓
[Get Variable: NetMovementClientRef]
  ↓
[Get Variable: WebSocketRef]
  │   └─ Target: [NetMovementClientRef]
  ↓
[Call Function: Close]
  │   └─ Target: [WebSocketRef]
  ↓
[Delay: 1.0]
  ↓
[Open Level]
  │   └─ Level Name: "Lvl_Login2"
```

---

## 🎯 **NÓS NECESSÁRIOS:**

1. **OnClicked** (evento do botão - criado automaticamente)
2. **Get Variable: NetMovementClientRef**
3. **Get Variable: WebSocketRef** (do NetMovementClientRef)
4. **Call Function: Close** (do WebSocketRef)
5. **Delay** (Duration: 1.0)
6. **Open Level** (Level Name: "Lvl_Login2")
7. **Print String** (opcional, para debug)

---

## ✅ **PRONTO!**

**Conecte os nós nessa ordem e está funcionando!**

**O botão vai:**
1. ✅ Fechar o WebSocket
2. ✅ Aguardar 1 segundo
3. ✅ Retornar ao level de login

---

**🚀 Tudo pronto em menos de 5 minutos!**

