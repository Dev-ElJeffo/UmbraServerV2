# 📋 **GUIA: Spawnar Placeholders em Posições Específicas**

## 🎯 **OBJETIVO**

Spawnar placeholders via código em posições específicas, permitindo controle individual de cada placeholder e respawnar exatamente como da primeira vez.

---

## ✅ **SOLUÇÃO IMPLEMENTADA**

### **Funções C++ Criadas:**

1. **`SpawnPlaceholderAtLocation`**
   - Spawna um placeholder em uma posição específica
   - Parâmetros: `PlaceholderClass`, `Location`, `Rotation`, `ClassID`
   - Retorna: `AActor*` (o placeholder spawnado)

2. **`SpawnAllPlaceholders`**
   - Spawna múltiplos placeholders usando um array de configurações
   - Parâmetro: `TArray<FPlaceholderSpawnConfig>`
   - Retorna: `int32` (número de placeholders spawnados)

### **Struct Criada:**

**`FPlaceholderSpawnConfig`** (em `UmbraDataStructures.h`):
- `ClassID` (Integer)
- `Location` (Vector)
- `Rotation` (Rotator)
- `PlaceholderClass` (Actor Class Reference)

---

## 📝 **PASSO 1: Criar Variáveis no WBP_CreateCharacter**

**My Blueprint** → **Variables**:

1. **`PlaceholderConfigs`** (Array of Placeholder Spawn Config)
   - **Category:** "Placeholders"
   - **Type:** Array                            
   - **Array Element Type:** Placeholder Spawn Config

**Esta variável armazena todas as configurações de spawn (posição, rotação, classe, ClassID) de cada placeholder.**

---

## 📝 **PASSO 2: Salvar Configurações no Event Construct**

**⚠️ IMPORTANTE:** Não use `Get Class` - ele não funciona! Vamos salvar apenas ClassID, Location e Rotation. A classe será determinada depois usando Switch.

**No `Event Construct` do `WBP_CreateCharacter`:**

```
[Event Construct]
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder (ou a classe base que todos os placeholders herdam)
    ↓
[For Each Loop]
    • Array: (resultado do Get All Actors)
    ↓ Loop Body                     
    • Array Element: (Placeholder Actor)
    ↓
[Get ClassID] (do Array Element)
    • Target: Array Element
    • Retorna: ClassID (Integer)
    ↓
[Get Actor Location]
    • Target: Array Element
    • Retorna: Location (Vector)
    ↓
[Get Actor Rotation]
    • Target: Array Element
    • Retorna: Rotation (Rotator)
    ↓
[Make Placeholder Spawn Config]
    • ClassID: (ClassID obtido acima)
    • Location: (Location obtida acima)
    • Rotation: (Rotation obtida acima)
    • PlaceholderClass: (DEIXE VAZIO/NULL por enquanto - será preenchido depois)
    ↓
[Add to Array] PlaceholderConfigs
    • Array: PlaceholderConfigs
    • New Item: (Placeholder Spawn Config criado)
```

**Como fazer no Blueprint:**

1. **Obter ClassID:**
   - Arraste do `Array Element` (que é um Actor)
   - Digite: `Get ClassID` ou `Get` → Procure pela propriedade `ClassID`
   - **Se não aparecer:** O placeholder precisa ter uma função `Get ClassID` ou uma propriedade `ClassID` exposta

2. **Obter Location:**
   - Arraste do `Array Element`
   - Digite: `Get Actor Location`

3. **Obter Rotation:**
   - Arraste do `Array Element`
   - Digite: `Get Actor Rotation`

4. **Make Placeholder Spawn Config:**
   - Digite: `Make Placeholder Spawn Config`
   - Conecte: ClassID, Location, Rotation
   - **PlaceholderClass:** Deixe desconectado (será preenchido depois)

5. **Add to Array:**
   - Digite: `Add to Array`
   - Array: `PlaceholderConfigs`
   - New Item: (Placeholder Spawn Config)

---

## 📝 **PASSO 3: Criar Função (Blueprint)**

**My Blueprint** → **Functions** → **+ Function**:

- **Nome:** `SpawnAllPlaceholders`
- **Inputs:** Nenhum (usa o array `PlaceholderConfigs`)

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
[Get] PlaceholderConfigs
    ↓
[Get Array Length]
    • Array: PlaceholderConfigs
    ↓
[Greater] Length > 0
    ↓
[Branch] (True)
    ↓
[For Each Loop]
    • Array: PlaceholderConfigs
    ↓ Loop Body (exec)
    • Array Element: (Placeholder Spawn Config)
    ↓
[Break Placeholder Spawn Config]
    • Struct: Array Element
    • Retorna: ClassID, Location, Rotation, PlaceholderClass
    ↓
[Switch on Int]
    • Selection: ClassID (do Break)
    • Exec: (conecta do Loop Body)
    ↓
    ├─ [Case 1] (exec) → [Make Placeholder Spawn Config] (SEM exec - é puro)
    │                      • ClassID: (ClassID do Break)
    │                      • Location: (Location do Break)
    │                      • Rotation: (Rotation do Break)
    │                      • PlaceholderClass: BP_Barbarian_Placeholder
    │                      ↓
    │                  [Add to Array] TempConfigs
    │                      • Array: TempConfigs
    │                      • New Item: (Placeholder Spawn Config do Make)
    │                      • Exec: (conecta do Case 1)
    │
    ├─ [Case 2] (exec) → [Make Placeholder Spawn Config] (SEM exec - é puro)
    │                      • ClassID: (ClassID do Break)
    │                      • Location: (Location do Break)
    │                      • Rotation: (Rotation do Break)
    │                      • PlaceholderClass: BP_Templar_Placeholder
    │                      ↓
    │                  [Add to Array] TempConfigs
    │                      • Array: TempConfigs
    │                      • New Item: (Placeholder Spawn Config do Make)
    │                      • Exec: (conecta do Case 2)
    │
    ├─ [Case 3] (exec) → [Make Placeholder Spawn Config] (SEM exec - é puro)
    │                      • ClassID: (ClassID do Break)
    │                      • Location: (Location do Break)
    │                      • Rotation: (Rotation do Break)
    │                      • PlaceholderClass: BP_Assassin_Placeholder
    │                      ↓
    │                  [Add to Array] TempConfigs
    │                      • Array: TempConfigs
    │                      • New Item: (Placeholder Spawn Config do Make)
    │                      • Exec: (conecta do Case 3)
    │
    ├─ [Case 4] (exec) → [Make Placeholder Spawn Config] (SEM exec - é puro)
    │                      • PlaceholderClass: BP_Mage_Placeholder
    │                      ↓
    │                  [Add to Array] TempConfigs
    │
    ├─ [Case 5] (exec) → [Make Placeholder Spawn Config] (SEM exec - é puro)
    │                      • PlaceholderClass: BP_Ranger_Placeholder
    │                      ↓
    │                  [Add to Array] TempConfigs
    │
    ├─ [Case 6] (exec) → [Make Placeholder Spawn Config] (SEM exec - é puro)
    │                      • PlaceholderClass: BP_Paladin_Placeholder
    │                      ↓
    │                  [Add to Array] TempConfigs
    │
    ... (adicione um Case para cada ClassID que você tem)
    ↓ (TODOS os "Add to Array" de cada Case conectam no "Completed" do Loop)
[For Each Loop] (Completed - exec)
    ↓ (após o Loop terminar)
[Get] Manager
    ↓
[Call Function: Spawn All Placeholders]
    • Target: Manager
    • Placeholder Configs: TempConfigs
    • Return Value: SpawnedCount
    ↓
[Print String] (opcional, para debug)
    • In String: "Placeholders spawnados: " + (SpawnedCount como String)
```

**⚠️ IMPORTANTE - Como fazer o Switch corretamente:**

1. **Criar variável temporária:**
   - **`TempConfigs`** (Array of Placeholder Spawn Config)
   - Esta variável será usada apenas dentro da função

2. **Break Placeholder Spawn Config:**
   - Arraste do `Array Element`
   - Digite: `Break Placeholder Spawn Config`
   - Isso separa os campos: ClassID, Location, Rotation, PlaceholderClass

3. **Switch on Int:**
   - Digite: `Switch on Int`
   - Conecte o **ClassID** (do Break) no pin **Selection**
   - Conecte o **Loop Body** (exec) no pin **Exec** do Switch
   - **Adicione Cases:** Clique com botão direito no Switch → **Add Pin** → Repita para cada ClassID que você tem
   - **Configure cada Case:** Clique diretamente no **número** que aparece no Case (ex: "0", "1", "2") e digite o ClassID correto (1, 2, 3, etc.)
   - **OU:** Selecione o pin do Case e no painel **Details** (à direita), mude o campo **Value** para o ClassID correto

4. **Em cada Case do Switch:**
   - **Case 1 (exec):** Conecta em `Make Placeholder Spawn Config` (NÃO tem exec - é função pura)
   - **Case 2 (exec):** Conecta em `Make Placeholder Spawn Config` (NÃO tem exec - é função pura)
   - **Case 3 (exec):** Conecta em `Make Placeholder Spawn Config` (NÃO tem exec - é função pura)
   - E assim por diante...

5. **Make Placeholder Spawn Config em cada Case:**
   - **NÃO conecte exec** - é função pura (sem entrada exec)
   - Use os valores do **Break**: ClassID, Location, Rotation
   - Para **PlaceholderClass**, arraste o Blueprint correto (ex: `BP_Barbarian_Placeholder`) do Content Browser ou use uma variável
   - A saída do `Make Placeholder Spawn Config` conecta no **New Item** do `Add to Array`

6. **Add to Array em cada Case:**
   - Cada Case do Switch precisa ter seu próprio `Add to Array`
   - O **Exec** do Case conecta no **Exec** do `Add to Array`
   - O **New Item** do `Add to Array` recebe o `Placeholder Spawn Config` do `Make`
   - O **Array** do `Add to Array` é sempre `TempConfigs` (a mesma variável)
   - A saída **Exec** de cada `Add to Array` conecta no **Completed** do `For Each Loop`

7. **Após o Loop terminar:**
   - O **Completed** do Loop (exec) conecta em `Get Manager`
   - Chame `Spawn All Placeholders` do Manager com o array `TempConfigs` completo

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
[Reset Character Creation]
    • Target: Manager
    • Create Character Widget: Self
    • Placeholder Class: BP_Class_Placeholder (classe base)
    • Spacing: 300.0 (não importa, não será usado)
    ↓
[Call Function: SpawnAllPlaceholders]
    • Target: Self
```

---

## 🔍 **EXEMPLO VISUAL DO SWITCH**

**Estrutura correta do Switch (COM EXEC):**

```
[For Each Loop] (Loop Body - exec)
    ↓
[Break Placeholder Spawn Config] (SEM exec - é puro)
    ├─ ClassID: 1
    ├─ Location: (100, 0, 50)
    ├─ Rotation: (0, 90, 0)
    └─ PlaceholderClass: (vazio)
    ↓
[Switch on Int] (Exec: Loop Body, Selection: ClassID)
    ↓
    ├─ [Case 1] (exec) → [Make Placeholder Spawn Config] (SEM exec - é puro)
    │                      ClassID: 1
    │                      Location: (100, 0, 50)
    │                      Rotation: (0, 90, 0)
    │                      PlaceholderClass: BP_Barbarian_Placeholder
    │                      ↓ (saída do Make)
    │                  [Add to Array] TempConfigs
    │                      Exec: (Case 1 exec)
    │                      Array: TempConfigs
    │                      New Item: (Placeholder Spawn Config do Make)
    │                      ↓ (exec de saída)
    │
    ├─ [Case 2] (exec) → [Make Placeholder Spawn Config] (SEM exec - é puro)
    │                      ClassID: 2
    │                      Location: (100, 0, 50)
    │                      Rotation: (0, 90, 0)
    │                      PlaceholderClass: BP_Templar_Placeholder
    │                      ↓ (saída do Make)
    │                  [Add to Array] TempConfigs
    │                      Exec: (Case 2 exec)
    │                      Array: TempConfigs
    │                      New Item: (Placeholder Spawn Config do Make)
    │                      ↓ (exec de saída)
    │
    └─ [Default] (exec) → (não deve acontecer se todos os ClassIDs estão corretos)
    ↓ (TODOS os "Add to Array" conectam aqui)
[For Each Loop] (Completed - exec)
    ↓
[Get Manager] (exec)
```

**⚠️ LEMBRE-SE:**
- `Make Placeholder Spawn Config` **NÃO TEM ENTRADA EXEC** (é função pura)
- Cada Case do Switch tem **saída exec** que conecta no **exec** do `Add to Array`
- Cada Case precisa ter seu próprio `Add to Array`
- Todos os `Add to Array` adicionam ao mesmo array `TempConfigs`
- Todos os `Add to Array` conectam no **Completed** do Loop

---

## ✅ **VANTAGENS DESTA SOLUÇÃO**

- ✅ Controle individual de posição de cada placeholder
- ✅ Permite ajustes específicos (não espaçamento fixo)
- ✅ Respawna exatamente como da primeira vez
- ✅ Mais flexível que espaçamento automático
- ✅ Funciona perfeitamente com `ResetCharacterCreation`

---

## 📋 **RESUMO DO FLUXO**

1. **Event Construct:** Salva ClassID, Location e Rotation de todos os placeholders em `PlaceholderConfigs` (sem PlaceholderClass)
2. **SelectClassAndMoveCamera:** Destrói placeholders (exceto o selecionado)
3. **BTN_Return OnClicked:** Chama `ResetCharacterCreation` → Chama `SpawnAllPlaceholders`
4. **SpawnAllPlaceholders:** 
   - Itera sobre `PlaceholderConfigs`
   - Usa Switch on Int baseado no ClassID para determinar qual PlaceholderClass usar
   - Cria array temporário `TempConfigs` com todas as configurações completas
   - Chama `Spawn All Placeholders` do Manager com `TempConfigs`

---

## ⚠️ **NOTAS IMPORTANTES**

1. **NÃO use `Get Class`** - ele não funciona para Actor Class Reference
2. **Use Switch on Int** baseado no ClassID para determinar a classe
3. **Todos os Cases do Switch** devem conectar no mesmo `Add to Array`
4. **Crie uma variável temporária** `TempConfigs` dentro da função `SpawnAllPlaceholders`
5. **Certifique-se de ter um Case** para cada ClassID que você usa

---

**Fim do Guia**
