# 🔧 **CORREÇÃO: Ordem Visual dos Placeholders**

## ❌ **PROBLEMA**

A ordem visual dos placeholders no nível é:
1. Barbarian (ClassID 1)
2. Assassin (ClassID 3)
3. Cleric (ClassID 4)
4. Dark Mage (ClassID 5)
5. Monk (ClassID 6)
6. Templar (ClassID 2)

Mas o loop está salvando na ordem numérica (1, 2, 3, 4, 5, 6), causando troca de posições.

---

## ✅ **SOLUÇÃO: Usar Array de Ordem Visual**

### **PASSO 1: Criar Array de Ordem Visual**

**No `WBP_CreateCharacter`, crie uma variável:**

1. **`VisualOrderClassIDs`** (Array of Integer)
   - Category: "Placeholders"
   - Default Value: `[1, 3, 4, 5, 6, 2]` ← **ORDEM VISUAL!**

### **PASSO 2: Modificar o Event Construct**

**Substitua o `For Loop` (1 a 6) por um `For Each Loop` sobre `VisualOrderClassIDs`:**

1. **Delete o `For Loop` (FirstIndex: 1, LastIndex: 6)**

2. **Adicione `For Each Loop`:**
   - Array: `VisualOrderClassIDs` (a variável que você criou)
   - Array Element: será o ClassID na ordem visual correta

3. **O resto do código permanece igual:**
   - `Get All Actors of Class`
   - `For Each Loop with Break`
   - Comparar `ClassID` do placeholder com o `Array Element` (ClassID da ordem visual)
   - Adicionar aos arrays quando encontrar

---

## 📋 **ESTRUTURA CORRETA**

```
[Event Construct]
    ↓
[Clear Arrays]
    ↓
[For Each Loop] ← TROCAR AQUI!
    Array: VisualOrderClassIDs [1, 3, 4, 5, 6, 2]
    Array Element: ClassID (na ordem visual)
        ↓ Loop Body
    [Get All Actors of Class]
        ↓
    [For Each Loop with Break]
        ↓ Loop Body
        [Get ClassID] (do placeholder)
        ↓
        [Equal] (ClassID == Array Element do For Each Loop externo)
        ↓
        [Branch] (True)
            ↓
        [Get Actor Transform]
        ↓
        [Add to Array] PlaceholderTransforms
        [Add to Array] PlaceholderClassIDs
        ↓
        [Break]
        ↓
    [Completed] (For Each Loop with Break) ← NÃO CONECTA
        ↓
[Completed] (For Each Loop externo) ← CONECTA AQUI ao resto
```

---

## ⚠️ **IMPORTANTE**

**A ordem visual é:** `[1, 3, 4, 5, 6, 2]`
- Posição 0: Barbarian (ClassID 1)
- Posição 1: Assassin (ClassID 3)
- Posição 2: Cleric (ClassID 4)
- Posição 3: Dark Mage (ClassID 5)
- Posição 4: Monk (ClassID 6)
- Posição 5: Templar (ClassID 2)

**Isso garante que os arrays sejam populados na ordem visual correta!**

---

**FIM DA CORREÇÃO**

