# Guia Completo: Implementação do Sistema de Seleção de Jogadores

## ⚠️ ATENÇÃO: Se você está tendo dificuldade para criar a struct no Blueprint

**Leia primeiro:** [`GUIA_SIMPLES_CRIAR_STRUCT_BLUEPRINT.md`](./GUIA_SIMPLES_CRIAR_STRUCT_BLUEPRINT.md)

Este guia simples explica EXATAMENTE como criar a struct `FUmbraRemotePlayerInfo` no Blueprint, passo a passo, sem confusão.

Este guia fornece instruções **passo a passo** e **extremamente detalhadas** para implementar o sistema completo de seleção de jogadores remotos no Unreal Engine 5.

## 📋 Índice

1. [Visão Geral do Sistema](#visão-geral-do-sistema)
2. [Pré-requisitos](#pré-requisitos)
3. [Passo 1: Configuração do Remote Actor (BP_RemotePlayer)](#passo-1-configuração-do-remote-actor-bp_remoteplayer)
4. [Passo 2: Configuração de Input Actions](#passo-2-configuração-de-input-actions)
5. [Passo 3: Configuração do PlayerController Blueprint](#passo-3-configuração-do-playercontroller-blueprint)
6. [Passo 4: Criação do Widget WBP_SelectedPlayerInfo](#passo-4-criação-do-widget-wbp_selectedplayerinfo)
7. [Passo 5: Criação do Widget WBP_PlayerContextMenu](#passo-5-criação-do-widget-wbp_playercontextmenu)
8. [Passo 6: Integração com NetMovementClient](#passo-6-integração-com-netmovementclient)
9. [Passo 7: Conectar Delegates no HUD/Level Blueprint](#passo-7-conectar-delegates-no-hudlevel-blueprint)
10. [Passo 8: Integração com GameInstance (Ações Sociais)](#passo-8-integração-com-gameinstance-ações-sociais)
11. [Testes e Troubleshooting](#testes-e-troubleshooting)
12. [Checklist Final](#checklist-final)

---

## Visão Geral do Sistema

### O que o sistema faz:

- **Clique Esquerdo** em um jogador remoto → Seleciona e mostra widget com informações (Nome, HP, MP, Nível)
- **Clique Direito** em um jogador remoto → Abre menu de contexto com opções (Seguir, Grupo, Trade, Amigo, etc.)

### Arquitetura:

```
┌─────────────────────────────────────────────────────────────┐
│              PlayerController (Blueprint)                    │
│  ┌───────────────────────────────────────────────────────┐  │
│  │  UmbraPlayerSelectionComponent (C++)                  │  │
│  │  - Detecta cliques via trace                          │  │
│  │  - Gerencia cache de jogadores remotos               │  │
│  │  - Dispara delegates para UI                         │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                          │
        ┌─────────────────┴─────────────────┐
        │                                     │
        ▼                                     ▼
┌──────────────────┐              ┌──────────────────────┐
│  WBP_Selected    │              │  WBP_PlayerContext   │
│  PlayerInfo      │              │  Menu                 │
│  (Info Widget)   │              │  (Menu de Ações)     │
└──────────────────┘              └──────────────────────┘
        │                                     │
        └─────────────────┬───────────────────┘
                          ▼
              ┌───────────────────────┐
              │  UmbraGameInstance     │
              │  (Ações Sociais)       │
              └───────────────────────┘
```

---

## Pré-requisitos

### ✅ O que já está implementado (C++):

1. ✅ `FUmbraRemotePlayerInfo` struct (em `UmbraDataStructures.h`)
2. ✅ `EUmbraPlayerContextAction` enum (em `UmbraDataStructures.h`)
3. ✅ `UUmbraPlayerSelectionComponent` (componente C++)
4. ✅ `AUmbraEternumUEPlayerController` (atualizado com funções de seleção)
5. ✅ `UUmbraGameInstance` (com todas as funções sociais)

### ⬜ O que você precisa fazer (Blueprint):

1. ⬜ Configurar `BP_RemotePlayer` (tag e collision)
2. ⬜ Criar Input Actions (`IA_SelectPlayer`, `IA_ContextMenu`)
3. ⬜ Configurar PlayerController Blueprint
4. ⬜ Criar `WBP_SelectedPlayerInfo`
5. ⬜ Criar `WBP_PlayerContextMenu`
6. ⬜ Conectar delegates
7. ⬜ Integrar com `BP_NetMovementClient`

---

## Passo 1: Configuração do Remote Actor (BP_RemotePlayer)

### 1.1. Abrir o Blueprint

1. No **Content Browser**, navegue até o Blueprint do Remote Player
2. **Double-click** em `BP_RemotePlayer` para abrir o editor

### 1.2. Adicionar Tag "RemotePlayer"

1. No painel **Components** (esquerda), selecione o **root component** (geralmente `BP_RemotePlayer (Autônomo)`)
2. No painel **Details** (direita), role até a seção **Actor**
3. Encontre o campo **Tags**
4. Clique no **+** ao lado de "Tags" para adicionar uma nova tag
5. Digite: `RemotePlayer` (exatamente assim, sem espaços)
6. Pressione **Enter**

**Resultado esperado:**
```
Actor > Tags:
  [0] RemotePlayer
```

### 1.3. Configurar Collision no Capsule Component

1. No painel **Components**, selecione o **Capsule Component** (geralmente chamado `CollisionCylinder`)
2. No painel **Details**, role até a seção **Collision**
3. Configure:
   - **Collision Presets**: Selecione `Pawn` (ou `Custom` se preferir)
   - **Collision Enabled**: Selecione `Collision Enabled (Query and Physics)` ou `Query Only`
   - **Generate Overlap Events**: ✅ **Marcado** (importante!)

**Resultado esperado:**
```
Collision:
  Collision Presets: Pawn
  Collision Enabled: Collision Enabled (Query and Physics)
  Generate Overlap Events: ✅
```

### 1.4. Compilar e Salvar

1. Clique em **Compile** (botão no topo)
2. Verifique se não há erros no **Output Log**
3. Clique em **Save**

**✅ Passo 1 Concluído!**

---

## Passo 2: Configuração de Input Actions

### 2.1. Criar Pasta de Input (se não existir)

1. No **Content Browser**, navegue até `Content/`
2. Se não existir, crie uma pasta chamada `Input`:
   - **Right Click** em `Content/` → **New Folder** → Nome: `Input`

### 2.2. Criar IA_SelectPlayer

1. **Right Click** na pasta `Content/Input/`
2. Selecione: **Input** → **Input Action**
3. Nomeie: `IA_SelectPlayer`
4. **Double-click** para abrir
5. No painel **Details**, configure:
   - **Value Type**: `Digital (bool)`
   - **Consume Input**: ✅ (opcional, mas recomendado)
6. **Save** e feche

### 2.3. Criar IA_ContextMenu

1. **Right Click** na pasta `Content/Input/`
2. Selecione: **Input** → **Input Action**
3. Nomeie: `IA_ContextMenu`
4. **Double-click** para abrir
5. No painel **Details**, configure:
   - **Value Type**: `Digital (bool)`
   - **Consume Input**: ✅ (opcional, mas recomendado)
6. **Save** e feche

### 2.4. Configurar Input Mapping Context (IMC_Default)

1. No **Content Browser**, encontre `IMC_Default` (geralmente em `Content/Input/`)
2. Se não existir, crie:
   - **Right Click** em `Content/Input/` → **Input** → **Input Mapping Context**
   - Nomeie: `IMC_Default`
3. **Double-click** em `IMC_Default` para abrir

### 2.5. Adicionar Mappings ao IMC_Default

#### Mapping 1: IA_SelectPlayer → Left Mouse Button

1. No painel **Details**, encontre a seção **Mappings**
2. Clique no **+** ao lado de "Mappings"
3. Configure:
   - **Action**: Selecione `IA_SelectPlayer` (dropdown)
   - **Key**: Clique no dropdown e selecione `Left Mouse Button`
4. Deixe as outras opções como padrão

#### Mapping 2: IA_ContextMenu → Right Mouse Button

1. Clique no **+** novamente para adicionar outro mapping
2. Configure:
   - **Action**: Selecione `IA_ContextMenu` (dropdown)
   - **Key**: Selecione `Right Mouse Button`

**Resultado esperado:**
```
Mappings:
  [0] IA_SelectPlayer → Left Mouse Button
  [1] IA_ContextMenu → Right Mouse Button
```

### 2.6. Salvar IMC_Default

1. Clique em **Save**
2. Feche o editor

**✅ Passo 2 Concluído!**

---

## Passo 3: Configuração do PlayerController Blueprint

### 3.1. Encontrar/Criar o Blueprint do PlayerController

1. No **Content Browser**, procure por um Blueprint que herda de `UmbraEternumUEPlayerController`
2. Se não existir, crie:
   - **Right Click** em `Content/` → **Blueprint Class**
   - Na janela "Pick Parent Class", procure por `UmbraEternumUEPlayerController`
   - Selecione e clique em **Select**
   - Nomeie: `BP_UmbraPlayerController`

### 3.2. Abrir o Event Graph

1. **Double-click** no Blueprint do PlayerController
2. No painel **My Blueprint** (esquerda), clique em **EventGraph** (ou crie um novo)

### 3.3. Adicionar Event: IA_SelectPlayer (Started)

1. **Right Click** no Event Graph
2. Digite: `Enhanced Input Action IA_SelectPlayer`
3. Selecione: **Enhanced Input Action IA_SelectPlayer** (deve aparecer com ícone de raio)
4. No nó que aparece, conecte o pino **Started** (não Triggered!)
5. **Right Click** novamente e digite: `Handle Player Selection Click`
6. Selecione: **Handle Player Selection Click** (função do PlayerController)
7. **Conecte** o pino **Started** do evento ao pino **Execute** da função
8. Na função `Handle Player Selection Click`, configure:
   - **bRightClick**: `false` (ou deixe desmarcado)

**Visual do nó:**
```
[Enhanced Input Action IA_SelectPlayer]
    │
    └──► Started → [Handle Player Selection Click]
                        bRightClick = false
```

### 3.4. Adicionar Event: IA_ContextMenu (Started)

1. **Right Click** no Event Graph
2. Digite: `Enhanced Input Action IA_ContextMenu`
3. Selecione o evento
4. Conecte o pino **Started** a uma nova chamada de `Handle Player Selection Click`
5. Configure:
   - **bRightClick**: `true` (✅ marcado)

**Visual do nó:**
```
[Enhanced Input Action IA_ContextMenu]
    │
    └──► Started → [Handle Player Selection Click]
                        bRightClick = true ✅
```

### 3.5. Compilar e Salvar

1. Clique em **Compile**
2. Verifique se não há erros
3. Clique em **Save**

**✅ Passo 3 Concluído!**

---

## Passo 4: Criação do Widget WBP_SelectedPlayerInfo

### 4.1. Criar o Widget Blueprint

1. No **Content Browser**, navegue até `Content/Widgets/UI/` (ou crie a estrutura de pastas)
2. **Right Click** → **User Interface** → **Widget Blueprint**
3. Nomeie: `WBP_SelectedPlayerInfo`
4. **Double-click** para abrir o **Widget Designer**

### 4.2. Configurar Hierarchy (Designer)

#### 4.2.1. Adicionar Canvas Panel (Root)

1. No painel **Palette** (esquerda), arraste **Canvas Panel** para o **Hierarchy** (centro)
2. Se já houver um Canvas Panel, use-o

#### 4.2.2. Adicionar VerticalBox (Container Principal)

1. Arraste **Vertical Box** para dentro do **Canvas Panel**
2. No painel **Details**, configure:
   - **Anchors**: Clique no ícone de âncora → Selecione **Top Center**
   - **Size**: 
     - **Width**: `300`
     - **Height**: `Auto` (ou `150`)
   - **Position X**: `0` (centralizado)
   - **Position Y**: `50` (distância do topo)

#### 4.2.3. Adicionar Header (Nome e Nível)

1. Arraste **Horizontal Box** para dentro do **Vertical Box**
2. Nomeie: `HB_Header` (no painel **Details** → **Name**)
3. Dentro de `HB_Header`, adicione:
   - **Image** (nome: `Image_ClassIcon`)
     - **Size**: `32x32`
     - **Brush** → **Image**: (deixe vazio por enquanto ou adicione um ícone padrão)
   - **Vertical Box** (nome: `VB_HeaderText`)
     - Dentro de `VB_HeaderText`:
       - **Text Block** (nome: `Text_PlayerName`)
         - **Text**: `"Player Name"` (placeholder)
         - **Font**: **Bold**, **Size**: `16`
       - **Text Block** (nome: `Text_Level`)
         - **Text**: `"Lv. 1"` (placeholder)
         - **Font**: **Size**: `12`

#### 4.2.4. Adicionar HP Bar

1. Arraste **Horizontal Box** para dentro do **Vertical Box** (após o header)
2. Nomeie: `HB_HP`
3. Dentro de `HB_HP`, adicione:
   - **Text Block** (nome: `Text_HP_Label`)
     - **Text**: `"HP:"`
     - **Size**: `Auto`
   - **Progress Bar** (nome: `Progress_HP`)
     - **Size**: `Fill` (expande para preencher)
     - **Percent**: `0.85` (exemplo)
     - **Fill Color and Opacity**: **Red** (`R=1.0, G=0.0, B=0.0, A=1.0`)
   - **Text Block** (nome: `Text_HP`)
     - **Text**: `"850/1000"` (placeholder)
     - **Size**: `Auto`

#### 4.2.5. Adicionar MP Bar

1. Arraste **Horizontal Box** para dentro do **Vertical Box** (após HP)
2. Nomeie: `HB_MP`
3. Dentro de `HB_MP`, adicione (mesma estrutura do HP):
   - **Text Block** (nome: `Text_MP_Label`): `"MP:"`
   - **Progress Bar** (nome: `Progress_MP`): **Blue** (`R=0.0, G=0.5, B=1.0, A=1.0`)
   - **Text Block** (nome: `Text_MP`): `"200/500"` (placeholder)

**Hierarchy Final:**
```
Canvas Panel
└── VerticalBox (Anchor: Top Center)
    ├── HorizontalBox (HB_Header)
    │   ├── Image (Image_ClassIcon) [32x32]
    │   └── VerticalBox (VB_HeaderText)
    │       ├── TextBlock (Text_PlayerName) [Bold, 16pt]
    │       └── TextBlock (Text_Level) [12pt]
    ├── HorizontalBox (HB_HP)
    │   ├── TextBlock (Text_HP_Label) "HP:"
    │   ├── ProgressBar (Progress_HP) [Red]
    │   └── TextBlock (Text_HP) "850/1000"
    └── HorizontalBox (HB_MP)
        ├── TextBlock (Text_MP_Label) "MP:"
        ├── ProgressBar (Progress_MP) [Blue]
        └── TextBlock (Text_MP) "200/500"
```

### 4.3. Adicionar Variável

1. No painel **My Blueprint** (esquerda), vá para a aba **Variables**
2. Clique em **+ Variable**
3. Configure:
   - **Variable Name**: `CachedPlayerInfo`
   - **Variable Type**: `FUmbraRemotePlayerInfo` (struct)
   - **Category**: `Player Info`
   - **Instance Editable**: ❌ (não marcar)
   - **Expose on Spawn**: ❌

### 4.4. Criar Função: UpdateDisplay

1. No painel **My Blueprint**, vá para a aba **Functions**
2. Clique em **+ Function**
3. Nomeie: `UpdateDisplay`
4. **Double-click** para abrir o **Function Graph**

#### 4.4.1. Adicionar Input Parameter

1. No painel **Details** (direita), encontre **Inputs**
2. Clique em **+** para adicionar um input
3. Configure:
   - **Name**: `PlayerInfo`
   - **Type**: `FUmbraRemotePlayerInfo` (struct)
   - **Pass by Reference**: ✅ (recomendado)

#### 4.4.2. Implementar a Função

No **Function Graph**, adicione os seguintes nós:

```
[Function Entry: UpdateDisplay]
    │
    ├──► [Set CachedPlayerInfo] = PlayerInfo
    │
    ├──► [Text_PlayerName] → Set Text = PlayerInfo.CharacterName
    │
    ├──► [Text_Level] → Set Text = "Lv. " + ToString(PlayerInfo.Level)
    │
    ├──► [Progress_HP] → Set Percent = PlayerInfo.GetHealthPercent()
    │
    ├──► [Text_HP] → Set Text = 
    │       ToString(PlayerInfo.CurrentHealth) + "/" + ToString(PlayerInfo.MaxHealth)
    │
    ├──► [Progress_MP] → Set Percent = PlayerInfo.GetManaPercent()
    │
    └──► [Text_MP] → Set Text = 
            ToString(PlayerInfo.CurrentMana) + "/" + ToString(PlayerInfo.MaxMana)
```

**Passo a passo detalhado:**

1. **Set CachedPlayerInfo:**
   - **Right Click** → Digite: `Set CachedPlayerInfo`
   - Selecione o nó
   - Conecte o pino **PlayerInfo** (input) ao pino **CachedPlayerInfo** (target)

2. **Set Text_PlayerName:**
   - **Right Click** → Digite: `Set Text` (Text Block)
   - Selecione: **Set Text (Text Block)**
   - No dropdown **Target**, selecione `Text_PlayerName`
   - Conecte `PlayerInfo.CharacterName` ao pino **In Text**

3. **Set Text_Level:**
   - Adicione outro **Set Text**
   - Target: `Text_Level`
   - Use **Append String** para concatenar:
     - `"Lv. "` + `ToString(PlayerInfo.Level)`

4. **Set Progress_HP Percent:**
   - **Right Click** → Digite: `Set Percent` (Progress Bar)
   - Target: `Progress_HP`
   - Conecte `PlayerInfo.GetHealthPercent()` ao pino **In Percent**

5. **Set Text_HP:**
   - Adicione **Set Text** para `Text_HP`
   - Use **Append String** para: `ToString(CurrentHealth)` + `"/"` + `ToString(MaxHealth)`

6. **Repetir para MP** (mesma lógica do HP)

### 4.5. Compilar e Salvar

1. Clique em **Compile**
2. Verifique se não há erros
3. Clique em **Save**

**✅ Passo 4 Concluído!**

---

## Passo 5: Criação do Widget WBP_PlayerContextMenu

### 5.1. Criar o Widget Blueprint

1. **Right Click** em `Content/Widgets/UI/`
2. **User Interface** → **Widget Blueprint**
3. Nomeie: `WBP_PlayerContextMenu`
4. **Double-click** para abrir

### 5.2. Configurar Hierarchy

#### 5.2.1. Canvas Panel + Border

1. Adicione **Canvas Panel** (root)
2. Dentro, adicione **Border** (nome: `Border_Menu`)
   - **Anchors**: **Custom** (será posicionado via código)
   - **Size**: `Width=200, Height=Auto`
   - **Brush** → **Color and Opacity**: Escuro/Transparente (ex: `R=0.1, G=0.1, B=0.1, A=0.9`)

#### 5.2.2. VerticalBox (Container de Botões)

1. Dentro de `Border_Menu`, adicione **Vertical Box** (nome: `VB_MenuItems`)

#### 5.2.3. Header (Nome do Jogador)

1. Dentro de `VB_MenuItems`, adicione:
   - **Text Block** (nome: `Text_PlayerName`)
     - **Text**: `"Player Name"` (placeholder)
     - **Font**: **Bold**, **Size**: `14`
     - **Color**: Branco

#### 5.2.4. Botões do Menu

Para cada botão, adicione dentro de `VB_MenuItems`:

1. **Button** (nome: `BTN_Follow`)
   - Dentro do botão: **Text Block** com texto `"Seguir"`

2. **Button** (nome: `BTN_InviteParty`)
   - Text: `"Convidar para Grupo"`

3. **Button** (nome: `BTN_Trade`)
   - Text: `"Trocar"`

4. **Button** (nome: `BTN_AddFriend`)
   - Text: `"Adicionar Amigo"`

5. **Spacer** (altura: `8`) - Separador visual

6. **Button** (nome: `BTN_Whisper`)
   - Text: `"Sussurrar"`

7. **Button** (nome: `BTN_Inspect`)
   - Text: `"Inspecionar"`

8. **Spacer** (altura: `8`)

9. **Button** (nome: `BTN_Duel`)
   - Text: `"Duelar"`

10. **Button** (nome: `BTN_Report`)
    - Text: `"Denunciar"`

11. **Button** (nome: `BTN_Block`)
    - Text: `"Bloquear"`

**Hierarchy Final:**
```
Canvas Panel
└── Border (Border_Menu)
    └── VerticalBox (VB_MenuItems)
        ├── TextBlock (Text_PlayerName) [Header]
        ├── Spacer [Height: 4]
        ├── Button (BTN_Follow) → "Seguir"
        ├── Button (BTN_InviteParty) → "Convidar para Grupo"
        ├── Button (BTN_Trade) → "Trocar"
        ├── Button (BTN_AddFriend) → "Adicionar Amigo"
        ├── Spacer [Height: 8]
        ├── Button (BTN_Whisper) → "Sussurrar"
        ├── Button (BTN_Inspect) → "Inspecionar"
        ├── Spacer [Height: 8]
        ├── Button (BTN_Duel) → "Duelar"
        ├── Button (BTN_Report) → "Denunciar"
        └── Button (BTN_Block) → "Bloquear"
```

### 5.3. Adicionar Variáveis

No painel **My Blueprint** → **Variables**:

1. **CachedPlayerInfo**
   - Type: `FUmbraRemotePlayerInfo`
   - Category: `Player Info`

2. **SelectionComponent**
   - Type: `Umbra Player Selection Component` (Object Reference)
   - Category: `References`

### 5.4. Criar Função: ShowAtPosition

1. **+ Function** → Nome: `ShowAtPosition`
2. **Inputs:**
   - `PlayerInfo` (Type: `FUmbraRemotePlayerInfo`, Pass by Reference: ✅)
   - `ScreenPosition` (Type: `Vector 2D`)

3. **Function Graph:**
```
[Function Entry: ShowAtPosition]
    │
    ├──► [Set CachedPlayerInfo] = PlayerInfo
    │
    ├──► [Text_PlayerName] → Set Text = PlayerInfo.CharacterName
    │
    ├──► [Border_Menu] → Set Render Transform
    │       Position = ScreenPosition
    │
    └──► [Set Visibility] = Visible
```

**Detalhes:**
- **Set Render Transform**: 
  - Target: `Border_Menu`
  - **Translation**: Conecte `ScreenPosition` (X, Y)

### 5.5. Criar Função: Hide

1. **+ Function** → Nome: `Hide`
2. **Function Graph:**
```
[Function Entry: Hide]
    │
    └──► [Set Visibility] = Collapsed
```

### 5.6. Implementar Button Click Events

Para cada botão, adicione um evento **OnClicked**:

#### Exemplo: BTN_Follow

1. No **Designer**, selecione `BTN_Follow`
2. No painel **Details**, role até **Events**
3. Clique em **+ OnClicked**
4. Isso criará um evento no **Event Graph**

5. No **Event Graph**, conecte:
```
[BTN_Follow OnClicked]
    │
    ├──► [SelectionComponent] → Execute Context Action
    │       Action = Follow
    │
    └──► [Hide] (chama a função Hide)
```

**Repetir para todos os botões:**

- `BTN_InviteParty` → Action: `InviteToParty`
- `BTN_Trade` → Action: `Trade`
- `BTN_AddFriend` → Action: `AddFriend`
- `BTN_Whisper` → Action: `Whisper`
- `BTN_Inspect` → Action: `Inspect`
- `BTN_Duel` → Action: `Duel`
- `BTN_Report` → Action: `Report`
- `BTN_Block` → Action: `Block`

**Nota:** Se preferir chamar diretamente o GameInstance em vez do SelectionComponent, você pode usar:

```
[BTN_Trade OnClicked]
    │
    ├──► [Get Game Instance] → Cast to UmbraGameInstance
    │       │
    │       └──► [Request Trade]
    │               TargetPlayerID = CachedPlayerInfo.PlayerID
    │               TargetPlayerName = CachedPlayerInfo.CharacterName
    │
    └──► [Hide]
```

### 5.7. Compilar e Salvar

1. **Compile**
2. Verifique erros
3. **Save**

**✅ Passo 5 Concluído!**

---

## Passo 6: Integração com NetMovementClient

### 6.1. Encontrar o Blueprint BP_NetMovementClient

1. No **Content Browser**, procure por `BP_NetMovementClient`
2. **Double-click** para abrir

### 6.2. Registrar Jogador ao Spawnar

Quando um jogador remoto é criado (spawnado), você precisa registrá-lo no `PlayerSelectionComponent`.

#### 6.2.1. Localizar EXATAMENTE Onde Adicionar o Código

**📍 LOCALIZAÇÃO EXATA:** A função `ProcessNextFrame` no Blueprint `BP_NetMovementClient`

**Fluxo do Sistema:**
```
OnWSBinaryMessage (Event) 
    ↓
ProcessNextFrame (Function/Custom Event)
    ↓
ParseStateUpdateFrame (decodifica dados)
    ↓
Branch: Actor existe? (Array_Find)
    ├─ True: Atualiza actor existente
    └─ False: Spawna novo actor ← AQUI!
```

**📍 **Passo a Passo para Encontrar o Local Exato:**

1. **Abrir o Blueprint:**
   - No **Content Browser**, encontre `BP_NetMovementClient`
   - **Double-click** para abrir

2. **Encontrar a Função ProcessNextFrame:**
   - No painel **My Blueprint** (esquerda), vá para a aba **Functions**
   - Procure por: `ProcessNextFrame`
   - **Double-click** para abrir o **Function Graph**

3. **Localizar o Spawn do Actor:**
   - Dentro de `ProcessNextFrame`, procure por um **Branch** (If-Then-Else)
   - Este Branch verifica se o actor já existe (usando `Array_Find`)
   - O **pino `False`** (ou `Else`) deste Branch é onde o **novo actor é spawnado**

4. **Estrutura Visual Esperada:**
   ```
   [ProcessNextFrame Function]
       │
       ├──► [ParseStateUpdateFrame] → OutPlayerId, OutLocation, OutYawDegrees
       │
       ├──► [Array_Find] (RemoteActorIds, OutPlayerId) → FoundIndex
       │
       ├──► [Branch: FoundIndex >= 0?]
       │       │
       │       ├──► True (Actor existe):
       │       │       └──► [Get Array Item] → ExistingActorRef
       │       │               └──► [Atualiza posição/rotação]
       │       │
       │       └──► False (Novo Actor) ← ⭐ AQUI É ONDE VOCÊ ADICIONA O CÓDIGO!
       │               │
       │               ├──► [SpawnActorFromClass] → SpawnedActor
       │               ├──► [Set Variable: RemoteActorRef = SpawnedActor]
       │               ├──► [Array_Add] (RemoteActorIds, OutPlayerId)
       │               ├──► [Array_Add] (RemoteActors, SpawnedActor)
       │               │
       │               └──► ⭐ ADICIONE O CÓDIGO DE REGISTRO AQUI (após Array_Add)
   ```

**📍 Localização EXATA no Blueprint:**

Após estes nós (na ordem):
1. ✅ `SpawnActorFromClass` (spawna o `BP_RemotePlayer`)
2. ✅ `Set Variable: RemoteActorRef = SpawnedActor` (ou similar)
3. ✅ `Array_Add (RemoteActorIds, OutPlayerId)`
4. ✅ `Array_Add (RemoteActors, SpawnedActor)`

**⭐ ADICIONE O CÓDIGO DE REGISTRO IMEDIATAMENTE APÓS o último `Array_Add`**

**Visual no Blueprint:**
```
[Array_Add (RemoteActors, SpawnedActor)]
    │
    └──► [EXECUTE PIN] → ⭐ CONECTE SEU CÓDIGO AQUI
            │
            └──► [Seu código de registro do jogador]
```

#### 6.2.2. Adicionar Lógica de Registro - LOCAL EXATO

**📍 ONDE ADICIONAR:** Imediatamente após `Array_Add (RemoteActors, SpawnedActor)` no branch `False` da função `ProcessNextFrame`

**Estrutura Exata do Código a Adicionar:**

```
[Array_Add (RemoteActors, SpawnedActor)]
    │
    └──► [EXECUTE PIN] → [Seu código começa aqui]
            │
            ├──► [Add Actor Tag] (OPCIONAL - se BP_RemotePlayer não tem tag no editor)
            │       Target = SpawnedActor
            │       Tag = "RemotePlayer"
            │       │
            │       └──► [Create Local Variable: NewPlayerInfo (FUmbraRemotePlayerInfo)]
            │               │
            │               └──► [Set Members in Struct]
            │                       Target = NewPlayerInfo
            │                       (preencher campos - ver abaixo)
            │                       │
            │                       Struct (saída) → [Get Player Controller]
            │                                           │
            │                                           └──► [Cast to UmbraEternumUEPlayerController]
            │                                                   │
            │                                                   └──► [Get Player Selection Component]
            │                                                           │
            │                                                           └──► [Register Remote Player]
            │                                                                   PlayerInfo = [Struct]
```

**⚠️ LEIA PRIMEIRO:** Se você está tendo dificuldade para criar a struct, veja o guia simples: [`GUIA_SIMPLES_CRIAR_STRUCT_BLUEPRINT.md`](./GUIA_SIMPLES_CRIAR_STRUCT_BLUEPRINT.md)

**Passo a Passo EXATO:**

1. **Localizar o Último Array_Add:**
   - No branch `False` de `ProcessNextFrame`
   - Encontre o nó: `Array_Add (RemoteActors, SpawnedActor)`
   - Este é o **último nó antes de convergir** com o branch `True`

2. **Criar a Struct (MÉTODO SIMPLES):**
   
   **Opção A: Se "Make FUmbraRemotePlayerInfo" aparecer:**
   - **Right Click** após o `Array_Add` → Digite: `Make FUmbraRemotePlayerInfo`
   - Se aparecer, use este nó e preencha os campos (veja passo 3)
   
   **Opção B: Se "Make" NÃO aparecer:**
   - **Right Click** → Digite: `Local Variable`
   - **Variable Type**: `FUmbraRemotePlayerInfo`
   - **Name**: `NewPlayerInfo`
   - Arraste a variável para o graph → Selecione **"Break FUmbraRemotePlayerInfo"**
   - Preencha os campos do Break
   - **Right Click** → Digite: `Make FUmbraRemotePlayerInfo` (agora deve aparecer)
   - Conecte todos os pinos do Break para o Make

3. **Preencher Campos do Struct:**
   
   **Campos que você TEM disponíveis em ProcessNextFrame:**
   - `OutPlayerId` (do `ParseStateUpdateFrame`) → **PlayerID**
   - `SpawnedActor` (do `SpawnActorFromClass`) → **RemoteActor** ⭐

   **Campos que você NÃO TEM (use valores fixos):**
   - **CharacterName**: `"Player_" + ToString(OutPlayerId)` (use `Append String`)
   - **Level**: `1` (use `Make Literal Int`)
   - **CurrentHealth**: `100` (use `Make Literal Int`)
   - **MaxHealth**: `100` (use `Make Literal Int`)
   - **CurrentMana**: `50` (use `Make Literal Int`)
   - **MaxMana**: `50` (use `Make Literal Int`)
   - **ClassID, GuildID**: `0` (use `Make Literal Int`)
   - **ClassName, GuildName, TitleName**: `""` (use `Make Literal String` vazio)
   - **bIsPvPEnabled, bIsInCombat**: `false` (use `Make Literal Bool`)

4. **Conectar ao PlayerSelectionComponent:**
   - Do pino de saída do **Make FUmbraRemotePlayerInfo** (ou variável)
   - **Right Click** → `Get Player Controller`
   - **Right Click** → `Cast to UmbraEternumUEPlayerController`
   - Do Cast, chame `Get Player Selection Component`
   - Do Component, chame `Register Remote Player`
   - Conecte a **struct** ao parâmetro `PlayerInfo`

**Visual Completo do Código a Adicionar:**

```
[Array_Add (RemoteActors, SpawnedActor)]
    │
    └──► [Add Actor Tag] (OPCIONAL - pule se BP_RemotePlayer já tem tag no editor)
            Target = SpawnedActor
            Tag = "RemotePlayer"
            │
            └──► [Create Local Variable: NewPlayerInfo (FUmbraRemotePlayerInfo)]
                    │
                    └──► [Set Members in Struct (FUmbraRemotePlayerInfo)]
                            Target = NewPlayerInfo
                            │
                            PlayerID = OutPlayerId ⭐ (do ParseStateUpdateFrame)
                            CharacterName = "Player_" + ToString(OutPlayerId)
                            Level = [Make Literal Int: 1]
                            CurrentHealth = [Make Literal Int: 100]
                            MaxHealth = [Make Literal Int: 100]
                            CurrentMana = [Make Literal Int: 50]
                            MaxMana = [Make Literal Int: 50]
                            ClassID = [Make Literal Int: 0]
                            ClassName = [Make Literal String: ""]
                            GuildID = [Make Literal Int: 0]
                            GuildName = [Make Literal String: ""]
                            TitleName = [Make Literal String: ""]
                            RemoteActor = SpawnedActor ⭐ (do SpawnActorFromClass)
                            bIsPvPEnabled = [Make Literal Bool: false]
                            bIsInCombat = [Make Literal Bool: false]
                            │
                            Struct (saída) → [Get Player Controller]
                                                │
                                                └──► [Cast to UmbraEternumUEPlayerController]
                                                        │
                                                        └──► [Get Player Selection Component]
                                                                │
                                                                └──► [Register Remote Player]
                                                                        PlayerInfo = [Struct do Set Members]
```

**⚠️ IMPORTANTE:** 
- Este código deve estar no **pino de execução (Then)** do último `Array_Add`
- Deve estar **ANTES** do ponto de convergência com o branch `True`
- Não deve interferir com a atualização de posição/rotação que vem depois

**🔍 Como Identificar Visualmente no Blueprint:**

1. **Procure por esta sequência de nós:**
   ```
   [SpawnActorFromClass]
       │
       └──► [Set Variable: RemoteActorRef = ...]
               │
               └──► [Array_Add] (RemoteActorIds, ...)
                       │
                       └──► [Array_Add] (RemoteActors, ...)
                               │
                               └──► ⭐ AQUI! (pino de execução)
   ```

2. **Verifique se há um "CONVERGE" ou "MERGE" depois:**
   - Se houver um nó que une os branches `True` e `False`, seu código deve estar **ANTES** desse nó
   - Geralmente há `Set Actor Location` ou `Set Actor Rotation` após a convergência

3. **Confirmação Visual:**
   - O último `Array_Add` deve ter um pino de execução (seta branca) saindo dele
   - Este pino deve estar **vazio** ou conectado a algo que você adicionou
   - Se estiver conectado diretamente a `Set Actor Location`, você precisa **quebrar essa conexão** e inserir seu código no meio

**Exemplo de Como Deve Ficar:**

**ANTES (sem seu código):**
```
[Array_Add (RemoteActors, SpawnedActor)]
    │
    └──► [Set Actor Location] ← Conexão direta
```

**DEPOIS (com seu código):**
```
[Array_Add (RemoteActors, SpawnedActor)]
    │
    └──► [Add Actor Tag] (OPCIONAL - pule se BP_RemotePlayer já tem tag) ← Seu código começa aqui
            │
            └──► [Create Local Variable: NewPlayerInfo] ← OU comece direto aqui se pular a tag
                    │
                    └──► [Set Members in Struct]
                    │
                    └──► [Register Remote Player]
                            │
                            └──► [Set Actor Location] ← Continua para atualização
```

**Se você ver uma conexão direta do `Array_Add` para `Set Actor Location`:**
1. **Selecione a conexão** (clique na linha)
2. **Delete** (Delete ou Backspace)
3. **Adicione seu código** no meio
4. **Reconecte** o final do seu código ao `Set Actor Location`

**Alternativa: Se "Add Actor Tag" também não aparecer:**
- **Pule completamente** o passo de adicionar tag via código
- Certifique-se apenas de que o `BP_RemotePlayer` tem a tag `RemotePlayer` configurada no editor (Passo 1.2)
- Comece direto com `Create Local Variable: NewPlayerInfo`

Após o spawn do Remote Actor, você precisa criar a struct `FUmbraRemotePlayerInfo` e registrá-la no `PlayerSelectionComponent`.

**⚠️ IMPORTANTE:** No Unreal Engine, não existe um nó "Make FUmbraRemotePlayerInfo" direto. Você precisa usar **"Set Members in Struct"** ou criar uma variável local.

##### Método 1: Usando "Set Members in Struct" (Recomendado)

**Passo a passo detalhado:**

1. **Localizar o Event de Spawn:**
   - No **Event Graph** do `BP_NetMovementClient`, encontre onde o Remote Actor é spawnado
   - Geralmente há um evento como `OnWSBinaryMessage` que processa mensagens e spawna actors
   - Ou um evento customizado como `OnReceivePlayerSpawn` ou `OnPlayerJoined`

2. **Criar Variável Local (Temporária):**
   - **Right Click** no Event Graph → Digite: `Local Variable`
   - Selecione: **Local Variable** (ou use **Promote to Variable** depois)
   - Configure:
     - **Variable Type**: `FUmbraRemotePlayerInfo` (struct)
     - **Name**: `TempPlayerInfo` (ou qualquer nome)

3. **Criar o Nó "Set Members in Struct":**
   - **Right Click** no Event Graph → Digite: `Set Members in Struct`
   - Selecione: **Set Members in Struct (FUmbraRemotePlayerInfo)**
   - Este nó aparecerá com TODOS os campos da struct expostos

4. **Conectar a Variável Local:**
   - Conecte a variável `TempPlayerInfo` ao pino **Target** (ou **Struct**) do nó "Set Members in Struct"
   - O nó agora terá um pino de saída **Struct** com os valores atualizados

5. **Preencher os Campos:**
   
   No nó "Set Members in Struct", você verá todos os campos da struct. Preencha cada um:

   - **PlayerID**: 
     - Conecte o valor recebido do servidor (ex: variável `ReceivedPlayerID` ou parâmetro do evento)
     - Se não tiver, use o `PlayerID` que você recebeu quando spawnou o actor
   
   - **CharacterName**: 
     - Conecte uma **String** (ex: variável `ReceivedPlayerName` ou `"Player_" + ToString(PlayerID)`)
     - Se não tiver o nome, você pode usar: `"Player_" + ToString(PlayerID)` temporariamente
   
   - **Level**: 
     - Conecte um **Integer** (ex: `ReceivedLevel` ou `1` como padrão)
   
   - **CurrentHealth**: 
     - Conecte um **Integer** (ex: `ReceivedHP` ou `100` como padrão)
   
   - **MaxHealth**: 
     - Conecte um **Integer** (ex: `ReceivedMaxHP` ou `100` como padrão)
   
   - **CurrentMana**: 
     - Conecte um **Integer** (ex: `ReceivedMP` ou `50` como padrão)
   
   - **MaxMana**: 
     - Conecte um **Integer** (ex: `ReceivedMaxMP` ou `50` como padrão)
   
   - **ClassID**: 
     - Conecte um **Integer** (ex: `ReceivedClassID` ou `0` se não disponível)
   
   - **ClassName**: 
     - Conecte uma **String** (ex: `ReceivedClassName` ou `""` se não disponível)
   
   - **GuildID**: 
     - Conecte um **Integer** (ex: `ReceivedGuildID` ou `0` se não disponível)
   
   - **GuildName**: 
     - Conecte uma **String** (ex: `ReceivedGuildName` ou `""` se não disponível)
   
   - **TitleName**: 
     - Conecte uma **String** (ex: `ReceivedTitleName` ou `""` se não disponível)
   
   - **RemoteActor**: 
     - **IMPORTANTE:** Conecte a referência ao **Actor spawnado**
     - Se você tem uma variável `SpawnedActor` ou `NewRemoteActor`, conecte aqui
     - Este é o Actor que representa o jogador remoto no mundo
   
   - **bIsPvPEnabled**: 
     - Conecte um **Boolean** (ex: `false` como padrão)
   
   - **bIsInCombat**: 
     - Conecte um **Boolean** (ex: `false` como padrão)

6. **Obter o Struct Atualizado:**
   - O pino de saída **Struct** do nó "Set Members in Struct" contém a struct completa
   - Use este pino para passar para a próxima função

7. **Registrar no PlayerSelectionComponent:**
   ```
   [Set Members in Struct] → Struct (saída)
       │
       └──► [Get Player Controller]
               │
               └──► [Cast to UmbraEternumUEPlayerController]
                       │
                       └──► [Get Player Selection Component]
                               │
                               └──► [Register Remote Player]
                                       PlayerInfo = [Struct do Set Members]
   ```

**Visual Completo do Fluxo:**
```
[Event: OnReceivePlayerSpawn ou OnWSBinaryMessage]
    │
    ├──► [Spawn Remote Actor] → SpawnedActor (variável)
    │
    ├──► [Create Local Variable: TempPlayerInfo (FUmbraRemotePlayerInfo)]
    │
    ├──► [Set Members in Struct]
    │       Target = TempPlayerInfo
    │       │
    │       PlayerID = [ReceivedPlayerID]
    │       CharacterName = [ReceivedPlayerName] ou "Player_" + ToString(PlayerID)
    │       Level = [ReceivedLevel] ou 1
    │       CurrentHealth = [ReceivedHP] ou 100
    │       MaxHealth = [ReceivedMaxHP] ou 100
    │       CurrentMana = [ReceivedMP] ou 50
    │       MaxMana = [ReceivedMaxMP] ou 50
    │       ClassID = [ReceivedClassID] ou 0
    │       ClassName = [ReceivedClassName] ou ""
    │       GuildID = [ReceivedGuildID] ou 0
    │       GuildName = [ReceivedGuildName] ou ""
    │       TitleName = [ReceivedTitleName] ou ""
    │       RemoteActor = [SpawnedActor] ⭐ IMPORTANTE
    │       bIsPvPEnabled = false
    │       bIsInCombat = false
    │
    │       Struct (saída) → [Get Player Controller]
    │                           │
    │                           └──► [Cast to UmbraEternumUEPlayerController]
    │                                   │
    │                                   └──► [Get Player Selection Component]
    │                                           │
    │                                           └──► [Register Remote Player]
    │                                                   PlayerInfo = [Struct]
```

##### Método 2: Usando Variável de Classe (Alternativa)

Se preferir usar uma variável de classe em vez de local:

1. No painel **My Blueprint** → **Variables**, crie:
   - **Name**: `TempPlayerInfo`
   - **Type**: `FUmbraRemotePlayerInfo`
   - **Instance Editable**: ❌

2. No Event Graph:
   - Use `Set TempPlayerInfo` em vez de variável local
   - O resto é igual ao Método 1

##### Exemplo Prático: Se Você Recebe Dados via WebSocket

Se o `BP_NetMovementClient` recebe dados binários e você precisa decodificar:

```
[OnWSBinaryMessage] → Data (TArray<uint8>)
    │
    ├──► [Decode Binary Message] → PlayerID, PlayerName, Level, HP, MP, etc.
    │
    ├──► [Spawn Actor BP_RemotePlayer] → SpawnedActor
    │
    └──► [Set Members in Struct]
            (preencher com dados decodificados + SpawnedActor)
            │
            └──► [Register Remote Player]
```

##### Exemplo Prático: Se Você Já Tem os Dados em Variáveis

Se você já tem variáveis com os dados do jogador:

```
[Event: OnPlayerJoined]
    PlayerID = 123
    PlayerName = "TestPlayer"
    Level = 50
    HP = 850
    MaxHP = 1000
    │
    ├──► [Spawn Actor BP_RemotePlayer] → SpawnedActor
    │
    └──► [Set Members in Struct]
            PlayerID = PlayerID (do evento)
            CharacterName = PlayerName (do evento)
            Level = Level (do evento)
            CurrentHealth = HP (do evento)
            MaxHealth = MaxHP (do evento)
            RemoteActor = SpawnedActor ⭐
            (outros campos com valores padrão)
            │
            └──► [Register Remote Player]
```

**⚠️ DICA IMPORTANTE:** Se você não tiver todos os dados imediatamente (ex: nome, classe), pode:
1. Registrar com dados básicos primeiro (PlayerID, RemoteActor)
2. Atualizar depois quando receber mais informações usando `UpdateRemotePlayerInfo`

##### Como Usar "Set Members in Struct" - Tutorial Visual

**Passo 1: Criar o Nó**
1. **Right Click** no Event Graph
2. Digite: `set members` ou `set members in struct`
3. Você verá uma lista de opções. Procure por:
   - **"Set Members in Struct (FUmbraRemotePlayerInfo)"**
   - Ou simplesmente **"Set Members in Struct"** e depois selecione o tipo `FUmbraRemotePlayerInfo`

**Passo 2: Entender a Estrutura do Nó**

O nó "Set Members in Struct" tem esta estrutura:
```
┌─────────────────────────────────────┐
│  Set Members in Struct              │
│  (FUmbraRemotePlayerInfo)           │
├─────────────────────────────────────┤
│  Target (FUmbraRemotePlayerInfo)    │ ← Conecte variável aqui
│                                     │
│  PlayerID (Integer)                 │ ← Preencha com valor
│  CharacterName (String)            │ ← Preencha com valor
│  Level (Integer)                   │ ← Preencha com valor
│  CurrentHealth (Integer)           │ ← Preencha com valor
│  MaxHealth (Integer)               │ ← Preencha com valor
│  CurrentMana (Integer)             │ ← Preencha com valor
│  MaxMana (Integer)                 │ ← Preencha com valor
│  ClassID (Integer)                 │ ← Preencha com valor
│  ClassName (String)                │ ← Preencha com valor
│  GuildID (Integer)                │ ← Preencha com valor
│  GuildName (String)                │ ← Preencha com valor
│  TitleName (String)                 │ ← Preencha com valor
│  RemoteActor (Actor Reference)     │ ← ⭐ Conecte o Actor spawnado
│  bIsPvPEnabled (Boolean)           │ ← Preencha com valor
│  bIsInCombat (Boolean)             │ ← Preencha com valor
│                                     │
│  Struct (FUmbraRemotePlayerInfo)   │ ← Saída: struct completa
└─────────────────────────────────────┘
```

**Passo 3: Conectar a Variável**

**Opção A: Variável Local (Recomendado para valores temporários)**
1. **Right Click** → Digite: `Local Variable`
2. Selecione: **Local Variable**
3. Configure o tipo como `FUmbraRemotePlayerInfo`
4. Conecte esta variável ao pino **Target** do "Set Members in Struct"

**Opção B: Variável de Classe**
1. No **My Blueprint** → **Variables**, crie uma variável `TempPlayerInfo` do tipo `FUmbraRemotePlayerInfo`
2. Use o nó `Set TempPlayerInfo` e conecte ao **Target**

**Passo 4: Preencher os Campos**

Para cada campo do nó:
- **Se você tem o valor:** Conecte diretamente (ex: variável `ReceivedPlayerID`)
- **Se você não tem o valor:** Use um valor padrão:
  - **Integer**: Use o nó **Make Literal Int** com valor `0`, `1`, `100`, etc.
  - **String**: Use o nó **Make Literal String** com valor `""` ou `"Player_1"`
  - **Boolean**: Use o nó **Make Literal Bool** com valor `false` ou `true`
  - **Actor Reference**: Conecte a variável que contém o Actor spawnado

**Exemplo de Preenchimento:**

```
[Set Members in Struct]
    Target = [TempPlayerInfo] (variável local)
    │
    ├──► PlayerID = [ReceivedPlayerID] (do evento ou variável)
    ├──► CharacterName = [ReceivedPlayerName] ou [Make Literal String: "Player_1"]
    ├──► Level = [ReceivedLevel] ou [Make Literal Int: 1]
    ├──► CurrentHealth = [ReceivedHP] ou [Make Literal Int: 100]
    ├──► MaxHealth = [ReceivedMaxHP] ou [Make Literal Int: 100]
    ├──► CurrentMana = [ReceivedMP] ou [Make Literal Int: 50]
    ├──► MaxMana = [ReceivedMaxMP] ou [Make Literal Int: 50]
    ├──► ClassID = [ReceivedClassID] ou [Make Literal Int: 0]
    ├──► ClassName = [ReceivedClassName] ou [Make Literal String: ""]
    ├──► GuildID = [ReceivedGuildID] ou [Make Literal Int: 0]
    ├──► GuildName = [ReceivedGuildName] ou [Make Literal String: ""]
    ├──► TitleName = [ReceivedTitleName] ou [Make Literal String: ""]
    ├──► RemoteActor = [SpawnedActor] ⭐ (variável que contém o Actor spawnado)
    ├──► bIsPvPEnabled = [Make Literal Bool: false]
    └──► bIsInCombat = [Make Literal Bool: false]
    │
    └──► Struct (saída) → [Próximo nó]
```

**Passo 5: Usar a Struct de Saída**

O pino **Struct** (saída) do nó contém a struct completa com todos os valores preenchidos. Use este pino para:
- Passar para `Register Remote Player`
- Passar para `Update Remote Player Info`
- Armazenar em uma variável

##### Exemplo Completo: Fluxo Completo de Registro

```
[Event: OnReceivePlayerSpawn]
    ReceivedPlayerID = 123
    ReceivedPlayerName = "TestPlayer"
    ReceivedLevel = 50
    │
    ├──► [Spawn Actor from Class]
    │       Class = BP_RemotePlayer
    │       Transform = [Calculated Transform]
    │       │
    │       └──► SpawnedActor (variável)
    │
    ├──► [Create Local Variable: TempPlayerInfo (FUmbraRemotePlayerInfo)]
    │
    ├──► [Set Members in Struct]
    │       Target = TempPlayerInfo
    │       │
    │       PlayerID = ReceivedPlayerID (123)
    │       CharacterName = ReceivedPlayerName ("TestPlayer")
    │       Level = ReceivedLevel (50)
    │       CurrentHealth = [Make Literal Int: 100]
    │       MaxHealth = [Make Literal Int: 100]
    │       CurrentMana = [Make Literal Int: 50]
    │       MaxMana = [Make Literal Int: 50]
    │       ClassID = [Make Literal Int: 0]
    │       ClassName = [Make Literal String: ""]
    │       GuildID = [Make Literal Int: 0]
    │       GuildName = [Make Literal String: ""]
    │       TitleName = [Make Literal String: ""]
    │       RemoteActor = SpawnedActor ⭐
    │       bIsPvPEnabled = [Make Literal Bool: false]
    │       bIsInCombat = [Make Literal Bool: false]
    │       │
    │       Struct (saída) → [Get Player Controller]
    │                           │
    │                           └──► [Cast to UmbraEternumUEPlayerController]
    │                                   │
    │                                   └──► [Get Player Selection Component]
    │                                           │
    │                                           └──► [Register Remote Player]
    │                                                   PlayerInfo = [Struct do Set Members]
    │
    └──► [Log] "Jogador registrado: TestPlayer (ID: 123)"
```

##### Troubleshooting: "Set Members in Struct" Não Aparece

Se você não encontrar o nó "Set Members in Struct":

1. **Verifique se a struct está exposta ao Blueprint:**
   - A struct `FUmbraRemotePlayerInfo` deve ter `USTRUCT(BlueprintType)`
   - Recompile o projeto C++ se necessário

2. **Tente digitar o nome completo:**
   - **Right Click** → Digite: `Set Members in FUmbraRemotePlayerInfo`
   - Ou: `Set Members in Struct FUmbraRemotePlayerInfo`

3. **Use "Break" e depois "Make":**
   - **Right Click** → Digite: `Break FUmbraRemotePlayerInfo`
   - Isso quebra a struct em campos individuais
   - Depois use `Make FUmbraRemotePlayerInfo` (se disponível) ou `Set Members in Struct`

4. **Alternativa: Criar uma Function no Blueprint:**
   - Crie uma **Function** chamada `CreatePlayerInfo`
   - Parâmetros de entrada: PlayerID, CharacterName, Level, etc.
   - Retorno: `FUmbraRemotePlayerInfo`
   - Dentro da função, use `Set Members in Struct` ou `Make` a struct
   - Chame esta função quando precisar criar a struct

### 6.3. Verificação: Confirmar que o Código Está no Local Correto

**Checklist Visual:**

Após adicionar o código, a estrutura deve ficar assim:

```
[ProcessNextFrame]
    │
    ├──► [ParseStateUpdateFrame] → OutPlayerId, OutLocation, OutYawDegrees
    │
    ├──► [Array_Find] → FoundIndex
    │
    ├──► [Branch: FoundIndex >= 0?]
    │       │
    │       ├──► True:
    │       │       └──► [Atualiza actor existente]
    │       │
    │       └──► False:
    │               ├──► [SpawnActorFromClass] → SpawnedActor
    │               ├──► [Set Variable: RemoteActorRef = SpawnedActor]
    │               ├──► [Array_Add (RemoteActorIds, OutPlayerId)]
    │               ├──► [Array_Add (RemoteActors, SpawnedActor)]
    │               │
    │               ├──► ⭐ [Add Actor Tag] (OPCIONAL - pule se BP_RemotePlayer já tem tag) ← SEU CÓDIGO COMEÇA AQUI
               ├──► ⭐ [Create Local Variable: NewPlayerInfo] ← OU comece direto aqui se pular a tag
    │               ├──► ⭐ [Set Members in Struct]
    │               ├──► ⭐ [Get Player Controller]
    │               ├──► ⭐ [Cast to UmbraEternumUEPlayerController]
    │               ├──► ⭐ [Get Player Selection Component]
    │               └──► ⭐ [Register Remote Player]
    │                       │
    │                       └──► [CONVERGE] (continua para atualização de posição)
    │
    └──► [Set Actor Location/Rotation] (após convergência)
```

**Se o código estiver correto:**
- ✅ Está no branch `False` (novo actor)
- ✅ Está após `Array_Add (RemoteActors, SpawnedActor)`
- ✅ Está antes da convergência com o branch `True`
- ✅ Não interfere com `Set Actor Location/Rotation`

### 6.4. Identificar Dados Disponíveis no Spawn

**Como saber quais dados você tem disponíveis:**

1. **Verifique o Event que Spawna o Actor:**
   - Olhe os **parâmetros de entrada** do evento
   - Olhe as **variáveis locais** criadas antes do spawn
   - Olhe as **variáveis de classe** que podem conter dados do jogador

2. **Dados Comuns que Podem Estar Disponíveis:**
   - `PlayerID` ou `PlayerId` (geralmente sempre disponível)
   - `PlayerName` ou `CharacterName` (pode estar disponível)
   - `Level` (pode estar disponível)
   - `HP`, `MaxHP`, `CurrentHealth`, `MaxHealth` (pode estar disponível)
   - `MP`, `MaxMP`, `CurrentMana`, `MaxMana` (pode estar disponível)
   - `ClassID`, `ClassName` (pode estar disponível)
   - `GuildID`, `GuildName` (pode não estar disponível inicialmente)
   - `Position`, `Rotation` (para spawn)

3. **Se Você Não Tem Todos os Dados:**
   - **Registre com dados básicos primeiro:**
     - Mínimo necessário: `PlayerID` e `RemoteActor`
     - Recomendado: `PlayerID`, `CharacterName`, `Level`, `RemoteActor`
   - **Atualize depois:**
     - Quando receber mais dados via WebSocket, use `UpdateRemotePlayerInfo`
     - Isso permite que o sistema funcione mesmo sem todos os dados imediatamente

4. **Exemplo de Dados Mínimos:**
   ```
   [Set Members in Struct]
       PlayerID = [ReceivedPlayerID] ⭐ OBRIGATÓRIO
       RemoteActor = [SpawnedActor] ⭐ OBRIGATÓRIO
       CharacterName = "Player_" + ToString(ReceivedPlayerID) (temporário)
       Level = 1 (padrão)
       CurrentHealth = 100 (padrão)
       MaxHealth = 100 (padrão)
       CurrentMana = 50 (padrão)
       MaxMana = 50 (padrão)
       (outros campos com valores padrão)
   ```

### 6.5. Atualizar Informações do Jogador

Quando receber updates de HP/MP via WebSocket ou outras informações:

**Método: Usar UpdateRemotePlayerInfo**

```
[On Receive Player Update]
    UpdatedPlayerID = 123
    UpdatedHP = 850
    UpdatedMaxHP = 1000
    UpdatedMP = 200
    UpdatedMaxMP = 500
    │
    ├──► [Get Player Selection Component]
    │       │
    │       └──► [Get Remote Player Info]
    │               PlayerID = UpdatedPlayerID
    │               OutPlayerInfo = [ExistingPlayerInfo] (variável local)
    │
    ├──► [Set Members in Struct]
    │       Target = ExistingPlayerInfo
    │       │
    │       CurrentHealth = UpdatedHP (850)
    │       MaxHealth = UpdatedMaxHP (1000)
    │       CurrentMana = UpdatedMP (200)
    │       MaxMana = UpdatedMaxMP (500)
    │       (outros campos mantêm valores existentes)
    │       │
    │       Struct (saída) → [Get Player Selection Component]
    │                           │
    │                           └──► [Update Remote Player Info]
    │                                   PlayerInfo = [Updated Struct]
```

**Passo a passo detalhado:**

1. **Obter Informações Existentes:**
   - Chame `Get Remote Player Info` do `PlayerSelectionComponent`
   - Isso retorna a struct atual do jogador (se existir)

2. **Atualizar Apenas Campos que Mudaram:**
   - Use `Set Members in Struct` na struct existente
   - Preencha apenas os campos que mudaram (ex: HP, MP)
   - Deixe os outros campos vazios (eles manterão os valores existentes)

3. **Registrar Atualização:**
   - Chame `Update Remote Player Info` com a struct atualizada
   - Isso atualizará o cache e, se o jogador estiver selecionado, atualizará o widget automaticamente

### 6.5. Remover Jogador ao Desconectar

Quando um jogador desconecta:

```
[On Player Disconnect]
    │
    └──► [Get Player Selection Component]
            │
            └──► [Unregister Remote Player]
                    PlayerID = [DisconnectedPlayerID]
```

### 6.7. Exemplo Prático Completo: Código Exato para ProcessNextFrame

**Cenário Real:** No `BP_NetMovementClient`, a função `ProcessNextFrame` já spawna o Remote Actor. Você só precisa adicionar o registro após o spawn.

**Estrutura Exata no ProcessNextFrame:**

```
[ProcessNextFrame Function]
    │
    ├──► [ParseStateUpdateFrame] → OutPlayerId, OutLocation, OutYawDegrees
    │
    ├──► [Array_Find (RemoteActorIds, OutPlayerId)] → FoundIndex
    │
    ├──► [Branch: FoundIndex >= 0?]
    │       │
    │       ├──► True: [Atualiza actor existente]
    │       │
    │       └──► False: [Novo actor - SPAWN]
    │               │
    │               ├──► [SpawnActorFromClass]
    │               │       Class = BP_RemotePlayer
    │               │       Transform = [Make Transform com OutLocation e OutYawDegrees]
    │               │       │
    │               │       └──► SpawnedActor (variável de saída)
    │               │
    │               ├──► [Set Variable: RemoteActorRef = SpawnedActor]
    │               │
    │               ├──► [Array_Add (RemoteActorIds, OutPlayerId)]
    │               │
    │               ├──► [Array_Add (RemoteActors, SpawnedActor)]
    │               │
    │               └──► ⭐ [AQUI ADICIONE SEU CÓDIGO DE REGISTRO]
    │                       │
    │                       ├──► [Add Actor Tag] (OPCIONAL - pule se BP_RemotePlayer já tem tag no editor)
    │                       │       Target = SpawnedActor
    │                       │       Tag = "RemotePlayer"
    │                       │
    │                       ├──► [Local Variable: NewPlayerInfo (FUmbraRemotePlayerInfo)]
    │                       │
    │                       ├──► [Set Members in Struct]
    │                       │       Target = NewPlayerInfo
    │                       │       PlayerID = OutPlayerId ⭐
    │                       │       CharacterName = "Player_" + ToString(OutPlayerId)
    │                       │       Level = 1
    │                       │       CurrentHealth = 100
    │                       │       MaxHealth = 100
    │                       │       CurrentMana = 50
    │                       │       MaxMana = 50
    │                       │       RemoteActor = SpawnedActor ⭐
    │                       │       (outros campos com padrões)
    │                       │       │
    │                       │       Struct → [Get Player Controller]
    │                       │                   │
    │                       │                   └──► [Cast to UmbraEternumUEPlayerController]
    │                       │                           │
    │                       │                           └──► [Get Player Selection Component]
    │                       │                                   │
    │                       │                                   └──► [Register Remote Player]
    │                       │                                           PlayerInfo = [Struct]
    │                       │
    │                       └──► [CONVERGE] (continua para atualização de posição)
    │
    └──► [Set Actor Location/Rotation] (após convergência)
```

**Dados Disponíveis em ProcessNextFrame:**
- ✅ `OutPlayerId` (do `ParseStateUpdateFrame`) → Use para `PlayerID`
- ✅ `OutLocation` (do `ParseStateUpdateFrame`) → Não usado na struct, mas disponível
- ✅ `OutYawDegrees` (do `ParseStateUpdateFrame`) → Não usado na struct, mas disponível
- ✅ `SpawnedActor` (do `SpawnActorFromClass`) → Use para `RemoteActor` ⭐

**Dados NÃO Disponíveis (use padrões):**
- ❌ `CharacterName` → Use: `"Player_" + ToString(OutPlayerId)`
- ❌ `Level` → Use: `1`
- ❌ `HP/MP` → Use: `100/100` e `50/50`
- ❌ `ClassID/ClassName` → Use: `0` e `""`
- ❌ `GuildID/GuildName` → Use: `0` e `""`

**Nota:** Você pode atualizar esses dados depois quando receber mais informações via WebSocket usando `UpdateRemotePlayerInfo`.

### 6.8. Exemplo Visual: Como Deve Ficar no Blueprint

**Antes (sem registro):**
```
[Array_Add (RemoteActors, SpawnedActor)]
    │
    └──► [CONVERGE] → [Set Actor Location]
```

**Depois (com registro):**
```
[Array_Add (RemoteActors, SpawnedActor)]
    │
    └──► [Add Actor Tag] (OPCIONAL - pule se BP_RemotePlayer já tem tag no editor)
            Target = SpawnedActor
            Tag = "RemotePlayer"
            │
            └──► [Create Local Variable: NewPlayerInfo] ← OU comece direto aqui se pular a tag
                    │
                    └──► [Set Members in Struct]
                            Target = NewPlayerInfo
                            PlayerID = OutPlayerId
                            CharacterName = "Player_" + ToString(OutPlayerId)
                            Level = 1
                            CurrentHealth = 100
                            MaxHealth = 100
                            CurrentMana = 50
                            MaxMana = 50
                            RemoteActor = SpawnedActor
                            (outros campos com padrões)
                            │
                            Struct → [Get Player Controller]
                                      │
                                      └──► [Cast to UmbraEternumUEPlayerController]
                                              │
                                              └──► [Get Player Selection Component]
                                                      │
                                                      └──► [Register Remote Player]
                                                              PlayerInfo = [Struct]
                                                              │
                                                              └──► [CONVERGE] → [Set Actor Location]
```

### 6.9. Compilar e Salvar

1. **Compile**
2. Verifique erros
3. **Save**

### 6.10. Troubleshooting: Problemas ao Criar a Struct

#### ❌ "Set Members in Struct" não aparece no menu

**Causa:** A struct pode não estar exposta ao Blueprint ou o projeto não foi recompilado.

**Solução:**
1. Verifique se `FUmbraRemotePlayerInfo` tem `USTRUCT(BlueprintType)` no código C++
2. **Recompile o projeto C++** (Build → Rebuild Project)
3. Feche e reabra o Blueprint
4. Tente novamente: **Right Click** → Digite: `set members in struct`

#### ❌ Erro: "Cannot find struct FUmbraRemotePlayerInfo"

**Causa:** O Blueprint não está encontrando a struct.

**Solução:**
1. Verifique se o arquivo `UmbraDataStructures.h` está incluído corretamente
2. Verifique se o módulo `UmbraEternumUE` está sendo compilado
3. **Recompile o projeto C++**
4. No Blueprint, tente criar uma variável do tipo `FUmbraRemotePlayerInfo` primeiro
   - Se aparecer no dropdown, a struct está disponível
   - Se não aparecer, há um problema de compilação

#### ❌ Erro ao conectar RemoteActor ao campo

**Causa:** O tipo do Actor não corresponde ao esperado.

**Solução:**
1. Certifique-se de que a variável `SpawnedActor` é do tipo `Actor` ou `BP_RemotePlayer`
2. Se necessário, use **Cast to Actor** antes de conectar
3. O campo `RemoteActor` aceita qualquer `AActor*`, então qualquer referência de Actor deve funcionar

#### ❌ Struct criada mas Register Remote Player não funciona

**Causa:** Algum campo obrigatório está faltando ou com valor inválido.

**Solução:**
1. Verifique se `PlayerID` é maior que 0
2. Verifique se `RemoteActor` não é `nullptr`
3. Adicione logs temporários:
   ```
   [Set Members in Struct] → Struct
       │
       └──► [Print String]
               In String = "PlayerID: " + ToString(Struct.PlayerID) + ", Actor: " + ToString(Struct.RemoteActor)
   ```
4. Verifique o **Output Log** para ver os valores

#### ❌ Widget não atualiza quando HP/MP mudam

**Causa:** `UpdateRemotePlayerInfo` não está sendo chamado ou a struct não está sendo atualizada corretamente.

**Solução:**
1. Certifique-se de chamar `UpdateRemotePlayerInfo` quando receber updates
2. Verifique se está usando a struct completa (não apenas campos individuais)
3. Verifique se o delegate `OnSelectedPlayerInfoUpdated` está conectado no HUD

### 6.9. Verificação: Testar se o Registro Funcionou

**Adicione logs temporários para verificar:**

```
[Register Remote Player]
    PlayerInfo = [Struct]
    │
    └──► [Print String]
            In String = "✅ Jogador registrado: " + PlayerInfo.CharacterName + " (ID: " + ToString(PlayerInfo.PlayerID) + ")"
```

**No Output Log, você deve ver:**
```
LogTemp: [UmbraPlayerSelection] ✅ Jogador registrado: TestPlayer (ID: 123)
```

**Se não aparecer:**
- Verifique se `Get Player Selection Component` está retornando um componente válido
- Verifique se o PlayerController é do tipo correto
- Verifique se o componente foi inicializado no `BeginPlay`

**✅ Passo 6 Concluído!**

### 6.12. Referência Rápida: Criar FUmbraRemotePlayerInfo

**Resumo rápido para consulta:**

1. **Right Click** → Digite: `Set Members in Struct` → Selecione `FUmbraRemotePlayerInfo`

2. **Criar variável local:**
   - **Right Click** → `Local Variable` → Tipo: `FUmbraRemotePlayerInfo`

3. **Conectar variável ao Target do nó**

4. **Preencher campos obrigatórios:**
   - `PlayerID` = [valor recebido]
   - `RemoteActor` = [Actor spawnado] ⭐

5. **Preencher campos opcionais (ou usar padrões):**
   - `CharacterName` = [nome] ou `"Player_" + ToString(PlayerID)`
   - `Level` = [nível] ou `1`
   - `CurrentHealth` = [HP] ou `100`
   - `MaxHealth` = [MaxHP] ou `100`
   - `CurrentMana` = [MP] ou `50`
   - `MaxMana` = [MaxMP] ou `50`
   - Outros campos com valores padrão

6. **Usar a saída Struct:**
   - Conecte ao `Register Remote Player` do `PlayerSelectionComponent`

**Valores Padrão Recomendados:**
```
PlayerID: [OBRIGATÓRIO - valor recebido]
CharacterName: "Player_" + ToString(PlayerID) (se não disponível)
Level: 1
CurrentHealth: 100
MaxHealth: 100
CurrentMana: 50
MaxMana: 50
ClassID: 0
ClassName: ""
GuildID: 0
GuildName: ""
TitleName: ""
RemoteActor: [OBRIGATÓRIO - Actor spawnado]
bIsPvPEnabled: false
bIsInCombat: false
```

---

## Passo 7: Conectar Delegates no HUD/Level Blueprint

### 7.1. Escolher Local: HUD ou Level Blueprint?

- **HUD Blueprint**: Se você tem um HUD que gerencia widgets
- **Level Blueprint**: Se prefere gerenciar no nível

**Recomendação:** Use um **HUD Blueprint** se possível.

### 7.2. Criar/Encontrar HUD Blueprint

1. Se não existir, crie:
   - **Right Click** → **Blueprint Class**
   - Parent: `HUD`
   - Nomeie: `BP_UmbraHUD`

### 7.3. Adicionar Variáveis de Widget

No **My Blueprint** → **Variables**:

1. **SelectedPlayerInfoWidget**
   - Type: `WBP_SelectedPlayerInfo` (Widget)
   - Category: `UI Widgets`
   - **Instance Editable**: ✅ (para testar no editor)

2. **ContextMenuWidget**
   - Type: `WBP_PlayerContextMenu` (Widget)
   - Category: `UI Widgets`
   - **Instance Editable**: ✅

### 7.4. Event BeginPlay - Criar Widgets

```
[Event BeginPlay]
    │
    ├──► [Create Widget] → Class: WBP_SelectedPlayerInfo
    │       │
    │       └──► [Set SelectedPlayerInfoWidget] = [Created Widget]
    │               │
    │               └──► [Add to Viewport]
    │                       ZOrder = 100 (ou outro valor alto)
    │
    └──► [Create Widget] → Class: WBP_PlayerContextMenu
            │
            └──► [Set ContextMenuWidget] = [Created Widget]
                    │
                    └──► [Add to Viewport]
                            ZOrder = 200 (mais alto que SelectedPlayerInfo)
```

**Inicialmente, os widgets devem estar ocultos:**
- Após `Add to Viewport`, adicione `Set Visibility` = `Collapsed`

### 7.5. Conectar Delegate: OnPlayerSelected

```
[Event BeginPlay]
    │
    └──► [Get Player Controller] → Cast to UmbraEternumUEPlayerController
            │
            └──► [Get Player Selection Component]
                    │
                    └──► [Bind Event to OnPlayerSelected]
                            │
                            └──► [CustomEvent: OnPlayerSelected]
                                    PlayerInfo = [PlayerInfo do Event]
                                    │
                                    ├──► [SelectedPlayerInfoWidget] → Set Visibility (Visible)
                                    │
                                    └──► [SelectedPlayerInfoWidget] → UpdateDisplay(PlayerInfo)
```

**Passo a passo:**

1. **Right Click** → `Get Player Controller`
2. **Right Click** → `Cast to UmbraEternumUEPlayerController`
3. Do Cast, chame `Get Player Selection Component`
4. Do Component, encontre `OnPlayerSelected` (delegate)
5. **Right Click** no delegate → `Assign OnPlayerSelected`
6. Isso criará um **Custom Event**
7. No Custom Event, adicione:
   - `Set Visibility` → Target: `SelectedPlayerInfoWidget`, Visibility: `Visible`
   - `UpdateDisplay` → Target: `SelectedPlayerInfoWidget`, PlayerInfo: `[PlayerInfo do Event]`

### 7.6. Conectar Delegate: OnPlayerDeselected

```
[Bind Event to OnPlayerDeselected]
    │
    └──► [CustomEvent: OnPlayerDeselected]
            │
            └──► [SelectedPlayerInfoWidget] → Set Visibility (Collapsed)
```

### 7.7. Conectar Delegate: OnPlayerContextMenuRequested

```
[Bind Event to OnPlayerContextMenuRequested]
    │
    └──► [CustomEvent: OnContextMenuRequested]
            PlayerInfo = [PlayerInfo do Event]
            ScreenPosition = [ScreenPosition do Event]
            │
            ├──► [ContextMenuWidget] → Set SelectionComponent
            │       (Get Player Selection Component)
            │
            └──► [ContextMenuWidget] → ShowAtPosition(PlayerInfo, ScreenPosition)
```

**Passo a passo:**

1. Bind `OnPlayerContextMenuRequested` do `PlayerSelectionComponent`
2. No Custom Event, você receberá `PlayerInfo` e `ScreenPosition`
3. Chame `ShowAtPosition` no `ContextMenuWidget`

### 7.8. Conectar Delegate: OnSelectedPlayerInfoUpdated

```
[Bind Event to OnSelectedPlayerInfoUpdated]
    │
    └──► [CustomEvent: OnInfoUpdated]
            PlayerInfo = [PlayerInfo do Event]
            │
            └──► [SelectedPlayerInfoWidget] → UpdateDisplay(PlayerInfo)
```

### 7.9. (Opcional) Conectar OnPlayerContextActionSelected

Se você quiser processar ações no HUD em vez de no widget do menu:

```
[Bind Event to OnPlayerContextActionSelected]
    │
    └──► [CustomEvent: OnContextAction]
            PlayerInfo = [PlayerInfo do Event]
            Action = [Action do Event]
            │
            └──► [Get Game Instance] → Cast to UmbraGameInstance
                    │
                    └──► [Handle Player Context Action]
                            PlayerInfo = [PlayerInfo]
                            Action = [Action]
```

### 7.10. Compilar e Salvar

1. **Compile**
2. Verifique erros
3. **Save**

**✅ Passo 7 Concluído!**

---

## Passo 8: Integração com GameInstance (Ações Sociais)

### 8.1. Verificar se GameInstance está Configurado

O `UmbraGameInstance` já possui todas as funções necessárias. Você só precisa chamá-las.

### 8.2. Opção A: Usar HandlePlayerContextAction (Recomendado)

Se você conectou o delegate `OnPlayerContextActionSelected` no HUD:

1. No Custom Event `OnContextAction`, chame:
```
[Get Game Instance] → Cast to UmbraGameInstance
    │
    └──► [Handle Player Context Action]
            PlayerInfo = [PlayerInfo]
            Action = [Action]
```

Isso automaticamente roteará para a função correta (SendPartyInvite, RequestTrade, etc.).

### 8.3. Opção B: Chamar Funções Individuais

Se preferir chamar diretamente do widget do menu:

#### Exemplo: BTN_Trade

No `WBP_PlayerContextMenu`, no evento `BTN_Trade OnClicked`:

```
[BTN_Trade OnClicked]
    │
    ├──► [Get Game Instance] → Cast to UmbraGameInstance
    │       │
    │       └──► [Request Trade]
    │               TargetPlayerID = CachedPlayerInfo.PlayerID
    │               TargetPlayerName = CachedPlayerInfo.CharacterName
    │
    └──► [Hide]
```

### 8.4. Conectar Delegates do GameInstance (Opcional)

Se você quiser mostrar notificações quando ações sociais acontecem:

No **HUD Blueprint** ou **Level Blueprint**:

```
[Event BeginPlay]
    │
    └──► [Get Game Instance] → Cast to UmbraGameInstance
            │
            ├──► [Bind Event to OnPartyInviteSent]
            │       └──► [Mostrar notificação: "Convite enviado!"]
            │
            ├──► [Bind Event to OnPartyInviteReceived]
            │       └──► [Mostrar notificação: "Você recebeu um convite de grupo!"]
            │
            ├──► [Bind Event to OnTradeRequestReceived]
            │       └──► [Mostrar notificação: "Solicitação de troca recebida!"]
            │
            └──► [etc... para outros eventos]
```

**✅ Passo 8 Concluído!**

---

## Testes e Troubleshooting

### Teste 1: Verificar Tag no Remote Actor

1. **Play in Editor**
2. Spawne um `BP_RemotePlayer` no nível
3. No **World Outliner**, selecione o actor
4. No **Details**, verifique se a tag `RemotePlayer` está presente

### Teste 2: Verificar Collision

1. Com o `BP_RemotePlayer` selecionado
2. No **Details**, verifique o **Capsule Component**
3. **Collision Enabled** deve ser `Query and Physics` ou `Query Only`
4. **Generate Overlap Events** deve estar ✅

### Teste 3: Verificar Input Actions

1. **Play in Editor**
2. Abra **Window** → **Input Debugger**
3. Clique com o mouse esquerdo/direito
4. Verifique se `IA_SelectPlayer` e `IA_ContextMenu` aparecem como ativados

### Teste 4: Verificar Trace

1. Adicione logs temporários no `UmbraPlayerSelectionComponent.cpp`:
   - Em `TrySelectPlayerUnderCursor`, adicione: `UE_LOG(LogTemp, Log, TEXT("TrySelectPlayerUnderCursor chamado"));`
2. **Recompile** o projeto C++
3. **Play in Editor**
4. Clique em um Remote Player
5. Verifique o **Output Log** para ver se o trace está funcionando

### Teste 5: Verificar Widgets

1. **Play in Editor**
2. Clique em um Remote Player (esquerdo)
3. Verifique se `WBP_SelectedPlayerInfo` aparece
4. Clique direito em um Remote Player
5. Verifique se `WBP_PlayerContextMenu` aparece

### Problemas Comuns

#### ❌ Jogador não é selecionado ao clicar

**Causas possíveis:**
1. Tag `RemotePlayer` não está no actor
2. Collision não está ativa
3. Input Actions não estão configuradas
4. PlayerController Blueprint não está conectado

**Solução:**
1. Verifique a tag no `BP_RemotePlayer`
2. Verifique collision no Capsule Component
3. Verifique se `IA_SelectPlayer` está no `IMC_Default`
4. Verifique se o PlayerController Blueprint chama `HandlePlayerSelectionClick`

#### ❌ Widget não aparece

**Causas possíveis:**
1. Widget não foi adicionado ao viewport
2. Delegates não estão conectados
3. Widget está com Visibility = Collapsed

**Solução:**
1. Verifique se `Add to Viewport` foi chamado no `BeginPlay`
2. Verifique se os delegates estão conectados
3. Verifique a visibilidade inicial do widget

#### ❌ Menu de contexto na posição errada

**Causa:** A posição do mouse não está sendo convertida corretamente

**Solução:**
1. Verifique se `GetCursorScreenPosition` está retornando valores corretos
2. Ajuste o `Set Render Transform` para usar coordenadas de tela
3. Considere usar `Set Position in Viewport` em vez de `Set Render Transform`

#### ❌ Ações do menu não funcionam

**Causa:** GameInstance não está sendo chamado ou não está autenticado

**Solução:**
1. Verifique se `Cast to UmbraGameInstance` está funcionando
2. Verifique se o jogador está autenticado (`bIsAuthenticated`)
3. Adicione logs nas funções do GameInstance para debug

---

## Checklist Final

Antes de considerar o sistema completo, verifique:

### Configuração Básica
- [ ] `BP_RemotePlayer` tem tag `RemotePlayer`
- [ ] `BP_RemotePlayer` tem collision ativa
- [ ] `IA_SelectPlayer` criado e mapeado para Left Mouse Button
- [ ] `IA_ContextMenu` criado e mapeado para Right Mouse Button
- [ ] PlayerController Blueprint conecta input actions a `HandlePlayerSelectionClick`

### Widgets
- [ ] `WBP_SelectedPlayerInfo` criado com todos os elementos
- [ ] Função `UpdateDisplay` implementada
- [ ] `WBP_PlayerContextMenu` criado com todos os botões
- [ ] Funções `ShowAtPosition` e `Hide` implementadas
- [ ] Todos os botões têm eventos `OnClicked` configurados

### Integração
- [ ] Delegates conectados no HUD/Level Blueprint
- [ ] Widgets criados e adicionados ao viewport no `BeginPlay`
- [ ] `BP_NetMovementClient` registra jogadores ao spawnar
- [ ] `BP_NetMovementClient` atualiza informações quando recebe updates
- [ ] `BP_NetMovementClient` remove jogadores ao desconectar

### Ações Sociais
- [ ] Menu de contexto chama funções do GameInstance
- [ ] GameInstance está autenticado antes de chamar ações
- [ ] Delegates do GameInstance estão conectados (opcional, para notificações)

### Testes
- [ ] Clique esquerdo seleciona jogador e mostra widget de info
- [ ] Clique direito abre menu de contexto
- [ ] Widget de info atualiza quando HP/MP mudam
- [ ] Menu de contexto aparece na posição correta do mouse
- [ ] Ações do menu funcionam (enviar convite, trocar, etc.)

---

## Próximos Passos

Após implementar este sistema, você pode:

1. **Adicionar Visual Feedback:**
   - Outline/glow no jogador selecionado
   - Animação no widget de info
   - Efeitos sonoros

2. **Expandir Informações:**
   - Mostrar guild, título, classe no widget de info
   - Adicionar ícone de classe
   - Mostrar status PvP/Combat

3. **Melhorar Menu de Contexto:**
   - Adicionar ícones nos botões
   - Agrupar ações por categoria
   - Adicionar tooltips

4. **Integrar com Sistema Social:**
   - Mostrar notificações de convites recebidos
   - Atualizar lista de amigos em tempo real
   - Sincronizar estado de grupo/troca

---

## Conclusão

Este guia forneceu instruções detalhadas para implementar o sistema completo de seleção de jogadores. Se você seguiu todos os passos, o sistema deve estar funcionando!

**Dúvidas?** Consulte:
- `GUIA_INTEGRACAO_SISTEMA_SOCIAL.md` (para WebSocket e APIs)
- Código C++ em `UmbraPlayerSelectionComponent.cpp` (para entender a lógica)
- Logs no **Output Log** do Unreal Editor (para debug)

**Boa sorte com a implementação!** 🎮
