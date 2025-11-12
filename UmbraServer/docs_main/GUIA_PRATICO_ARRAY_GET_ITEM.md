# 🔧 **GUIA PRÁTICO: Como Obter Primeiro Item do Array no Blueprint**

## 🎯 **PROBLEMA:**

Você precisa obter o primeiro elemento do array `OutActors` retornado por `GetAllActorsOfClass` para definir `NetMovementClientRef`.

---

## ✅ **SOLUÇÃO 1: Usar "Get" (Recomendado)**

### **PASSO 1: Arrastar o Array para o Event Graph**

1. **No Event Graph**, localize o nó `GetAllActorsOfClass`
2. **Encontre o pin `OutActors`** (saída do array)
3. **Arraste o pin `OutActors`** para uma área vazia do Event Graph
4. **Solte o pin** → Um menu de contexto aparecerá

### **PASSO 2: Selecionar "Get" no Menu**

**No menu de contexto que aparece, procure por:**

- **"Get"** (primeira opção geralmente)
- **"Get Element"**
- **"Array Get"**
- **"Get Array Item"**

**Selecione "Get"** → Isso criará um nó que permite acessar elementos do array.

### **PASSO 3: Configurar o Nó "Get"**

**O nó "Get" criado terá:**

- **Array:** Já conectado ao `OutActors` (automaticamente)
- **Index:** Defina como `0` (primeiro elemento)
- **ReturnValue:** Este é o elemento que você precisa

### **PASSO 4: Conectar ao Set Variable**

1. **Arraste o pin `ReturnValue`** do nó "Get"
2. **Conecte ao pin de entrada** do `Set Variable: NetMovementClientRef`

**Pronto!** Agora o `NetMovementClientRef` receberá o primeiro elemento do array.

---

## ✅ **SOLUÇÃO 2: Usar "Break" (Alternativa)**

Se você não encontrar "Get", pode usar "Break" para acessar o primeiro elemento:

### **PASSO 1: Adicionar "Break"**

1. **Clique com botão direito** no Event Graph
2. **Digite:** `break array`
3. **Selecione:** **"Break Array"** ou **"Array Break"**

### **PASSO 2: Conectar o Array**

1. **Conecte o `OutActors`** do `GetAllActorsOfClass` ao pin `Array` do "Break Array"
2. **Defina `Index` como `0`**

### **PASSO 3: Usar o Element**

O "Break Array" retornará o elemento no índice especificado. Conecte esse elemento ao `Set Variable: NetMovementClientRef`.

---

## ✅ **SOLUÇÃO 3: Usar "ForEachLoop" (Última Alternativa)**

Se nenhuma das soluções acima funcionar, use `ForEachLoop`:

### **PASSO 1: Adicionar ForEachLoop**

1. **Clique com botão direito** no Event Graph
2. **Digite:** `foreach`
3. **Selecione:** **"ForEachLoop"** ou **"For Each Loop"**

### **PASSO 2: Configurar o Loop**

1. **Conecte o `OutActors`** do `GetAllActorsOfClass` ao pin `Array` do `ForEachLoop`
2. **No `Loop Body` (corpo do loop):**
   - **Conecte `Array Element`** ao `Set Variable: NetMovementClientRef`
   - **Adicione `Break`** logo após o `Set Variable` para parar após o primeiro elemento

**Estrutura:**
```
[ForEachLoop]
  ├─ Array: OutActors
  ├─ Loop Body:
  │   ├─ [Set Variable: NetMovementClientRef] ← Conectar Array Element aqui
  │   └─ [Break] ← Parar após primeiro elemento
  └─ Completed
```

---

## 🔍 **COMO IDENTIFICAR O NÓ CORRETO:**

### **Características do Nó "Get" ou "Get Array Item":**

- **Não possui pin de execução** (não precisa de conexão de execução)
- **Possui pin `Array`** (entrada do array)
- **Possui pin `Index`** (índice do elemento, geralmente um int)
- **Possui pin `ReturnValue` ou `Output`** (saída do elemento)

### **Visual do Nó:**

```
┌─────────────────────┐
│   Get Array Item    │
│                     │
│  Array ──────────── │ ← Conecte OutActors aqui
│  Index: 0           │ ← Defina como 0
│                     │
│  ReturnValue ────── │ ← Conecte ao Set Variable aqui
└─────────────────────┘
```

---

## 📋 **ESTRUTURA COMPLETA DO EVENT CONSTRUCT:**

```
[Event Construct]
  ↓
[GetAllActorsOfClass: BP_NetMovementClient]
  └─ OutActors (Array)
      ↓
[Array Length]
  └─ ReturnValue (int)
      ↓
[Greater: Length > 0?]
  ├─ True:
  │   ├─ [Get Array Item] ← ADICIONAR AQUI
  │   │   ├─ Array: OutActors (conectado)
  │   │   ├─ Index: 0
  │   │   └─ ReturnValue: (primeiro elemento)
  │   │       ↓
  │   └─ [Set Variable: NetMovementClientRef] ← Conectar ReturnValue aqui
  └─ False:
      └─ [Print String: "BP_NetMovementClient não encontrado!"]
```

---

## 🎯 **MÉTODO MAIS SIMPLES (Arrastar e Soltar):**

### **Passo a Passo Visual:**

1. **Localize o `GetAllActorsOfClass`** no Event Graph
2. **Encontre o pin `OutActors`** (geralmente na parte inferior do nó)
3. **Clique e arraste** o pin `OutActors` para uma área vazia
4. **Solte o mouse** → Menu de contexto aparece
5. **No menu, procure por "Get"** ou "Get Element"
6. **Clique em "Get"** → Nó é criado automaticamente
7. **O array já estará conectado!**
8. **Defina `Index` como `0`**
9. **Arraste o `ReturnValue`** do "Get" e conecte ao `Set Variable: NetMovementClientRef`

---

## ⚠️ **IMPORTANTE:**

- **O nó "Get" NÃO precisa de conexão de execução!** Ele é um nó "puro" que apenas retorna um valor.
- **Você pode colocá-lo em qualquer lugar** do Event Graph, desde que esteja conectado ao array e ao `Set Variable`.
- **O `Index` deve ser `0`** para obter o primeiro elemento (arrays começam em 0).

---

## 🔍 **SE AINDA NÃO FUNCIONAR:**

### **Verificação 1: Tipo do Array**

Certifique-se de que o `OutActors` é um array de `BP_NetMovementClient`. Se o tipo estiver errado, o nó pode não aparecer.

### **Verificação 2: Versão do Unreal Engine**

- **UE 4:** O nó pode se chamar "Get" ou "Array Get"
- **UE 5:** O nó pode se chamar "Get Array Item" ou "Array Get Item"

### **Verificação 3: Categoria**

Tente procurar em:
- **Array** → **Get**
- **Utilities** → **Array** → **Get**
- **Array Functions** → **Get**

---

## 📝 **RESUMO ULTRA-RÁPIDO:**

1. **Arraste** o pin `OutActors` do `GetAllActorsOfClass` para o Event Graph
2. **Solte** → Menu aparece
3. **Selecione "Get"** ou "Get Element"
4. **Defina `Index` como `0`**
5. **Conecte `ReturnValue`** ao `Set Variable: NetMovementClientRef`

**Pronto!** ✅

---

**✅ Guia prático para obter primeiro item do array!**

