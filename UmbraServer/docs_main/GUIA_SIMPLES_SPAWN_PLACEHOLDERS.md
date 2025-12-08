# 📋 **GUIA SIMPLES: Spawnar Placeholders**

## 🎯 **OBJETIVO**

Spawnar placeholders de forma SIMPLES, sem Switch on Int, sem structs complexas.

---

## ✅ **SOLUÇÃO: Função C++ Simples**

Nova função `SpawnPlaceholdersSimple` que recebe:
- Array de ClassIDs
- Array de Locations
- Array de Rotations (opcional)
- **Map de ClassID -> PlaceholderClass** (você preenche uma vez no Blueprint)

---

## 📝 **PASSO 1: Criar Variáveis no WBP_CreateCharacter**

**My Blueprint** → **Variables**:

1. **`PlaceholderClassIDs`** (Array of Integer)
   - Category: "Placeholders"

2. **`PlaceholderLocations`** (Array of Vector)
   - Category: "Placeholders"

3. **`PlaceholderRotations`** (Array of Rotator) - **OPCIONAL**
   - Category: "Placeholders"

4. **`ClassIDToPlaceholderClass`** (Map)
   - **Key Type:** Integer
   - **Value Type:** Actor Class Reference
   - Category: "Placeholders"
   - **Default Value:** Preencha uma vez com todos os mapeamentos:
     - Key: 1 → Value: BP_Barbarian_Placeholder
     - Key: 2 → Value: BP_Templar_Placeholder
     - Key: 3 → Value: BP_Assassin_Placeholder
     - Key: 4 → Value: BP_Mage_Placeholder
     - Key: 5 → Value: BP_Ranger_Placeholder
     - Key: 6 → Value: BP_Paladin_Placeholder
     - (adicione todos os seus ClassIDs)

---

## 📝 **PASSO 2: Salvar Dados no Event Construct**

**No `Event Construct` do `WBP_CreateCharacter`:**

```
[Event Construct]
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder
    ↓
[For Each Loop]
    • Array: (resultado do Get All Actors)
    ↓ Loop Body
    • Array Element: (Placeholder Actor)
    ↓
[Get ClassID] (do Array Element)
    ↓
[Add to Array] PlaceholderClassIDs
    • Array: PlaceholderClassIDs
    • New Item: (ClassID)
    ↓
[Get Actor Location] (do Array Element)
    ↓
[Add to Array] PlaceholderLocations
    • Array: PlaceholderLocations
    • New Item: (Location)
    ↓
[Get Actor Rotation] (do Array Element) - OPCIONAL
    ↓
[Add to Array] PlaceholderRotations
    • Array: PlaceholderRotations
    • New Item: (Rotation)
```

**Só isso! Não precisa salvar a classe.**

---

## 📝 **PASSO 3: Preencher Map ClassIDToPlaceholderClass**

**No `Event Construct`, ANTES do For Each Loop:**

```
[Event Construct]
    ↓
[Clear Map] ClassIDToPlaceholderClass
    ↓
[Add to Map] ClassIDToPlaceholderClass
    • Key: 1
    • Value: BP_Barbarian_Placeholder (arraste do Content Browser)
    ↓
[Add to Map] ClassIDToPlaceholderClass
    • Key: 2
    • Value: BP_Templar_Placeholder
    ↓
[Add to Map] ClassIDToPlaceholderClass
    • Key: 3
    • Value: BP_Assassin_Placeholder
    ↓
... (adicione todos os seus ClassIDs)
    ↓
[Get All Actors of Class] (continua com o Passo 2)
```

**OU configure diretamente no Default Value da variável no Editor.**

---

## 📝 **PASSO 4: Criar Função SpawnAllPlaceholders (Blueprint)**

**My Blueprint** → **Functions** → **+ Function**:

- **Nome:** `SpawnAllPlaceholders`
- **Inputs:** Nenhum

**Implementação:**

```
[SpawnAllPlaceholders]
    ↓
[Get] Manager
    ↓
[Is Valid] Manager
    ↓
[Branch] (True)
    ↓
[Get Array Length]
    • Array: PlaceholderClassIDs
    ↓
[Greater] Length > 0
    ↓
[Branch] (True)
    ↓
[Call Function: Spawn Placeholders Simple]
    • Target: Manager
    • Class IDs: PlaceholderClassIDs
    • Locations: PlaceholderLocations
    • Rotations: PlaceholderRotations (ou deixe vazio se não usar)
    • Class ID to Placeholder Class: ClassIDToPlaceholderClass
    • Return Value: SpawnedCount
    ↓
[Print String] (opcional)
    • In String: "Placeholders spawnados: " + (SpawnedCount como String)
```

---

## 📝 **PASSO 5: Chamar Após Reset**

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

## ✅ **VANTAGENS**

- ✅ **SIMPLES** - Sem Switch on Int
- ✅ **SEM structs complexas** - Apenas arrays simples
- ✅ **Map editável** - Configure uma vez, use sempre
- ✅ **Fácil de entender** - Lógica direta

---

**Fim do Guia**

