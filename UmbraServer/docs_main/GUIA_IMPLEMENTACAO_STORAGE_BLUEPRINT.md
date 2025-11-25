# 🏦 GUIA COMPLETO: Implementação do Sistema de Armazém no Blueprint

**OBJETIVO:** Implementar o widget `WBP_Storage` e conectar com o actor de interação para criar um sistema funcional de armazém.

---

## 📋 **ÍNDICE:**

1. [Estrutura do Sistema](#estrutura-do-sistema)
2. [Configuração do WBP_Storage](#configuração-do-wbp_storage)
3. [Implementação dos Eventos](#implementação-dos-eventos)
4. [Criação dos Slots](#criação-dos-slots)
5. [Drag & Drop entre Inventário e Armazém](#drag--drop-entre-inventário-e-armazém)
6. [Configuração do Actor](#configuração-do-actor)
7. [Teste e Validação](#teste-e-validação)

---

## 🏗️ **ESTRUTURA DO SISTEMA:**

### **Componentes:**

1. **`UmbraStorageWidget` (C++)** - Classe base do widget
   - Herda de `UmbraInventoryWidget`
   - Gerencia 100 slots de armazém
   - Funções para carregar, mover itens, etc.

2. **`WBP_Storage` (Blueprint)** - Widget visual
   - Parent: `UmbraStorageWidget`
   - Grid de 10x10 (100 slots)
   - Visual similar ao inventário

3. **`UmbraStorageActor` (C++)** - Actor no mundo
   - Gerencia interação do jogador
   - Abre/fecha o widget
   - Validação de distância e keys

4. **`BP_StorageChest` (Blueprint)** - Actor visual
   - Parent: `UmbraStorageActor`
   - Mesh, colisão, interação

---

## 🎨 **CONFIGURAÇÃO DO WBP_Storage:**

### **PASSO 1: Criar o Widget**

1. **Content Browser** → `Content/Widgets/UI/Storage/`
2. **Right Click** → **User Interface** → **Widget Blueprint**
3. **Parent Class:** `UmbraStorageWidget`
4. **Nome:** `WBP_Storage`

### **PASSO 2: Design do Widget**

**Estrutura Visual:**
```
WBP_Storage (Canvas Panel)
├─ Border_Background (fundo escuro)
│  └─ VerticalBox
│     ├─ Border_TitleBar (arrastável)
│     │  ├─ TextBlock_Title ("Armazém")
│     │  └─ Button_Close (X)
│     ├─ HorizontalBox
│     │  ├─ TextBlock_Info ("Slots: X/100")
│     │  └─ Button_DepositAll ("Depositar Tudo")
│     └─ UniformGridPanel_Storage (10 colunas × 10 linhas)
│        ├─ WBP_InventorySlot (slot 0)
│        ├─ WBP_InventorySlot (slot 1)
│        └─ ... (98 slots)
```

**Configurações:**
- **UniformGridPanel_Storage:**
  - **Slot Padding:** (5, 5)
  - **Columns:** 10
  - **Rows:** 10 (automático)

---

## 📝 **IMPLEMENTAÇÃO DOS EVENTOS:**

### **1. Event Construct**

```
Event Construct
  ↓
Get Game Instance (Cast to Umbra Game Instance)
  ↓
(Guardar em variável: MyGameInstance)
  ↓
Load Storage  ← Carrega o armazém automaticamente
```

**Variáveis Criadas:**
- `MyGameInstance` (UmbraGameInstance) - Referência ao GameInstance
- `SlotWidgets` (Array of WBP_InventorySlot) - Array com os 100 slots

---

### **2. Event On Storage Loaded**

**Implemente o BlueprintImplementableEvent `OnStorageLoaded`:**

```
Event On Storage Loaded (UsedSlots, TotalSlots)
  ↓
Print String: "Armazém carregado! Slots: X/100"  ← DEBUG
  ↓
Get Storage Data  ← Obtém array com todos os slots
  ↓
Create Storage Slots  ← Cria os widgets dos slots
  ↓
Update All Slots Visual  ← Atualiza visual de todos os slots
```

---

### **3. Create Storage Slots (Custom Function)**

**Crie uma função `CreateStorageSlots`:**

```
Function: Create Storage Slots
  ↓
Clear Array (SlotWidgets)  ← Limpa array anterior
  ↓
For Loop (First: 0, Last: 99)  ← 100 slots
  Loop Body:
    ├─ Create Widget (WBP_InventorySlot)
    ├─ Set Slot Index (do widget criado, Index: Index)
    ├─ Add to Array (SlotWidgets, Item: widget criado)
    └─ Add Child to Uniform Grid Panel
         ├─ Parent: UniformGridPanel_Storage
         ├─ Content: widget criado
         ├─ Column: Index % 10  ← Coluna (0-9)
         └─ Row: Index / 10     ← Linha (0-9)
```

**Nota:** Use `Floor (Index / 10)` para obter a linha corretamente.

---

### **4. Update All Slots Visual (Custom Function)**

**Crie uma função `UpdateAllSlotsVisual`:**

```
Function: Update All Slots Visual
  ↓
Get Storage Data  ← Array com todos os slots do armazém
  ↓
ForEachLoop (Storage Data)
  Loop Body:
    ├─ Break Umbra Inventory Slot
    │     └─ Slot Index
    ├─ Get Array Item (SlotWidgets, Index: Slot Index)
    ├─ Is Valid? (do Array Item)
    │   └─ TRUE:
    │       ├─ Set Slot Data (do Array Item)
    │       │     └─ New Slot Data: Array Element (do ForEachLoop)
    │       └─ Update Slot Visual (do Array Item)
```

**IMPORTANTE:** Esta função é similar ao `OnInventoryLoaded_Event` do inventário, mas usa `Get Storage Data` em vez de `Current Inventory`.

---

## 🔄 **DRAG & DROP ENTRE INVENTÁRIO E ARMAZÉM:**

### **📋 VISÃO GERAL:**

O sistema de drag & drop permite mover itens entre:
- **Inventário do Jogador** (50 slots) ↔ **Armazém** (100 slots)
- **Armazém** ↔ **Armazém** (reorganizar dentro do armazém)

**IMPORTANTE:** A implementação é feita no `WBP_InventorySlot`, que é usado tanto no inventário quanto no armazém.

---

## 🎯 **IMPLEMENTAÇÃO DO OnDragDetected:**

### **PASSO 1: Criar o Event OnDragDetected**

1. **Abra `WBP_InventorySlot`** no Blueprint Editor
2. **Event Graph** → **Functions** → **Override** → **OnDragDetected**
3. O evento terá 2 parâmetros:
   - `Geometry` (Geometry)
   - `PointerEvent` (PointerEvent)

---

### **PASSO 2: Verificar se o Slot tem Item**

**Nó 1: Get Slot Data**

1. **Do `self`**, arraste → **Get Slot Data**
2. **Propósito:** Obtém os dados do slot atual

**Ligações:**
```
self → Get Slot Data (Target)
```

**Nó 2: Break Umbra Inventory Slot**

1. **Right Click** → **Break Umbra Inventory Slot**
2. **Conecte** o `Return Value` do `Get Slot Data` ao pin de entrada do `Break`
3. **Propósito:** Extrai o `InventoryID` para verificar se há item

**Ligações:**
```
Get Slot Data (Return Value) → Break Umbra Inventory Slot (Umbra Inventory Slot)
```

**Nó 3: Branch (InventoryID > 0?)**

1. **Right Click** → **Branch**
2. **Conecte** o `InventoryID` do `Break` ao pin `A` de um **Greater (Int Int)**
3. **Conecte** um **Make Literal Int** com valor `0` ao pin `B` do `Greater`
4. **Conecte** o `Return Value` do `Greater` ao pin `Condition` do `Branch`
5. **Conecte** o pin `execute` do `OnDragDetected` ao pin `execute` do `Branch`
6. **Propósito:** Só permite arrastar se houver um item no slot

**Ligações:**
```
OnDragDetected (execute) → Branch (execute)
Break (InventoryID) → Greater (A)
Make Literal Int (0) → Greater (B)
Greater (Return Value) → Branch (Condition)
```

---

### **PASSO 3: Criar a Drag Drop Operation**

**Nó 4: Create Item Drag Operation**

1. **Do `self`**, arraste → **Create Item Drag Operation**
2. **Conecte** o pin `then` do `Branch` ao pin `execute` do `Create Item Drag Operation`
3. **Propósito:** Cria a operação de drag & drop com referência ao slot

**Ligações:**
```
Branch (then) → Create Item Drag Operation (execute)
```

**Nó 5: Detect Drag If Pressed**

1. **Right Click** → **Detect Drag If Pressed**
2. **Parâmetros:**
   - `Geometry`: Conecte o `Geometry` do `OnDragDetected`
   - `PointerEvent`: Conecte o `PointerEvent` do `OnDragDetected`
   - `Key`: `LeftMouseButton` (EKeys)
   - `WidgetDetectingDrag`: Conecte o `self`
   - `DragVisual`: Deixe vazio ou use um widget customizado
   - `DragPivot`: `MouseDown` (EDragPivot)
   - `Offset`: `(0, 0)` (FVector2D)
3. **Propósito:** Detecta quando o mouse é pressionado e arrastado

**Como obter EKeys::LeftMouseButton:**
- **Right Click** → Digite "Make Literal Key"
- **Set Value:** `LeftMouseButton`

**Como obter EDragPivot::MouseDown:**
- **Right Click** → Digite "Make Literal Byte"
- **Set Value:** `0` (MouseDown) ou use o dropdown do pin

**Ligações:**
```
Create Item Drag Operation (then) → Detect Drag If Pressed (execute)
OnDragDetected (Geometry) → Detect Drag If Pressed (Geometry)
OnDragDetected (PointerEvent) → Detect Drag If Pressed (PointerEvent)
Make Literal Key (LeftMouseButton) → Detect Drag If Pressed (Key)
self → Detect Drag If Pressed (WidgetDetectingDrag)
Make Literal Byte (0) → Detect Drag If Pressed (DragPivot)
Make Vector2D (0, 0) → Detect Drag If Pressed (Offset)
```

**Nó 6: Return Node**

1. **Right Click** → **Return Node**
2. **Conecte** o `Return Value` do `Detect Drag If Pressed` ao pin `Return Value` do `Return Node`
3. **Conecte** o pin `then` do `Detect Drag If Pressed` ao pin `execute` do `Return Node`
4. **Propósito:** Retorna a operação de drag para o sistema

**Ligações:**
```
Detect Drag If Pressed (then) → Return Node (execute)
Detect Drag If Pressed (Return Value) → Return Node (Return Value)
```

---

### **PASSO 4: Caso Slot Vazio (FALSE path)**

**Nó 7: Return Node (Slot Vazio)**

1. **Right Click** → **Return Node**
2. **Conecte** o pin `else` do `Branch` ao pin `execute` do `Return Node`
3. **Deixe o `Return Value` vazio** (ou conecte `Unhandled()`)
4. **Propósito:** Retorna sem fazer nada se o slot está vazio

**Ligações:**
```
Branch (else) → Return Node (execute)
```

---

## 📊 **DIAGRAMA DO OnDragDetected:**

```
Event OnDragDetected (Geometry, PointerEvent)
  ↓
Get Slot Data (self)
  ↓
Break Umbra Inventory Slot
  └─ InventoryID
  ↓
Branch (InventoryID > 0?)
  ├─ TRUE: (Slot tem item)
  │   ↓
  │   Create Item Drag Operation (self)
  │   ↓ (then)
  │   Detect Drag If Pressed
  │   ├─ Geometry: (do OnDragDetected)
  │   ├─ PointerEvent: (do OnDragDetected)
  │   ├─ Key: LeftMouseButton
  │   ├─ WidgetDetectingDrag: self
  │   ├─ DragPivot: MouseDown
  │   └─ Offset: (0, 0)
  │   ↓
  │   Return Node (Return Value: Detect Drag If Pressed)
  │
  └─ FALSE: (Slot vazio)
      └─ Return Node (Return Value: vazio/Unhandled)
```

---

## 🎯 **IMPLEMENTAÇÃO COMPLETA DO OnDrop:**

### **⚠️ IMPORTANTE: TARGET OBRIGATÓRIO!**

**TODAS as funções abaixo REQUEREM que o pin `Target` esteja conectado!**

- **`Get Source Slot Widget`** → Target: `Cast (As Umbra Item Drag Drop Operation)`
- **`Get Parent`** → Target: `Get Source Slot Widget (Return Value)` ou `self`
- **`Get Slot Data`** → Target: `Get Source Slot Widget (Return Value)` ou `self`
- **`Move Item To Storage`** → Target: `Cast to WBP Storage (As WBP Storage)`
- **`Move Item From Storage`** → Target: `Cast to WBP Storage (As WBP Storage)`

**Se o Target não estiver conectado, o Blueprint NÃO compilará!**

**DICA:** Sempre arraste do objeto para criar o nó automaticamente - o Target será conectado automaticamente!

---

### **PASSO 1: Criar o Event OnDrop**

1. **Abra `WBP_InventorySlot`** no Blueprint Editor
2. **Event Graph** → **Functions** → **Override** → **OnDrop**
3. O evento terá 3 parâmetros:
   - `Geometry` (Geometry)
   - `PointerEvent` (PointerEvent)
   - `Operation` (DragDropOperation)

---

### **PASSO 2: Obter o Source Slot Widget**

**⚠️ IMPORTANTE: TARGET OBRIGATÓRIO!**

Todas as funções abaixo **REQUEREM** que o pin `Target` esteja conectado. Se não conectar, o Blueprint não compilará!

---

**Nó 1: Cast to Umbra Item Drag Drop Operation**

1. **Right Click** no Event Graph → **Cast to Umbra Item Drag Drop Operation**
2. **Conecte** o pin `Operation` (do `OnDrop`) ao pin `Object` do Cast
3. **Propósito:** Converte o `Operation` genérico para o tipo específico que contém o slot de origem

**Como obter:**
- **Right Click** → Digite "Cast to Umbra Item Drag Drop Operation"
- Se não aparecer, verifique se a classe `UUmbraItemDragDropOperation` está compilada

**Ligações:**
```
OnDrop (Operation) → Cast to Umbra Item Drag Drop Operation (Object)
```

---

**Nó 2: Get Source Slot Widget**

**⚠️ CRÍTICO: O pin `Target` DEVE estar conectado!**

1. **Arraste** o pin `As Umbra Item Drag Drop Operation` do Cast
2. **Solte** no Event Graph
3. **Menu aparece** → Selecione **Get Source Slot Widget**
4. **✅ O Target será conectado automaticamente!**

**OU manualmente:**
1. **Right Click** → **Get Source Slot Widget**
2. **Conecte** o pin `Target` ao pin `As Umbra Item Drag Drop Operation` do Cast

**Propósito:** Obtém o widget do slot que está sendo arrastado

**Ligações CORRETAS:**
```
Cast (As Umbra Item Drag Drop Operation) → Get Source Slot Widget (Target)  ← OBRIGATÓRIO!
```

**❌ ERRADO:**
```
Get Source Slot Widget (Target: vazio)  ← NÃO COMPILA!
```

**✅ CORRETO:**
```
Cast (As Umbra Item Drag Drop Operation) → Get Source Slot Widget (Target)
```

**Variável temporária:**
- Crie uma variável local: `SourceSlotWidget` (WBP_InventorySlot)
- Ou use diretamente o output do `Get Source Slot Widget`

---

### **PASSO 3: Validar o Source Slot Widget**

**Nó 3: Is Valid**

1. **Right Click** → **Is Valid**
2. **Conecte** o output do `Get Source Slot Widget` ao pin `Object` do `Is Valid`
3. **Propósito:** Verifica se o slot de origem é válido

**Ligações:**
```
Get Source Slot Widget (Return Value) → Is Valid (Object)
```

**Nó 4: Branch**

1. **Right Click** → **Branch**
2. **Conecte** o `Return Value` do `Is Valid` ao pin `Condition` do `Branch`
3. **Conecte** o pin `then` do Cast ao pin `execute` do `Branch`
4. **Propósito:** Decide se continua a lógica ou retorna

**Ligações:**
```
Cast (then) → Branch (execute)
Is Valid (Return Value) → Branch (Condition)
```

---

### **PASSO 4: Obter os Dados do Item Arrastado**

**Nó 5: Get Slot Data**

1. **Do `Get Source Slot Widget`**, arraste o output
2. **Right Click** → **Get Slot Data**
3. **Propósito:** Obtém os dados do slot que está sendo arrastado

**Como obter:**
- Arraste do `Get Source Slot Widget` → **Get Slot Data**
- Ou **Right Click** → Digite "Get Slot Data"

**Ligações:**
```
Get Source Slot Widget (Return Value) → Get Slot Data (Target)
```

**Nó 6: Break Umbra Inventory Slot**

1. **Right Click** → **Break Umbra Inventory Slot**
2. **Conecte** o `Return Value` do `Get Slot Data` ao pin de entrada do `Break`
3. **Propósito:** Extrai os campos individuais do slot (InventoryID, SlotIndex, etc.)

**Ligações:**
```
Get Slot Data (Return Value) → Break Umbra Inventory Slot (Umbra Inventory Slot)
```

**Campos importantes a extrair:**
- `InventoryID` (int32) - ID do item no inventário/armazém
- `SlotIndex` (int32) - Índice do slot de origem
- `ItemTemplateID` (int32) - ID do template do item

---

### **PASSO 5: Verificar se o Item é Válido**

**Nó 7: Branch (InventoryID > 0)**

1. **Right Click** → **Branch**
2. **Conecte** o `InventoryID` do `Break` ao pin `A` de um **Greater (Int Int)**
3. **Conecte** um **Make Literal Int** com valor `0` ao pin `B` do `Greater`
4. **Conecte** o `Return Value` do `Greater` ao pin `Condition` do `Branch`
5. **Conecte** o pin `then` do `Branch` anterior ao pin `execute` deste `Branch`
6. **Propósito:** Verifica se há um item válido sendo arrastado (InventoryID > 0)

**Ligações:**
```
Branch (then) → Branch (execute) [novo Branch]
Break (InventoryID) → Greater (A)
Make Literal Int (0) → Greater (B)
Greater (Return Value) → Branch (Condition)
```

**Como obter Make Literal Int:**
- **Right Click** → Digite "Make Literal Int"
- **Set Value:** 0

---

### **PASSO 6: Determinar a Origem do Item (Inventário ou Armazém)**

**ESTA É A PARTE CRÍTICA!** Precisamos descobrir se o item veio do inventário ou do armazém.

**MÉTODO 1: Verificar o Parent Widget (RECOMENDADO)**

**Nó 8: Get Parent**

**⚠️ CRÍTICO: O pin `Target` DEVE estar conectado!**

1. **Arraste** o pin `Return Value` do `Get Source Slot Widget`
2. **Solte** no Event Graph
3. **Menu aparece** → Selecione **Get Parent**
4. **✅ O Target será conectado automaticamente!**

**OU manualmente:**
1. **Right Click** → **Get Parent**
2. **Conecte** o pin `Target` ao pin `Return Value` do `Get Source Slot Widget`

**Propósito:** Obtém o widget pai do slot (será `WBP_Inventory` ou `WBP_Storage`)

**Ligações CORRETAS:**
```
Get Source Slot Widget (Return Value) → Get Parent (Target)  ← OBRIGATÓRIO!
```

**❌ ERRADO:**
```
Get Parent (Target: vazio)  ← NÃO COMPILA!
```

**✅ CORRETO:**
```
Get Source Slot Widget (Return Value) → Get Parent (Target)
```

**Nó 9: Cast to WBP Inventory**

1. **Right Click** → **Cast to WBP Inventory**
2. **Conecte** o `Return Value` do `Get Parent` ao pin `Object` do Cast
3. **Propósito:** Tenta converter o parent para `WBP_Inventory`

**Ligações:**
```
Get Parent (Return Value) → Cast to WBP Inventory (Object)
```

**Nó 10: Branch (Is Inventory?)**

1. **Use o pin `bSuccess` do Cast** (ou o pin `then` vs `CastFailed`)
2. **Conecte** o pin `then` do Cast anterior ao pin `execute` de um novo `Branch`
3. **Conecte** o pin `bSuccess` do Cast ao pin `Condition` do `Branch`
4. **Propósito:** Se o cast for bem-sucedido, o item veio do inventário. Se falhar, veio do armazém.

**Ligações:**
```
Branch (then) → Cast to WBP Inventory (execute)
Cast to WBP Inventory (bSuccess) → Branch (Condition)
```

**Alternativa (usando then vs CastFailed):**
```
Branch (then) → Cast to WBP Inventory (execute)
Cast (then) → Branch (Condition: TRUE)  ← Veio do inventário
Cast (CastFailed) → Branch (Condition: FALSE)  ← Veio do armazém
```

---

**MÉTODO 2: Usar Variável de Contexto (ALTERNATIVA)**

Se preferir, você pode adicionar uma variável booleana `bIsStorageSlot` no `WBP_InventorySlot` e setá-la durante a criação dos slots.

**No `CreateStorageSlots` do `WBP_Storage`:**
```
Create Widget (WBP_InventorySlot)
  ↓
Set bIsStorageSlot (do widget criado, Value: true)
```

**No `CreateInventorySlots` do `WBP_Inventory`:**
```
Create Widget (WBP_InventorySlot)
  ↓
Set bIsStorageSlot (do widget criado, Value: false)
```

**No `OnDrop`:**
```
Get Source Slot Widget
  ↓
Get bIsStorageSlot (do Source Slot Widget)
  ↓
Branch (bIsStorageSlot?)
  ├─ TRUE: Veio do armazém
  └─ FALSE: Veio do inventário
```

---

### **PASSO 7: Mover Item do Inventário para o Armazém**

**Caminho TRUE do Branch (Item veio do Inventário):**

**Nó 11: Get Parent (do self - slot de destino)**

**⚠️ CRÍTICO: O pin `Target` DEVE estar conectado ao `self`!**

1. **Arraste** o pin `self` (do Event Graph)
2. **Solte** no Event Graph
3. **Menu aparece** → Selecione **Get Parent**
4. **✅ O Target será conectado automaticamente!**

**OU manualmente:**
1. **Right Click** → **Get Parent**
2. **Conecte** o pin `Target` ao pin `self`

**Propósito:** Obtém o widget pai do slot de destino (deve ser `WBP_Storage`)

**Ligações CORRETAS:**
```
self → Get Parent (Target)  ← OBRIGATÓRIO!
```

**❌ ERRADO:**
```
Get Parent (Target: vazio)  ← NÃO COMPILA!
```

**✅ CORRETO:**
```
self → Get Parent (Target)
```

**Ligações:**
```
self → Get Parent (Target)
```

**Nó 12: Cast to WBP Storage**

1. **Right Click** → **Cast to WBP Storage**
2. **Conecte** o `Return Value` do `Get Parent` ao pin `Object` do Cast
3. **Propósito:** Converte o parent para `WBP_Storage` para acessar suas funções

**Ligações:**
```
Get Parent (Return Value) → Cast to WBP Storage (Object)
```

**Nó 13: Move Item To Storage**

1. **Do `Cast to WBP Storage`**, arraste o pin `As WBP Storage`
2. **Right Click** → **Move Item To Storage**
3. **Propósito:** Chama a função C++ que move o item do inventário para o armazém

**Parâmetros:**
- `InventoryItemID` (int32): Conecte o `InventoryID` do `Break`
- `TargetSlotIndex` (int32): Conecte o `SlotIndex` do `self` (slot de destino)

**Como obter SlotIndex do self:**
- **Do `self`**, arraste → **Get Slot Data**
- **Break Umbra Inventory Slot** → `SlotIndex`

**Ligações:**
```
Branch (then) → Cast to WBP Storage (execute)
Cast (As WBP Storage) → Move Item To Storage (Target)
Break (InventoryID) → Move Item To Storage (InventoryItemID)
self → Get Slot Data → Break → SlotIndex → Move Item To Storage (TargetSlotIndex)
```

**Nó 14: Branch (Move Success?)**

1. **Right Click** → **Branch**
2. **Conecte** o `Return Value` do `Move Item To Storage` ao pin `Condition` do `Branch`
3. **Conecte** o pin `then` do Cast ao pin `execute` do `Branch`
4. **Propósito:** Verifica se a operação foi bem-sucedida

**Ligações:**
```
Cast (then) → Branch (execute)
Move Item To Storage (Return Value) → Branch (Condition)
```

**Nó 15: Load Inventory (TRUE path)**

1. **Get Game Instance** → **Cast to Umbra Game Instance**
2. **Cast** → **Load Inventory**
3. **Propósito:** Recarrega o inventário após mover o item

**Ligações:**
```
Branch (then) → Get Game Instance → Cast → Load Inventory (execute)
```

**Nó 16: Load Storage (TRUE path)**

1. **Do `Cast to WBP Storage`**, arraste → **Load Storage**
2. **Conecte** o pin `then` do `Load Inventory` ao pin `execute` do `Load Storage`
3. **Propósito:** Recarrega o armazém para atualizar o visual

**Ligações:**
```
Load Inventory (then) → Load Storage (execute)
```

**Nó 17: On Drop Failed (FALSE path)**

1. **Do `self`**, arraste → **On Drop Failed**
2. **Conecte** o pin `else` do `Branch` ao pin `execute` do `On Drop Failed`
3. **Parâmetro:** `Reason` (String) - Ex: "Falha ao mover item para o armazém"
4. **Propósito:** Notifica que a operação falhou

**Ligações:**
```
Branch (else) → On Drop Failed (execute)
Make Literal String ("Falha ao mover item") → On Drop Failed (Reason)
```

---

### **PASSO 8: Mover Item do Armazém para o Inventário**

**Caminho FALSE do Branch (Item veio do Armazém):**

**Nó 18: Get Parent (do self - slot de destino)**

**⚠️ CRÍTICO: O pin `Target` DEVE estar conectado ao `self`!**

1. **Arraste** o pin `self` (do Event Graph)
2. **Solte** no Event Graph
3. **Menu aparece** → Selecione **Get Parent**
4. **✅ O Target será conectado automaticamente!**

**OU manualmente:**
1. **Right Click** → **Get Parent**
2. **Conecte** o pin `Target` ao pin `self`

**Propósito:** Obtém o widget pai do slot de destino

**Ligações CORRETAS:**
```
Branch (else) → Get Parent (execute)
self → Get Parent (Target)  ← OBRIGATÓRIO!
Get Parent (Return Value) → Cast to WBP Inventory (Object)
```

**Nó 19: Cast to WBP Inventory**

1. **Right Click** → **Cast to WBP Inventory**
2. **Conecte** o `Return Value` do `Get Parent` ao pin `Object` do Cast
3. **Propósito:** Converte o parent para `WBP_Inventory`

**Ligações:**
```
Branch (else) → Cast to WBP Inventory (execute)
Get Parent (Return Value) → Cast to WBP Inventory (Object)
```

**Nó 20: Get Parent (do Source Slot Widget)**

**⚠️ CRÍTICO: O pin `Target` DEVE estar conectado ao `Get Source Slot Widget (Return Value)`!**

1. **Arraste** o pin `Return Value` do `Get Source Slot Widget`
2. **Solte** no Event Graph
3. **Menu aparece** → Selecione **Get Parent**
4. **✅ O Target será conectado automaticamente!**

**OU manualmente:**
1. **Right Click** → **Get Parent**
2. **Conecte** o pin `Target` ao pin `Return Value` do `Get Source Slot Widget`

**Propósito:** Obtém o widget pai do slot de origem (deve ser `WBP_Storage`)

**Ligações CORRETAS:**
```
Get Source Slot Widget (Return Value) → Get Parent (Target)  ← OBRIGATÓRIO!
Get Parent (Return Value) → Cast to WBP Storage (Object)
```

**Nó 21: Cast to WBP Storage (do Source)**

1. **Right Click** → **Cast to WBP Storage**
2. **Conecte** o `Return Value` do `Get Parent` (do Source) ao pin `Object` do Cast
3. **Propósito:** Converte o parent do slot de origem para `WBP_Storage`

**Ligações:**
```
Get Source Slot Widget → Get Parent → Cast to WBP Storage (Object)
```

**Nó 22: Move Item From Storage**

1. **Do `Cast to WBP Storage`**, arraste → **Move Item From Storage**
2. **Propósito:** Chama a função C++ que move o item do armazém para o inventário

**Parâmetros:**
- `StorageItemID` (int32): Conecte o `InventoryID` do `Break` (do item arrastado)
- `TargetSlotIndex` (int32): Conecte o `SlotIndex` do `self` (slot de destino)

**Ligações:**
```
Cast to WBP Storage (then) → Move Item From Storage (execute)
Cast (As WBP Storage) → Move Item From Storage (Target)
Break (InventoryID) → Move Item From Storage (StorageItemID)
self → Get Slot Data → Break → SlotIndex → Move Item From Storage (TargetSlotIndex)
```

**Nó 23: Branch (Move Success?)**

1. **Right Click** → **Branch**
2. **Conecte** o `Return Value` do `Move Item From Storage` ao pin `Condition` do `Branch`
3. **Conecte** o pin `then` do Cast ao pin `execute` do `Branch`

**Ligações:**
```
Cast (then) → Branch (execute)
Move Item From Storage (Return Value) → Branch (Condition)
```

**Nó 24: Load Inventory (TRUE path)**

1. **Get Game Instance** → **Cast to Umbra Game Instance**
2. **Cast** → **Load Inventory**
3. **Conecte** o pin `then` do `Branch` ao pin `execute` do `Load Inventory`

**Ligações:**
```
Branch (then) → Get Game Instance → Cast → Load Inventory (execute)
```

**Nó 25: Load Storage (TRUE path)**

1. **Do `Cast to WBP Storage`**, arraste → **Load Storage**
2. **Conecte** o pin `then` do `Load Inventory` ao pin `execute` do `Load Storage`

**Ligações:**
```
Load Inventory (then) → Load Storage (execute)
```

**Nó 26: On Drop Failed (FALSE path)**

1. **Do `self`**, arraste → **On Drop Failed**
2. **Conecte** o pin `else` do `Branch` ao pin `execute` do `On Drop Failed`
3. **Parâmetro:** `Reason` (String) - Ex: "Falha ao mover item para o inventário"

**Ligações:**
```
Branch (else) → On Drop Failed (execute)
Make Literal String ("Falha ao mover item") → On Drop Failed (Reason)
```

---

### **PASSO 9: Casos Especiais**

**Caso 1: Slot Vazio (InventoryID <= 0)**

**Nó 27: Return Node (FALSE path do Branch InventoryID > 0)**

1. **Right Click** → **Return Node**
2. **Conecte** o pin `else` do `Branch` (InventoryID > 0) ao pin `execute` do `Return Node`
3. **Deixe o `Return Value` vazio** (ou conecte `Unhandled()`)
4. **Propósito:** Retorna sem fazer nada se não há item sendo arrastado

**Ligações:**
```
Branch (else) → Return Node (execute)
```

**Caso 2: Source Inválido**

**Nó 28: Return Node (FALSE path do Is Valid)**

1. **Right Click** → **Return Node**
2. **Conecte** o pin `else` do `Branch` (Is Valid) ao pin `execute` do `Return Node`
3. **Deixe o `Return Value` vazio** (ou conecte `Unhandled()`)
4. **Propósito:** Retorna sem fazer nada se o source é inválido

**Ligações:**
```
Branch (else) → Return Node (execute)
```

**Caso 3: Mover dentro do Mesmo Armazém (Reorganizar)**

Se o item veio do armazém e está sendo solto no mesmo armazém, você pode usar a função `MoveItem` do GameInstance:

**Nó 29: Get Game Instance**

1. **Right Click** → **Get Game Instance**
2. **Cast to Umbra Game Instance**

**Nó 30: Move Item**

1. **Do Cast**, arraste → **Move Item**
2. **Parâmetros:**
   - `InventoryID`: `Break (InventoryID)` (do item arrastado)
   - `FromSlotIndex`: `Break (SlotIndex)` (do item arrastado)
   - `ToSlotIndex`: `self → Get Slot Data → Break → SlotIndex` (slot de destino)

**Ligações:**
```
Cast (then) → Move Item (execute)
Break (InventoryID) → Move Item (InventoryID)
Break (SlotIndex) → Move Item (FromSlotIndex)
self → Get Slot Data → Break → SlotIndex → Move Item (ToSlotIndex)
```

**Nó 31: Load Storage**

1. **Do `Cast to WBP Storage`**, arraste → **Load Storage**
2. **Conecte** o pin `then` do `Move Item` ao pin `execute` do `Load Storage`

**Ligações:**
```
Move Item (then) → Load Storage (execute)
```

---

## 📊 **DIAGRAMA COMPLETO DO OnDrop:**

```
Event On Drop
  ↓
Cast to Umbra Item Drag Drop Operation (Operation)
  ↓ (then)
Get Source Slot Widget (do Cast)
  ↓
Is Valid? (Source Slot Widget)
  ├─ TRUE:
  │   ↓
  │   Get Slot Data (Source Slot Widget)
  │   ↓
  │   Break Umbra Inventory Slot
  │   │   ├─ InventoryID
  │   │   └─ SlotIndex
  │   ↓
  │   Branch (InventoryID > 0?)
  │   ├─ TRUE: (Item válido)
  │   │   ↓
  │   │   Get Parent (Source Slot Widget)
  │   │   ↓
  │   │   Cast to WBP Inventory (Parent)
  │   │   ├─ then: (Veio do Inventário)
  │   │   │   ↓
  │   │   │   Get Parent (self)
  │   │   │   ↓
  │   │   │   Cast to WBP Storage (Parent)
  │   │   │   ↓ (then)
  │   │   │   Move Item To Storage
  │   │   │   ├─ InventoryItemID: Break (InventoryID)
  │   │   │   └─ TargetSlotIndex: self → Get Slot Data → Break (SlotIndex)
  │   │   │   ↓
  │   │   │   Branch (Return Value?)
  │   │   │   ├─ TRUE:
  │   │   │   │   ├─ Load Inventory
  │   │   │   │   └─ Load Storage
  │   │   │   └─ FALSE:
  │   │   │       └─ On Drop Failed
  │   │   │
  │   │   └─ CastFailed: (Veio do Armazém)
  │   │       ↓
  │   │       Get Parent (self)
  │   │       ↓
  │   │       Cast to WBP Inventory (Parent)
  │   │       ↓ (then)
  │   │       Get Parent (Source Slot Widget)
  │   │       ↓
  │   │       Cast to WBP Storage (Parent)
  │   │       ↓ (then)
  │   │       Move Item From Storage
  │   │       ├─ StorageItemID: Break (InventoryID)
  │   │       └─ TargetSlotIndex: self → Get Slot Data → Break (SlotIndex)
  │   │       ↓
  │   │       Branch (Return Value?)
  │   │       ├─ TRUE:
  │   │       │   ├─ Load Inventory
  │   │       │   └─ Load Storage
  │   │       └─ FALSE:
  │   │           └─ On Drop Failed
  │   │
  │   └─ FALSE: (Slot vazio)
  │       └─ Return Node
  │
  └─ FALSE: (Source inválido)
      └─ Return Node
```

---

## 🎯 **RESUMO DOS NÓS NECESSÁRIOS:**

### **Nós de Entrada:**
1. ✅ **Event On Drop** (override)
2. ✅ **Cast to Umbra Item Drag Drop Operation**

### **Nós de Validação:**
3. ✅ **Get Source Slot Widget**
4. ✅ **Is Valid**
5. ✅ **Branch** (Is Valid?)
6. ✅ **Get Slot Data**
7. ✅ **Break Umbra Inventory Slot**
8. ✅ **Greater (Int Int)**
9. ✅ **Make Literal Int** (0)
10. ✅ **Branch** (InventoryID > 0?)

### **Nós de Determinação de Origem:**
11. ✅ **Get Parent** (Source Slot Widget)
12. ✅ **Cast to WBP Inventory**
13. ✅ **Branch** (Is Inventory?)

### **Nós para Mover do Inventário para Armazém:**
14. ✅ **Get Parent** (self)
15. ✅ **Cast to WBP Storage**
16. ✅ **Move Item To Storage**
17. ✅ **Branch** (Move Success?)
18. ✅ **Get Game Instance**
19. ✅ **Cast to Umbra Game Instance**
20. ✅ **Load Inventory**
21. ✅ **Load Storage**
22. ✅ **On Drop Failed**

### **Nós para Mover do Armazém para Inventário:**
23. ✅ **Get Parent** (self)
24. ✅ **Cast to WBP Inventory**
25. ✅ **Get Parent** (Source Slot Widget)
26. ✅ **Cast to WBP Storage**
27. ✅ **Move Item From Storage**
28. ✅ **Branch** (Move Success?)
29. ✅ **Get Game Instance**
30. ✅ **Cast to Umbra Game Instance**
31. ✅ **Load Inventory**
32. ✅ **Load Storage**
33. ✅ **On Drop Failed**

### **Nós de Retorno:**
34. ✅ **Return Node** (casos especiais)

---

## ⚠️ **IMPORTANTE:**

1. **Sempre valide** o `Source Slot Widget` antes de usar
2. **Sempre verifique** se `InventoryID > 0` antes de mover
3. **Sempre recarregue** o inventário e o armazém após mover itens
4. **Trate erros** usando `On Drop Failed` quando a operação falhar
5. **Use `Return Node`** para casos onde não há nada a fazer (slot vazio, source inválido)

---

## 🔧 **TESTE:**

1. **Arraste um item do inventário para o armazém** → Deve mover e atualizar ambos
2. **Arraste um item do armazém para o inventário** → Deve mover e atualizar ambos
3. **Arraste um item dentro do armazém** → Deve reorganizar
4. **Arraste um slot vazio** → Não deve fazer nada
5. **Teste com armazém cheio** → Deve mostrar erro apropriado

---

## 🎮 **CONFIGURAÇÃO DO ACTOR:**

### **PASSO 1: Criar o Blueprint do Actor**

1. **Content Browser** → `Content/Actors/Storage/`
2. **Right Click** → **Blueprint Class**
3. **Parent Class:** `UmbraStorageActor`
4. **Nome:** `BP_StorageChest`

### **PASSO 2: Configurar o Actor**

**No Blueprint Editor:**

**Components:**
- **MeshComponent:** Adicione um Static Mesh (ex: baú, cofre)
- **InteractionBox:** Ajuste o tamanho da caixa de interação

**Details Panel:**
- **Storage Widget Class:** `WBP_Storage`
- **Storage ID:** 0 (ou um ID único)
- **Storage Name:** "Armazém" (ou nome personalizado)
- **Max Interaction Distance:** 200.0
- **Requires Key:** false (ou true se precisar de key)
- **Required Key Item ID:** 0 (se `bRequiresKey = true`)

### **PASSO 3: Implementar Interação**

**No Event Graph do `BP_StorageChest`:**

**OPÇÃO A: Usar Input Action (RECOMENDADO)**

```
Event Begin Play
  ↓
(Configurar Input Action "Interact" se necessário)
```

**No Character Blueprint ou PlayerController:**

```
Input Action Interact (pressionado)
  ↓
Line Trace (do jogador para frente)
  ↓
Get Hit Result
  ↓
Is Valid? (Hit Actor)
  └─ TRUE:
      ├─ Cast to Umbra Storage Actor (Hit Actor)
      └─ TRUE:
          ├─ Get Player Controller
          ├─ Open Storage (do Storage Actor)
          │     └─ Player Controller: (do Get Player Controller)
          └─ Branch (Return Value)
              ├─ TRUE: (Armazém aberto)
              └─ FALSE: (Erro ao abrir)
```

**OPÇÃO B: Usar Botão de Interação (se já implementado)**

Se você já tem um botão de interação no actor:

```
Button_Interact (On Clicked)
  ↓
Get Player Controller
  ↓
Open Storage (self)
  │   └─ Player Controller: (do Get Player Controller)
  └─ Branch (Return Value)
      ├─ TRUE: (Sucesso)
      └─ FALSE: (Erro)
```

---

### **PASSO 4: Implementar Callbacks do Actor**

**Event On Storage Opened:**
```
Event On Storage Opened (PlayerController)
  ↓
Print String: "Armazém aberto!"  ← DEBUG
  ↓
(Adicionar efeitos visuais, sons, etc.)
```

**Event On Storage Closed:**
```
Event On Storage Closed
  ↓
Print String: "Armazém fechado!"  ← DEBUG
  ↓
(Adicionar efeitos visuais, sons, etc.)
```

**Event On Access Denied:**
```
Event On Access Denied (Reason)
  ↓
Print String: "Acesso negado: [Reason]"  ← DEBUG
  ↓
(Mostrar mensagem ao jogador)
```

---

## 🔧 **FUNÇÕES ÚTEIS DO STORAGE:**

### **Depositar Todos os Itens:**

```
Button_DepositAll (On Clicked)
  ↓
Deposit All  ← Função do UmbraStorageWidget
  │     └─ Return Value: ItemsDeposited
  ↓
Print String: "X itens depositados"  ← Feedback
  ↓
Load Storage  ← Recarrega para atualizar visual
```

### **Retirar Todos os Itens:**

```
Button_WithdrawAll (On Clicked)
  ↓
Withdraw All  ← Função do UmbraStorageWidget
  │     └─ Return Value: ItemsWithdrawn
  ↓
Print String: "X itens retirados"  ← Feedback
  ↓
Load Storage  ← Recarrega para atualizar visual
```

### **Mostrar Informações:**

```
TextBlock_Info (Set Text)
  ↓
Get Used Slots  ← Slots ocupados
  ↓
Get Max Capacity  ← Capacidade máxima
  ↓
Format Text: "Slots: {0}/{1}"
  │   ├─ {0}: Used Slots
  │   └─ {1}: Max Capacity
```

---

## ✅ **TESTE E VALIDAÇÃO:**

### **Checklist:**

- [ ] `WBP_Storage` criado com parent `UmbraStorageWidget`
- [ ] Grid de 10x10 (100 slots) criado
- [ ] `CreateStorageSlots` implementado
- [ ] `UpdateAllSlotsVisual` implementado
- [ ] `OnStorageLoaded` implementado
- [ ] `BP_StorageChest` criado e configurado
- [ ] Interação funcionando (abre/fecha armazém)
- [ ] Slots aparecem corretamente
- [ ] Drag & drop entre inventário e armazém funciona
- [ ] Botões "Depositar Tudo" e "Retirar Tudo" funcionam

---

## 🎯 **RESUMO RÁPIDO:**

1. **Criar `WBP_Storage`** com parent `UmbraStorageWidget`
2. **Criar grid 10x10** com `WBP_InventorySlot`
3. **Implementar `OnStorageLoaded`** para criar e atualizar slots
4. **Criar `BP_StorageChest`** com parent `UmbraStorageActor`
5. **Configurar interação** (botão ou input action)
6. **Testar** abrindo o armazém e movendo itens

---

**IMPLEMENTE PASSO A PASSO E TESTE!** 🚀

