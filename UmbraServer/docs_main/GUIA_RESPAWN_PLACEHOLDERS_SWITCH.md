# 📋 **GUIA: Respawnar Placeholders Usando Switch (Solução Simples)**

## ❌ **PROBLEMA**

`Get Class` não conecta com `Array of Actor Class Reference` no Blueprint.

---

## ✅ **SOLUÇÃO: Usar Switch Baseado no ClassID**

Não salve a classe do placeholder. Use um `Switch on Int` para determinar qual classe spawnar baseado no ClassID.

---

## 📝 **PASSO 1: Criar Apenas 2 Arrays**

**My Blueprint** → **Variables**:

1. **`PlaceholderPositions`** (Array of Vector)
2. **`PlaceholderClassIDs`** (Array of Integer)

**⚠️ NÃO PRECISA salvar a classe!**

---

## 📝 **PASSO 2: Salvar Dados no Event Construct**

**No `Event Construct` do `WBP_CreateCharacter`:**

```
[Event Construct]
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder (ou a classe base)
    ↓
[For Each Loop]
    • Array: (resultado do Get All Actors)
    ↓ Loop Body
    • Array Element: (Placeholder Actor)
    ↓
[Get Actor Location]
    • Target: Array Element
    ↓
[Add to Array] PlaceholderPositions
    • Array: PlaceholderPositions
    • New Item: (Actor Location)
    ↓
[Get ClassID] (do placeholder)
    • Target: Array Element
    ↓
[Add to Array] PlaceholderClassIDs
    • Array: PlaceholderClassIDs
    • New Item: (ClassID)
```

**Só isso! Não precisa salvar a classe.**

---

## 📝 **PASSO 3: Criar Função SpawnAllPlaceholders**

**My Blueprint** → **Functions** → **+ Function**:

- **Nome:** `SpawnAllPlaceholders`
- **Inputs:** Nenhum

**Implementação:**

```
[SpawnAllPlaceholders]
    ↓
[Get Array Length]
    • Array: PlaceholderPositions
    ↓
[Greater] Length > 0
    ↓
[Branch] (True)
    ↓
[For Each Loop]
    • Array: PlaceholderPositions
    ↓ Loop Body
    • Array Element: (Vector - posição)
    • Array Index: (Integer - índice)
    ↓
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
    │          • Location: Array Element (posição)
    │          • Rotation: (0, 0, 0)
    │          • Return Value: SpawnedPlaceholder
    │
    ├─ Case 2: [Spawn Actor] BP_Templar_Placeholder
    │          • Location: Array Element
    │          • Rotation: (0, 0, 0)
    │          • Return Value: SpawnedPlaceholder
    │
    ├─ Case 3: [Spawn Actor] BP_Assassin_Placeholder
    │          • Location: Array Element
    │          • Rotation: (0, 0, 0)
    │          • Return Value: SpawnedPlaceholder
    │
    ├─ Case 4: [Spawn Actor] BP_Cleric_Placeholder
    │          • Location: Array Element
    │          • Rotation: (0, 0, 0)
    │          • Return Value: SpawnedPlaceholder
    │
    ├─ Case 5: [Spawn Actor] BP_DarkMage_Placeholder
    │          • Location: Array Element
    │          • Rotation: (0, 0, 0)
    │          • Return Value: SpawnedPlaceholder
    │
    └─ Case 6: [Spawn Actor] BP_Monk_Placeholder
              • Location: Array Element
              • Rotation: (0, 0, 0)
              • Return Value: SpawnedPlaceholder
    ↓ (todos os cases conectam aqui)
[Is Valid]
    • Object: SpawnedPlaceholder
    ↓
[Branch] (True)
    ↓
[Set ClassID] (no placeholder)
    • Target: SpawnedPlaceholder
    • ClassID: (PlaceholderClassIDs[Index])
```

---

## 📝 **PASSO 4: Como Adicionar Cases no Switch**

### **4.1. Adicionar Switch on Int**

1. Arraste do pino de saída do `Get` (PlaceholderClassIDs[Index])
2. Digite: **"Switch on Int"**
3. Selecione: **Switch on Int**
4. Conecte o **Selection** ao ClassID

### **4.2. Adicionar Cases**

**No nó `Switch on Int`:**

1. Clique com botão direito no nó → **Add Pin** (ou use o botão **+** no nó)
2. Adicione **6 cases** (ou quantos placeholders você tiver)
3. Para cada case:
   - **Case 1:** Valor = `1` → Spawn `BP_Barbarian_Placeholder`
   - **Case 2:** Valor = `2` → Spawn `BP_Templar_Placeholder`
   - **Case 3:** Valor = `3` → Spawn `BP_Assassin_Placeholder`
   - **Case 4:** Valor = `4` → Spawn `BP_Cleric_Placeholder`
   - **Case 5:** Valor = `5` → Spawn `BP_DarkMage_Placeholder`
   - **Case 6:** Valor = `6` → Spawn `BP_Monk_Placeholder`

**⚠️ IMPORTANTE:** Verifique os ClassIDs corretos no banco de dados! Eles podem não ser 1, 2, 3, 4, 5, 6.

### **4.3. Conectar Todos os Cases ao Mesmo Fluxo**

**Todos os cases devem conectar ao mesmo nó `Is Valid`:**

```
Case 1 → Spawn Actor → Return Value
Case 2 → Spawn Actor → Return Value
Case 3 → Spawn Actor → Return Value
... (todos)
    ↓ (todos conectam aqui)
[Is Valid] SpawnedPlaceholder
```

**Como fazer:**
- Arraste do pino **Return Value** de cada `Spawn Actor`
- Conecte todos ao mesmo pino **Object** do `Is Valid`

---

## 📝 **PASSO 5: Chamar Após Reset**

**No `BTN_Return OnClicked`:**

```
[Reset Character Creation]
    ↓
[Call Function: SpawnAllPlaceholders]
    • Target: Self
```

---

## 🔍 **VERIFICAR ClassIDs CORRETOS**

**Execute no banco de dados:**

```sql
SELECT class_id, class_name FROM classes ORDER BY class_id;
```

**Use os ClassIDs retornados nos cases do Switch!**

---

## ✅ **VANTAGENS DESTA SOLUÇÃO**

- ✅ Não precisa lidar com tipos incompatíveis
- ✅ Mais simples e direto
- ✅ Funciona sempre
- ✅ Fácil de entender e manter

---

**Fim do Guia**

