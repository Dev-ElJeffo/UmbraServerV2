# 🎨 Guia Passo a Passo: Widgets de Inventário no Unreal Engine

**Nível de detalhe**: Iniciante  
**Tempo estimado**: 60-90 minutos  
**Pré-requisitos**: UmbraGameInstance compilado ✅ | APIs funcionais ✅

---

## 📋 Índice

1. [Preparação: Criar Pastas](#preparação)
2. [Parte 1: WBP_InventorySlot (1 item)](#parte-1)
3. [Parte 2: WBP_Inventory (Grid completo)](#parte-2)
4. [Parte 3: Configurar Character Blueprint](#parte-3)
5. [Parte 4: Testar no Editor](#parte-4)
6. [Parte 5: Drag & Drop (Avançado)](#parte-5)

---

<a name="preparação"></a>
## 🗂️ PREPARAÇÃO: Criar Estrutura de Pastas

### Passo 1: Criar pastas no Content Browser

1. Abra o **Unreal Editor**
2. No **Content Browser** (painel inferior), navegue para `Content/`
3. **Clique direito** no espaço vazio
4. Selecione **New Folder**
5. Nome: `UI`
6. **Clique direito** em `UI/` → **New Folder**
7. Nome: `Inventory`

**Resultado esperado**:
```
Content/
└─ UI/
   └─ Inventory/
      (vazio - vamos criar os widgets aqui)
```

---

<a name="parte-1"></a>
## 🎴 PARTE 1: WBP_InventorySlot (Widget de 1 Item)

Este widget representa **UM slot individual** do inventário (64x64 pixels).

---

### 📝 Passo 1.1: Criar o Widget

1. **Navegue** para `Content/UI/Inventory/`
2. **Clique direito** no espaço vazio
3. **User Interface** → **Widget Blueprint**
4. Nome: `WBP_InventorySlot`
5. **Pressione Enter**
6. **Clique duplo** no widget para abrir o editor

---

### 🎨 Passo 1.2: Configurar Hierarquia Visual

**Você verá 3 painéis**:
- **Esquerda**: Hierarchy (hierarquia de elementos)
- **Centro**: Designer (editor visual)
- **Direita**: Details (propriedades)

#### A) Remover Canvas Panel padrão

1. Na **Hierarchy** (esquerda), clique em `Canvas Panel`
2. **Delete** (tecla Del)

#### B) Adicionar Root: Canvas Panel

1. Na **Palette** (canto superior esquerdo), procure: `Canvas Panel`
2. **Arraste** para a Hierarchy
3. Deve ficar como root (elemento raiz)

#### C) Adicionar Border (fundo do slot)

1. Na **Palette**, procure: `Border`
2. **Arraste** para dentro de `Canvas Panel`
3. Na **Hierarchy**, clique em `Border_0`
4. No painel **Details** (direita):
   - **Slot (Canvas Panel Slot)**:
     - Anchors: **Clique** no quadrado → Selecione **canto superior esquerdo** (primeiro da grid)
     - Position X: `0`
     - Position Y: `0`
     - Size X: `64`
     - Size Y: `64`
   - **Appearance**:
     - Brush Color: `RGB(0.1, 0.1, 0.1, 0.8)` (cinza escuro semi-transparente)
     - Padding: `Left: 2, Top: 2, Right: 2, Bottom: 2`

5. **Renomeie** (clique direito): `Border_SlotBackground`

#### D) Adicionar Overlay (para sobrepor elementos)

1. Na **Palette**, procure: `Overlay`
2. **Arraste** para dentro de `Border_SlotBackground`

#### E) Adicionar Image (ícone do item)

1. Na **Palette**, procure: `Image`
2. **Arraste** para dentro de `Overlay`
3. Clique em `Image_0`
4. No **Details**:
   - **Slot (Overlay Slot)**:
     - Horizontal Alignment: `Fill`
     - Vertical Alignment: `Fill`
   - **Appearance**:
     - Brush: **None** (por enquanto - será setado dinamicamente)
     - Color and Opacity: `RGB(1, 1, 1, 1)` (branco)
     - Desired Size Override: ✅ Marque
       - Size X: `60`
       - Size Y: `60`

5. **Renomeie**: `Image_ItemIcon`

#### F) Adicionar TextBlock (quantidade)

1. Na **Palette**, procure: `Text`
2. **Arraste** para dentro de `Overlay`
3. Clique em `TextBlock_0`
4. No **Details**:
   - **Content**:
     - Text: `99` (placeholder)
   - **Slot (Overlay Slot)**:
     - Horizontal Alignment: `Right`
     - Vertical Alignment: `Bottom`
     - Padding: `Right: 4, Bottom: 4`
   - **Appearance**:
     - Color and Opacity: `RGB(1, 1, 1, 1)` (branco)
     - Font:
       - Size: `14`
       - Typeface: `Bold`
     - Shadow Offset: `X: 1, Y: 1` (sombra para legibilidade)
     - Shadow Color: `RGB(0, 0, 0, 1)` (preto)

5. **Renomeie**: `Text_Quantity`

#### G) Adicionar ProgressBar (durabilidade)

1. Na **Palette**, procure: `Progress Bar`
2. **Arraste** para dentro de `Overlay`
3. Clique em `ProgressBar_0`
4. No **Details**:
   - **Slot (Overlay Slot)**:
     - Horizontal Alignment: `Fill`
     - Vertical Alignment: `Bottom`
     - Padding: `Left: 4, Right: 4, Bottom: 2`
   - **Appearance**:
     - Percent: `0.75` (75% - placeholder)
     - Bar Fill Type: `Left to Right`
     - Fill Color and Opacity: `RGB(0, 1, 0.2, 1)` (verde)
   - **Style**:
     - Bar Image:
       - Tint: `RGB(0, 0.8, 0, 1)` (verde escuro)

5. **Renomeie**: `ProgressBar_Durability`

---

### 📊 Hierarquia Final (Passo 1.2)

Verifique se sua **Hierarchy** está assim:

```
Canvas Panel
└─ Border_SlotBackground
   └─ Overlay
      ├─ Image_ItemIcon
      ├─ Text_Quantity
      └─ ProgressBar_Durability
```

**✅ Checkpoint**: Sua tela deve mostrar um quadrado cinza (64x64) com:
- Área central (ícone - vazio por enquanto)
- Número "99" no canto inferior direito
- Barra verde na parte inferior (durabilidade)

---

### 🔧 Passo 1.3: Adicionar Variáveis

Agora vamos adicionar variáveis para armazenar dados do slot.

1. **Clique** na aba **Graph** (canto superior direito, ao lado de "Designer")
2. No painel **My Blueprint** (esquerda), encontre **Variables**
3. **Clique** no botão **+ Variable**

#### A) Variável: SlotData

1. Nome: `SlotData`
2. No **Details** (direita):
   - **Variable Type**: Procure por `Umbra Inventory Slot` (é o struct C++)
   - **Instance Editable**: ✅ Marque
   - **Expose on Spawn**: ❌ Desmarque
   - **Tooltip**: "Dados do item neste slot"

#### B) Variável: SlotIndex

1. **+ Variable**
2. Nome: `SlotIndex`
3. No **Details**:
   - **Variable Type**: `Integer`
   - **Instance Editable**: ✅ Marque
   - **Default Value**: `0` (compile primeiro para poder editar)
   - **Tooltip**: "Índice deste slot (0-49)"

#### C) Variável: bIsEmpty

1. **+ Variable**
2. Nome: `bIsEmpty`
3. No **Details**:
   - **Variable Type**: `Boolean`
   - **Instance Editable**: ❌ Desmarque
   - **Default Value**: `true`
   - **Tooltip**: "Se true, o slot está vazio"

---

### 🧩 Passo 1.4: Criar Função "UpdateSlotVisual"

Esta função atualiza o visual do slot com base nos dados.

1. No painel **My Blueprint**, encontre **Functions**
2. **Clique** no botão **+ Function**
3. Nome: `UpdateSlotVisual`

4. Na **Event Graph** desta função:

```
┌─────────────────────────────────────────────────────────────────┐
│  UpdateSlotVisual (Function)                                    │
└─────────────────────────────────────────────────────────────────┘

[Entry]
   ↓
[Get SlotData] (variável)
   ↓
[Break UmbraInventorySlot] (quebrar o struct)
   ↓ (pino InventoryID)
[Branch] (InventoryID > 0?)
   ├─ True:  Item existe no slot
   │  ↓
   │  [Set bIsEmpty] = False
   │  ↓
   │  ┌────────────────────────────────────────┐
   │  │ Atualizar ícone do item                │
   │  └────────────────────────────────────────┘
   │  [Get SlotData] → [Break UmbraInventorySlot]
   │     ↓ (pino ItemTemplate)
   │  [Break UmbraItemTemplate]
   │     ↓ (pino ItemIcon)
   │  [Get Image_ItemIcon] → [Set Brush from Texture]
   │                           - Texture: ItemIcon
   │                        → [Set Visibility] (Visible)
   │  ↓
   │  ┌────────────────────────────────────────┐
   │  │ Atualizar quantidade                   │
   │  └────────────────────────────────────────┘
   │  [Get SlotData] → [Break UmbraInventorySlot]
   │     ↓ (pino Quantity)
   │  [Branch] (Quantity > 1?)
   │     ├─ True:
   │     │  [Format Text] ("{0}", Quantity)
   │     │     ↓
   │     │  [Get Text_Quantity] → [Set Text]
   │     │     ↓
   │     │  [Set Visibility] = Visible
   │     │
   │     └─ False:
   │        [Get Text_Quantity] → [Set Visibility] = Hidden
   │  ↓
   │  ┌────────────────────────────────────────┐
   │  │ Atualizar durabilidade                 │
   │  └────────────────────────────────────────┘
   │  [Get SlotData] → [Break UmbraInventorySlot]
   │     ↓ (pino Durability)
   │  [Divide] (Durability / 100.0) = Percent
   │     ↓
   │  [Get ProgressBar_Durability] → [Set Percent]
   │  ↓
   │  ┌────────────────────────────────────────┐
   │  │ Cor da borda por raridade              │
   │  └────────────────────────────────────────┘
   │  [Get SlotData] → [Break UmbraInventorySlot]
   │     ↓ (ItemTemplate)
   │  [Break UmbraItemTemplate]
   │     ↓ (Rarity)
   │  [Switch on EUmbraItemRarity]
   │     ├─ Common: RGB(0.5, 0.5, 0.5) (cinza)
   │     ├─ Uncommon: RGB(0, 1, 0) (verde)
   │     ├─ Rare: RGB(0, 0.5, 1) (azul)
   │     ├─ Epic: RGB(0.6, 0, 1) (roxo)
   │     └─ Legendary: RGB(1, 0.5, 0) (laranja)
   │        ↓
   │  [Get Border_SlotBackground] → [Set Brush Color]
   │
   └─ False: Slot vazio
      ↓
      [Set bIsEmpty] = True
      ↓
      [Get Image_ItemIcon] → [Set Visibility] = Hidden
      ↓
      [Get Text_Quantity] → [Set Visibility] = Hidden
      ↓
      [Get ProgressBar_Durability] → [Set Visibility] = Hidden
      ↓
      [Get Border_SlotBackground] → [Set Brush Color] = RGB(0.1, 0.1, 0.1, 0.5)
```

**📝 INSTRUÇÕES DETALHADAS PARA CRIAR ESTE GRAFO**:

**⚠️ IMPORTANTE**: `SlotData` é um **STRUCT**, não um Object Reference. Por isso **não podemos usar `Is Valid`**. Em vez disso, verificamos se `InventoryID > 0` (se for maior que 0, significa que tem um item no slot).

#### Parte A: Verificar se slot tem item

1. **Clique direito** na Event Graph → `Get SlotData`
2. Arraste do pino de `SlotData` → **procure** `Break UmbraInventorySlot`
   - Isso "quebra" o struct e mostra todos os campos
3. Do pino **`Inventory ID`** (campo do struct):
   - Arraste → **procure** `>` (Greater)
   - No segundo pino, digite `0`
4. Do resultado (Boolean) → `Branch`

#### Parte B: Se TRUE (tem item):

1. Do pino `True` do Branch:
   - **Clique direito** → `Set bIsEmpty`
   - Marque o checkbox como `false`

2. Atualizar ícone:
   
   - `Get SlotData` → `Break UmbraInventorySlot`
   - Do pino `Item Template` → `Break UmbraItemTemplate`
   - Do pino `Item Icon` (UTexture2D) → conecte ao próximo nó
   - `Get Image_ItemIcon` → `Set Brush from Texture`
     - Conecte o pino `Item Icon` no pino `Texture` do `Set Brush from Texture`
   - Do mesmo `Get Image_ItemIcon` → `Set Visibility` → selecione `Visible`

3. Atualizar quantidade:
   - `Get SlotData` → `Break UmbraInventorySlot`
   - Do pino `Quantity` → **clique direito** → `>` (Greater)
   - Conecte `1` no outro pino
   - Conecte ao `Branch`
   - Se TRUE:
     - `Format Text` (procure na palette)
     - Pattern: `{0}`
     - Conecte `Quantity` ao pino `0`
     - `Get Text_Quantity` → `Set Text`
     - `Get Text_Quantity` → `Set Visibility` → `Visible`
   - Se FALSE:
     - `Get Text_Quantity` → `Set Visibility` → `Hidden`

4. Atualizar durabilidade:
   - `Get SlotData` → `Break UmbraInventorySlot`
   - Do pino `Durability` → `/` (Divide) → `100.0`
   - `Get ProgressBar_Durability` → `Set Percent`

5. Cor da borda:
   - `Get SlotData` → `Break UmbraInventorySlot` → `Item Template`
   - `Break UmbraItemTemplate` → pino `Rarity`
   - **Clique direito** → `Switch on EUmbraItemRarity`
   - Para cada case (Common, Uncommon, Rare, Epic, Legendary):
     - **Clique direito** → `Make Linear Color`
     - Defina RGB para cada raridade
     - Conecte ao `Get Border_SlotBackground` → `Set Brush Color`

#### Parte C: Se FALSE (vazio):

1. Do pino `False` do Branch principal:
   - `Set bIsEmpty` = `true`
   - `Get Image_ItemIcon` → `Set Visibility` → `Hidden`
   - `Get Text_Quantity` → `Set Visibility` → `Hidden`
   - `Get ProgressBar_Durability` → `Set Visibility` → `Hidden`
   - `Get Border_SlotBackground` → `Set Brush Color` → RGB(0.1, 0.1, 0.1, 0.5)

---

### 🎯 Passo 1.5: Event Construct

Quando o widget é criado, deve atualizar visual automaticamente.

1. **Volte** para o Event Graph principal (não na função)
2. **Clique direito** → `Event Construct`
3. Do pino de execução de `Event Construct`:
   - Conecte ao nó `UpdateSlotVisual` (a função que criamos)

```
[Event Construct]
   ↓
[UpdateSlotVisual]
```

---

### ✅ Passo 1.6: Compilar e Testar

1. **Clique** no botão **Compile** (canto superior esquerdo)
2. **Aguarde** compilação
3. Se houver erros, leia a mensagem e corrija
4. **Save** (Ctrl+S)

**🎉 WBP_InventorySlot está pronto!**

---

<a name="parte-2"></a>
## 📦 PARTE 2: WBP_Inventory (Widget Grid Completo)

Este widget cria o inventário completo com 50 slots (5 colunas × 10 linhas).

---

### 📝 Passo 2.1: Criar o Widget

1. **Navegue** para `Content/UI/Inventory/`
2. **Clique direito** → **User Interface** → **Widget Blueprint**
3. Nome: `WBP_Inventory`
4. **Clique duplo** para abrir

---

### 🎨 Passo 2.2: Configurar Hierarquia Visual

#### A) Root: Canvas Panel

1. Mantenha o `Canvas Panel` padrão

#### B) Adicionar Border (fundo escuro tela cheia)

1. **Palette** → Arraste `Border` para dentro de `Canvas Panel`
2. No **Details**:
   - **Slot (Canvas Panel Slot)**:
     - Anchors: **Selecione "Fill" (último da grid - preenche tudo)**
     - Offsets: `Left: 0, Top: 0, Right: 0, Bottom: 0`
   - **Appearance**:
     - Brush Color: `RGB(0, 0, 0, 0.7)` (preto semi-transparente)

3. **Renomeie**: `Border_FullscreenBackground`

#### C) Adicionar Canvas Panel (para centralizar conteúdo)

1. Arraste `Canvas Panel` para dentro de `Border_FullscreenBackground`
2. **Renomeie**: `Canvas_Centered`

#### D) Adicionar Border (fundo do inventário)

1. Arraste `Border` para dentro de `Canvas_Centered`
2. No **Details**:
   - **Slot (Canvas Panel Slot)**:
     - Anchors: **Middle Center** (centro da grid)
     - Alignment: `X: 0.5, Y: 0.5`
     - Position: `X: 0, Y: 0`
     - Size: `X: 400, Y: 700`
   - **Appearance**:
     - Brush Color: `RGB(0.15, 0.15, 0.15, 0.95)` (cinza escuro)
     - Padding: `20` (todos os lados)

3. **Renomeie**: `Border_InventoryPanel`

#### E) Adicionar VerticalBox

1. Arraste `Vertical Box` para dentro de `Border_InventoryPanel`

#### F) Adicionar TextBlock (título)

1. Arraste `Text` para dentro de `Vertical Box`
2. No **Details**:
   - **Content**:
     - Text: `INVENTÁRIO`
   - **Appearance**:
     - Font Size: `24`
     - Justification: `Center`
     - Color: `RGB(1, 0.8, 0, 1)` (dourado)

3. **Renomeie**: `Text_Title`

#### G) Adicionar Spacer

1. Arraste `Spacer` para dentro de `Vertical Box` (abaixo do título)
2. No **Details**:
   - **Slot (Vertical Box Slot)**:
     - Size: `Fill`, valor `0.1` (pequeno espaço)

#### H) Adicionar Uniform Grid Panel (o grid de slots!)

1. Arraste `Uniform Grid Panel` para dentro de `Vertical Box`
2. No **Details**:
   - **Slot (Vertical Box Slot)**:
     - Size: `Fill`, valor `1.0`
   - **Appearance**:
     - Slot Padding: `2` (espaçamento entre slots)

3. **Renomeie**: `Grid_InventorySlots`

#### I) Adicionar TextBlock (info)

1. Arraste `Spacer` para dentro de `Vertical Box`
   - Size: `Fill`, valor `0.1`

2. Arraste `Text` para dentro de `Vertical Box`
3. No **Details**:
   - **Content**:
     - Text: `Slots: 0/50 | Peso: 0/100`
   - **Appearance**:
     - Font Size: `12`
     - Justification: `Center`
     - Color: `RGB(0.7, 0.7, 0.7, 1)` (cinza claro)

4. **Renomeie**: `Text_Info`

---

### 📊 Hierarquia Final (Passo 2.2)

```
Canvas Panel
└─ Border_FullscreenBackground
   └─ Canvas_Centered
      └─ Border_InventoryPanel
         └─ Vertical Box
            ├─ Text_Title ("INVENTÁRIO")
            ├─ Spacer
            ├─ Grid_InventorySlots (Uniform Grid Panel)
            ├─ Spacer
            └─ Text_Info ("Slots: 0/50...")
```

---

### 🔧 Passo 2.3: Adicionar Variáveis

1. Vá para a aba **Graph**
2. **+ Variable**:

#### A) GameInstanceRef

- Nome: `GameInstanceRef`
- Type: `Umbra Game Instance` (Object Reference)
- Instance Editable: ❌
- Tooltip: "Referência ao Game Instance"

#### B) SlotWidgets

- Nome: `SlotWidgets`
- Type: `WBP_Inventory Slot` (Widget Reference)
- **Clique** no tipo → Mude para **Array** (ícone de grade)
- Instance Editable: ❌
- Tooltip: "Array com os 50 widgets de slots"

---

### 🧩 Passo 2.4: Função "CreateInventorySlots"

Esta função cria os 50 slots visuais no grid.

1. **+ Function**
2. Nome: `CreateInventorySlots`

**Event Graph desta função**:

```
[Entry]
   ↓
[For Loop] (First Index: 0, Last Index: 49)
   ↓ (Loop Body)
   ├─ [Divide] (Index / 5) = Row
   ├─ [Mod] (Index % 5) = Column
   │  ↓
   ├─ [Create Widget] (Class: WBP_InventorySlot)
   │     ↓
   ├─ [Set SlotIndex] (no widget criado) = Index
   │     ↓
   ├─ [Add Child to Uniform Grid]
   │  - Target: Grid_InventorySlots
   │  - Content: Widget criado
   │  - Row: Row calculado
   │  - Column: Column calculado
   │     ↓
   └─ [Add] (ao array SlotWidgets)
```

**📝 INSTRUÇÕES DETALHADAS**:

1. **Clique direito** → `For Loop`
   - First Index: `0`
   - Last Index: `49`

2. Do pino `Loop Body`:
   - **Clique direito** → `/` (Divide)
     - Conecte `Index` → primeiro pino
     - `5` → segundo pino
     - Isso calcula a **linha** (Row)

3. **Clique direito** → `%` (Modulo/Mod)
   - Conecte `Index` → primeiro pino
   - `5` → segundo pino
   - Isso calcula a **coluna** (Column)

4. **Clique direito** → `Create Widget`
   - Class: Selecione `WBP_InventorySlot`
   - Owning Player: `Get Player Controller` → index 0

5. Do widget criado:
   - Arraste o pino → `Set Slot Index`
   - Conecte `Index` do For Loop

6. `Get Grid_InventorySlots` (arraste da variável)
   - Arraste o pino → `Add Child to Uniform Grid`
   - Content: Widget criado
   - Row: Resultado do Divide
   - Column: Resultado do Mod

7. `Get SlotWidgets` (a variável array)
   - Arraste o pino → `Add`
   - Conecte o widget criado ao pino do Add

---

### 🎯 Passo 2.5: Event Construct

1. **Event Construct**
2. Sequência:

```
[Event Construct]
   ↓
[Get Game Instance]
   ↓
[Cast to UmbraGameInstance]
   ↓ (Success)
   ├─ [Set GameInstanceRef] (salvar referência)
   │     ↓
   ├─ [CreateInventorySlots] (nossa função)
   │     ↓
   ├─ [Bind Event to OnInventoryLoaded]
   │  - Event: OnInventoryLoaded (do GameInstance)
   │  - Delegate: Criar custom event "OnInventoryLoaded_Event"
   │     ↓
   ├─ [Bind Event to OnItemAdded]
   │  - Event: OnItemAdded (do GameInstance)
   │  - Delegate: Criar custom event "OnItemAdded_Event"
   │     ↓
   ├─ [Bind Event to OnItemMoved]
   │  - Event: OnItemMoved (do GameInstance)
   │  - Delegate: Criar custom event "OnItemMoved_Event"
   │     ↓
   ├─ [Delay] (0.5 segundos) // Esperar grid ser criado
   │     ↓
   └─ [LoadInventory] (do GameInstance)
```

**📝 INSTRUÇÕES DETALHADAS**:

1. `Event Construct`
2. **Clique direito** → `Get Game Instance`
3. Arraste do pino → `Cast to UmbraGameInstance`
4. Do pino de sucesso:
   - `Set GameInstanceRef` (salvar a referência)

5. `CreateInventorySlots` (função que criamos)

6. **Bind Delegates**:
   - Do `GameInstanceRef`:
     - Arraste o pino → procure `Assign On Inventory Loaded`
     - Isso cria automaticamente um **Custom Event**
     - **Renomeie** o event para: `OnInventoryLoaded_Event`

   - Repita para:
     - `Assign On Item Added` → Event: `OnItemAdded_Event`
     - `Assign On Item Moved` → Event: `OnItemMoved_Event`

7. **Clique direito** → `Delay` (0.5 segundos)

8. Do `GameInstanceRef`:
   - Arraste o pino → `Load Inventory` (função C++)

---

### 🔄 Passo 2.6: Implementar Custom Event "OnInventoryLoaded_Event"

Quando o inventário é carregado do servidor, atualiza todos os slots visuais.

1. Encontre o Custom Event `OnInventoryLoaded_Event` criado no passo anterior
2. Event Graph:

```
[OnInventoryLoaded_Event]
   ↓
[For Each Loop] (Array: SlotWidgets)
   ↓ (Loop Body)
   ├─ [Get] Array Element (widget do slot)
   │     ↓
   ├─ [Get SlotIndex] (do widget)
   │     ↓
   ├─ [Get GameInstanceRef]
   │     ↓
   ├─ [Get Inventory Slot By Index] (função C++)
   │  - Target: GameInstanceRef
   │  - Slot Index: SlotIndex do widget
   │     ↓
   ├─ [Set SlotData] (no widget)
   │  - Conectar o resultado de Get Inventory Slot By Index
   │     ↓
   └─ [UpdateSlotVisual] (do widget)
```

**📝 INSTRUÇÕES DETALHADAS**:

1. Do event `OnInventoryLoaded_Event`:
2. `Get SlotWidgets` → arraste o pino → `For Each Loop`
3. Do `Array Element` do loop:
   - Arraste → `Get Slot Index`
4. Salve esse Index em uma variável temporária
5. `Get GameInstanceRef`
   - Arraste → `Get Inventory Slot By Index`
   - Conecte o Index ao parâmetro `Slot Index`
6. Do resultado:
   - Conecte ao widget (Array Element) → `Set Slot Data`
7. Do widget:
   - `Update Slot Visual`

---

### 🔄 Passo 2.7: Implementar "OnItemAdded_Event"

Quando um item é adicionado, atualiza apenas o slot afetado.

```
[OnItemAdded_Event] (tem parâmetro: SlotIndex)
   ↓
[Get] (SlotWidgets[SlotIndex])
   ↓
[Get GameInstanceRef]
   ↓
[Get Inventory Slot By Index] (SlotIndex)
   ↓
[Set SlotData] (no widget)
   ↓
[UpdateSlotVisual] (no widget)
```

---

### 🔄 Passo 2.8: Implementar "OnItemMoved_Event"

Quando um item é movido, atualiza os 2 slots (origem e destino).

```
[OnItemMoved_Event] (parâmetros: FromSlotIndex, ToSlotIndex)
   ↓
├─ Atualizar FromSlot:
│  [Get SlotWidgets[FromSlotIndex]]
│     ↓
│  [Get Inventory Slot By Index] (FromSlotIndex)
│     ↓
│  [Set SlotData] → [UpdateSlotVisual]
│
└─ Atualizar ToSlot:
   [Get SlotWidgets[ToSlotIndex]]
      ↓
   [Get Inventory Slot By Index] (ToSlotIndex)
      ↓
   [Set SlotData] → [UpdateSlotVisual]
```

---

### ✅ Passo 2.9: Compilar e Salvar

1. **Compile**
2. **Save**

**🎉 WBP_Inventory está pronto!**

---

<a name="parte-3"></a>
## 🎮 PARTE 3: Configurar Character Blueprint

Agora vamos fazer o personagem abrir/fechar o inventário com a tecla "I".

---

### 📝 Passo 3.1: Criar Input Action

#### A) Criar o Input Action

1. **Content Browser** → `Content/Input/`
2. **Clique direito** → **Input** → **Input Action**
3. Nome: `IA_OpenInventory`
4. **Clique duplo** para abrir
5. **Value Type**: `Digital (bool)` (padrão - tecla pressionada ou não)
6. **Save**

#### B) Adicionar ao Input Mapping Context

1. Abra `IMC_Default` (ou seu Input Mapping Context atual)
2. **Mappings** → **+ (Add Mapping)**
3. **Action**: Selecione `IA_OpenInventory`
4. **Key**: Pressione `I` no teclado
5. **Save**

---

### 📝 Passo 3.2: Editar BP_ThirdPersonCharacter

1. Abra `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter`
2. Vá para **Event Graph**

---

### 🔧 Passo 3.3: Adicionar Variáveis

No painel **My Blueprint**:

#### A) InventoryWidgetRef

- Nome: `InventoryWidgetRef`
- Type: `WBP_Inventory` (Widget Reference)
- Instance Editable: ❌
- Default Value: `None`

#### B) bIsInventoryOpen

- Nome: `bIsInventoryOpen`
- Type: `Boolean`
- Instance Editable: ❌
- Default Value: `false`

---

### 🎯 Passo 3.4: Event Graph - Input Action

1. **Clique direito** → procure `IA_OpenInventory`
2. Selecione o evento `Started` (quando a tecla é pressionada)

3. Event Graph:

```
[IA_OpenInventory (Started)]
   ↓
[Branch] (bIsInventoryOpen?)
   ├─ True: FECHAR inventário
   │  ↓
   │  [Get InventoryWidgetRef]
   │     ↓
   │  [Is Valid?]
   │     ↓ True
   │     [Remove from Parent]
   │        ↓
   │     [Set Input Mode Game Only]
   │     - Player Controller: Get Player Controller (index 0)
   │        ↓
   │     [Set Show Mouse Cursor] = False
   │     - Target: Get Player Controller (index 0)
   │        ↓
   │     [Set bIsInventoryOpen] = False
   │
   └─ False: ABRIR inventário
      ↓
      [Create Widget] (Class: WBP_Inventory)
      - Owning Player: Get Player Controller (index 0)
         ↓
      [Set InventoryWidgetRef] (salvar referência)
         ↓
      [Add to Viewport]
         ↓
      [Set Input Mode UI Only]
      - Player Controller: Get Player Controller (index 0)
      - Widget to Focus: InventoryWidgetRef
         ↓
      [Set Show Mouse Cursor] = True
      - Target: Get Player Controller (index 0)
         ↓
      [Set bIsInventoryOpen] = True
```

**📝 INSTRUÇÕES DETALHADAS**:

1. Do evento `IA_OpenInventory (Started)`:

2. `Get bIsInventoryOpen` → `Branch`

3. **Ramo TRUE** (fechar):
   - `Get InventoryWidgetRef`
   - `Is Valid?` (verificar se existe)
   - Se válido:
     - `Remove from Parent`
     - **Clique direito** → `Set Input Mode Game Only`
       - Player Controller: `Get Player Controller` → index 0
     - `Get Player Controller` (0) → `Set Show Mouse Cursor` = `false`
     - `Set bIsInventoryOpen` = `false`

4. **Ramo FALSE** (abrir):
   - **Clique direito** → `Create Widget`
     - Class: `WBP_Inventory`
     - Owning Player: `Get Player Controller` (0)
   - Do widget criado:
     - `Set InventoryWidgetRef` (salvar)
     - `Add to Viewport`
   - **Clique direito** → `Set Input Mode UI Only`
     - Player Controller: `Get Player Controller` (0)
     - Widget to Focus: `InventoryWidgetRef`
   - `Get Player Controller` (0) → `Set Show Mouse Cursor` = `true`
   - `Set bIsInventoryOpen` = `true`

---

### ✅ Passo 3.5: Compilar e Salvar

1. **Compile**
2. **Save**

---

<a name="parte-4"></a>
## 🧪 PARTE 4: Testar no Editor

Hora de testar se tudo funciona!

---

### 📝 Passo 4.1: Verificar GameInstance

1. `Edit > Project Settings`
2. `Maps & Modes`
3. **Game Instance Class**: Deve ser `UmbraGameInstance` ✅

---

### 📝 Passo 4.2: Play in Editor (PIE)

1. **Clique** no botão **Play** (ou Alt+P)
2. O jogo inicia

---

### 🎮 Passo 4.3: Testar Inventário

1. **Faça login** normalmente no jogo
2. **Pressione "I"**
3. O inventário deve aparecer:
   - Fundo escuro semi-transparente
   - Painel cinza no centro
   - Título "INVENTÁRIO"
   - Grid 5×10 (50 slots)
   - Slots vazios (cinza escuro)

4. **Observe o Output Log**:
   ```
   [UmbraGameInstance] Carregando inventário do servidor...
   [UmbraGameInstance] ✅ Inventário carregado com sucesso: X itens
   ```

5. Se tiver itens no banco de dados, devem aparecer:
   - Ícone do item (se tiver textura)
   - Quantidade no canto inferior direito
   - Barra de durabilidade verde
   - Borda colorida por raridade

6. **Pressione "I" novamente**:
   - Inventário deve fechar
   - Cursor do mouse deve sumir
   - Controles voltam ao normal

---

### 🐛 Passo 4.4: Troubleshooting

#### Problema 1: Inventário não abre

**Soluções**:
- Verifique se `IA_OpenInventory` está no `IMC_Default`
- Verifique se `IMC_Default` está aplicado no Character

#### Problema 2: Slots aparecem, mas sem itens

**Soluções**:
- Verifique Output Log por erros HTTP
- Teste a API manualmente: `http://localhost/umbra_api/test_inventory.php`
- Verifique se o token JWT está sendo enviado

#### Problema 3: Erro de compilação

**Soluções**:
- Leia a mensagem de erro
- Verifique se todas as variáveis têm o tipo correto
- Verifique se todos os nós estão conectados corretamente

---

<a name="parte-5"></a>
## 🖱️ PARTE 5: Drag & Drop (Avançado)

Implementar arrastar e soltar itens entre slots.

---

### 📝 Passo 5.1: Em WBP_InventorySlot

#### A) Implementar OnMouseButtonDown

1. Abra `WBP_InventorySlot`
2. **Override** → `On Mouse Button Down`

```
[OnMouseButtonDown]
   ↓
[Get bIsEmpty]
   ↓
[Branch] (Is Empty?)
   ├─ True: Não fazer nada
   │  └─ [Return] (Handled = false)
   │
   └─ False: Iniciar drag
      ↓
      [Create DragDropOperation]
      - Operation Class: Widget Drag Drop Operation
         ↓
      [Set Payload] = Self (this widget)
      [Set Pivot] = Mouse Down (0.5, 0.5)
      [Set Drag Visual] = Duplicate de Image_ItemIcon
         ↓
      [Detect Drag if Pressed]
      - Drag Key: Left Mouse Button
         ↓
      [Return] (Handled = true)
```

#### B) Implementar OnDrop

1. **Override** → `On Drop`

```
[OnDrop]
   ↓
[Cast to WidgetDragDropOperation]
   ↓
[Get Payload] = Source Widget
   ↓
[Get SlotIndex] (do Source)
   ↓
[Get SlotIndex] (do Self/Target)
   ↓
[Get Game Instance]
   ↓
[Cast to UmbraGameInstance]
   ↓
[Get SlotData] (do Source)
   ↓
[Get InventoryID] (do SlotData)
   ↓
[MoveItem] (função C++)
- InventoryID: ID do item
- TargetSlotIndex: SlotIndex do Self (destino)
   ↓
[Return] (Handled = true)
```

---

### ✅ Passo 5.2: Testar Drag & Drop

1. **Compile** e **Save** tudo
2. **Play**
3. Abra inventário (I)
4. **Clique e arraste** um item para outro slot
5. O item deve mover de posição
6. Verifique no banco de dados se o `slot_index` mudou

---

## 🎉 CONCLUSÃO

**Parabéns!** Você implementou um sistema completo de inventário:

✅ **Backend**:
- MySQL com 17 templates
- 6 APIs REST funcionais
- JWT autenticação

✅ **C++**:
- UmbraGameInstance (970+ linhas)
- 11 Delegates
- Parsing JSON completo

✅ **Blueprint UI**:
- WBP_InventorySlot (visual de 1 item)
- WBP_Inventory (grid 5×10)
- Input Action (tecla I)
- Drag & Drop

✅ **Integração**:
- Character controla UI
- UI se comunica com C++
- C++ faz requisições HTTP
- Tudo sincronizado!

---

## 📚 Próximos Passos Opcionais

1. **Tooltips**: Mostrar detalhes ao passar mouse
2. **Filtros**: Separar por tipo (armas, armaduras, etc)
3. **Equipar/Desequipar**: UI para slots de equipamento
4. **Busca**: Campo de texto para filtrar itens
5. **Ordenação**: Botões para ordenar (A-Z, raridade, etc)
6. **Animações**: Transições suaves ao abrir/fechar
7. **Sons**: Efeitos sonoros ao clicar, mover, etc
8. **Drag & Drop**: Arrastar itens entre slots

---

## 🖼️ Como Configurar os Ícones dos Itens

🔒 **SEGURANÇA PRIMEIRO**: O Data Table NO CLIENTE contém **APENAS ÍCONES** (texturas visuais). 

✅ **Todas as informações de gameplay** (stats, dano, valor, peso, raridade, etc.) **vêm da API**.  
✅ **Impossível hackear valores** modificando arquivos locais.  
✅ **Servidor é a única fonte confiável** de dados.

---

### **1️⃣ Importar as Imagens dos Ícones**

1. **Preparar os arquivos**:
   - Formato: `.png` ou `.tga` (com transparência)
   - Tamanho recomendado: `64x64`, `128x128` ou `256x256` pixels
   - Nomeie de forma organizada: `Icon_Sword_Iron.png`, `Icon_Potion_Health.png`, etc.

2. **Importar no Unreal**:
   - Abra o **Content Browser**
   - Crie uma pasta `Content/UI/Icons/Items/`
   - Arraste os arquivos `.png` para dentro da pasta
   - Unreal vai criar automaticamente os assets `Texture2D`

3. **Configurar importação** (opcional):
   - Clique direito na textura → **Edit**
   - Em **Compression Settings**: `UserInterface2D (RGBA)`
   - Em **Texture Group**: `UI`
   - Salve (Ctrl+S)

---

### **2️⃣ Criar Data Table de Mapeamento de Ícones**

🔒 **ATENÇÃO DE SEGURANÇA**: Este Data Table contém **APENAS** o mapeamento `ItemID → Ícone`. **SEM STATS, SEM VALORES, SEM DADOS SENSÍVEIS**.

#### A. Criar o Data Table:

1. **Content Browser** → Right Click → **Miscellaneous** → **Data Table**
2. **Pick Structure**: Procure por `UmbraItemIconMapping` ⭐
3. Nome: `DT_ItemIcons` ⭐
4. Salve em `Content/Data/`

#### B. Preencher com os ícones:

Abra `DT_ItemIcons` e adicione uma linha **para cada item** do seu banco MySQL:

**Exemplo para Item ID 1 (Espada de Ferro):**
- **Row Name**: `Item_1` ⭐ (formato obrigatório: `Item_X`)
- **Item ID**: `1` (corresponde ao `item_id` do MySQL)
- **Item Icon**: 🔍 **Clique e selecione** `Icon_Sword_Iron`

**É ISSO!** Apenas 2 campos por item:
1. `Item ID` (número)
2. `Item Icon` (textura)

**❌ NÃO há campos para:**
- Nome, descrição → vêm da API
- Stats (dano, defesa) → vêm da API
- Valor, peso → vêm da API
- Raridade, nível → vêm da API

Repita para **todos os 17 itens** do banco de dados.

---

### **3️⃣ Como o Sistema Funciona (Já Implementado no C++)**

✅ **O C++ JÁ ESTÁ CONFIGURADO** para buscar ícones de forma segura.

🔒 **SEGURANÇA**: A API retorna o item COMPLETO (nome, stats, valor, etc.), mas o ícone vem do Data Table local.

#### **Fluxo de Dados Seguro:**

```
┌─────────────────────────────────────────────────────────────┐
│  1. API PHP (Servidor) → Retorna TUDO                      │
│     {                                                       │
│       "item_id": 1,                                        │
│       "item_name": "Espada de Ferro",                     │
│       "damage": 25,        ← DADOS SENSÍVEIS              │
│       "value": 150,        ← IMPOSSÍVEL HACKEAR           │
│       "rarity": "rare",    ← VÊM DO SERVIDOR              │
│       "icon_path": "..."   ← Ignorado pelo cliente       │
│     }                                                      │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│  2. C++ ParseItemTemplate → Converte TODOS os dados da API │
│     FUmbraItemTemplate.ItemName = "Espada de Ferro"       │
│     FUmbraItemTemplate.Damage = 25                         │
│     FUmbraItemTemplate.Value = 150                         │
│     FUmbraItemTemplate.Rarity = Rare                       │
│     FUmbraItemTemplate.ItemIcon = nullptr (ainda)          │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│  3. C++ GetItemIconByID → Busca APENAS o ícone local      │
│     ItemID: 1 → DT_ItemIcons → UTexture2D*                │
│     (SEM STATS, SEM VALORES, SÓ VISUAL)                    │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│  4. C++ Combina → Dados da API + Ícone local               │
│     Slot.ItemTemplate = (dados da API)                     │
│     Slot.ItemTemplate.ItemIcon = (ícone local)             │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│  5. Blueprint Widget → Exibe tudo                          │
│     Texto: ItemName, Damage, Value (da API)                │
│     Imagem: ItemIcon (local)                               │
└─────────────────────────────────────────────────────────────┘
```

#### **Por Que É Seguro:**

✅ **Servidor é a fonte confiável** de todos os dados de gameplay  
✅ **Cliente não pode modificar** stats, valores ou raridade (vêm da API)  
✅ **Ícones são apenas visuais** (não afetam gameplay)  
✅ **Mesmo hackeando arquivos locais**, não muda dano/valor no servidor  
✅ **Validações de itens** são feitas no servidor (PHP + MySQL)

---

### **4️⃣ Configurar o Data Table no Game Instance**

1. Abra o **Project Settings**
2. Vá em **Maps & Modes** → **Game Instance Class**
3. Certifique-se de que está usando `UmbraGameInstance`
4. Abra o **World Settings** do seu level principal
5. Em **Game Instance**, na seção **Inventory**, procure `Item Icons Data Table` ⭐
6. Selecione `DT_ItemIcons` ⭐ que você criou

**Ou configure direto no Blueprint da GameInstance** (se existir):
- Abra `BP_UmbraGameInstance` (se houver)
- Na aba **Details**, procure `Item Icons Data Table` ⭐
- Selecione `DT_ItemIcons` ⭐

---

### **5️⃣ Testar**

1. **Recompile o C++**: Build → Compile
2. **Reinicie o Unreal Editor**
3. **Entre no jogo** e abra o inventário (tecla `I`)
4. **Os ícones agora devem aparecer** conforme configurado no Data Table!

---

### **📝 Checklist:**

- ✅ C++ modificado (`ItemIcon` adicionado + `FUmbraItemIconMapping` criado)
- ✅ Sistema de segurança implementado (dados da API + ícones locais)
- ✅ Função `GetItemIconByID` implementada (busca apenas ícones)
- ✅ `ParseInventorySlot` modificado para combinar API + ícones
- [ ] **Você deve fazer:** Importar imagens (`Content/UI/Icons/Items/`)
- [ ] **Você deve fazer:** Data Table criado (`DT_ItemIcons`)
- [ ] **Você deve fazer:** Todos os 17 ícones configurados no Data Table
- [ ] **Você deve fazer:** Data Table referenciado no GameInstance
- [ ] **Você deve fazer:** Blueprint usa `Set Brush from Texture` com `ItemIcon`

---

---

## 🆘 Precisa de Ajuda?

Se encontrar problemas:

1. **Verifique o Output Log** (Window → Developer Tools → Output Log)
2. **Teste a API** manualmente: `http://localhost/umbra_api/test_inventory.php`
3. **Recompile tudo**: C++ → Blueprint → PIE
4. **Leia os erros**: Mensagens de erro são suas amigas!

**Boa sorte e divirta-se criando seu inventário! 🚀**

