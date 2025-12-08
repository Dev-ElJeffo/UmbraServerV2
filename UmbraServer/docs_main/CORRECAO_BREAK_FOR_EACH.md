# 🔧 **CORREÇÃO: Usar For Each Loop with Break**

## ❌ **PROBLEMA IDENTIFICADO**

Você já está usando o `For Loop` de 1 a 6 e comparando o ClassID, mas está usando o **`For Each Loop` normal** em vez do **`For Each Loop with Break`**.

**Sem o `Break`:**
- O loop continua mesmo depois de encontrar o placeholder correto
- Pode adicionar múltiplos placeholders se houver duplicatas
- Pode adicionar o placeholder errado se a ordem estiver incorreta

---

## ✅ **SOLUÇÃO: Substituir por For Each Loop with Break**

**No seu `Event Construct`, substitua o `For Each Loop` por `For Each Loop with Break`:**

### **PASSO 1: Remover o For Each Loop Atual**

1. **Selecione o nó `For Each Loop`** que está dentro do `For Loop` (1 a 6)
2. **Delete-o** (Delete ou Del)

### **PASSO 2: Adicionar For Each Loop with Break**

1. **Clique com botão direito** no gráfico
2. **Digite "For Each Loop with Break"** na busca
3. **Selecione "For Each Loop with Break"**

### **PASSO 3: Conectar os Pins**

**Conecte os pins do mesmo jeito que estava antes:**

```
[For Loop] (1 a 6)
    ↓ Loop Body
    [Get All Actors of Class]
        ↓
    [For Each Loop with Break] ← NOVO!
        • Array: (resultado do Get All Actors)
        ↓ Loop Body
        [Get ClassID] (do Array Element)
        ↓
        [Equal] (Integer)
            • A: (ClassID)
            • B: (Loop Index)
            ↓
        [Branch] (True)
            ↓
        [Add to Array] PlaceholderClassIDs
            • New Item: (Loop Index)
            ↓
        [Get Actor Transform]
            • Target: (Array Element)
            ↓
        [Add to Array] PlaceholderTransforms
            • New Item: (Actor Transform)
            ↓
        [Break] ← PIN DO FOR EACH LOOP WITH BREAK!
```

### **PASSO 4: Conectar o Break**

**O `For Each Loop with Break` tem um pin `Break`:**

1. **Após o `Add to Array` do Transform**, conecte o `then` ao pin **`Break`** do `For Each Loop with Break`
2. **O pin `Break` está na parte inferior do nó `For Each Loop with Break`**

---

## 📋 **ESTRUTURA COMPLETA**

```
[For Loop]
    • First Index: 1
    • Last Index: 6
    ↓ Loop Body
    [Get All Actors of Class]
        • Actor Class: BP_Class_Placeholder
        ↓
    [For Each Loop with Break] ← SUBSTITUIR AQUI
        • Array: (Out Actors)
        ↓ Loop Body
        [Get ClassID] (do Array Element)
        ↓
        [Equal] (Integer)
            • A: (ClassID)
            • B: (Index do For Loop)
            ↓
        [Branch] (True)
            ↓
        [Add to Array] PlaceholderClassIDs
            • New Item: (Index do For Loop)
            ↓
        [Get Actor Transform]
            • Target: (Array Element)
            ↓
        [Add to Array] PlaceholderTransforms
            • New Item: (Actor Transform)
            ↓
        [Break] ← CONECTAR AQUI (pin do For Each Loop with Break)
```

---

## ⚠️ **IMPORTANTE**

**O `For Each Loop with Break` tem um pin `Break` na parte inferior do nó.**

**Conecte o `then` do último `Add to Array` (do Transform) ao pin `Break` do `For Each Loop with Break`.**

**Isso faz com que o loop pare imediatamente após encontrar e adicionar o placeholder correto.**

---

## ✅ **VERIFICAÇÃO**

**Após substituir por `For Each Loop with Break`:**

1. ✅ O nó `For Each Loop with Break` está conectado ao array de placeholders
2. ✅ O pin `Break` está conectado ao `then` do último `Add to Array`
3. ✅ O `For Loop` externo continua normalmente

---

## 🎯 **RESULTADO ESPERADO**

**Com o `For Each Loop with Break`:**
- O loop para imediatamente após encontrar o placeholder correto
- Apenas um placeholder é adicionado por ClassID
- A ordem dos arrays é garantida (1, 2, 3, 4, 5, 6)

---

**FIM DA CORREÇÃO**

