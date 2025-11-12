# 🔧 **GUIA DETALHADO: Como Usar Get Array Item no Blueprint**

## 🎯 **PROBLEMA:**

Você precisa obter o primeiro elemento do array retornado por `GetAllActorsOfClass` para definir `NetMovementClientRef`, mas não sabe como fazer isso no Blueprint.

---

## 📋 **PASSO A PASSO: Event Construct**

### **PASSO 1: Localizar o Nó Correto**

O nó que você precisa se chama **"Get"** ou **"Array Get"** ou **"Get Array Item"**. No Unreal Engine, ele pode aparecer com nomes diferentes dependendo da versão.

**Como encontrar:**

1. **No Event Graph**, clique com botão direito em uma área vazia
2. **Digite na busca:** `get array` ou `array get` ou `get item`
3. **Procure por um destes nós:**
   - **"Get"** (com um ícone de array)
   - **"Array Get"**
   - **"Get Array Item"**
   - **"Array Get Item"**

**IMPORTANTE:** Este nó **NÃO possui pin de execução** (não precisa de conexão de execução). É um nó "puro" (pure function) que apenas retorna um valor.

---

### **PASSO 2: Conectar o Array**

**Após encontrar o nó "Get" ou "Array Get":**

1. **Conecte o `OutActors` do `GetAllActorsOfClass` ao pin de entrada do "Get":**
   - **Array:** Conecte o `OutActors` (array de `BP_NetMovementClient`) do `GetAllActorsOfClass`
   - **Index:** Defina como `0` (primeiro elemento do array)

**Visualização:**
```
[GetAllActorsOfClass]
  └─ OutActors (Array) ──→ [Get Array Item] ──→ (Output: primeiro elemento)
                              ↑
                           Index: 0
```

---

### **PASSO 3: Conectar ao Set Variable**

**Após obter o elemento do array:**

1. **Conecte o Output do "Get Array Item" ao Input do "Set Variable: NetMovementClientRef":**
   - **Output do "Get Array Item"** → **Input do "Set Variable: NetMovementClientRef"**

**Estrutura completa:**
```
[Event Construct]
  ↓
[GetAllActorsOfClass: BP_NetMovementClient]
  └─ OutActors (Array)
      ↓
[Array_Length]
  └─ ReturnValue (int)
      ↓
[Greater: Length > 0?]
  ├─ True:
  │   ├─ [Get Array Item]
  │   │   ├─ Array: OutActors (conectado)
  │   │   └─ Index: 0
  │   │   └─ Output: (primeiro elemento do array)
  │   │       ↓
  │   └─ [Set Variable: NetMovementClientRef] ← Conectar aqui
  └─ False:
      └─ [Print String: "BP_NetMovementClient não encontrado!"]
```

---

## 🔍 **ALTERNATIVA: Se Não Encontrar "Get Array Item"**

### **Método Alternativo: Usar "ForEachLoop" (Mais Complexo)**

Se você não encontrar o nó "Get Array Item", pode usar um `ForEachLoop` que executa apenas uma vez:

```
[GetAllActorsOfClass]
  └─ OutActors
      ↓
[Array_Length]
  └─ ReturnValue
      ↓
[Greater: Length > 0?]
  ├─ True:
  │   ├─ [ForEachLoop]
  │   │   ├─ Array: OutActors
  │   │   ├─ Loop Body:
  │   │   │   ├─ [Set Variable: NetMovementClientRef] ← Conectar Array Element aqui
  │   │   │   └─ [Break] ← Parar após primeiro elemento
  │   │   └─ Completed
  └─ False:
      └─ [Print String: "BP_NetMovementClient não encontrado!"]
```

**NOTA:** Este método é mais complexo e não recomendado. É melhor encontrar o "Get Array Item".

---

## 🔍 **COMO ENCONTRAR O NÓ NO BLUEPRINT:**

### **Método 1: Busca Direta**

1. **Clique com botão direito** no Event Graph
2. **Digite:** `get`
3. **Procure por:**
   - **"Get"** (com ícone de array)
   - **"Array Get"**
   - **"Get Array Element"**

### **Método 2: Categoria Array**

1. **Clique com botão direito** no Event Graph
2. **Navegue até:** **Array** → **Get**
3. **Ou:** **Utilities** → **Array** → **Get**

### **Método 3: Arrastar do Array**

1. **No `GetAllActorsOfClass`, arraste o pin `OutActors`**
2. **Solte no Event Graph**
3. **No menu de contexto, procure por:** **"Get"** ou **"Get Element"**

---

## 📝 **ESTRUTURA VISUAL COMPLETA:**

### **Event Construct - Estrutura Correta:**

```
┌─────────────────────────────────────────┐
│ Event Construct                         │
└─────────────────────────────────────────┘
              ↓
┌─────────────────────────────────────────┐
│ GetAllActorsOfClass                     │
│   ActorClass: BP_NetMovementClient      │
│   OutActors: [Array] ───────────────┐  │
└──────────────────────────────────────┼──┘
                                       │
              ┌────────────────────────┘
              ↓
┌─────────────────────────────────────────┐
│ Array Length                            │
│   TargetArray: OutActors (conectado)    │
│   ReturnValue: (int) ───────────────┐   │
└─────────────────────────────────────┼───┘
                                       │
              ┌────────────────────────┘
              ↓
┌─────────────────────────────────────────┐
│ Greater (A > B)                         │
│   A: Array Length ReturnValue           │
│   B: 0                                  │
│   ReturnValue: (bool) ──────────────┐   │
└─────────────────────────────────────┼───┘
                                       │
        ┌──────────────────────────────┴──────────────┐
        │                                             │
    True (then)                                  False (else)
        │                                             │
        ↓                                             ↓
┌───────────────────────────┐        ┌───────────────────────────┐
│ Get Array Item            │        │ Print String              │
│   Array: OutActors ───────┼───┐    │   "BP_NetMovementClient   │
│   Index: 0                │   │    │    não encontrado!"      │
│   ReturnValue: (Object) ──┼───┼───┐└───────────────────────────┘
└───────────────────────────┘   │   │
                                 │   │
                                 │   │
                    ┌────────────┘   │
                    │                │
                    ↓                │
        ┌───────────────────────────┐│
        │ Set Variable              ││
        │   NetMovementClientRef    ││
        │   Value: Get Array Item   ││
        │        ReturnValue ───────┘│
        └───────────────────────────┘
```

---

## ✅ **CHECKLIST PASSO A PASSO:**

### **Event Construct:**

1. **Encontrar o nó "Get Array Item":**
   - [ ] Clique com botão direito no Event Graph
   - [ ] Digite `get array` ou `array get`
   - [ ] Selecione o nó "Get" ou "Array Get" ou "Get Array Item"

2. **Conectar o Array:**
   - [ ] Conecte o `OutActors` do `GetAllActorsOfClass` ao pin `Array` do "Get Array Item"
   - [ ] Defina o `Index` como `0`

3. **Conectar ao Set Variable:**
   - [ ] Conecte o `ReturnValue` (ou `Output`) do "Get Array Item" ao pin de entrada do `Set Variable: NetMovementClientRef`

4. **Verificar conexões:**
   - [ ] O `Get Array Item` está entre `GetAllActorsOfClass` e `Set Variable: NetMovementClientRef`
   - [ ] O `Index` está definido como `0`
   - [ ] O `ReturnValue` do `Get Array Item` está conectado ao `Set Variable`

---

## 🔍 **SE AINDA NÃO ENCONTRAR O NÓ:**

### **Verificação 1: Versão do Unreal Engine**

Algumas versões do Unreal Engine podem ter nomes diferentes. Tente:

- **Unreal Engine 4:** `Get` ou `Array Get`
- **Unreal Engine 5:** `Get Array Item` ou `Array Get Item`

### **Verificação 2: Categoria Correta**

O nó pode estar em:
- **Array** → **Get**
- **Utilities** → **Array** → **Get**
- **Array Functions** → **Get**

### **Verificação 3: Tipo do Array**

Certifique-se de que o array é do tipo correto:
- O `OutActors` deve ser um array de `BP_NetMovementClient`
- Se o tipo estiver errado, o nó pode não aparecer

---

## 📝 **EXEMPLO PRÁTICO COM SCREENSHOT (Descrição):**

**No Event Graph, você deve ter:**

1. **`Event Construct`** (nó de evento)
2. **`GetAllActorsOfClass`** (nó de função)
   - **OutActors:** Array de `BP_NetMovementClient`
3. **`Array Length`** (nó de função)
   - **TargetArray:** Conectado ao `OutActors`
4. **`Greater`** (nó de operador)
   - **A:** Conectado ao `ReturnValue` do `Array Length`
   - **B:** `0`
5. **`Branch`** ou **`IfThenElse`** (nó de condição)
   - **Condition:** Conectado ao `ReturnValue` do `Greater`
6. **`Get Array Item`** (nó de função) ← **ADICIONAR AQUI**
   - **Array:** Conectado ao `OutActors` do `GetAllActorsOfClass`
   - **Index:** `0`
   - **ReturnValue:** Conectado ao `Set Variable: NetMovementClientRef`
7. **`Set Variable: NetMovementClientRef`** (nó de variável)
   - **Value:** Conectado ao `ReturnValue` do `Get Array Item`

---

## 🎯 **RESUMO RÁPIDO:**

1. **Encontre o nó:** Clique direito → Digite `get array` → Selecione "Get" ou "Array Get"
2. **Conecte o array:** `OutActors` do `GetAllActorsOfClass` → `Array` do "Get Array Item"
3. **Defina o índice:** `Index` = `0`
4. **Conecte ao Set Variable:** `ReturnValue` do "Get Array Item" → `Value` do `Set Variable: NetMovementClientRef`

**O nó "Get Array Item" NÃO precisa de conexão de execução!** Ele é um nó puro que apenas retorna um valor.

---

**✅ Guia detalhado sobre como usar Get Array Item!**

