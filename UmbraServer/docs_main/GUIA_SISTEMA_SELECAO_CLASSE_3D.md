# 🎮 **GUIA COMPLETO: Sistema de Seleção de Classe com Placeholders 3D**

## 📋 **ÍNDICE**

1. [Visão Geral](#visão-geral)
2. [Pré-requisitos](#pré-requisitos)
3. [Passo 1: Criar Blueprint Pai BP_Class_Placeholder](#passo-1-criar-blueprint-pai-bp_class_placeholder)
4. [Passo 2: Criar Blueprints Filhos para Cada Classe](#passo-2-criar-blueprints-filhos-para-cada-classe)
5. [Passo 3: Configurar Level e Placeholders](#passo-3-configurar-level-e-placeholders)
6. [Passo 4: Implementar Sistema de Click e Seleção](#passo-4-implementar-sistema-de-click-e-seleção)
7. [Passo 5: Implementar Movimento de Câmera](#passo-5-implementar-movimento-de-câmera)
8. [Passo 6: Atualizar Widget de Criação de Personagem](#passo-6-atualizar-widget-de-criação-de-personagem)
9. [Passo 7: Testar Implementação](#passo-7-testar-implementação)
10. [Troubleshooting](#troubleshooting)

---

## 🎯 **VISÃO GERAL**

Este guia implementa um sistema de seleção de classe interativo onde:

- ✅ **Placeholders 3D** representam cada classe no level
- ✅ **Click no personagem 3D** seleciona a classe
- ✅ **Outros placeholders são despawnados** quando uma classe é selecionada
- ✅ **Câmera se move** para mostrar apenas o personagem selecionado à direita da tela
- ✅ **Widget de informações** aparece mostrando apenas os dados da classe selecionada
- ✅ **Campos de customização** (Hair, Head, Nome) aparecem após seleção

---

## ✅ **PRÉ-REQUISITOS**

Antes de começar, certifique-se de que:

- ✅ Unreal Engine 5 está instalado e configurado
- ✅ Projeto `UmbraEternumUE` está aberto no Unreal Editor
- ✅ **C++ foi recompilado** (as novas funções foram adicionadas)
- ✅ Level `Lvl_Character_Creation` existe e tem os placeholders posicionados
- ✅ Widget `WBP_CreateCharacter` já existe (ou será criado)
- ✅ Sistema de classes já está implementado (API e C++)

### **⚠️ IMPORTANTE: Recompilar C++**

Antes de começar a implementação no Blueprint, você **DEVE** recompilar o projeto C++:

1. **Feche o Unreal Editor completamente**
2. **Abra o Visual Studio** (ou seu IDE)
3. **Compile o projeto** (Build → Build Solution)
4. **Aguarde a compilação terminar**
5. **Abra o Unreal Editor novamente**

**Novas funções C++ adicionadas:**
- ✅ `SelectClass(int32 ClassID)` - Seleciona uma classe e dispara o delegate
- ✅ `GetClassDataByID(int32 ClassID, FUmbraClassData& OutClassData)` - Obtém dados de uma classe específica
- ✅ `GetSelectedClassData()` - Obtém dados da classe selecionada
- ✅ `GetSelectedClassID()` - Obtém ID da classe selecionada
- ✅ `ClearClassSelection()` - Limpa a seleção de classe
- ✅ Delegate `OnClassSelected` - Disparado quando uma classe é selecionada

---

## 🔨 **PASSO 1: CRIAR BLUEPRINT PAI BP_Class_Placeholder**

### **1.1. Criar o Blueprint Base**

1. No **Content Browser**, navegue até: `Content/Blueprints/` (ou crie a pasta se não existir)
2. Clique com botão direito → **Blueprint Class**
3. Selecione **Actor** como classe pai
4. Nomeie como: `BP_Class_Placeholder`
5. Abra o Blueprint para edição

### **1.2. Adicionar Componentes**

No **Viewport** do Blueprint, adicione os seguintes componentes:

**Hierarquia de Componentes:**
```
BP_Class_Placeholder (Root)
├── Static Mesh Component (ou Skeletal Mesh Component)
│   └── Nome: "Mesh_Character"
├── Box Collision
│   └── Nome: "Collision_Box"
└── Widget Component (Opcional - para mostrar nome da classe)
    └── Nome: "Widget_Name"
```

**Passo a Passo:**

1. **Adicionar Static/Skeletal Mesh:**
   - No painel **Components**, clique em **Add Component**
   - Selecione **Static Mesh Component** (ou **Skeletal Mesh Component** se usar esqueleto)
   - Nomeie como: `Mesh_Character`
   - No **Details**, configure:
     - **Static Mesh:** (selecione o mesh do placeholder)
     - **Location:** (0, 0, 0)
     - **Scale:** (1, 1, 1)

2. **Adicionar Box Collision:**
   - Clique em **Add Component** → **Box Collision**
   - Nomeie como: `Collision_Box`
   - No **Details**, configure:
     - **Box Extent:** (50, 50, 100) - ajuste conforme necessário
     - **Location:** (0, 0, 50) - centralizado no personagem

3. **Adicionar Widget Component (Opcional):**
   - Clique em **Add Component** → **Widget Component**
   - Nomeie como: `Widget_Name`
   - No **Details**, configure:
     - **Widget Class:** (criar um widget simples para mostrar nome)
     - **Draw Size:** (200, 50)
     - **Location:** (0, 0, 200) - acima do personagem

### **1.3. Adicionar Variáveis**

No painel **My Blueprint** → **Variables**, adicione:

**Variáveis Essenciais:**

1. **`ClassID`** (Integer)
   - **Category:** "Class Data"
   - **Default Value:** 0
   - **Tooltip:** "ID da classe que este placeholder representa"

2. **`ClassData`** (Umbra Class Data)
   - **Category:** "Class Data"
   - **Default Value:** (vazio)
   - **Tooltip:** "Dados completos da classe"

3. **`bIsSelected`** (Boolean)
   - **Category:** "State"
   - **Default Value:** false
   - **Tooltip:** "Se este placeholder está selecionado"

4. **`OnClassSelected`** (Event Dispatcher)
   - **Category:** "Events"
   - **Type:** Custom Event com parâmetro `ClassID` (Integer)
   - **Tooltip:** "Disparado quando este placeholder é clicado"

### **1.4. Implementar Lógica de Click**

No **Event Graph**, adicione:

**Event BeginPlay:**
```
[Event BeginPlay]
    ↓
[Enable Input]
    • Player Controller: Get Player Controller (0)
    ↓
[Set Input Mode Game Only]
    • Target: Get Player Controller (0)
```

**Event OnClicked (do Collision Box):**
```
[OnClicked] (do Collision_Box)
    ↓
[Get] ClassID
    ↓
[Call Function: SelectClass]
    • Target: (Self)
    • Class ID: ClassID
```

**Função SelectClass:**
Crie uma função **Custom Function** chamada `SelectClass`:

**Inputs:**
- `ClassID` (Integer)

**Lógica:**
```
[SelectClass]
    ↓
[SET] bIsSelected = true
    ↓
[Broadcast] OnClassSelected
    • ClassID: ClassID (input)
```

### **1.5. Adicionar Função SetClassData**

Crie uma função **Custom Function** chamada `SetClassData`:

**Inputs:**
- `NewClassData` (Umbra Class Data)

**Lógica:**
```
[SetClassData]
    ↓
[SET] ClassData = NewClassData (input)
    ↓
[SET] ClassID = ClassData.ClassID
    ↓
[Update Visual] (se necessário - atualizar mesh, cor, etc.)
```

---

## 🎨 **PASSO 2: CRIAR BLUEPRINTS FILHOS PARA CADA CLASSE**

### **2.1. Criar Blueprint para Barbarian**

1. No **Content Browser**, clique com botão direito em `BP_Class_Placeholder`
2. Selecione **Create Child Blueprint Class**
3. Nomeie como: `BP_Barbarian_Placeholder`
4. Abra o Blueprint

**Configurações Específicas:**

1. **No Viewport:**
   - Selecione `Mesh_Character`
   - No **Details**, configure:
     - **Static Mesh:** (selecione o mesh do Barbarian)
     - **Materials:** (configure materiais se necessário)

2. **No Event Graph:**
   - No **Event BeginPlay**, adicione:
   ```
   [Event BeginPlay]
       ↓
   [Parent: BeginPlay]
       ↓
   [SET] ClassID = 1 (ou o ID correto do Barbarian no banco)
   ```

3. **Repita para cada classe:**
   - `BP_Templar_Placeholder` → ClassID = 2
   - `BP_DarkMage_Placeholder` → ClassID = 3
   - `BP_Cleric_Placeholder` → ClassID = 4
   - `BP_Assassin_Placeholder` → ClassID = 5
   - `BP_Monk_Placeholder` → ClassID = 6

**⚠️ IMPORTANTE:** Verifique os IDs corretos no banco de dados executando:
```sql
SELECT class_id, class_name FROM classes ORDER BY class_id;
```

---

## 🗺️ **PASSO 3: CONFIGURAR LEVEL E PLACEHOLDERS**

### **3.1. Posicionar Placeholders no Level**

1. Abra o level `Lvl_Character_Creation`
2. No **World Outliner**, verifique se os placeholders existem:
   - `Assassin_Placeholder`
   - `Barbarian_F_Placeholder` (ou `Barbarian_Placeholder`)
   - `Cleric_Placeholder`
   - `DarkMage`
   - `Monk_Placeholder`
   - (adicione `Templar_Placeholder` se não existir)

3. **Substituir Placeholders por Blueprints:**
   - Selecione cada placeholder no level
   - No **Details**, clique em **Change Class** (ou delete e adicione novo)
   - Selecione o Blueprint correspondente:
     - `Assassin_Placeholder` → `BP_Assassin_Placeholder`
     - `Barbarian_Placeholder` → `BP_Barbarian_Placeholder`
     - `Cleric_Placeholder` → `BP_Cleric_Placeholder`
     - `DarkMage` → `BP_DarkMage_Placeholder`
     - `Monk_Placeholder` → `BP_Monk_Placeholder`
     - (adicione `BP_Templar_Placeholder` se necessário)

4. **Posicionar Placeholders:**
   - Organize os placeholders em uma linha ou grid
   - Espaçamento recomendado: 200-300 unidades entre cada um
   - Altura: todos na mesma altura (Z = 0 ou altura do chão)

### **3.2. Configurar Câmera**

1. No **World Outliner**, selecione `CameraActor`
2. No **Details**, configure:
   - **Location:** Posição inicial (ex: X: -200, Y: -540, Z: 330)
   - **Rotation:** (0, 0, 0) ou ajuste conforme necessário
   - **Field Of View:** 121.68 (ou ajuste conforme necessário)

3. **Criar Blueprint de Câmera (Opcional mas Recomendado):**
   - Crie um Blueprint chamado `BP_CharacterCreationCamera`
   - Adicione um **Camera Component**
   - Adicione funções para:
     - `MoveToSelectionPosition` (mover para posição de seleção)
     - `MoveToInitialPosition` (voltar para posição inicial)

---

## 🖱️ **PASSO 4: IMPLEMENTAR SISTEMA DE CLICK E SELEÇÃO**

### **4.1. Criar Game Mode ou Player Controller Customizado**

**Opção 1: Usar Game Mode Existente**

1. Crie ou edite o **Game Mode** usado no level `Lvl_Character_Creation`
2. Adicione variáveis:
   - `SelectedClassID` (Integer)
   - `SelectedPlaceholder` (BP_Class_Placeholder Reference)
   - `AllPlaceholders` (Array of BP_Class_Placeholder Reference)

**Opção 2: Criar Blueprint de Gerenciador**

1. Crie um Blueprint chamado `BP_CharacterCreationManager`
2. Classe pai: **Actor**
3. Adicione variáveis:
   - `SelectedClassID` (Integer)
   - `SelectedPlaceholder` (BP_Class_Placeholder Reference)
   - `AllPlaceholders` (Array of BP_Class_Placeholder Reference)
   - `CameraActor` (Camera Actor Reference)

### **4.2. Implementar Lógica de Seleção**

No **Event Graph** do `BP_CharacterCreationManager` (ou Game Mode):

**Event BeginPlay:**
```
[Event BeginPlay]
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder
    ↓
[SET] AllPlaceholders = (resultado do Get All Actors)
    ↓
[For Each Loop]
    • Array: AllPlaceholders
    ↓ Loop Body
    • Array Element: (BP_Class_Placeholder)
    ↓
[Assign] OnClassSelected (do Array Element)
    • Target: Array Element
    • Event: OnClassSelected_Handler (criar este evento)
```

**Event OnClassSelected_Handler:**
Crie um **Custom Event** chamado `OnClassSelected_Handler`:

**Inputs:**
- `ClassID` (Integer)

**Lógica:**
```
[OnClassSelected_Handler]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[Select Class]
    • Target: (cast result)
    • Class ID: ClassID (input)
    ↓
[SET] SelectedClassID = ClassID (input)
    ↓
[Find Placeholder by ClassID]
    • ClassID: ClassID
    ↓
[SET] SelectedPlaceholder = (resultado)
    ↓
[Call Function: DespawnOtherPlaceholders]
    • Target: (Self)
    • Keep Placeholder: SelectedPlaceholder
    ↓
[Call Function: MoveCameraToSelection]
    • Target: (Self)
    • Target Placeholder: SelectedPlaceholder
    ↓
[Call Function: ShowClassInfoWidget]
    • Target: (Self)
    • ClassID: ClassID
```

**📌 NOTA:** A função `SelectClass` do Game Instance já dispara o delegate `OnClassSelected` automaticamente. Você pode conectar esse delegate diretamente no widget ou no manager.

**Função DespawnOtherPlaceholders:**
Crie uma função **Custom Function** chamada `DespawnOtherPlaceholders`:

**Inputs:**
- `KeepPlaceholder` (BP_Class_Placeholder Reference)

**Lógica:**
```
[DespawnOtherPlaceholders]
    ↓
[For Each Loop]
    • Array: AllPlaceholders
    ↓ Loop Body
    • Array Element: (BP_Class_Placeholder)
    ↓
[Branch]
    • Condition: Array Element != KeepPlaceholder
    ↓ (True)
    [Destroy Actor]
        • Target: Array Element
    ↓ (False)
    [Do Nothing]
```

**Função FindPlaceholderByClassID:**
Crie uma função **Custom Function** chamada `FindPlaceholderByClassID`:

**Inputs:**
- `ClassID` (Integer)

**Outputs:**
- `FoundPlaceholder` (BP_Class_Placeholder Reference)

**Lógica:**
```
[FindPlaceholderByClassID]
    ↓
[For Each Loop]
    • Array: AllPlaceholders
    ↓ Loop Body
    • Array Element: (BP_Class_Placeholder)
    ↓
[Get] ClassID (do Array Element)
    ↓
[Branch]
    • Condition: ClassID == ClassID (input)
    ↓ (True)
    [SET] FoundPlaceholder = Array Element
    [Break]
    ↓ (False)
    [Continue Loop]
```

### **4.3. Conectar Event Dispatcher nos Placeholders**

No Blueprint `BP_Class_Placeholder`, atualize a função `SelectClass`:

```
[SelectClass]
    ↓
[SET] bIsSelected = true
    ↓
[Broadcast] OnClassSelected
    • ClassID: ClassID
    ↓
[Call Function: HighlightSelected] (opcional - visual feedback)
    • Target: (Self)
```

---

## 📹 **PASSO 5: IMPLEMENTAR MOVIMENTO DE CÂMERA**

### **5.1. Usar Função C++ MoveCameraToSelection**

**✅ SOLUÇÃO C++ (RECOMENDADA):**

A classe `AUmbraCharacterCreationManager` já possui a função `MoveCameraToSelection` implementada em C++.

**Como usar no Blueprint:**

1. **Certifique-se de que seu `BP_CharacterCreationManager` herda de `Umbra Character Creation Manager`** (classe C++)

2. **No Event Graph, quando selecionar uma classe:**

```
[OnClassSelected_Handler]
    • ClassID (input)
    ↓
[Find Placeholder By Class ID]
    • ClassID: ClassID
    • Found Placeholder: (variável local)
    ↓
[Get] CameraActor
    ↓
[Call Function: Move Camera To Selection]
    • Target: (Self - BP_CharacterCreationManager)
    • Target Placeholder: Found Placeholder
    • Camera Actor: CameraActor
    • Duration: 1.0 (padrão: 1 segundo)
    • Offset X: 200.0 (padrão)
    • Offset Z: 100.0 (padrão)
```

**A função C++ faz:**
- ✅ Interpolação suave em 1 segundo (configurável)
- ✅ Curva de easing (Ease In Out)
- ✅ Timer interno (sem Tick, sem Timeline)
- ✅ Para automaticamente ao chegar ao destino

**Veja o guia completo:** `GUIA_USAR_MOVER_CAMERA_CPP.md` para detalhes de implementação.

### **5.2. Calcular Posição da Câmera**

A posição da câmera deve mostrar o personagem à **direita da tela**. Use:

**Cálculo de Posição:**
- **X:** `PlaceholderLocation.X + 200` (200 unidades à direita)
- **Y:** `PlaceholderLocation.Y` (mesma posição Y)
- **Z:** `PlaceholderLocation.Z + 100` (100 unidades acima)

**Rotação da Câmera:**
- **Pitch:** -10 a -20 graus (olhando ligeiramente para baixo)
- **Yaw:** -90 graus (olhando para a esquerda, em direção ao personagem)
- **Roll:** 0

**Implementação:**
```
[MoveCameraToSelection]
    ↓
[Get Actor Location] (do TargetPlaceholder)
    ↓
[Break Vector]
    • Vector: (Location)
    ↓
[Make Vector]
    • X: (Break.X + 200)
    • Y: (Break.Y)
    • Z: (Break.Z + 100)
    ↓
[Make Rotator]
    • Pitch: -15
    • Yaw: -90
    • Roll: 0
    ↓
[Lerp Transform] (usando Timeline para animação)
    • A: (transform atual da câmera)
    • B: (novo transform)
    • Alpha: (do Timeline)
    ↓
[Set Actor Transform]
    • Target: CameraActor
    • New Transform: (resultado)
```

---

## 🎨 **PASSO 6: ATUALIZAR WIDGET DE CRIAÇÃO DE PERSONAGEM**

### **6.1. Modificar WBP_CreateCharacter**

**Estrutura do Widget (Designer):**

```
Canvas Panel (Root)
├── Border_Background (fundo escuro/semi-transparente)
├── Vertical Box: "VBox_Main"
│   ├── Text Block: "TXT_Title" ("Selecione uma Classe")
│   ├── Horizontal Box: "HBox_Content"
│   │   ├── [ESPAÇO PARA VIEWPORT 3D - opcional]
│   │   └── Vertical Box: "VBox_ClassInfo"
│   │       ├── Text Block: "TXT_ClassName"
│   │       ├── Text Block: "TXT_ClassDescription"
│   │       ├── Scroll Box: "ScrollBox_Stats"
│   │       │   └── Vertical Box: "VBox_Stats"
│   │       │       ├── Text Block: "TXT_BaseStats"
│   │       │       ├── Text Block: "TXT_BaseResources"
│   │       │       └── Text Block: "TXT_BaseCombat"
│   │       ├── Horizontal Box: "HBox_Customization"
│   │       │   ├── Text Block: "TXT_HairLabel" ("Hair:")
│   │       │   ├── Spin Box: "SPIN_Hair" (Integer, Min: 0, Max: 10)
│   │       │   ├── Text Block: "TXT_HeadLabel" ("Head:")
│   │       │   └── Spin Box: "SPIN_Head" (Integer, Min: 0, Max: 10)
│   │       ├── Text Block: "TXT_NameLabel" ("Nome:")
│   │       ├── Editable Text Box: "TXT_Name"
│   │       └── Button: "BTN_CreateCharacter"
│   └── Button: "BTN_Back" (voltar para seleção)
```

### **6.2. Adicionar Variáveis ao Widget**

No **My Blueprint** → **Variables**, adicione:

1. **`SelectedClassID`** (Integer)
   - **Category:** "Selection"
   - **Default Value:** 0

2. **`SelectedClassData`** (Umbra Class Data)
   - **Category:** "Selection"
   - **Default Value:** (vazio)

3. **`MyGameInstance`** (Umbra Game Instance Reference)
   - **Category:** "References"
   - **Default Value:** (vazio)

4. **`CharacterCreationManager`** (BP_CharacterCreationManager Reference)
   - **Category:** "References"
   - **Default Value:** (vazio)

### **6.3. Implementar Event Construct**

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[SET] MyGameInstance = (cast result)
    ↓
[Get All Actors of Class]
    • Actor Class: BP_CharacterCreationManager
    ↓
[Get] (primeiro elemento do array)
    ↓
[Cast to BP_CharacterCreationManager]
    ↓ (Success)
[SET] CharacterCreationManager = (cast result)
    ↓
[Set Visibility] VBox_ClassInfo
    • Visibility: Collapsed (inicialmente oculto)
    ↓
[Set Visibility] HBox_Customization
    • Visibility: Collapsed (inicialmente oculto)
    ↓
[Set Visibility] TXT_Name
    • Visibility: Collapsed (inicialmente oculto)
    ↓
[Set Visibility] BTN_CreateCharacter
    • Visibility: Collapsed (inicialmente oculto)
```

### **6.4. Criar Função ShowClassInfo**

Crie uma função **Custom Function** chamada `ShowClassInfo`:

**Inputs:**
- `ClassID` (Integer)
- `ClassData` (Umbra Class Data)

**Alternativa: Usar GetSelectedClassData**

Você também pode usar a função C++ `GetSelectedClassData` diretamente:

```
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[Get Selected Class Data]
    • Target: (cast result)
    ↓
[Break Umbra Class Data]
    • Input: (resultado)
    ↓
[Usar os dados para preencher o widget]
```

**Lógica:**
```
[ShowClassInfo]
    ↓
[SET] SelectedClassID = ClassID (input)
    ↓
[SET] SelectedClassData = ClassData (input)
    ↓
[Break Umbra Class Data]
    • Input: ClassData
    ↓
[Set Text] TXT_ClassName
    • Text: ClassName (do Break)
    ↓
[Set Text] TXT_ClassDescription
    • Text: ClassDescription (do Break)
    ↓
[Format Text] "Base Stats:\nStrength: {0}\nDexterity: {1}\nIntelligence: {2}\nVitality: {3}\nLuck: {4}"
    • {0}: BaseStrength
    • {1}: BaseDexterity
    • {2}: BaseIntelligence
    • {3}: BaseVitality
    • {4}: BaseLuck
    ↓
[Set Text] TXT_BaseStats
    • Text: (resultado do Format Text)
    ↓
[Format Text] "Base Resources:\nHealth: {0}\nMana: {1}\nStamina: {2}"
    • {0}: BaseHealth
    • {1}: BaseMana
    • {2}: BaseStamina
    ↓
[Set Text] TXT_BaseResources
    • Text: (resultado do Format Text)
    ↓
[Format Text] "Base Combat:\nPhys. Atk: {0}\nMag. Atk: {1}\nPhys. Def: {2}\nMag. Def: {3}\nCritical: {4}"
    • {0}: BasePhysicalAttack
    • {1}: BaseMagicAttack
    • {2}: BasePhysicalDefense
    • {3}: BaseMagicDefense
    • {4}: BaseCritical
    ↓
[Set Text] TXT_BaseCombat
    • Text: (resultado do Format Text)
    ↓
[Set Visibility] VBox_ClassInfo
    • Visibility: Visible
    ↓
[Set Visibility] HBox_Customization
    • Visibility: Visible
    ↓
[Set Visibility] TXT_Name
    • Visibility: Visible
    ↓
[Set Visibility] BTN_CreateCharacter
    • Visibility: Visible
```

### **6.5. Conectar ao CharacterCreationManager**

No `BP_CharacterCreationManager`, atualize a função `ShowClassInfoWidget`:

**Inputs:**
- `ClassID` (Integer)

**Lógica:**
```
[ShowClassInfoWidget]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[Get Class Data By ID]
    • Target: (cast result)
    • Class ID: ClassID (input)
    • Out Class Data: (variável local)
    • Return Value: (bFound - Boolean)
    ↓
[Branch]
    • Condition: bFound
    ↓ (True)
    [Get Widget of Class]
        • Widget Class: WBP_CreateCharacter
        • Owning Player: Get Player Controller (0)
        ↓
    [Call Function: ShowClassInfo]
        • Target: (widget obtido)
        • ClassID: ClassID
        • ClassData: Out Class Data (variável local)
    ↓ (False)
    [Print String] "Classe não encontrada!"
```

**📌 NOTA:** A função `GetClassDataByID` do Game Instance já faz a busca automaticamente. Não é necessário criar uma função auxiliar no Blueprint.

### **6.6. Implementar Botão Create Character**

No `WBP_CreateCharacter`, no **Event Graph**:

```
[OnClicked] BTN_CreateCharacter
    ↓
[Get] SelectedClassID
    ↓
[Branch]
    • Condition: SelectedClassID > 0
    ↓ (True)
    [Get] SPIN_Hair
    • Value: (valor do spin box)
    ↓
    [Get] SPIN_Head
    • Value: (valor do spin box)
    ↓
    [Get Text] TXT_Name
    • Text: (texto do campo)
    ↓
    [Branch]
        • Condition: (Text length >= 3 AND Text length <= 20)
        ↓ (True)
        [Get] MyGameInstance
        ↓
    [Get Selected Class ID]
        • Target: MyGameInstance
        ↓
    [Branch]
        • Condition: (Selected Class ID > 0)
        ↓ (True)
        [Create Character]
            • Target: MyGameInstance
            • Character Name: (texto)
            • Class ID: Selected Class ID (do Get Selected Class ID)
            • Hair: (valor do spin box)
            • Head: (valor do spin box)
        ↓ (False)
        [Show Error Message] "Selecione uma classe primeiro"
        ↓ (False)
        [Show Error Message] "Nome deve ter entre 3 e 20 caracteres"
    ↓ (False)
    [Show Error Message] "Selecione uma classe primeiro"
```

---

## 🧪 **PASSO 7: TESTAR IMPLEMENTAÇÃO**

### **7.1. Checklist de Testes**

1. ✅ **Placeholders aparecem no level:**
   - Abra o level `Lvl_Character_Creation`
   - Verifique se todos os placeholders estão visíveis

2. ✅ **Click nos placeholders funciona:**
   - Clique em um placeholder
   - Verifique se o evento `OnClassSelected` é disparado

3. ✅ **Outros placeholders são despawnados:**
   - Após clicar, verifique se apenas o selecionado permanece

4. ✅ **Câmera se move:**
   - Verifique se a câmera se move suavemente para a posição de seleção
   - Verifique se o personagem está visível à direita da tela

5. ✅ **Widget aparece com informações:**
   - Verifique se o widget `WBP_CreateCharacter` aparece
   - Verifique se mostra os dados corretos da classe

6. ✅ **Campos de customização aparecem:**
   - Verifique se os campos Hair, Head e Nome aparecem
   - Verifique se os valores podem ser alterados

7. ✅ **Criação de personagem funciona:**
   - Preencha todos os campos
   - Clique em "Create Character"
   - Verifique se o personagem é criado no banco

---

## 🔧 **TROUBLESHOOTING**

### **Problema 1: Placeholders não aparecem no level**

**Solução:**
- Verifique se os Blueprints foram criados corretamente
- Verifique se os meshes estão configurados
- Verifique se os placeholders estão posicionados acima do chão

### **Problema 2: Click não funciona**

**Solução:**
- Verifique se o `Box Collision` está configurado corretamente
- Verifique se o `Enable Input` está sendo chamado no `BeginPlay`
- Verifique se o `OnClicked` está conectado ao `Collision_Box`

### **Problema 3: Câmera não se move**

**Solução:**
- Verifique se o `CameraActor` está referenciado corretamente
- Verifique se a Timeline está configurada e sendo executada
- Verifique se as posições calculadas estão corretas

### **Problema 4: Widget não aparece**

**Solução:**
- Verifique se o `WBP_CreateCharacter` está sendo criado
- Verifique se a função `ShowClassInfo` está sendo chamada
- Verifique se as visibilidades estão sendo configuradas corretamente

### **Problema 5: Dados da classe não aparecem**

**Solução:**
- Verifique se `LoadClasses` foi chamado e completou
- Verifique se `GetAvailableClasses` retorna dados
- Verifique se a função `FindClassByID` está funcionando corretamente

---

## 📝 **NOTAS FINAIS**

- Este sistema cria uma experiência mais imersiva de seleção de classe
- A câmera deve se mover suavemente (use Timeline com curva Ease In Out)
- Os placeholders podem ter animações ou efeitos visuais quando selecionados
- Considere adicionar som de feedback quando uma classe é selecionada
- O widget pode ser expandido com mais informações visuais (ícones, imagens, etc.)

---

## ✅ **CHECKLIST FINAL**

- [ ] **C++ recompilado** (novas funções disponíveis)
- [ ] Blueprint pai `BP_Class_Placeholder` criado
- [ ] Blueprints filhos para cada classe criados
- [ ] Placeholders posicionados no level
- [ ] Sistema de click implementado
- [ ] Sistema de despawn implementado
- [ ] Movimento de câmera implementado
- [ ] Widget atualizado com informações da classe
- [ ] Campos de customização funcionando
- [ ] Delegate `OnClassSelected` conectado
- [ ] Função `SelectClass` sendo usada corretamente
- [ ] Criação de personagem testada e funcionando

---

## 📚 **REFERÊNCIA: Funções C++ Disponíveis**

### **Delegates:**
- `OnClassSelected` - Disparado quando uma classe é selecionada (parâmetro: `ClassID`)

### **Funções BlueprintCallable:**
- `SelectClass(int32 ClassID)` - Seleciona uma classe e dispara o delegate
  - **Retorna:** `bool` (true se encontrada, false caso contrário)
- `GetClassDataByID(int32 ClassID, FUmbraClassData& OutClassData)` - Obtém dados de uma classe
  - **Retorna:** `bool` (true se encontrada, false caso contrário)
- `ClearClassSelection()` - Limpa a seleção de classe

### **Funções BlueprintPure:**
- `GetSelectedClassData()` - Obtém dados da classe selecionada
  - **Retorna:** `FUmbraClassData`
- `GetSelectedClassID()` - Obtém ID da classe selecionada
  - **Retorna:** `int32` (0 se nenhuma estiver selecionada)

### **Variáveis BlueprintReadWrite/ReadOnly:**
- `SelectedClassID` (int32) - ID da classe selecionada
- `SelectedClassData` (FUmbraClassData) - Dados da classe selecionada
- `AvailableClasses` (Array of FUmbraClassData) - Lista de todas as classes disponíveis

---

**Fim do Guia**

