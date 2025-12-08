# 🔧 **CORREÇÃO: Break Encontra Placeholder Errado**

## ❌ **PROBLEMA**

O `For Each Loop with Break` está encontrando o placeholder errado quando há múltiplos placeholders. O `Break` para o loop assim que encontra um match, mas pode estar encontrando o placeholder errado primeiro.

**Causa:**
- `Get All Actors of Class` retorna os placeholders em ordem não determinística
- Quando Loop Index = 3, o loop pode encontrar o Dark Mage (ClassID 5) antes do Assassin (ClassID 3) se ambos estiverem no array
- O `Break` para o loop assim que encontra qualquer match, não necessariamente o correto

---

## ✅ **SOLUÇÃO: Garantir que Encontra o Placeholder Correto**

### **OPÇÃO 1: Usar Array Find (Recomendado)**

**Em vez de usar `For Each Loop with Break`, use `Array Find` para encontrar o placeholder específico:**

1. **Após `Get All Actors of Class`, adicione `Array Find`:**
   - Array: resultado do `Get All Actors of Class`
   - Item to Find: você precisa criar uma função que retorna o placeholder com o ClassID específico

**OU melhor:**

### **OPÇÃO 2: Filtrar Array por ClassID Antes do Loop**

**Criar uma função helper que filtra o array por ClassID:**

1. **Criar função `FindPlaceholderByClassID`:**
   - Input: `ClassID` (Integer)
   - Input: `PlaceholdersArray` (Array of BP_Class_Placeholder)
   - Output: `FoundPlaceholder` (BP_Class_Placeholder) ou `None`

2. **Na função, usar `For Each Loop` sem Break:**
   - Iterar sobre todos os placeholders
   - Comparar ClassID de cada um com o ClassID procurado
   - Quando encontrar match, retornar esse placeholder
   - Se não encontrar, retornar `None`

3. **No Event Construct, usar essa função:**
   ```
   [For Loop] (1 a 6)
       ↓ Loop Body
       [FindPlaceholderByClassID]
           ClassID: Loop Index
           PlaceholdersArray: resultado do Get All Actors of Class
           ↓
       [Is Valid] (FoundPlaceholder)
           ↓ True
       [Get Actor Transform] (do FoundPlaceholder)
       ↓
       [Add to Array] PlaceholderTransforms
       [Add to Array] PlaceholderClassIDs
   ```

---

## 📋 **ESTRUTURA CORRETA COM FUNÇÃO HELPER**

### **Função: FindPlaceholderByClassID**

```
[FindPlaceholderByClassID]
    Inputs:
        - ClassID (Integer)
        - PlaceholdersArray (Array of BP_Class_Placeholder)
    Outputs:
        - FoundPlaceholder (BP_Class_Placeholder)
    ↓
[For Each Loop] (sobre PlaceholdersArray)
    ↓ Loop Body
    [Get ClassID] (do Array Element)
    ↓
    [Equal] (ClassID == Input ClassID)
    ↓
    [Branch] (True)
        ↓
    [Return Node]
        Return Value: Array Element (o placeholder encontrado)
```

### **Event Construct Modificado**

```
[Event Construct]
    ↓
[Clear Arrays]
    ↓
[Get All Actors of Class]
    ↓
[For Loop] (1 a 6)
    ↓ Loop Body
    [FindPlaceholderByClassID]
        ClassID: Loop Index
        PlaceholdersArray: resultado do Get All Actors
        ↓
    [Is Valid] (FoundPlaceholder)
        ↓ True
    [Get Actor Transform] (do FoundPlaceholder)
    ↓
    [Add to Array] PlaceholderTransforms
    [Add to Array] PlaceholderClassIDs
        ↓
[Completed] (For Loop)
```

---

## ⚠️ **IMPORTANTE**

**A função `FindPlaceholderByClassID` deve iterar sobre TODOS os placeholders e retornar o que tem o ClassID correto, não apenas o primeiro que encontra por acaso.**

**Isso garante que sempre encontra o placeholder correto, independente da ordem do `Get All Actors of Class`.**

---

**FIM DA CORREÇÃO**

