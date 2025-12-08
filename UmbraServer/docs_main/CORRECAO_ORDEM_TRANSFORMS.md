# 🔧 **CORREÇÃO: Ordem dos Transforms**

## ❌ **PROBLEMA**

Mesmo usando `Transform`, os placeholders continuam spawnando em posições erradas.

**Causa:**
- `Get All Actors of Class` retorna os actors em **ordem aleatória**
- Os Transforms estão sendo salvos na ordem que os actors aparecem, não na ordem dos ClassIDs (1, 2, 3, 4, 5, 6)

---

## ✅ **SOLUÇÃO: Garantir Ordem Correta dos Transforms**

### **PASSO 1: Salvar Transforms na Ordem Correta (1, 2, 3, 4, 5, 6)**

**No `Event Construct`, use um `For Loop` de 1 a 6 e busque cada ClassID individualmente:**

```
[Event Construct]
    ↓
[Clear Array] PlaceholderClassIDs
[Clear Array] PlaceholderTransforms
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
            • B: (Loop Index) ← BUSCAR ClassID ESPECÍFICO (1, 2, 3, 4, 5, 6)
            ↓
        [Branch] (True)
            ↓
        [Add to Array] PlaceholderClassIDs
            • New Item: (Loop Index) ← GARANTE ORDEM 1, 2, 3, 4, 5, 6
            ↓
        [Get Actor Transform]
            • Target: (Array Element)
            ↓
        [Add to Array] PlaceholderTransforms
            • New Item: (Actor Transform)
            ↓
        [Break] ← IMPORTANTE: Parar quando encontrar
```

**Isso garante que:**
- `PlaceholderClassIDs[0]` = 1 (Barbarian)
- `PlaceholderClassIDs[1]` = 2 (Templar)
- `PlaceholderClassIDs[2]` = 3 (Assassin)
- `PlaceholderClassIDs[3]` = 4 (Cleric)
- `PlaceholderClassIDs[4]` = 5 (DarkMage)
- `PlaceholderClassIDs[5]` = 6 (Monk)

**E os Transforms correspondem na mesma ordem!**

---

### **PASSO 2: Adicionar Logs para Verificar Ordem**

**Após popular os arrays, adicione logs:**

```
[For Each Loop]
    • Array: PlaceholderClassIDs
    ↓ Loop Body
    [Get] PlaceholderTransforms
    ↓
    [Get] (Array Index)
        • Array: PlaceholderTransforms
        • Index: (Array Index do Loop)
        ↓
    [Break Transform]
        ↓
    [Get Location] (do Break Transform)
        ↓
    [Print String]
        • In String: "ClassID " + (Array Element como String) + " em posição " + (Location como String)
        • bPrintToScreen: true
        • Text Color: (0, 255, 0) ← Verde
```

**Isso vai mostrar se a ordem está correta.**

---

### **PASSO 3: Solução Alternativa - Usar Switch on Int**

**Se o For Loop não funcionar, use `Switch on Int` para garantir ordem:**

```
[Event Construct]
    ↓
[Clear Array] PlaceholderClassIDs
[Clear Array] PlaceholderTransforms
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
             → [Add to Array] PlaceholderTransforms (Get Actor Transform)
        ↓
    [Case 2] → [Add to Array] PlaceholderClassIDs (2)
             → [Add to Array] PlaceholderTransforms (Get Actor Transform)
        ↓
    [Case 3] → [Add to Array] PlaceholderClassIDs (3)
             → [Add to Array] PlaceholderTransforms (Get Actor Transform)
        ↓
    [Case 4] → [Add to Array] PlaceholderClassIDs (4)
             → [Add to Array] PlaceholderTransforms (Get Actor Transform)
        ↓
    [Case 5] → [Add to Array] PlaceholderClassIDs (5)
             → [Add to Array] PlaceholderTransforms (Get Actor Transform)
        ↓
    [Case 6] → [Add to Array] PlaceholderClassIDs (6)
             → [Add to Array] PlaceholderTransforms (Get Actor Transform)
```

**⚠️ PROBLEMA:** Isso ainda pode salvar em ordem aleatória se os actors aparecerem em ordem aleatória.

---

### **PASSO 4: Solução Mais Robusta - Usar Map Temporário**

**Criar um Map temporário para garantir ordem:**

```
[Event Construct]
    ↓
[Clear Array] PlaceholderClassIDs
[Clear Array] PlaceholderTransforms
[Clear Map] TempTransformMap
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder
    ↓
[For Each Loop]
    • Array: (resultado)
    ↓ Loop Body
    [Get ClassID] (do Array Element)
    ↓
    [Get Actor Transform]
        • Target: (Array Element)
        ↓
    [Add to Map] TempTransformMap
        • Key: (ClassID)
        • Value: (Actor Transform)
    ↓
[For Loop]
    • First Index: 1
    • Last Index: 6
    • Loop Body
    [Add to Array] PlaceholderClassIDs
        • New Item: (Loop Index)
        ↓
    [Get] TempTransformMap
    ↓
    [Find in Map]
        • Key: (Loop Index)
        • Found: (Boolean)
        • Value: (Transform)
        ↓
    [Branch] (Found = True)
        ↓
    [Add to Array] PlaceholderTransforms
        • New Item: (Value do Map)
```

**Isso garante ordem correta independente da ordem de `Get All Actors`.**

---

### **PASSO 5: Verificar se os Arrays Têm o Mesmo Tamanho**

**Antes de chamar `SpawnPlaceholdersWithTransforms`, adicione validação:**

```
[Get Array Length]
    • Array: PlaceholderClassIDs
    ↓
[Get Array Length]
    • Array: PlaceholderTransforms
    ↓
[Equal] (Integer)
    • A: (Length ClassIDs)
    • B: (Length Transforms)
    ↓
[Branch] (True)
    ↓
[Spawn Placeholders With Transforms]
    • Class IDs: PlaceholderClassIDs
    • Transforms: PlaceholderTransforms
    ↓
[Branch] (False)
    ↓
[Print String]
    • In String: "ERRO: Arrays têm tamanhos diferentes! ClassIDs: " + (Length ClassIDs) + " Transforms: " + (Length Transforms)
    • bPrintToScreen: true
    • Text Color: (255, 0, 0) ← Vermelho
```

---

## ✅ **SOLUÇÃO RECOMENDADA: Usar Map Temporário**

**A solução mais robusta é usar um Map temporário:**

1. **Salvar todos os Transforms em um Map** (Key = ClassID, Value = Transform)
2. **Iterar de 1 a 6** e buscar cada Transform do Map
3. **Adicionar aos arrays na ordem correta**

**Isso garante que os arrays sempre estejam na ordem 1, 2, 3, 4, 5, 6.**

---

## 📋 **CHECKLIST**

- [ ] Usei `For Loop` de 1 a 6 para buscar cada ClassID individualmente
- [ ] Adicionei `Break` após encontrar cada ClassID
- [ ] Adicionei logs para verificar a ordem
- [ ] Verifiquei que os arrays têm o mesmo tamanho
- [ ] Testei e os placeholders aparecem nas posições corretas

---

**FIM DA CORREÇÃO**

