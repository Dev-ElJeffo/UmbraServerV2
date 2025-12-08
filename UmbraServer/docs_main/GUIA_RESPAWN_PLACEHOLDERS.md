# 📋 **GUIA: Respawnar Placeholders Após Reset**

## ❌ **PROBLEMA**

Após clicar em "Return to Selection":
- ✅ Placeholders são destruídos corretamente
- ❌ Placeholders NÃO são respawnados
- ❌ Não é possível selecionar uma classe novamente (placeholders não existem)

**Log de erro:**
```
LogTemp: Error: [UmbraCharacterCreationManager] ❌❌❌ Placeholder não encontrado para ClassID X
```

---

## 🎯 **SOLUÇÃO**

Criar uma função no Blueprint que respawna todos os placeholders nas posições originais após o reset.

---

## 📝 **PASSO 1: Criar Função SpawnAllPlaceholders no WBP_CreateCharacter**

### **1.1. Criar a Função**

1. Abra `WBP_CreateCharacter`
2. **My Blueprint** → **Functions** → **+ Function**
3. **Nome:** `SpawnAllPlaceholders`
4. **Inputs:**
   - `PlaceholderClass` (Actor Class Reference) - Classe base dos placeholders (ex: `BP_Class_Placeholder`)
   - `Spacing` (Float) - Espaçamento entre placeholders (padrão: 300.0)

### **1.2. Implementar a Função**

**Estrutura básica:**

```
[SpawnAllPlaceholders]
    • PlaceholderClass (input)
    • Spacing (input)
    ↓
[Get All Actors of Class]
    • Actor Class: BP_CharacterCreationManager
    ↓
[Get] (primeiro elemento do array)
    ↓
[Is Valid]
    ↓
[Branch] (True)
    ↓
[Get World]
    ↓
[For Each Loop] (Classes Array)
    • Array: [Array de classes - você precisa criar isso]
    ↓ Loop Body
    • Array Element: (FUmbraClassData ou Integer ClassID)
    ↓
[Break Struct] (se for struct) ou [Get] ClassID
    ↓
[Get Class Placeholder Class] (função que retorna a classe do placeholder baseado no ClassID)
    ↓
[Calculate Position] (baseado no índice do loop e Spacing)
    ↓
[Spawn Actor]
    • Actor Class: (classe do placeholder)
    • Location: (posição calculada)
    • Rotation: (0, 0, 0)
    ↓
[Set ClassID] (no placeholder spawnado)
    • ClassID: (ClassID do loop)
```

---

## 📝 **PASSO 2: Método Simples (Recomendado) - Usar Array de Posições**

### **2.1. Criar Variáveis no WBP_CreateCharacter**

**My Blueprint** → **Variables**:

1. **`PlaceholderPositions`** (Array of Vector)
   - **Category:** "Placeholders"
   - **Default Value:** Array vazio (será preenchido no Event Construct)

2. **`PlaceholderClasses`** (Array of Actor Class Reference)
   - **Category:** "Placeholders"
   - **Default Value:** Array vazio (será preenchido no Event Construct)

3. **`PlaceholderClassIDs`** (Array of Integer)
   - **Category:** "Placeholders"
   - **Default Value:** Array vazio (será preenchido no Event Construct)

### **2.2. Preencher Arrays no Event Construct**

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
[Get ClassID] (do placeholder - via função ou propriedade)
    ↓
[Add to Array] PlaceholderClassIDs
    • Array: PlaceholderClassIDs
    • New Item: (ClassID)
    ↓
[Get Class] (do Array Element)
    ↓
[Add to Array] PlaceholderClasses
    • Array: PlaceholderClasses
    • New Item: (Class)
```

**⚠️ IMPORTANTE:** Faça isso ANTES de qualquer placeholder ser destruído (no Event Construct, quando o level inicia).

### **2.3. Implementar SpawnAllPlaceholders**

```
[SpawnAllPlaceholders]
    • PlaceholderClass (input - não usado, mas mantido para compatibilidade)
    • Spacing (input - não usado, mas mantido para compatibilidade)
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
[Get] PlaceholderClasses
    ↓
[Get] (Array Index)
    • Array: PlaceholderClasses
    • Index: Array Index
    ↓
[Is Valid]
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
    • Actor Class: (classe do PlaceholderClasses[Index])
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

---

## 📝 **PASSO 3: Chamar SpawnAllPlaceholders Após Reset**

### **3.1. Modificar o Botão Return**

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
    • Placeholder Class: BP_Class_Placeholder
    • Spacing: 300.0
```

---

## 📝 **PASSO 4: Método Alternativo - Usar Level Blueprint**

### **4.1. Se Preferir Fazer no Level Blueprint**

**No Level Blueprint, criar função `SpawnAllPlaceholders`:**

1. **Criar variáveis no Level Blueprint:**
   - `PlaceholderPositions` (Array of Vector)
   - `PlaceholderClasses` (Array of Actor Class Reference)
   - `PlaceholderClassIDs` (Array of Integer)

2. **No Event BeginPlay, salvar posições originais:**

```
[Event BeginPlay]
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder
    ↓
[For Each Loop]
    • Array: (resultado)
    ↓
[Salvar posições, classes e IDs nos arrays]
```

3. **Criar função `SpawnAllPlaceholders` no Level Blueprint** (mesma lógica do Passo 2.3)

4. **Chamar após ResetCharacterCreation:**

```
[Reset Character Creation]
    ↓
[Call Function: SpawnAllPlaceholders]
    • Target: (Level Blueprint)
```

---

## 📝 **PASSO 5: Método Mais Simples - Respawnar Diretamente no C++**

### **5.1. Modificar ResetCharacterCreation no C++**

**Adicionar função para respawnar placeholders no C++:**

```cpp
void AUmbraCharacterCreationManager::SpawnAllPlaceholders(TSubclassOf<AActor> PlaceholderClass, float Spacing)
{
    // Implementar lógica de spawn aqui
    // Similar ao SpawnAllCharacterPreviews do UmbraCharacterSelectionManager
}
```

**E chamar no final de `ResetCharacterCreation`:**

```cpp
// Respawnar todos os placeholders
if (PlaceholderClass && IsValid(GetWorld()))
{
    SpawnAllPlaceholders(PlaceholderClass, Spacing);
}
```

**⚠️ NOTA:** Este método requer saber as posições originais dos placeholders, que podem ser salvas no BeginPlay ou passadas como parâmetro.

---

## ✅ **SOLUÇÃO RECOMENDADA (Mais Simples)**

### **Método: Salvar Posições no Event Construct e Respawnar**

1. **No `Event Construct` do `WBP_CreateCharacter`:**
   - Salvar posições, classes e IDs de todos os placeholders em arrays

2. **Criar função `SpawnAllPlaceholders`:**
   - Loop através dos arrays
   - Spawnar cada placeholder na posição salva
   - Setar ClassID correto

3. **Chamar após `ResetCharacterCreation`:**
   - Adicionar chamada a `SpawnAllPlaceholders` no botão Return

---

## 🔍 **VERIFICAÇÃO**

Após implementar, teste:

1. ✅ Selecione uma classe → Deve funcionar
2. ✅ Clique em "Return to Selection" → Placeholders devem ser destruídos
3. ✅ Placeholders devem ser respawnados automaticamente
4. ✅ Selecione outra classe → Deve funcionar novamente

**Se os placeholders não aparecerem:**
- Verifique se as posições foram salvas corretamente
- Verifique se os ClassIDs estão corretos
- Verifique os logs do Output Log

---

**Fim do Guia**

