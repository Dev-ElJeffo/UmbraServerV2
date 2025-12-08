# 🔧 **CORREÇÃO: Get Class Não Conecta com Array de Actor Class Reference**

## ❌ **PROBLEMA**

No Blueprint, `Get Class` retorna um tipo `Class Reference` que não é diretamente compatível com `Array of Actor Class Reference`.

**Erro comum:**
- `Get Class` → Retorna `Class Reference`
- `PlaceholderClasses` (Array of Actor Class Reference) → Não aceita `Class Reference`

---

## ✅ **SOLUÇÃO: Usar Nome da Classe (String)**

Em vez de salvar a classe diretamente, salve o **nome da classe** como String e faça lookup depois.

---

## 📝 **PASSO 1: Mudar Tipo do Array**

**No `WBP_CreateCharacter`:**

1. **Remova ou ignore** o array `PlaceholderClasses` (Array of Actor Class Reference)
2. **Crie novo array:** `PlaceholderClassNames` (Array of String)

---

## 📝 **PASSO 2: Salvar Nome da Classe**

**No `Event Construct`, em vez de `Get Class`:**

```
[Get Class Name]
    • Target: Array Element (Placeholder)
    • Return Value: (String - ex: "BP_Barbarian_Placeholder_C")
    ↓
[Add to Array] PlaceholderClassNames
    • Array: PlaceholderClassNames
    • New Item: (Class Name)
```

**Como fazer no Blueprint:**

1. Arraste do `Array Element` (do For Each Loop)
2. Digite: **"Get Class Name"**
3. Selecione: **Get Class Name** (retorna String)
4. Conecte o **Return Value** ao **New Item** do **Add to Array**

---

## 📝 **PASSO 3: Carregar Classe no Spawn**

**Na função `SpawnAllPlaceholders`, em vez de usar o array diretamente:**

```
[Get] PlaceholderClassNames
    ↓
[Get] (Array Index)
    • Array: PlaceholderClassNames
    • Index: Array Index
    ↓
[Load Class from Asset Path] ou [Load Class from Name]
    • Class Name: (PlaceholderClassNames[Index])
    • Return Value: PlaceholderClass
    ↓
[Spawn Actor]
    • Actor Class: PlaceholderClass
```

**⚠️ PROBLEMA:** `Load Class from Asset Path` requer o path completo (ex: `/Game/Widgets/UI/ClassSelectorItem/BP_Barbarian_Placeholder.BP_Barbarian_Placeholder_C`)

**SOLUÇÃO ALTERNATIVA:** Usar um Map que mapeia ClassID para a classe do placeholder.

---

## 📝 **PASSO 4: Solução Alternativa - Usar Map**

### **4.1. Criar Map no WBP_CreateCharacter**

**My Blueprint** → **Variables**:

1. **`ClassIDToPlaceholderClass`** (Map)
   - **Key Type:** Integer (ClassID)
   - **Value Type:** Actor Class Reference
   - **Category:** "Placeholders"

### **4.2. Preencher Map no Event Construct**

```
[Event Construct]
    ↓
[Get All Actors of Class] BP_Class_Placeholder
    ↓
[For Each Loop]
    ↓ Loop Body
    • Array Element: (Placeholder)
    ↓
[Get Actor Location] → Add to PlaceholderPositions
[Get ClassID] → Add to PlaceholderClassIDs
    ↓
[Get Class] (do Array Element)
    • Return Value: (Class Reference)
    ↓
[Add to Map] ClassIDToPlaceholderClass
    • Target: ClassIDToPlaceholderClass
    • Key: (ClassID)
    • Value: (Class Reference do Get Class)
```

**⚠️ PROBLEMA:** Ainda terá o mesmo problema de tipo incompatível.

---

## 📝 **PASSO 5: Solução Mais Simples - Usar Switch por ClassID**

### **5.1. Não Salvar a Classe - Determinar pela ClassID**

**Na função `SpawnAllPlaceholders`:**

```
[Get] PlaceholderClassIDs
    ↓
[Get] (Array Index)
    • Array: PlaceholderClassIDs
    • Index: Array Index
    ↓
[Switch on Int] (ClassID)
    • Selection: (PlaceholderClassIDs[Index])
    ↓
    ├─ Case 1: [Spawn Actor] BP_Barbarian_Placeholder
    ├─ Case 2: [Spawn Actor] BP_Templar_Placeholder
    ├─ Case 3: [Spawn Actor] BP_Assassin_Placeholder
    ├─ Case 4: [Spawn Actor] BP_Cleric_Placeholder
    ├─ Case 5: [Spawn Actor] BP_DarkMage_Placeholder
    └─ Case 6: [Spawn Actor] BP_Monk_Placeholder
```

**Esta é a solução mais simples e confiável!**

---

## ✅ **SOLUÇÃO RECOMENDADA (Mais Simples)**

**Não salve a classe do placeholder. Use um Switch baseado no ClassID:**

1. **Salve apenas:**
   - `PlaceholderPositions` (Array of Vector)
   - `PlaceholderClassIDs` (Array of Integer)

2. **Na função `SpawnAllPlaceholders`:**
   - Use `Switch on Int` com o ClassID
   - Spawne a classe correta baseada no ClassID

**Vantagens:**
- ✅ Não precisa lidar com tipos incompatíveis
- ✅ Mais simples e direto
- ✅ Funciona sempre

---

**Fim do Guia**

