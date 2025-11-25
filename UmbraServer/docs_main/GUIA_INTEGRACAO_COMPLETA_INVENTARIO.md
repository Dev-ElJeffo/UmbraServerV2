# Guia de Integração Completa - Sistema de Inventário

## 📖 Índice

1. [Configuração do Banco de Dados MySQL](#1-configuração-do-banco-de-dados-mysql)
2. [Verificação da API REST PHP](#2-verificação-da-api-rest-php)
3. [Compilação do Projeto Unreal Engine](#3-compilação-do-projeto-unreal-engine)
4. [Implementação do Widget Blueprint UI](#4-implementação-do-widget-blueprint-ui)
5. [Integração com o Personagem](#5-integração-com-o-personagem)
6. [Testes Completos](#6-testes-completos)
7. [Solução de Problemas](#7-solução-de-problemas)

---

## 1. Configuração do Banco de Dados MySQL

### 📋 Passo 1.1: Iniciar o MySQL

**Opção A: Via WAMP**
1. Abra o **WAMP64**
2. Clique com o botão direito no ícone do WAMP (bandeja do sistema)
3. Selecione **Start All Services**
4. Aguarde até o ícone ficar **verde**

**Opção B: Via PowerShell**
```powershell
# Abrir PowerShell como Administrador
Start-Service MySQL80
# ou
net start MySQL80
```

**Verificar se o MySQL está rodando**:
```powershell
Get-Service MySQL80
# ou
sc query MySQL80 | find "RUNNING"
```

### 📋 Passo 1.2: Executar o Script SQL

**Método 1: Via MySQL Command Line**

1. Abra o **PowerShell** no diretório do projeto:
```powershell
cd D:\UmbraServerV2\UmbraServer
```

2. Execute o script:
```powershell
& "C:\wamp64\bin\mysql\mysql9.1.0\bin\mysql.exe" -u root -p'!Mister4126' umbra_eternum -e "source D:/UmbraServerV2/UmbraServer/setup_inventory_system.sql"
```

3. Verifique se não houve erros. Se tudo estiver correto, você verá apenas avisos sobre senha.

**Método 2: Via MySQL Workbench**

1. Abra o **MySQL Workbench**
2. Conecte-se ao servidor local (`localhost:3306`)
   - Username: `root`
   - Password: `!Mister4126`
3. Selecione o schema `umbra_eternum` na barra lateral esquerda
4. Clique em **File → Open SQL Script**
5. Navegue até `D:\UmbraServerV2\UmbraServer\setup_inventory_system.sql`
6. Clique no ícone de **raio** (⚡) para executar o script completo
7. Aguarde a execução (pode levar 5-10 segundos)

### 📋 Passo 1.3: Verificar as Tabelas Criadas

Execute a seguinte query no **MySQL Workbench** ou via command line:

```sql
USE umbra_eternum;

-- Verificar tabelas criadas
SHOW TABLES LIKE '%item%';
-- Deve retornar: item_templates, player_inventory

-- Verificar estrutura da tabela item_templates
DESCRIBE item_templates;

-- Verificar estrutura da tabela player_inventory
DESCRIBE player_inventory;

-- Contar templates de itens
SELECT COUNT(*) as total_templates FROM item_templates;
-- Deve retornar: 17

-- Contar itens no inventário do jogador 1
SELECT COUNT(*) as total_items FROM player_inventory WHERE player_id = 1;
-- Deve retornar: 8

-- Ver todos os templates de itens
SELECT item_id, item_name, item_type, rarity FROM item_templates;

-- Ver inventário do jogador 1
SELECT 
    pi.inventory_id,
    pi.slot_index,
    it.item_name,
    pi.quantity,
    it.rarity
FROM player_inventory pi
JOIN item_templates it ON pi.item_template_id = it.item_id
WHERE pi.player_id = 1
ORDER BY pi.slot_index;
```

**Resultado Esperado**:
- ✅ 17 templates de itens criados
- ✅ 8 itens no inventário do jogador 1
- ✅ Itens nos slots 0, 1, 2, 10, 11, 12, 13, 14

---

## 2. Verificação da API REST PHP

### 📋 Passo 2.1: Verificar Estrutura de Arquivos

Navegue até `D:\UmbraServerV2\www\umbra_api\api\inventory\` e verifique se os seguintes arquivos existem:

```
inventory/
├── get_inventory.php       (127 linhas)
├── add_item.php            (226 linhas)
├── remove_item.php         (151 linhas)
├── move_item.php           (205 linhas)
├── equip_item.php          (227 linhas)
└── get_item_templates.php  (94 linhas)
```

### 📋 Passo 2.2: Testar Endpoint Público (Sem Autenticação)

**Teste 1: Listar Templates de Itens**

Abra o navegador e acesse:
```
http://localhost/umbra_api/api/inventory/get_item_templates.php
```

**Resultado Esperado**:
```json
{
  "success": true,
  "message": "Templates carregados com sucesso",
  "templates": [
    {
      "item_id": 1,
      "item_name": "Espada de Ferro",
      "item_type": "weapon",
      "rarity": "common",
      ...
    },
    ...
  ],
  "total": 17
}
```

**Teste 2: Filtrar por Tipo**
```
http://localhost/umbra_api/api/inventory/get_item_templates.php?type=weapon
```
Deve retornar apenas 3 armas.

**Teste 3: Filtrar por Raridade**
```
http://localhost/umbra_api/api/inventory/get_item_templates.php?rarity=legendary
```
Deve retornar apenas 1 item (Espada Flamejante).

### 📋 Passo 2.3: Testar Endpoints Protegidos (Com JWT)

**Preparação: Obter Token JWT**

1. **Login via API**:
```powershell
# Método PowerShell
$body = @{
    username = "testuser"
    password = "senha123"
} | ConvertTo-Json

$response = Invoke-RestMethod -Uri "http://localhost/umbra_api/api/login.php" -Method POST -Body $body -ContentType "application/json"
$token = $response.token
Write-Host "Token: $token"
```

2. **Selecionar Personagem**:
```powershell
$headers = @{
    "Authorization" = "Bearer $token"
}
$body = @{
    character_id = 1
} | ConvertTo-Json

$response = Invoke-RestMethod -Uri "http://localhost/umbra_api/api/character/select_character.php" -Method POST -Headers $headers -Body $body -ContentType "application/json"
$jwtToken = $response.jwt_token
Write-Host "JWT Token: $jwtToken"
```

**Teste 4: Obter Inventário**
```powershell
$headers = @{
    "Authorization" = "Bearer $jwtToken"
}

$response = Invoke-RestMethod -Uri "http://localhost/umbra_api/api/inventory/get_inventory.php" -Method GET -Headers $headers
$response | ConvertTo-Json -Depth 10
```

**Resultado Esperado**:
```json
{
  "success": true,
  "message": "Inventário carregado com sucesso",
  "player": {
    "player_id": 1,
    "character_name": "Jefor",
    "level": 15
  },
  "inventory": [
    {
      "inventory_id": 1,
      "slot_index": 0,
      "item_name": "Espada de Ferro",
      "quantity": 1,
      ...
    },
    ...
  ],
  "total_items": 8
}
```

**Teste 5: Adicionar Item**
```powershell
$body = @{
    item_template_id = 7
    quantity = 5
} | ConvertTo-Json

$response = Invoke-RestMethod -Uri "http://localhost/umbra_api/api/inventory/add_item.php" -Method POST -Headers $headers -Body $body -ContentType "application/json"
$response | ConvertTo-Json
```

**Resultado Esperado**:
```json
{
  "success": true,
  "message": "Item adicionado ao inventário",
  "inventory_id": 26,
  "item_name": "Poção de Vida Menor",
  "slot_index": 15,
  "quantity": 5
}
```

---

## 3. Compilação do Projeto Unreal Engine

### 📋 Passo 3.1: Fechar o Unreal Engine

1. **Salve tudo** no Unreal Engine (Ctrl+S)
2. **Feche completamente** o editor
3. Aguarde até que o processo `UE5Editor.exe` não esteja mais rodando (Task Manager)

### 📋 Passo 3.2: Regenerar Arquivos do Projeto

**Opção A: Via Script Batch**
```powershell
cd D:\UmbraServerV2\UmbraEternumUE
.\RegenerarProjeto.bat
```

**Opção B: Via Clique Direito**
1. Navegue até `D:\UmbraServerV2\UmbraEternumUE\`
2. Clique com o botão direito em `UmbraEternumUE.uproject`
3. Selecione **Generate Visual Studio project files**
4. Aguarde a conclusão (pode levar 1-2 minutos)

### 📋 Passo 3.3: Abrir no Visual Studio

1. Abra `D:\UmbraServerV2\UmbraEternumUE\UmbraEternumUE.sln` no **Visual Studio 2022**
2. Aguarde a indexação dos arquivos (barra inferior do VS)
3. Certifique-se de que a configuração está em:
   - **Configuration**: `Development Editor`
   - **Platform**: `Win64`

### 📋 Passo 3.4: Verificar Arquivos Modificados

No **Solution Explorer**, navegue até:
```
UmbraEternumUE
└── Source
    └── UmbraEternumUE
        ├── Core
        │   ├── UmbraGameInstance.h      (modificado, +135 linhas)
        │   └── UmbraGameInstance.cpp    (modificado, +970 linhas)
        └── Data
            └── UmbraDataStructures.h    (modificado, +183 linhas)
```

**Verificar Erros de Sintaxe**:
1. Abra cada arquivo
2. Procure por linhas sublinhadas em vermelho
3. Se houver erros, reporte-os antes de compilar

### 📋 Passo 3.5: Compilar o Projeto

**Método 1: Via Visual Studio**
1. Clique em **Build → Build Solution** (Ctrl+Shift+B)
2. Aguarde a compilação (pode levar 5-15 minutos na primeira vez)
3. Observe a janela **Output** na parte inferior

**Resultado Esperado**:
```
========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
```

**Método 2: Via PowerShell**
```powershell
cd D:\UmbraServerV2\UmbraEternumUE
& "D:\UE_5.6\Engine\Build\BatchFiles\Build.bat" UmbraEternumUEEditor Win64 Development -Project="D:\UmbraServerV2\UmbraEternumUE\UmbraEternumUE.uproject" -WaitMutex -FromMsBuild
```

### 📋 Passo 3.6: Verificar Compilação

**Se a compilação falhar**, verifique:
1. **Erros de Link**: `unresolved external symbol`
   - Solução: Adicione `#include "VaRestSubsystem.h"` no topo do `.cpp` se necessário
2. **Erros de Delegate**: `cannot convert from 'void (__cdecl *)' to 'FOnInventoryLoaded'`
   - Solução: Verifique se os delegates estão marcados como `UFUNCTION()`
3. **Erros de TSharedPtr**: `no member named 'AsObject'`
   - Solução: Adicione `#include "Dom/JsonObject.h"` no topo do `.cpp`

**Se a compilação for bem-sucedida**:
- ✅ Arquivo `UmbraEternumUE-Win64-Development.dll` foi atualizado
- ✅ Arquivo `UmbraEternumUE.lib` foi atualizado

### 📋 Passo 3.7: Abrir o Unreal Engine

1. Abra `D:\UmbraServerV2\UmbraEternumUE\UmbraEternumUE.uproject`
2. Aguarde o carregamento (pode levar 2-5 minutos)
3. Se aparecer uma janela perguntando sobre recompilar, clique em **Yes**

### 📋 Passo 3.8: Verificar C++ no Blueprint

1. No **Content Browser**, navegue até `Content/Blueprints/`
2. Abra `BP_UmbraGameInstance` (ou crie se não existir)
3. No **Event Graph**, clique com o botão direito
4. Digite "Load Inventory" na busca
5. Você deve ver a função **Load Inventory** aparecer

**Outras funções que devem aparecer**:
- `Add Item`
- `Remove Item`
- `Move Item`
- `Equip Item`
- `Unequip Item`
- `Get Inventory Slot`
- `Get Inventory Slot By Index`
- `Get Equipped Item`
- `Is Slot Empty`
- `Clear Inventory`

**Delegates que devem aparecer**:
- `On Inventory Loaded`
- `On Item Added`
- `On Item Removed`
- `On Item Moved`
- `On Item Equipped`
- `On Item Unequipped`
- Todos os `On...Failed`

---

## 4. Implementação do Widget Blueprint UI

### 📋 Passo 4.1: Criar Widget de Slot de Inventário

1. No **Content Browser**, navegue até `Content/UI/Inventory/` (crie as pastas se necessário)
2. Clique com o botão direito → **User Interface → Widget Blueprint**
3. Nome: `WBP_InventorySlot`
4. Abra o widget

**Estrutura do Widget**:

```
Canvas Panel (Root)
└── Overlay
    ├── Border (slot_border)
    │   └── Size Box (64x64)
    │       └── Image (item_icon)
    ├── Text Block (item_quantity)
    ├── Progress Bar (durability_bar)
    └── Border (rarity_border)
```

**Passo a Passo Detalhado**:

1. **Canvas Panel (Root)**:
   - Já existe por padrão

2. **Adicionar Overlay**:
   - Arraste **Overlay** da paleta para o Canvas Panel
   - Renomeie para `slot_overlay`
   - Anchor: Fill
   - Position: (0, 0)
   - Size: (64, 64)

3. **Adicionar Border (slot_border)**:
   - Arraste **Border** para dentro do Overlay
   - Renomeie para `slot_border`
   - Appearance → Brush Color: (0.2, 0.2, 0.2, 1.0) [Cinza escuro]
   - Appearance → Padding: (2, 2, 2, 2)

4. **Adicionar Size Box**:
   - Arraste **Size Box** para dentro do Border
   - Width Override: 60
   - Height Override: 60

5. **Adicionar Image (item_icon)**:
   - Arraste **Image** para dentro do Size Box
   - Renomeie para `item_icon`
   - Appearance → Color and Opacity: (1, 1, 1, 0) [Transparente por padrão]
   - Appearance → Brush → Draw As: Image
   - Appearance → Tiling: No Tile

6. **Adicionar Text Block (item_quantity)**:
   - Arraste **Text Block** para dentro do Overlay (não dentro do Border!)
   - Renomeie para `item_quantity`
   - Anchor: Bottom Right
   - Position: (-8, -8) [canto inferior direito]
   - Text: "1"
   - Font → Size: 14
   - Font → Color: White
   - Shadow → Shadow Offset: (1, 1)
   - Visibility: Collapsed (será mostrado apenas para itens empilháveis)

7. **Adicionar Progress Bar (durability_bar)**:
   - Arraste **Progress Bar** para dentro do Overlay
   - Renomeie para `durability_bar`
   - Anchor: Bottom (stretch horizontal)
   - Position: (0, -4)
   - Size: (64, 4)
   - Fill Color and Opacity: (0.2, 1.0, 0.2, 1.0) [Verde]
   - Percent: 1.0
   - Visibility: Collapsed (será mostrado apenas para itens com durabilidade)

8. **Adicionar Border (rarity_border)**:
   - Arraste **Border** para dentro do Overlay
   - Renomeie para `rarity_border`
   - Anchor: Fill
   - Brush Color: (1, 1, 1, 0) [Transparente por padrão]
   - Padding: (0, 0, 0, 0)
   - Brush → Draw As: RoundedBox
   - Visibility: HitTestInvisible

**Variáveis do Blueprint**:

Clique em **+ Variable** na paleta à esquerda e crie as seguintes variáveis:

```
Nome: ItemSlot
Tipo: Umbra Inventory Slot (struct)
Instance Editable: False
Tooltip: "Dados do item neste slot"

Nome: SlotIndex
Tipo: Integer
Instance Editable: True
Default Value: 0
Tooltip: "Índice deste slot (0-49)"

Nome: IsEmpty
Tipo: Boolean
Instance Editable: False
Default Value: True
Tooltip: "Se este slot está vazio"
```

**Função: SetItemData**

1. Clique em **+ Function** na paleta à esquerda
2. Nome: `SetItemData`
3. Adicione um input:
   - Nome: `NewItemSlot`
   - Tipo: `Umbra Inventory Slot` (struct)

**Event Graph da Função SetItemData**:

```
SetItemData (Input: NewItemSlot)
  → Set ItemSlot = NewItemSlot
  → Set IsEmpty = False
  → Set item_icon → SetBrushFromTexture
      → LoadObject(Texture2D, NewItemSlot.ItemTemplate.IconPath)
  → Branch (ItemSlot.Quantity > 1)
      → True: Set item_quantity → SetText(ToString(Quantity))
              Set item_quantity → SetVisibility(Visible)
      → False: Set item_quantity → SetVisibility(Collapsed)
  → Branch (ItemSlot.Durability < 100)
      → True: Set durability_bar → SetPercent(Durability / 100)
              Set durability_bar → SetVisibility(Visible)
      → False: Set durability_bar → SetVisibility(Collapsed)
  → Switch on Enum (ItemSlot.ItemTemplate.Rarity)
      → Common: Set rarity_border → SetBrushColor(White)
      → Uncommon: Set rarity_border → SetBrushColor(Green)
      → Rare: Set rarity_border → SetBrushColor(Blue)
      → Epic: Set rarity_border → SetBrushColor(Purple)
      → Legendary: Set rarity_border → SetBrushColor(Orange)
```

**Função: ClearSlot**

```
ClearSlot
  → Set IsEmpty = True
  → Set item_icon → SetBrushFromTexture(nullptr)
  → Set item_icon → SetColorAndOpacity(0, 0, 0, 0)
  → Set item_quantity → SetVisibility(Collapsed)
  → Set durability_bar → SetVisibility(Collapsed)
  → Set rarity_border → SetBrushColor(0, 0, 0, 0)
```

**Compile e Save** o widget.

### 📋 Passo 4.2: Criar Widget Principal do Inventário

1. No **Content Browser**, `Content/UI/Inventory/`
2. Clique com o botão direito → **User Interface → Widget Blueprint**
3. Nome: `WBP_Inventory`
4. Abra o widget

**Estrutura do Widget**:

```
Canvas Panel (Root)
└── Vertical Box (main_container)
    ├── Text Block (title_text) [título "Inventário"]
    ├── Horizontal Box (info_bar)
    │   ├── Text Block (player_name)
    │   ├── Text Block (slots_used)
    │   └── Button (close_button)
    └── Uniform Grid Panel (inventory_grid)
        └── [50x WBP_InventorySlot criados dinamicamente]
```

**Passo a Passo Detalhado**:

1. **Canvas Panel**: Root (já existe)

2. **Vertical Box (main_container)**:
   - Arraste para o Canvas Panel
   - Anchor: Center
   - Position: (0, 0)
   - Size: (640, 500)
   - Alignment: (0.5, 0.5)

3. **Text Block (title_text)**:
   - Arraste para dentro do Vertical Box
   - Text: "INVENTÁRIO"
   - Font Size: 24
   - Font → Typeface: Bold
   - Justification: Center
   - Padding: (0, 10, 0, 10)

4. **Horizontal Box (info_bar)**:
   - Arraste para dentro do Vertical Box
   - Padding: (10, 5, 10, 5)

5. **Text Block (player_name)**:
   - Arraste para dentro do Horizontal Box
   - Text: "Jogador: Loading..."
   - Font Size: 14
   - Size → Fill: 1.0

6. **Text Block (slots_used)**:
   - Arraste para dentro do Horizontal Box
   - Text: "Slots: 0/50"
   - Font Size: 14
   - Justification: Right
   - Size → Fill: 1.0

7. **Button (close_button)**:
   - Arraste para dentro do Horizontal Box
   - Text: "X"
   - Size → Fixed Width: 30

8. **Uniform Grid Panel (inventory_grid)**:
   - Arraste para dentro do Vertical Box
   - Slot → Fill: 1.0
   - Slot Padding: (2, 2, 2, 2)

**Variáveis do Blueprint**:

```
Nome: InventorySlots
Tipo: Array of WBP_InventorySlot (Widget Reference)
Instance Editable: False
Tooltip: "Array de 50 slots do inventário"

Nome: GameInstanceRef
Tipo: Umbra Game Instance (Object Reference)
Instance Editable: False
Tooltip: "Referência ao Game Instance"

Nome: IsInventoryOpen
Tipo: Boolean
Instance Editable: False
Default Value: False
Tooltip: "Se o inventário está aberto"
```

**Event Construct**:

```
Event Construct
  → Get Game Instance
      → Cast to Umbra Game Instance
          → Set GameInstanceRef
          → Branch (IsValid)
              → True:
                  → Bind Event to OnInventoryLoaded
                  → Bind Event to OnItemAdded
                  → Bind Event to OnItemRemoved
                  → Bind Event to OnItemMoved
                  → LoadInventory()
                  → CreateInventorySlots()
              → False:
                  → Print String "Game Instance inválido!"
```

**Função: CreateInventorySlots**

```
CreateInventorySlots
  → Clear InventorySlots Array
  → Clear inventory_grid (RemoveAllChildren)
  → ForLoop (0 to 49)
      → Index = Loop Index
      → Row = Index / 10
      → Column = Index % 10
      → Create Widget (WBP_InventorySlot)
          → Set SlotIndex = Index
          → ClearSlot()
          → Add to InventorySlots Array
          → Add Child to Grid Slot (inventory_grid)
              → Row = Row
              → Column = Column
```

**Event: OnInventoryLoaded**

```
OnInventoryLoaded (Bound to GameInstanceRef)
  → Get CurrentInventory from GameInstanceRef
  → ForEach (CurrentInventory)
      → SlotIndex = Array Element.SlotIndex
      → Get InventorySlots[SlotIndex]
          → SetItemData(Array Element)
  → Get GetOccupiedSlotsCount from GameInstanceRef
  → Set slots_used → SetText(Format("{0}/50", OccupiedCount))
  → Get GetActiveCharacter from GameInstanceRef
  → Set player_name → SetText(Format("Jogador: {0}", CharacterName))
```

**Event: OnItemAdded**

```
OnItemAdded (InventoryID, ItemSlot)
  → LoadInventory() [recarrega tudo para sincronizar]
```

**Event: close_button OnClicked**

```
OnClicked (close_button)
  → Remove from Parent
  → Set IsInventoryOpen = False
```

**Compile e Save** o widget.

### 📋 Passo 4.3: Adicionar Drag & Drop (Opcional - Avançado)

Se quiser implementar drag & drop entre slots:

1. **No WBP_InventorySlot**:
   - Adicione variável `IsDragging` (bool)
   - Override `OnMouseButtonDown`: Cria um `Drag Drop Operation`
   - Override `OnDragDetected`: Inicia o drag
   - Override `OnDrop`: Recebe o drop e chama `GameInstanceRef.MoveItem()`

2. **Blueprint detalhado** (muito longo, não incluído neste guia básico)

---

## 5. Integração com o Personagem

### 📋 Passo 5.1: Adicionar Input para Abrir Inventário

**Método 1: Via Enhanced Input**

1. No **Content Browser**, navegue até `Content/Input/`
2. Abra `IMC_Default` (Input Mapping Context)
3. Clique em **+ Mappings**
4. Nome: `IA_OpenInventory`
5. Key: **I** (tecla I)
6. Save

**Método 2: Via Project Settings (Legacy)**

1. **Edit → Project Settings**
2. **Input**
3. **Action Mappings → +**
4. Nome: `OpenInventory`
5. Key: **I**
6. Save

### 📋 Passo 5.2: Modificar o Blueprint do Personagem

1. Abra `BP_ThirdPersonCharacter` (ou seu Blueprint de personagem)
2. No **Event Graph**, adicione o seguinte:

**Event Graph - Abrir Inventário**:

```
InputAction OpenInventory (ou IA_OpenInventory se Enhanced Input)
  → Branch (Is Valid? InventoryWidgetRef)
      → True:
          → Branch (IsInventoryOpen?)
              → True: Remove from Parent
                      Set IsInventoryOpen = False
              → False: Add to Viewport
                       Set IsInventoryOpen = True
      → False:
          → Create Widget (WBP_Inventory)
              → Set InventoryWidgetRef
              → Add to Viewport
              → Set IsInventoryOpen = True
```

**Variável**: `InventoryWidgetRef` (tipo: `WBP_Inventory`, Object Reference)

**Variável**: `IsInventoryOpen` (tipo: Boolean, Default: False)

### 📋 Passo 5.3: Carregar Inventário ao Entrar no Jogo

No **Event BeginPlay** do personagem:

```
Event BeginPlay
  → Parent: BeginPlay
  → Delay 1.0 second
  → Get Game Instance
      → Cast to Umbra Game Instance
          → Branch (HasActiveCharacter?)
              → True: LoadInventory()
              → False: Print String "Nenhum personagem ativo"
```

### 📋 Passo 5.4: (Opcional) Auto-salvar Inventário

Se quiser salvar automaticamente o inventário ao pegar itens:

```
On Item Pickup (Custom Event)
  → Get Game Instance
      → AddItem(ItemTemplateID, Quantity)
```

---

## 6. Testes Completos

### 📋 Teste 6.1: Carregar Inventário

**Procedimento**:
1. Compile e salve todos os Blueprints
2. Clique em **Play** (Alt+P)
3. Faça login com um personagem existente (player_id = 1)
4. Pressione **I** para abrir o inventário

**Resultado Esperado**:
- ✅ Widget do inventário aparece
- ✅ 8 slots estão preenchidos (slots 0, 1, 2, 10, 11, 12, 13, 14)
- ✅ Nome do jogador aparece no topo
- ✅ "Slots: 8/50" aparece
- ✅ Ícones dos itens aparecem (se os assets existirem)
- ✅ Quantidade aparece para itens empilháveis

**Se falhar**:
- Verifique os logs: **Window → Developer Tools → Output Log**
- Procure por `[AUDIT]` e `[SECURITY]`

### 📋 Teste 6.2: Adicionar Item via Console

1. Com o jogo rodando, abra o **Console** (tecla `~` ou `)
2. Digite:
```
ce AddItemToInventory 7 5
```

**Ou via Blueprint**:

Crie um **Custom Event** no personagem chamado `TestAddItem`:
```
TestAddItem
  → Get Game Instance
      → AddItem(ItemTemplateID = 7, Quantity = 5, SlotIndex = -1)
```

Chame este evento ao pressionar uma tecla de teste (ex: F8).

**Resultado Esperado**:
- ✅ Log: `✅ [AUDIT] Item adicionado com sucesso`
- ✅ Widget é atualizado automaticamente (via `OnItemAdded`)
- ✅ Novo slot aparece preenchido

### 📋 Teste 6.3: Remover Item

**Via Blueprint**:
```
On Key Press (F9)
  → Get InventorySlots[0]
      → Get InventoryID
      → Get Game Instance
          → RemoveItem(InventoryID, Quantity = 1)
```

**Resultado Esperado**:
- ✅ Log: `✅ [AUDIT] Item removido com sucesso`
- ✅ Quantidade diminui ou slot fica vazio

### 📋 Teste 6.4: Mover Item (se drag & drop implementado)

1. Clique e segure um item
2. Arraste para outro slot
3. Solte

**Resultado Esperado**:
- ✅ Log: `✅ [AUDIT] Item movido com sucesso`
- ✅ Item aparece no novo slot
- ✅ Se havia item no slot de destino, faz swap

### 📋 Teste 6.5: Equipar Item

**Via Blueprint**:
```
On Double-Click (Inventory Slot)
  → Get InventoryID
  → Get Game Instance
      → Branch (ItemSlot.bIsEquipped)
          → True: UnequipItem(InventoryID)
          → False: EquipItem(InventoryID)
```

**Resultado Esperado**:
- ✅ Log: `✅ [AUDIT] Item equipado com sucesso`
- ✅ Borda do slot muda de cor (ex: amarelo para equipado)
- ✅ Item anterior no mesmo slot é desequipado automaticamente

### 📋 Teste 6.6: Validação de Segurança

**Teste: Adicionar Item Inválido**
```
AddItem(ItemTemplateID = -1, Quantity = 1)
```
**Esperado**: `❌ [VALIDATION] ItemTemplateID inválido: -1`

**Teste: Adicionar Quantidade Inválida**
```
AddItem(ItemTemplateID = 7, Quantity = -5)
```
**Esperado**: `❌ [VALIDATION] Quantidade inválida: -5`

**Teste: Adicionar em Slot Inválido**
```
AddItem(ItemTemplateID = 7, Quantity = 1, SlotIndex = 100)
```
**Esperado**: `❌ [VALIDATION] SlotIndex fora do intervalo: 100`

### 📋 Teste 6.7: Logs de Auditoria

Filtre os logs por `[AUDIT]` para ver todas as operações:

```
Window → Developer Tools → Output Log
  → Filtro: [AUDIT]
```

**Logs esperados**:
```
[AUDIT] Tentando carregar inventário - Player: 1, Username: testuser
[AUDIT] Inventário carregado - Total: 17, Equipados: 2
[AUDIT] Tentando adicionar item - ItemTemplateID: 7, Quantity: 5
[AUDIT] Item adicionado com sucesso - InventoryID: 26, ItemName: Poção
[AUDIT] Tentando remover item - InventoryID: 26, Quantity: 1
[AUDIT] Item removido com sucesso - RemovedQuantity: 1, Remaining: 4
```

---

## 7. Solução de Problemas

### ❌ Problema 1: "Token inválido ou expirado"

**Causa**: JWT expirou ou não foi gerado corretamente.

**Solução**:
1. Faça logout e login novamente
2. Selecione o personagem novamente
3. Verifique se o `CurrentToken` no Game Instance não está vazio:
```
Get Game Instance → Get Current Token → Print String
```

### ❌ Problema 2: "Nenhum personagem selecionado"

**Causa**: `ActivePlayerID` é 0.

**Solução**:
1. Certifique-se de que selecionou um personagem via `select_character.php`
2. Verifique o log: `[UmbraGameInstance] ✅ Personagem selecionado`
3. Verifique no Blueprint:
```
Get Game Instance → Get Active Player ID → Print String
```
Deve retornar um número > 0.

### ❌ Problema 3: "Inventário vazio (nenhum item aparece)"

**Causa**: Player ID não tem itens no banco de dados.

**Solução**:
1. Execute a query SQL:
```sql
SELECT * FROM player_inventory WHERE player_id = 1;
```
2. Se estiver vazio, execute novamente o `setup_inventory_system.sql` (seção INSERT)
3. Ou adicione itens manualmente:
```sql
INSERT INTO player_inventory (player_id, item_template_id, quantity, slot_index)
VALUES (1, 7, 5, 0);
```

### ❌ Problema 4: "Widget não aparece"

**Causas possíveis**:
- Widget não foi adicionado ao Viewport
- Z-Order muito baixo (atrás de outros widgets)
- Visibility está como `Collapsed` ou `Hidden`

**Solução**:
1. No Blueprint, adicione logs:
```
Create Widget → Print String "Widget criado"
Add to Viewport → Print String "Widget adicionado"
```
2. Verifique se `IsValid(InventoryWidgetRef)` retorna `True`
3. Aumente o Z-Order:
```
Add to Viewport → ZOrder = 999
```

### ❌ Problema 5: "Função LoadInventory não aparece"

**Causa**: Projeto não foi recompilado corretamente.

**Solução**:
1. Feche o Unreal Engine
2. Delete a pasta `Intermediate/` e `Saved/`
3. Regenere o projeto:
```powershell
cd D:\UmbraServerV2\UmbraEternumUE
.\RegenerarProjeto.bat
```
4. Recompile no Visual Studio
5. Abra o Unreal Engine novamente

### ❌ Problema 6: "Erro de compilação C++"

**Erro comum**: `unresolved external symbol`

**Solução**:
1. Adicione os includes necessários no topo do `.cpp`:
```cpp
#include "VaRestSubsystem.h"
#include "VaRestJsonObject.h"
#include "Dom/JsonObject.h"
```
2. Verifique se o módulo `VaRest` está no `Build.cs`:
```csharp
PublicDependencyModuleNames.AddRange(new string[] { "VaRest" });
```

### ❌ Problema 7: "Items não empilham automaticamente"

**Causa**: API não está detectando itens empilháveis existentes.

**Solução**:
1. Verifique se o `max_stack_size` do item é > 1:
```sql
SELECT item_id, item_name, max_stack_size FROM item_templates WHERE item_id = 7;
```
2. Verifique se há slots parcialmente preenchidos:
```sql
SELECT * FROM player_inventory 
WHERE player_id = 1 
AND item_template_id = 7 
AND quantity < (SELECT max_stack_size FROM item_templates WHERE item_id = 7);
```

### ❌ Problema 8: "Logs de auditoria não aparecem"

**Causa**: Log level está muito alto.

**Solução**:
1. Abra **Window → Developer Tools → Output Log**
2. No dropdown superior, selecione **Verbose**
3. Certifique-se de que `LogTemp` está habilitado

---

## 8. Próximos Passos (Opcional)

### 🎨 Melhorias Visuais

1. **Adicionar ícones de itens**:
   - Crie texturas 64x64 para cada item
   - Salve em `Content/UI/Icons/Items/`
   - Atualize o `icon_path` no banco de dados

2. **Adicionar sons**:
   - Som de pegar item
   - Som de mover item
   - Som de equipar/desequipar

3. **Adicionar animações**:
   - Fade in/out ao abrir/fechar
   - Highlight ao passar o mouse
   - Shake ao tentar ação inválida

### ⚙️ Funcionalidades Adicionais

1. **Filtros de inventário**:
   - Botões para filtrar por tipo (Armas, Armaduras, etc.)
   - Busca por nome

2. **Ordenação**:
   - Botão para ordenar por raridade
   - Botão para ordenar por tipo
   - Botão para empilhar automaticamente

3. **Tooltip avançado**:
   - Widget popup ao passar o mouse
   - Mostra stats detalhados
   - Mostra descrição completa

4. **Contexto de ações**:
   - Clique direito para menu contextual
   - Opções: Usar, Equipar, Descartar, Dividir Stack

### 🔒 Segurança Avançada

1. **Rate limiting**:
   - Limitar número de operações por minuto
   - Prevenir spam de requisições

2. **Validação server-side**:
   - Verificar se o jogador realmente pode adicionar o item
   - Verificar se o item existe no jogo

3. **Logs de auditoria persistentes**:
   - Salvar logs de auditoria em arquivo
   - Criar tabela `inventory_audit_log` no MySQL

---

## 9. Checklist de Verificação Final

Antes de considerar o sistema completo, verifique:

### Banco de Dados
- [ ] MySQL está rodando
- [ ] Tabela `item_templates` existe com 17 itens
- [ ] Tabela `player_inventory` existe
- [ ] Jogador 1 tem 8 itens no inventário

### API REST
- [ ] 6 endpoints PHP existem em `/api/inventory/`
- [ ] `get_item_templates.php` funciona sem autenticação
- [ ] `get_inventory.php` funciona com JWT válido
- [ ] `add_item.php` adiciona itens corretamente
- [ ] Logs de auditoria aparecem no `error_log`

### C++ / Unreal Engine
- [ ] Projeto compila sem erros
- [ ] `UmbraGameInstance.h` tem +135 linhas
- [ ] `UmbraGameInstance.cpp` tem +970 linhas
- [ ] `UmbraDataStructures.h` tem +183 linhas
- [ ] Funções aparecem no Blueprint (LoadInventory, AddItem, etc.)
- [ ] Delegates aparecem no Blueprint (OnInventoryLoaded, etc.)

### Widget Blueprint
- [ ] `WBP_InventorySlot` existe e funciona
- [ ] `WBP_Inventory` existe e funciona
- [ ] Widget abre ao pressionar **I**
- [ ] 50 slots são criados dinamicamente
- [ ] Dados dos itens aparecem nos slots

### Testes
- [ ] Carregar inventário funciona
- [ ] Adicionar item funciona
- [ ] Remover item funciona
- [ ] Mover item funciona
- [ ] Equipar item funciona
- [ ] Logs de auditoria aparecem
- [ ] Validações de segurança funcionam

---

## 10. Referências

### Documentação Criada
- `GUIA_API_INVENTARIO.md` - Documentação completa da API REST
- `GUIA_IMPLEMENTACAO_INVENTARIO_CPP.md` - Guia técnico da implementação C++
- `IMPLEMENTACAO_COMPLETA_INVENTARIO.md` - Resumo executivo do sistema

### Arquivos de Código
- `setup_inventory_system.sql` - Script SQL do banco de dados
- `www/umbra_api/api/inventory/*.php` - Endpoints REST (6 arquivos)
- `UmbraDataStructures.h` - Structs e enums C++
- `UmbraGameInstance.h` - Declarações C++
- `UmbraGameInstance.cpp` - Implementação C++ (970+ linhas)

---

## ✅ Conclusão

Seguindo este guia passo a passo, você terá:

1. ✅ Banco de dados configurado com 17 templates e 8 itens de teste
2. ✅ API REST funcionando com 6 endpoints protegidos
3. ✅ Sistema C++ compilado com 970+ linhas de código
4. ✅ Widget Blueprint funcional para exibir o inventário
5. ✅ Integração completa com o personagem
6. ✅ Logs de auditoria detalhados
7. ✅ Validações de segurança robustas

O sistema de inventário está **100% funcional** e pronto para uso!

---

**Data de criação**: 14/11/2025  
**Versão**: 1.0  
**Autor**: Dev-ElJeffo  
**Tempo estimado de implementação**: 2-4 horas  
**Dificuldade**: Intermediário-Avançado

