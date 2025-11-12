# 🔧 **GUIA: Como Acessar Variável de Outro Objeto no Blueprint**

## 🎯 **PROBLEMA:**

Você precisa acessar a variável `WebSocketRef` do `BP_NetMovementClient` através da variável `NetMovementClientRef` do widget.

---

## ✅ **SOLUÇÃO: Acessar Variável Através da Referência**

### **PASSO 1: Obter a Referência do Objeto**

**No Event Graph do `WBP_TestDisconnect`:**

1. **Localize a variável `NetMovementClientRef`** no painel **My Blueprint** (lado esquerdo)
2. **Arraste a variável `NetMovementClientRef`** para o Event Graph
3. **Solte** → Isso criará um nó **"Get NetMovementClientRef"**

**Este nó retorna a referência do `BP_NetMovementClient`.**

---

### **PASSO 2: Acessar Variável do Objeto**

**A partir do nó "Get NetMovementClientRef":**

1. **Clique com botão direito** no nó "Get NetMovementClientRef"
2. **No menu de contexto, procure por:** **"Get WebSocketRef"** ou **"WebSocketRef"**
3. **Selecione** → Isso criará um nó que acessa a variável `WebSocketRef` do `BP_NetMovementClient`

**OU:**

1. **Arraste o pin de saída** do "Get NetMovementClientRef"
2. **Solte no Event Graph**
3. **No menu, procure por:** **"Get WebSocketRef"** ou **"WebSocketRef"**
4. **Selecione** → Nó é criado

---

### **PASSO 3: Usar a Variável**

**Agora você tem acesso ao `WebSocketRef`:**

1. **Conecte o `WebSocketRef`** obtido ao `Is Valid`
2. **Se válido, conecte ao `Close()`**

---

## 📋 **ESTRUTURA COMPLETA DO ONCLICKED:**

```
[BTN_Disconnect] → OnClicked
  ↓
[Is Valid: NetMovementClientRef?]
  ├─ True:
  │   ├─ [Get NetMovementClientRef] ← Obter referência do widget
  │   │   └─ (Output: BP_NetMovementClient)
  │   │       ↓
  │   ├─ [Get WebSocketRef] ← Acessar variável do BP_NetMovementClient
  │   │   └─ (Output: UmbraWSClient)
  │   │       ↓
  │   ├─ [Is Valid: WebSocketRef?]
  │   │   ├─ True:
  │   │   │   ├─ [Print String: "Desconectando WebSocket..."]
  │   │   │   ├─ [Call Function: Close] (Target: WebSocketRef)
  │   │   │   └─ [Print String: "WebSocket fechado!"]
  │   │   └─ False:
  │   │       └─ [Print String: "WebSocketRef inválido!"]
  │   └─ False:
  │       └─ [Print String: "NetMovementClientRef inválido!"]
```

---

## 🔍 **MÉTODO ALTERNATIVO: Usar "Cast"**

Se o método acima não funcionar, use `Cast`:

### **PASSO 1: Adicionar Cast**

1. **Após `Is Valid: NetMovementClientRef?`**, adicione:
   - **`Cast to BP_NetMovementClient`**
   - **Object:** Conecte ao `NetMovementClientRef` (variável do widget)

### **PASSO 2: Acessar Variável Após Cast**

1. **A partir do pin de saída do Cast** (geralmente "As BP Net Movement Client")
2. **Arraste o pin** para o Event Graph
3. **No menu, procure por:** **"Get WebSocketRef"**
4. **Selecione** → Nó é criado

**Estrutura:**
```
[Is Valid: NetMovementClientRef?]
  ├─ True:
  │   ├─ [Cast to BP_NetMovementClient]
  │   │   ├─ Object: NetMovementClientRef
  │   │   └─ As BP Net Movement Client (Output)
  │   │       ↓
  │   ├─ [Get WebSocketRef] ← Acessar variável
  │   │   └─ (Output: UmbraWSClient)
  │   │       ↓
  │   └─ [Is Valid: WebSocketRef?]
  │       └─ ...
```

---

## 📝 **DIFERENÇA ENTRE "Get (a copy)" E "Get (ref)":**

### **"Get (a copy)":**
- Retorna uma **cópia** do valor
- Usado para tipos de valor (int, float, string, structs)
- Mudanças na cópia não afetam o original

### **"Get (ref)":**
- Retorna uma **referência** ao valor
- Usado para objetos (Object Reference)
- Mudanças na referência afetam o original

**Para `WebSocketRef` (que é um Object Reference):**
- Use **"Get (ref)"** ou apenas **"Get"**
- Você precisa da referência ao objeto, não de uma cópia

---

## 🎯 **MÉTODO MAIS SIMPLES (Passo a Passo):**

### **No Event Graph do `WBP_TestDisconnect`:**

1. **No `OnClicked` do botão:**
   - Adicione `Is Valid: NetMovementClientRef?`

2. **Se válido (True):**
   - **Arraste a variável `NetMovementClientRef`** do painel **My Blueprint** para o Event Graph
   - Isso cria o nó **"Get NetMovementClientRef"**

3. **A partir do "Get NetMovementClientRef":**
   - **Arraste o pin de saída** (geralmente mostra o tipo `BP_NetMovementClient`)
   - **Solte no Event Graph**
   - **No menu, digite:** `websocket` ou `websocketref`
   - **Selecione:** **"Get WebSocketRef"** ou **"WebSocketRef"**
   - Isso cria um nó que acessa a variável `WebSocketRef`

4. **Use o `WebSocketRef` obtido:**
   - Conecte ao `Is Valid: WebSocketRef?`
   - Se válido, conecte ao `Close()`

---

## ⚠️ **IMPORTANTE:**

- **`WebSocketRef` é uma variável do `BP_NetMovementClient`**, não do widget
- **Você precisa da referência do `BP_NetMovementClient` primeiro** (`NetMovementClientRef`)
- **A partir dessa referência**, você pode acessar as variáveis do objeto
- **Use "Get (ref)"** para objetos, não "Get (a copy)"

---

## 🔍 **SE NÃO CONSEGUIR ACESSAR:**

### **Verificação 1: Variável é Pública?**

Certifique-se de que `WebSocketRef` é uma variável **pública** no `BP_NetMovementClient`:
- **No `BP_NetMovementClient`**, abra a variável `WebSocketRef`
- **Verifique:** `Instance Editable` deve estar marcado (ou a variável deve ser pública)

### **Verificação 2: Tipo da Variável**

Certifique-se de que `NetMovementClientRef` é do tipo `BP_NetMovementClient`:
- **No widget**, verifique o tipo da variável `NetMovementClientRef`
- Deve ser: `BP Net Movement Client` (Object Reference)

### **Verificação 3: Cast Necessário?**

Se o acesso direto não funcionar, use `Cast to BP_NetMovementClient`:
- Isso garante que o tipo está correto
- Permite acessar todas as variáveis públicas do `BP_NetMovementClient`

---

## 📝 **RESUMO ULTRA-RÁPIDO:**

1. **Arraste `NetMovementClientRef`** do painel My Blueprint para o Event Graph
2. **Arraste o pin de saída** do "Get NetMovementClientRef"
3. **No menu, selecione "Get WebSocketRef"**
4. **Use o `WebSocketRef`** obtido no `Is Valid` e `Close()`

**Pronto!** ✅

---

**✅ Guia completo para acessar variável de outro objeto!**

