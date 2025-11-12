# ⚡ **GUIA RÁPIDO: Botão Desconectar e Voltar ao Login**

## 🎯 **OBJETIVO:**

Adicionar um botão que:
1. Chama `Close()` no WebSocket
2. Aguarda 1 segundo
3. Retorna para `WBP_Login2`

---

## 📋 **SOLUÇÃO COMPLETA:**

### **OPÇÃO 1: Adicionar Botão no Widget Existente (Recomendado)**

**Se você já tem um widget (ex: `WBP_TestDisconnect` ou outro widget de HUD):**

#### **PASSO 1: Adicionar Botão no Designer**

1. **Abra o widget** (ex: `WBP_TestDisconnect`)
2. **Vá para a aba Designer**
3. **Adicione um Button:**
   - Arraste **Button** do Palette para o Canvas
   - Nome: `BTN_Disconnect`
   - Position: X=100, Y=100 (ou onde preferir)
   - Size: Width=200, Height=50
   - Text: "DESCONECTAR"
   - Visibility: `Visible`

---

#### **PASSO 2: Adicionar Variável no Widget**

1. **No painel My Blueprint**, adicione:
   - **NetMovementClientRef**
     - Type: `BP Net Movement Client` (Object Reference)
     - Instance Editable: `False`

---

#### **PASSO 3: Implementar Lógica no Event Graph**

**No Event Graph do widget, adicione:**

```
[BTN_Disconnect] → OnClicked
  ↓
[Print String: "🔴 Desconectando WebSocket..."]
  ↓
[Get Variable: NetMovementClientRef]
  ↓
[Is Valid: NetMovementClientRef?]
  ├─ True:
  │   ├─ [Get Variable: WebSocketRef] (do NetMovementClientRef)
  │   │   └─ Target: [NetMovementClientRef]
  │   ├─ [Is Valid: WebSocketRef?]
  │   │   ├─ True:
  │   │   │   ├─ [Call Function: Close] (do WebSocketRef)
  │   │   │   │   └─ Target: [WebSocketRef]
  │   │   │   ├─ [Print String: "✅ WebSocket fechado"]
  │   │   │   ├─ [Delay: 1.0]
  │   │   │   ├─ [Open Level]
  │   │   │   │   └─ Level Name: "Lvl_Login" (ou o nome do seu level de login)
  │   │   │   └─ [Print String: "✅ Retornando ao login"]
  │   │   └─ False:
  │   │       └─ [Print String: "⚠️ WebSocketRef é inválido!"]
  └─ False:
      └─ [Print String: "⚠️ NetMovementClientRef é inválido!"]
```

---

#### **PASSO 4: Passar Referência ao Widget**

**No `BP_ThirdPersonCharacter.CreateDisconnectWidget` (ou onde você cria o widget):**

```
[Create Widget: WBP_TestDisconnect]
  └─ ReturnValue: [Widget]
      ↓
[Call Function: SetNetMovementClient]
  ├─ Target: [ReturnValue]
  └─ NetMovementClient: [Get Variable: NetMovementClientRef]
      ↓
[Add to Viewport]
  └─ Target: [ReturnValue]
```

---

### **OPÇÃO 2: Criar Widget Novo (Se Não Tiver Widget)**

#### **PASSO 1: Criar Widget**

1. **No Content Browser:**
   - Clique direito → **User Interface** → **Widget Blueprint**
   - Nome: `WBP_DisconnectButton`

2. **No Designer:**
   - Adicione um **Button**
   - Nome: `BTN_Disconnect`
   - Position: X=100, Y=100
   - Size: Width=200, Height=50
   - Text: "DESCONECTAR"

---

#### **PASSO 2: Adicionar Variáveis**

**No painel My Blueprint:**

1. **NetMovementClientRef**
   - Type: `BP Net Movement Client` (Object Reference)
   - Instance Editable: `False`

2. **Função SetNetMovementClient:**
   - Painel My Blueprint → Functions → + (Add Function)
   - Nome: `SetNetMovementClient`
   - Input: `NetMovementClient` (BP Net Movement Client, Object Reference)
   - Implementação:
     ```
     [Function Entry: SetNetMovementClient]
       ↓
     [Set Variable: NetMovementClientRef]
       └─ Value: [NetMovementClient (input)]
     ```

---

#### **PASSO 3: Implementar OnClicked**

**No Event Graph:**

```
[BTN_Disconnect] → OnClicked
  ↓
[Get Variable: NetMovementClientRef]
  ↓
[Is Valid: NetMovementClientRef?]
  ├─ True:
  │   ├─ [Get Variable: WebSocketRef] (do NetMovementClientRef)
  │   │   └─ Target: [NetMovementClientRef]
  │   ├─ [Is Valid: WebSocketRef?]
  │   │   ├─ True:
  │   │   │   ├─ [Call Function: Close] (do WebSocketRef)
  │   │   │   │   └─ Target: [WebSocketRef]
  │   │   │   ├─ [Delay: 1.0]
  │   │   │   ├─ [Open Level]
  │   │   │   │   └─ Level Name: "Lvl_Login" (ou seu level de login)
  │   │   │   └─ [Print String: "✅ Retornando ao login"]
  │   │   └─ False:
  │   │       └─ [Print String: "⚠️ WebSocketRef inválido"]
  └─ False:
      └─ [Print String: "⚠️ NetMovementClientRef inválido"]
```

---

#### **PASSO 4: Criar e Adicionar Widget**

**No `BP_ThirdPersonCharacter.CreateDisconnectWidget` (ou `BeginPlay`):**

```
[Get Player Controller] (Index: 0)
  ↓
[Create Widget: WBP_DisconnectButton]
  └─ OwningPlayer: [Get Player Controller]
     ↓
[Call Function: SetNetMovementClient]
  ├─ Target: [ReturnValue]
  └─ NetMovementClient: [Get Variable: NetMovementClientRef]
     ↓
[Add to Viewport]
  ├─ Target: [ReturnValue]
  └─ ZOrder: 9999
```

---

## 🔧 **PASSO A PASSO DETALHADO (OPÇÃO 1 - Widget Existente):**

### **1. Abrir Widget no Designer**

1. **No Content Browser**, encontre `WBP_TestDisconnect` (ou seu widget)
2. **Duplo clique** para abrir
3. **Vá para a aba Designer**

---

### **2. Adicionar Botão (Se Não Tiver)**

1. **No painel Palette** (esquerda), encontre **Button**
2. **Arraste** para o Canvas
3. **Selecione o botão** e configure no painel Details:
   - **Name**: `BTN_Disconnect`
   - **Position X**: `100`
   - **Position Y**: `100`
   - **Size X**: `200`
   - **Size Y**: `50`
   - **Visibility**: `Visible`
   - **Is Enabled**: ✅ Marcado

---

### **3. Adicionar Variável (Se Não Tiver)**

1. **No painel My Blueprint**, clique em **+ Variable**
2. **Configure:**
   - **Variable Name**: `NetMovementClientRef`
   - **Variable Type**: `BP Net Movement Client` (Object Reference)
   - **Instance Editable**: ❌ Desmarcado

---

### **4. Implementar OnClicked no Event Graph**

1. **Vá para a aba Event Graph**
2. **Selecione o botão `BTN_Disconnect`** no Designer
3. **No painel Details**, encontre **Events** → **OnClicked**
4. **Clique no ícone de +** ao lado de **OnClicked**
5. **Isso criará automaticamente** o evento `OnClicked` no Event Graph

---

### **5. Conectar Lógica do OnClicked**

**No Event Graph, conecte os nós nesta ordem:**

1. **OnClicked** (evento do botão)
   - Conecte ao próximo nó

2. **Print String**
   - **In String**: "🔴 Desconectando WebSocket..."
   - Conecte `execute` do OnClicked ao `execute` do Print String

3. **Get Variable: NetMovementClientRef**
   - Conecte `then` do Print String ao próximo nó

4. **Is Valid**
   - **Object**: Conecte `NetMovementClientRef` (do Get Variable)
   - Conecte `then` do Print String ao `execute` do Is Valid

5. **Branch**
   - **Condition**: Conecte `ReturnValue` do Is Valid
   - Conecte `then` do Is Valid ao `execute` do Branch

6. **True (Branch):**
   - **Get Variable: WebSocketRef**
     - **Target**: Conecte `NetMovementClientRef` (do Get Variable anterior)
     - Conecte `then` do Branch ao próximo nó

   - **Is Valid**
     - **Object**: Conecte `WebSocketRef` (do Get Variable)
     - Conecte após o Get Variable

   - **Branch**
     - **Condition**: Conecte `ReturnValue` do Is Valid
     - Conecte `then` do Is Valid ao `execute` do Branch

   - **True (Branch):**
     - **Call Function: Close**
       - **Target**: Conecte `WebSocketRef` (do Get Variable)
       - Conecte `then` do Branch ao `execute` do Close

     - **Delay**
       - **Duration**: `1.0`
       - Conecte `then` do Close ao `execute` do Delay

     - **Open Level**
       - **Level Name**: `"Lvl_Login"` (ou o nome do seu level de login)
       - Conecte `Completed` do Delay ao `execute` do Open Level

     - **Print String**
       - **In String**: "✅ Retornando ao login"
       - Conecte `then` do Open Level ao `execute` do Print String

---

## 📝 **NOME DO LEVEL DE LOGIN:**

**Verifique o nome exato do seu level de login:**

1. **No Content Browser**, encontre o level de login (ex: `Lvl_Login`, `Lvl_Login2`, etc.)
2. **Use o nome exato** no `Open Level` (sem a extensão `.umap`)

**Exemplos:**
- Se o level se chama `Lvl_Login2.umap`, use: `"Lvl_Login2"`
- Se o level se chama `LoginLevel.umap`, use: `"LoginLevel"`

---

## ✅ **RESUMO DO FLUXO:**

```
[Botão Clicado]
  ↓
[Verificar NetMovementClientRef]
  ↓
[Verificar WebSocketRef]
  ↓
[Fechar WebSocket]
  ↓
[Aguardar 1 segundo]
  ↓
[Abrir Level de Login]
```

---

## 🧪 **TESTE:**

1. **Compile o Blueprint**
2. **Execute o jogo**
3. **Clique no botão "DESCONECTAR"**
4. **Verifique os logs:**
   ```
   🔴 Desconectando WebSocket...
   ✅ WebSocket fechado
   ✅ Retornando ao login
   ```
5. **O jogo deve retornar ao level de login após 1 segundo**

---

## ⚠️ **OBSERVAÇÕES:**

- **O `Open Level` pode demorar alguns segundos** para carregar o level
- **Certifique-se de que o nome do level está correto** (sem extensão `.umap`)
- **O WebSocket será fechado antes de mudar de level**, garantindo limpeza correta

---

**✅ Pronto! O botão deve desconectar o WebSocket e retornar ao login após 1 segundo!**

