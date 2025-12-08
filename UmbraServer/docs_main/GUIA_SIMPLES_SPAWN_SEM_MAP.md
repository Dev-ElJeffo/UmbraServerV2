# 📋 **GUIA SIMPLES: Spawnar Placeholders SEM MAP**

## 🎯 **OBJETIVO**

Spawnar placeholders usando apenas **arrays simples** - o mapeamento ClassID → PlaceholderClass é feito **internamente no C++**.

---

## ✅ **SOLUÇÃO: Função C++ Simplificada**

A função `SpawnPlaceholdersSimple` agora recebe apenas:
- Array de ClassIDs
- Array de Locations
- Array de Rotations (opcional)

**O mapeamento ClassID → PlaceholderClass é feito automaticamente no C++!**

---

## 📝 **PASSO 1: Criar Variáveis no WBP_CreateCharacter**

**My Blueprint** → **Variables**:

1. **`PlaceholderClassIDs`** (Array of Integer)
   - Category: "Placeholders"

2. **`PlaceholderLocations`** (Array of Vector)
   - Category: "Placeholders"

3. **`PlaceholderRotations`** (Array of Rotator) - **OPCIONAL**
   - Category: "Placeholders"

**NÃO PRECISA DE MAP!**

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

**Só isso! Não precisa salvar a classe nem criar Map.**

---

## 📝 **PASSO 3: Criar Função SpawnAllPlaceholders (Blueprint)**

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
    • Return Value: SpawnedCount
    ↓
[Print String] (opcional)
    • In String: "Placeholders spawnados: " + (SpawnedCount como String)
```

---

## 📝 **PASSO 4: Chamar Após Reset**

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

## ⚠️ **IMPORTANTE: Ajustar Caminhos no C++**

**Se seus Blueprints estão em caminhos diferentes, edite o arquivo:**

`UmbraEternumUE/Source/UmbraEternumUE/Actors/UmbraCharacterCreationManager.cpp`

**Procure pela função `SpawnPlaceholdersSimple` e ajuste os caminhos:**

```cpp
case 1:
    ClassPath = TEXT("/Game/Widgets/UI/ClassSelectorItem/BP_Barbarian_Placeholder.BP_Barbarian_Placeholder_C");
    break;
case 2:
    ClassPath = TEXT("/Game/Widgets/UI/ClassSelectorItem/BP_Templar_Placeholder.BP_Templar_Placeholder_C");
    break;
// ... etc
```

**Para descobrir o caminho correto:**
1. No Unreal Editor, **Content Browser**
2. **Botão direito** no Blueprint → **Copy Reference**
3. Cole no código (adicione `_C` no final se necessário)

---

## ✅ **VANTAGENS**

- ✅ **SEM MAP** - Funciona em qualquer versão do Unreal
- ✅ **SIMPLES** - Apenas 3 arrays
- ✅ **AUTOMÁTICO** - Mapeamento feito no C++
- ✅ **FÁCIL** - Não precisa configurar nada no Blueprint

---

## 🔧 **TROUBLESHOOTING**

### **Erro: "Falha ao carregar classe"**

**Solução:**
- Verifique se o caminho do Blueprint está correto no C++
- Use **Copy Reference** no Content Browser para pegar o caminho exato
- Certifique-se de que o Blueprint está compilado

### **Erro: "ClassID não mapeado"**

**Solução:**
- Adicione o ClassID no switch case do C++
- Recompile o projeto

---

**Fim do Guia**

