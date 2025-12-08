# 🔧 **CORREÇÃO: Escala e Posição dos Placeholders**

## ❌ **PROBLEMA**

Após o respawn:
- ✅ Placeholders são spawnados
- ❌ Não estão com a mesma **escala** dos spawns originais
- ❌ Não estão nas mesmas **posições** exatas

**Causa:**
- A função `SpawnPlaceholderAtLocation` não estava aplicando a **escala**
- A escala não estava sendo salva no `Event Construct`

---

## ✅ **SOLUÇÃO: Adicionar Suporte à Escala**

### **PASSO 1: Recompilar o C++**

**O código C++ foi atualizado para suportar escala. Recompile o projeto:**

1. **Feche o Unreal Editor**
2. **No Visual Studio**, pressione **F7** ou **Build** → **Build Solution**
3. **Aguarde a compilação terminar**
4. **Abra o Unreal Editor novamente**

---

### **PASSO 2: Adicionar Variável de Escala**

**No `WBP_CreateCharacter` → My Blueprint → Variables:**

1. **`PlaceholderScales`** (Array of Vector)
   - Category: "Placeholders"
   - Default Value: `[]` (vazio)

---

### **PASSO 3: Salvar Escala no Event Construct**

**No `Event Construct`, adicione para salvar a escala:**

**IMPORTANTE:** Se `Get Actor Scale` não existir, use `Get Actor Transform` e extraia a escala:

```
[Event Construct]
    ↓
[Clear Array] PlaceholderClassIDs
[Clear Array] PlaceholderLocations
[Clear Array] PlaceholderRotations
[Clear Array] PlaceholderScales ← ADICIONAR
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
        [Get Actor Location] (do Array Element)
            ↓
        [Add to Array] PlaceholderLocations
            • New Item: (Actor Location)
            ↓
        [Get Actor Rotation] (do Array Element)
            ↓
        [Add to Array] PlaceholderRotations
            • New Item: (Actor Rotation)
            ↓
        [Get Actor Transform] ← USAR ISSO
            • Target: (Array Element)
            ↓
        [Break Transform]
            ↓
        [Get Scale] (do Break Transform) ← EXTRAIR ESCALA
            ↓
        [Add to Array] PlaceholderScales ← ADICIONAR
            • New Item: (Scale)
            ↓
        [Break]
```

**Isso garante que a escala seja salva junto com a posição e rotação.**

---

### **PASSO 4: Passar Escala para SpawnPlaceholdersSimple**

**Na função que chama `SpawnPlaceholdersSimple` (geralmente após `ResetCharacterCreation`):**

```
[Get] Manager
    ↓
[Is Valid] Manager
    ↓
[Branch] (True)
    ↓
[Spawn Placeholders Simple]
    • Target: Manager
    • Class IDs: PlaceholderClassIDs
    • Locations: PlaceholderLocations
    • Rotations: PlaceholderRotations
    • Scales: PlaceholderScales ← ADICIONAR
    • Return Value: SpawnedCount
```

**Se o array `PlaceholderScales` estiver vazio, a função usará escala padrão (1,1,1).**

---

### **PASSO 5: Verificar se Get Actor Scale Existe**

**Se `Get Actor Scale` não existir no Blueprint, use `Get Actor Transform` e extraia a escala:**

```
[Get Actor Transform]
    • Target: (Array Element)
    ↓
[Break Transform]
    ↓
[Get Scale] (do Break Transform)
    ↓
[Add to Array] PlaceholderScales
    • New Item: (Scale)
```

---

## ✅ **RESUMO DAS MUDANÇAS**

1. ✅ **C++ atualizado** para aceitar parâmetro `Scale` em `SpawnPlaceholderAtLocation`
2. ✅ **C++ atualizado** para aceitar array `Scales` em `SpawnPlaceholdersSimple`
3. ✅ **Adicionar variável** `PlaceholderScales` no Blueprint
4. ✅ **Salvar escala** no `Event Construct`
5. ✅ **Passar escala** para `SpawnPlaceholdersSimple`

---

## 📋 **CHECKLIST**

- [ ] Recompilei o C++
- [ ] Adicionei variável `PlaceholderScales` (Array of Vector)
- [ ] Modifiquei `Event Construct` para salvar a escala
- [ ] Modifiquei a chamada de `SpawnPlaceholdersSimple` para passar `PlaceholderScales`
- [ ] Testei e os placeholders aparecem com a escala correta
- [ ] Testei e os placeholders aparecem nas posições corretas

---

**FIM DA CORREÇÃO**

