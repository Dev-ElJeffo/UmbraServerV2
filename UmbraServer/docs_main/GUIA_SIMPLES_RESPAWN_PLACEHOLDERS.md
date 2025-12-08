# 📋 **GUIA SIMPLES: Respawnar Placeholders**

## ❌ **PROBLEMA**

Após clicar em "Return to Selection", os placeholders são destruídos mas não respawnados. Por isso você não consegue selecionar outra classe.

**Erro no log:**
```
LogTemp: Error: [UmbraCharacterCreationManager] ❌❌❌ Placeholder não encontrado para ClassID X
```

---

## ✅ **SOLUÇÃO SIMPLES**

Salvar as posições e classes dos placeholders no `Event Construct` e respawná-los após o reset.

---

## 📝 **PASSO 1: Criar Variáveis no WBP_CreateCharacter**

**My Blueprint** → **Variables** → **+ Variable**:

1. **`PlaceholderPositions`** (Array of Vector)
   - **Category:** "Placeholders"

2. **`PlaceholderClassIDs`** (Array of Integer)
   - **Category:** "Placeholders"

**⚠️ IMPORTANTE:** Não precisamos salvar a classe do placeholder! Vamos usar um Switch baseado no ClassID para determinar qual classe spawnar.

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
    ↓
[Get Class Name]
    • Target: Array Element
    • Return Value: (String - nome da classe, ex: "BP_Barbarian_Placeholder_C")
    ↓
[Add to Array] PlaceholderClassNames
    • Array: PlaceholderClassNames
    • New Item: (Class Name)
```

**⚠️ IMPORTANTE:** Isso deve ser feito ANTES de qualquer placeholder ser destruído (no Event Construct, quando o level inicia).

---

## 📝 **PASSO 3: Criar Função SpawnAllPlaceholders**

**My Blueprint** → **Functions** → **+ Function**:

- **Nome:** `SpawnAllPlaceholders`
- **Inputs:** Nenhum (usa os arrays salvos)

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
[Is Valid]
    • Object: PlaceholderClass
    ↓
[Branch] (True)
    ↓
[Get] PlaceholderClassIDs
    ↓
[Get] (Array Index)
    • Array: PlaceholderClassIDs
    • Index: Array Index
    ↓
[Spawn Actor]
    • Actor Class: PlaceholderClass (do Load Class)
    • Location: Array Element (posição)
    • Rotation: (0, 0, 0)
    • Return Value: SpawnedPlaceholder
    ↓
[Is Valid]
    • Object: SpawnedPlaceholder
    ↓
[Branch] (True)
    ↓
[Set ClassID] (no placeholder)
    • Target: SpawnedPlaceholder
    • ClassID: (PlaceholderClassIDs[Index])
```

**NOTA:** Se o placeholder não tiver função `SetClassID`, use reflection ou set a propriedade diretamente.

---

## 📝 **PASSO 4: Chamar SpawnAllPlaceholders Após Reset**

**No `BTN_Return OnClicked`:**

```
[BTN_Return OnClicked]
    ↓
[Get] Manager
    ↓
[Is Valid] Manager
    ↓
[Branch] (True)
    ↓
[Get Self]
    ↓
[Reset Character Creation]
    • Target: Manager
    • Create Character Widget: Self
    • Placeholder Class: BP_Class_Placeholder
    • Spacing: 300.0
    ↓
[Call Function: SpawnAllPlaceholders]
    • Target: Self
```

---

## 🔍 **COMO OBTER ClassID DO PLACEHOLDER**

**No Passo 2, para obter o ClassID:**

### **Opção 1: Função GetClassID**

```
[Call Function: Get ClassID]
    • Target: Array Element (Placeholder)
    • Return Value: ClassID
```

### **Opção 2: Propriedade Direta**

Se o placeholder tiver uma propriedade `ClassID`:

```
[Get] ClassID
    • Target: Array Element (Placeholder)
```

### **Opção 3: Via Nome do Actor**

Se o nome do placeholder contém o ClassID (ex: "Barbarian_Placeholder_1"):

```
[Get Actor Name]
    • Target: Array Element
    ↓
[Parse String] ou [Split String]
    ↓
[String to Integer]
```

---

## ✅ **VERIFICAÇÃO**

Após implementar:

1. ✅ Abra o level → Placeholders devem existir
2. ✅ Selecione uma classe → Deve funcionar
3. ✅ Clique em "Return to Selection" → Placeholders são destruídos
4. ✅ Placeholders são respawnados automaticamente
5. ✅ Selecione outra classe → Deve funcionar novamente

**Se não funcionar, verifique:**
- Os arrays foram preenchidos no Event Construct? (adicione prints)
- Os ClassIDs estão corretos? (verifique os logs)
- As posições estão corretas? (verifique no level)

---

**Fim do Guia**

