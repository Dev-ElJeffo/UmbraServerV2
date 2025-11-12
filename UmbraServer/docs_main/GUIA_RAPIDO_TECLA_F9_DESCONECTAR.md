# ⚡ **GUIA RÁPIDO: Tecla F9 para Desconectar WebSocket**

## 🎯 **OBJETIVO:**

Mapear a tecla **F9** para chamar `Close()` no WebSocket diretamente, sem widgets.

---

## ✅ **SOLUÇÃO EM 3 PASSOS:**

### **PASSO 1: Adicionar Input Mapping**

1. **No Editor do Unreal:**
   - Menu **Edit** → **Project Settings**
   - No painel esquerdo: **Engine** → **Input**
   - Seção **Bindings** → **Action Mappings** (ou **Axis Mappings**)

2. **Adicionar Action Mapping:**
   - Clique em **+** ao lado de **Action Mappings**
   - **Action Name**: `DisconnectWebSocket`
   - **Key**: Selecione **F9**

---

### **PASSO 2: Criar Função no Character**

**No `BP_ThirdPersonCharacter`:**

1. **Painel My Blueprint** → **Functions** → **+ (Add Function)**
2. **Nome:** `DisconnectWebSocket`
3. **Compile**

---

### **PASSO 3: Implementar Função e Conectar Input**

**No Event Graph do `BP_ThirdPersonCharacter`:**

#### **3.1: Implementar Função DisconnectWebSocket**

```
[Function Entry: DisconnectWebSocket]
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
  │   │   │   └─ [Print String: "✅ WebSocket fechado via F9"]
  │   │   └─ False:
  │   │       └─ [Print String: "⚠️ WebSocketRef é inválido"]
  └─ False:
      └─ [Print String: "⚠️ NetMovementClientRef é inválido"]
```

---

#### **3.2: Conectar Input ao Evento**

**No Event Graph (não dentro da função):**

1. **Adicione o evento de input:**
   - Clique direito no Event Graph
   - Digite: `DisconnectWebSocket` (o nome do Action Mapping)
   - Selecione **Action DisconnectWebSocket** (aparecerá como evento)

2. **Conecte ao evento:**
   ```
   [Action DisconnectWebSocket] (evento de input)
     ↓
   [Call Function: DisconnectWebSocket]
     └─ Target: [Self]
   ```

---

## 🔧 **PASSO A PASSO DETALHADO:**

### **PASSO 1: Configurar Input Mapping**

1. **Menu Edit** → **Project Settings**
2. **No painel esquerdo:** **Engine** → **Input**
3. **Seção Bindings:**
   - **Action Mappings** → Clique em **+**
   - **Action Name**: Digite `DisconnectWebSocket`
   - **Key**: Clique no dropdown e selecione **F9**
4. **Salve** (Ctrl+S)

---

### **PASSO 2: Criar Função no Character**

1. **Abra `BP_ThirdPersonCharacter`**
2. **Painel My Blueprint** → **Functions** → **+ (Add Function)**
3. **Nome:** `DisconnectWebSocket`
4. **Compile** (botão Compile no topo)

---

### **PASSO 3: Implementar Função**

**No Event Graph da função `DisconnectWebSocket`:**

1. **Adicione os nós nesta ordem:**

   - **Get Variable: NetMovementClientRef**
     - Conecte `then` do Function Entry ao próximo nó

   - **Is Valid**
     - **Object**: Conecte `NetMovementClientRef` (do Get Variable)
     - Conecte após o Get Variable

   - **Branch**
     - **Condition**: Conecte `ReturnValue` do Is Valid
     - Conecte `then` do Is Valid ao `execute` do Branch

   - **True (Branch):**
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

       - **Print String**
         - **In String**: "✅ WebSocket fechado via F9"
         - Conecte `then` do Close ao `execute` do Print String

---

### **PASSO 4: Conectar Input ao Evento**

**No Event Graph principal (não dentro da função):**

1. **Adicione o evento de input:**
   - Clique direito no Event Graph
   - Digite: `DisconnectWebSocket`
   - Selecione **Action DisconnectWebSocket** (aparecerá como evento de input)

2. **Conecte à função:**
   - **Call Function: DisconnectWebSocket**
     - **Target**: Conecte `Self` (ou deixe vazio se for no próprio Character)
     - Conecte o pin de saída do evento **Action DisconnectWebSocket** ao `execute` do Call Function

---

## ⚡ **VERSÃO MÍNIMA (Sem Validações):**

**Se quiser a versão mais simples possível:**

```
[Action DisconnectWebSocket] (evento de input)
  ↓
[Get Variable: NetMovementClientRef]
  ↓
[Get Variable: WebSocketRef]
  │   └─ Target: [NetMovementClientRef]
  ↓
[Call Function: Close]
  │   └─ Target: [WebSocketRef]
  ↓
[Print String: "✅ WebSocket fechado"]
```

**Conecte tudo diretamente no Event Graph, sem criar função separada.**

---

## 🎯 **ESTRUTURA COMPLETA:**

### **Event Graph do BP_ThirdPersonCharacter:**

```
[Action DisconnectWebSocket] (evento de input - criado automaticamente)
  ↓
[Call Function: DisconnectWebSocket]
  └─ Target: [Self]
```

### **Função DisconnectWebSocket:**

```
[Function Entry: DisconnectWebSocket]
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
  │   │   │   └─ [Print String: "✅ WebSocket fechado via F9"]
  │   │   └─ False:
  │   │       └─ [Print String: "⚠️ WebSocketRef inválido"]
  └─ False:
      └─ [Print String: "⚠️ NetMovementClientRef inválido"]
```

---

## 🧪 **TESTE:**

1. **Compile** o Blueprint
2. **Execute** o jogo (PIE - Play In Editor)
3. **Pressione F9**
4. **Verifique os logs:**
   ```
   ✅ WebSocket fechado via F9
   ```
5. **O WebSocket deve ser fechado**

---

## 📋 **CHECKLIST:**

- [ ] Input Mapping `DisconnectWebSocket` criado com tecla F9?
- [ ] Função `DisconnectWebSocket` criada no Character?
- [ ] Função implementada com lógica de fechar WebSocket?
- [ ] Evento `Action DisconnectWebSocket` adicionado no Event Graph?
- [ ] Evento conectado à função `DisconnectWebSocket`?
- [ ] Blueprint compilado?

---

## ⚠️ **OBSERVAÇÕES:**

- **O input só funciona quando o jogo está rodando** (PIE ou Standalone)
- **Certifique-se de que `NetMovementClientRef` está definido** antes de pressionar F9
- **Se não funcionar, verifique se o Input Mapping está salvo** (Ctrl+S no Project Settings)

---

## 🔧 **ALTERNATIVA: Usar Input Action no BP_NetMovementClient**

**Se preferir adicionar no `BP_NetMovementClient` em vez do Character:**

1. **Siga os mesmos passos**, mas:
   - Crie a função no `BP_NetMovementClient`
   - No Event Graph do `BP_NetMovementClient`, adicione o evento `Action DisconnectWebSocket`
   - Na função, use `Self` para acessar `WebSocketRef` diretamente:

```
[Function Entry: DisconnectWebSocket]
  ↓
[Get Variable: WebSocketRef]
  ↓
[Is Valid: WebSocketRef?]
  ├─ True:
  │   ├─ [Call Function: Close]
  │   │   └─ Target: [WebSocketRef]
  │   └─ [Print String: "✅ WebSocket fechado via F9"]
  └─ False:
      └─ [Print String: "⚠️ WebSocketRef inválido"]
```

---

**✅ Pronto! Pressione F9 e o WebSocket será fechado!**

