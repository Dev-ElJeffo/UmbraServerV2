# 🔧 **CORREÇÃO: Placeholders Spawnados em Posições Erradas**

## ❌ **PROBLEMA**

Após o respawn:
- ✅ Placeholders são spawnados
- ❌ Estão em posições erradas
- ❌ Assassin e Dark Mage estão trocados
- ❌ Fora das dimensões originais

**Causa:**
- A ordem dos `ClassIDs` no array não corresponde à ordem das `Locations`
- Ou os arrays estão sendo populados na ordem errada no `Event Construct`

---

## ✅ **SOLUÇÃO: Garantir Ordem Correta dos Arrays**

### **PASSO 1: Verificar Ordem no Event Construct**

**No `Event Construct` do `WBP_CreateCharacter`, verifique a ordem em que os placeholders são salvos:**

**O problema é que `Get All Actors of Class` pode retornar os actors em ordem aleatória!**

**Solução: Ordenar os placeholders por ClassID ANTES de salvar:**

```
[Event Construct]
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder (ou classe base)
    ↓
[For Each Loop]
    • Array: (resultado)
    ↓ Loop Body
    [Get ClassID] (do Array Element)
    ↓
[Add to Array] PlaceholderClassIDs
    • New Item: (ClassID)
    ↓
[Add to Array] PlaceholderLocations
    • New Item: (Actor Location)
```

**⚠️ PROBLEMA:** `Get All Actors of Class` não garante ordem!

---

### **PASSO 2: Ordenar Arrays por ClassID**

**Criar função `SortPlaceholdersByClassID` no `WBP_CreateCharacter`:**

**Ou melhor: Salvar em ordem específica manualmente:**

```
[Event Construct]
    ↓
[Clear Array] PlaceholderClassIDs
[Clear Array] PlaceholderLocations
[Clear Array] PlaceholderRotations
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder
    ↓
[For Each Loop]
    • Array: (resultado)
    ↓ Loop Body
    [Get ClassID] (do Array Element)
    ↓
[Switch on Int]
    • Selection: (ClassID)
    ↓
[Case 1] → [Add to Array] PlaceholderClassIDs (1)
         → [Add to Array] PlaceholderLocations (Actor Location)
    ↓
[Case 2] → [Add to Array] PlaceholderClassIDs (2)
         → [Add to Array] PlaceholderLocations (Actor Location)
    ↓
[Case 3] → [Add to Array] PlaceholderClassIDs (3)
         → [Add to Array] PlaceholderLocations (Actor Location)
    ↓
... (para todos os ClassIDs)
```

**Isso garante que os arrays sejam populados na ordem correta (1, 2, 3, 4, 5, 6).**

---

### **PASSO 3: Solução Mais Simples - Buscar por ClassID Específico**

**Em vez de usar `Get All Actors` e ordenar, busque cada placeholder individualmente:**

```
[Event Construct]
    ↓
[Clear Array] PlaceholderClassIDs
[Clear Array] PlaceholderLocations
[Clear Array] PlaceholderRotations
    ↓
[For Loop]
    • First Index: 1
    • Last Index: 6
    • Loop Body
    [Get All Actors of Class]
        • Actor Class: BP_Class_Placeholder
        ↓
    [For Each Loop]
        • Array: (resultado)
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
        [Add to Array] PlaceholderLocations
            • New Item: (Actor Location do Array Element)
            ↓
        [Break] ← IMPORTANTE: Parar o loop quando encontrar
```

**Isso garante que os arrays sejam populados na ordem 1, 2, 3, 4, 5, 6.**

---

### **PASSO 4: Solução Mais Robusta - Usar Map Temporário**

**Criar um Map temporário para garantir ordem:**

```
[Event Construct]
    ↓
[Clear Array] PlaceholderClassIDs
[Clear Array] PlaceholderLocations
[Clear Map] TempPlaceholderMap
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder
    ↓
[For Each Loop]
    • Array: (resultado)
    ↓ Loop Body
    [Get ClassID] (do Array Element)
    ↓
    [Add to Map] TempPlaceholderMap
        • Key: (ClassID)
        • Value: (Actor Location)
    ↓
[For Loop]
    • First Index: 1
    • Last Index: 6
    • Loop Body
    [Get] TempPlaceholderMap
    ↓
    [Find in Map]
        • Key: (Loop Index)
        • Found: (Boolean)
        ↓
    [Branch] (Found = True)
        ↓
    [Add to Array] PlaceholderClassIDs
        • New Item: (Loop Index)
        ↓
    [Get] TempPlaceholderMap
    ↓
    [Get] (Loop Index)
        • Key: (Loop Index)
        ↓
    [Add to Array] PlaceholderLocations
        • New Item: (Value do Map)
```

**Isso garante ordem correta independente da ordem de `Get All Actors`.**

---

### **PASSO 5: Adicionar Logs para Verificar Ordem**

**No `Event Construct`, após popular os arrays, adicione:**

```
[For Each Loop]
    • Array: PlaceholderClassIDs
    ↓ Loop Body
    [Get] PlaceholderLocations
    ↓
    [Get] (Array Index)
        • Array: PlaceholderLocations
        • Index: (Array Index do Loop)
        ↓
    [Print String]
        • In String: "ClassID " + (Array Element como String) + " em posição " + (Location como String)
        • bPrintToScreen: true
```

**Isso vai mostrar se a ordem está correta.**

---

## ✅ **SOLUÇÃO MAIS SIMPLES E DIRETA**

**Se você sabe a ordem correta dos ClassIDs (1, 2, 3, 4, 5, 6), defina os arrays manualmente:**

**No `Event Construct`:**

```
[Event Construct]
    ↓
[Clear Array] PlaceholderClassIDs
[Clear Array] PlaceholderLocations
    ↓
[Add to Array] PlaceholderClassIDs
    • New Item: 1
[Add to Array] PlaceholderClassIDs
    • New Item: 2
[Add to Array] PlaceholderClassIDs
    • New Item: 3
[Add to Array] PlaceholderClassIDs
    • New Item: 4
[Add to Array] PlaceholderClassIDs
    • New Item: 5
[Add to Array] PlaceholderClassIDs
    • New Item: 6
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder
    ↓
[For Each Loop]
    • Array: (resultado)
    ↓ Loop Body
    [Get ClassID] (do Array Element)
    ↓
[Switch on Int]
    • Selection: (ClassID)
    ↓
[Case 1] → [Set Array Elem] PlaceholderLocations
              • Array: PlaceholderLocations
              • Index: 0
              • New Item: (Actor Location)
    ↓
[Case 2] → [Set Array Elem] PlaceholderLocations
              • Array: PlaceholderLocations
              • Index: 1
              • New Item: (Actor Location)
    ↓
[Case 3] → [Set Array Elem] PlaceholderLocations
              • Array: PlaceholderLocations
              • Index: 2
              • New Item: (Actor Location)
    ↓
... (para todos os ClassIDs)
```

**Isso garante que:**
- `PlaceholderClassIDs[0]` = 1 (Barbarian)
- `PlaceholderClassIDs[1]` = 2 (Templar)
- `PlaceholderClassIDs[2]` = 3 (Assassin)
- `PlaceholderClassIDs[3]` = 4 (Cleric)
- `PlaceholderClassIDs[4]` = 5 (DarkMage)
- `PlaceholderClassIDs[5]` = 6 (Monk)

**E as Locations correspondem na mesma ordem.**

---

## 📋 **CHECKLIST**

- [ ] Verifiquei a ordem dos arrays no `Event Construct`
- [ ] Garanti que `ClassIDs` e `Locations` estão na mesma ordem
- [ ] Adicionei logs para verificar a ordem
- [ ] Testei e os placeholders aparecem nas posições corretas
- [ ] Assassin e Dark Mage estão nas posições corretas

---

**FIM DA CORREÇÃO**

