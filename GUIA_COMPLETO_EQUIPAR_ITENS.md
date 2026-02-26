# 🎯 GUIA COMPLETO: Sistema de Equipar Itens

## ✅ **O QUE JÁ ESTÁ PRONTO:**

1. ✅ **API PHP:** `equip_item.php` (equipar/desequipar)
2. ✅ **Funções C++:** `EquipItem()` e `UnequipItem()` implementadas
3. ✅ **Delegates:** `OnItemEquipped` e `OnItemUnequipped` criados
4. ✅ **Enum:** `EUmbraEquipmentSlot` com todos os slots (Head, Chest, Hands, Feet, MainHand, OffHand, Ring, Amulet, Earring, Bracelet, Mount)

---

## 🎯 **OBJETIVO:**

Criar um sistema completo onde o jogador pode:
- **Equipar itens** arrastando do inventário para os slots de equipamento
- **Equipar itens** com duplo clique no inventário
- **Desequipar itens** arrastando dos slots de equipamento para o inventário
- **Desequipar itens** com duplo clique nos slots de equipamento
- **Visualizar itens equipados** nos slots corretos

---

## 📋 **ESTRUTURA DE SLOTS:**

### **11 Slots de Equipamento:**
1. **Head** (Cabeça)
2. **Chest** (Armadura)
3. **Hands** (Luvas)
4. **Feet** (Botas)
5. **MainHand** (Mão Principal - Arma)
6. **OffHand** (Mão Secundária - Escudo/Arma)
7. **Ring** (Anel)
8. **Amulet** (Colar)
9. **Earring** (Brinco)
10. **Bracelet** (Bracelete)
11. **Mount** (Montaria)

---

## 🔧 **PARTE 1: Criar Widget WBP_EquipmentSlot**

### **1.1 Criar Widget Blueprint**

**CRIAR WIDGET:**
- **Nome:** `WBP_EquipmentSlot`
- **Parent Class:** `User Widget`

### **1.2 Estrutura do Widget**

**COMPONENTES:**
1. **Border** (Root)
   - Nome: `Border_Slot`
   - Size: 64x64 (ou conforme design)
   - Brush Color: Cinza escuro (padrão vazio)

2. **Image** (Ícone do Item)
   - Nome: `Image_ItemIcon`
   - Size: 60x60
   - Visibility: Hidden (inicialmente)
   - Brush: Ícone do item

3. **TextBlock** (Nome do Slot - Opcional)
   - Nome: `Text_SlotName`
   - Text: "Head", "Chest", etc.
   - Position: Bottom ou como tooltip

### **1.3 Variáveis do Widget**

**CRIAR VARIÁVEIS:**
- `EquipmentSlot` (EUmbraEquipmentSlot) - Slot que este widget representa
- `EquippedItem` (FUmbraInventorySlot) - Item atualmente equipado (pode ser vazio)
- `bIsEmpty` (Boolean) - Se o slot está vazio

### **1.4 Funções do Widget**

**CRIAR FUNÇÕES:**

#### **1.4.1 Set Slot Type**
```
[Set Slot Type]
  ├─ Equipment Slot: (EUmbraEquipmentSlot)
  ↓
[Set Equipment Slot] ← Variável
  └─ Equipment Slot: Equipment Slot
```

#### **1.4.2 Criar Função GetRarityColor (PRIMEIRO)**

**IMPORTANTE:** Antes de criar `Update Slot Visual`, você precisa criar a função `GetRarityColor` no próprio `WBP_EquipmentSlot`.

**CRIAR FUNÇÃO:**
1. **Functions** → **+ (Add Function)**
2. Nome: `GetRarityColor`
3. **Input:** `Rarity` (type: `EUmbraItemRarity`)
4. **Output:** `ReturnValue` (type: `Linear Color`)
5. Marcar como **Pure** (não precisa de execução)

**LÓGICA DA FUNÇÃO:**
```
[GetRarityColor Entry]
  └─ Rarity: (EUmbraItemRarity)
       ↓
[Select] ← Nó Select (não Switch!)
  ├─ Index Pin Type: EUmbraItemRarity
  ├─ Option Pin Type: Linear Color
  ├─ Select: Rarity
  ├─ Common: [Make Linear Color] (R=0.6, G=0.6, B=0.6, A=1.0)
  ├─ Uncommon: [Make Linear Color] (R=0.0, G=1.0, B=0.0, A=1.0)
  ├─ Rare: [Make Linear Color] (R=0.0, G=0.5, B=1.0, A=1.0)
  ├─ Epic: [Make Linear Color] (R=0.7, G=0.0, B=1.0, A=1.0)
  └─ Legendary: [Make Linear Color] (R=1.0, G=0.5, B=0.0, A=1.0)
       ↓
[Return Node]
  └─ ReturnValue: Select Output
```

**COMO CRIAR O NÓ SELECT:**
1. Clique com botão direito → Digite: `Select`
2. Selecione o nó **Select**
3. No painel **Details**:
   - **Index Pin Type:** `EUmbraItemRarity`
   - **Option Pin Type:** `Linear Color`
4. Conecte `Rarity` ao pin **Select**
5. Para cada raridade, crie um nó **Make Linear Color** e conecte ao pin correspondente

#### **1.4.3 Update Slot Visual**
```
[Update Slot Visual]
  ├─ Item Slot: (FUmbraInventorySlot)
  ↓
[Break Umbra Inventory Slot]
  └─ Item Slot: Item Slot
  ↓
[Is Valid] ← Verificar se Item Template ID > 0
  ├─ True: [Set Brush from Texture] → Image_ItemIcon
  │         └─ Texture: Item Template > Item Icon
  │         ↓
  │         [Set Visibility] → Image_ItemIcon
  │         └─ Visibility: Visible
  │         ↓
  │         [Get Rarity Color] ← Função criada no próprio widget
  │         └─ Rarity: Item Template > Rarity
  │         └─ Target: self (não precisa, é função pura)
  │         ↓
  │         [Set Brush Color] → Border_Slot
  │         └─ Color: Return Value (do GetRarityColor)
  │
  └─ False: [Set Visibility] → Image_ItemIcon
            └─ Visibility: Hidden
            ↓
            [Set Brush Color] → Border_Slot
            └─ Color: Cinza escuro (padrão)
```

#### **1.4.3 Clear Slot**
```
[Clear Slot]
  ↓
[Make Umbra Inventory Slot] ← Slot vazio
  └─ Inventory ID: 0
  └─ Item Template ID: 0
  └─ Quantity: 0
  ↓
[Set Equipped Item] ← Variável
  └─ Equipped Item: Slot vazio
  ↓
[Update Slot Visual]
  └─ Item Slot: Slot vazio
```

---

## 🔧 **PARTE 2: Implementar Drag and Drop**

### **2.1 OnDragDetected (WBP_EquipmentSlot)**

**CRIAR EVENTO:**
```
[OnDragDetected]
  ├─ Geometry: (FGeometry)
  ├─ MouseEvent: (FPointerEvent)
  ↓
[Is Valid] ← Verificar Equipped Item > Item Template ID > 0
  ├─ True: [Create Drag Drop Operation]
  │         └─ Operation Class: Umbra Item Drag Drop Operation
  │         └─ Return Value: (Drag Drop Operation)
  │         ↓
  │         [Cast to Umbra Item Drag Drop Operation]
  │         ├─ Success: [Get Equipped Item] ← Variável
  │         │             └─ Equipped Item: (FUmbraInventorySlot)
  │         │             ↓
  │         │             [Set Dragged Item Data] ← USAR ESTE!
  │         │             └─ Target: As Umbra Item Drag Drop Operation
  │         │             └─ Dragged Item Data: Equipped Item
  │         │             ↓
  │         │             [Set Source Slot Widget] ← Opcional
  │         │             └─ Target: As Umbra Item Drag Drop Operation
  │         │             └─ Source Slot Widget: None (ou self)
  │         │             ↓
  │         │             [Set Source Slot Index] ← Opcional
  │         │             └─ Target: As Umbra Item Drag Drop Operation
  │         │             └─ Source Slot Index: -1
  │         │             ↓
  │         │             [Return Drag Drop Operation]
  │         │             └─ Drag Drop Operation: As Umbra Item Drag...
  │         │
  │         └─ Fail: [Return None]
  │
  └─ False: [Return None]
```

**IMPORTANTE:** 
- ✅ O pin "Payload" **EXISTE** no `Create Drag Drop Operation`, mas espera um `UObject`
- ❌ `Equipped Item` é uma **struct** (`FUmbraInventorySlot`), não um objeto
- ✅ **SOLUÇÃO:** Deixe "Payload" como **None** e use **`Set Dragged Item Data`** depois do Cast
- ✅ O `DraggedItemData` é a propriedade que armazena o item arrastado

### **2.2 OnDrop (WBP_EquipmentSlot)**

**⚠️ IMPORTANTE:** Não existe "Get Payload"! Use `Get Dragged Item Data`!

**CRIAR EVENTO:**
```
[OnDrop]
  ├─ Geometry: (FGeometry)
  ├─ Operation: (Drag Drop Operation)
  ↓
[Cast to Umbra Item Drag Drop Operation]
  ├─ Object: Operation
  │
  ├─ SUCCESS ───────────────────────────────────────────────┐
  │                                                         │
  │  [Get Dragged Item Data] ← ESTE É O "PAYLOAD"!          │
  │    ├─ Target: As Umbra Item Drag Drop Operation         │
  │    └─ Return Value: (FUmbraInventorySlot)                │
  │         │                                                │
  │         ▼                                                │
  │  [Break Umbra Inventory Slot]                            │
  │    ├─ Inventory ID: (int) ← Para equipar                 │
  │    ├─ Item Template ID: (int) ← Para validar            │
  │    └─ Item Template: (FUmbraItemTemplate)                │
  │         │                                                │
  │         ├─→ [Greater] → Item Template ID > 0?            │
  │         │    │                                           │
  │         │    ├─ TRUE ────────────────────────────────┐   │
  │         │    │                                       │   │
  │         │    │  [Break Umbra Item Template]          │   │
  │         │    │    └─ Equipment Slot: (Enum)          │   │
  │         │    │         │                             │   │
  │         │    │         ▼                             │   │
  │         │    │  [Equal (Enum Enum)]                   │   │
  │         │    │    ├─ A: Equipment Slot (do item)      │   │
  │         │    │    ├─ B: Equipment Slot Type (widget)  │   │
  │         │    │    └─ Return Value: (bool)            │   │
  │         │    │         │                             │   │
  │         │    │         ├─ TRUE ───────────────────┐   │   │
  │         │    │         │                        │   │   │
  │         │    │         │  [Get Game Instance]     │   │   │
  │         │    │         │    └─ Game Instance     │   │   │
  │         │    │         │         │                │   │   │
  │         │    │         │         ▼                │   │   │
  │         │    │         │  [Cast to Umbra Game Instance] │   │
  │         │    │         │    └─ Success            │   │   │
  │         │    │         │         │                │   │   │
  │         │    │         │         ▼                │   │   │
  │         │    │         │  [Equip Item]             │   │   │
  │         │    │         │    ├─ Target: Game Instance│   │   │
  │         │    │         │    └─ Inventory ID        │   │   │
  │         │    │         │         │                │   │   │
  │         │    │         │         ▼                │   │   │
  │         │    │         │  [Handled]                │   │   │
  │         │    │         │                        │   │   │
  │         │    │         └────────────────────────┘   │   │
  │         │    │                                        │   │
  │         │    │  [Unhandled] ← Slot incorreto         │   │
  │         │    │                                        │   │
  │         │    └──────────────────────────────────────┘   │
  │         │                                                │
  │         └─ FALSE                                         │
  │              │                                           │
  │              ▼                                           │
  │           [Unhandled] ← Item inválido                   │
  │                                                         │
  └─────────────────────────────────────────────────────────┘
  │
  └─ FAIL
       │
       ▼
    [Unhandled] ← Não é UmbraItemDragDropOperation
```

**📘 Veja o guia detalhado:** `GUIA_DETALHADO_ONDROP_WBP_EQUIPMENTSLOT.md`

---

## 🔧 **PARTE 3: Implementar Double Click**

### **3.1 OnMouseButtonDoubleClick (WBP_EquipmentSlot) - MÉTODO CORRETO!**

**✅ SOLUÇÃO:** Use `OnMouseButtonDoubleClick` - ele **JÁ É DISPARADO APENAS QUANDO HÁ DOUBLE CLICK!**

Não precisa verificar Click Count - o evento já faz isso automaticamente!

**CRIAR EVENTO:**
```
[OnMouseButtonDoubleClick]
  ├─ exec
  ├─ In My Geometry: (FGeometry)
  └─ In Mouse Event: (FPointerEvent)
       │
       ▼
  [Is Mouse Button Down] ← OPCIONAL (se quiser verificar o botão)
    ├─ Mouse Button: Left Mouse Button
    └─ Return Value: (bool)
         │
         ├─ TRUE ───────────────────────────────────────────┐
         │
         ├─ TRUE ───────────────────────────────────────────┐
         │                                                  │
         │  [Get Equipped Item] ← Variável                 │
         │    └─ Equipped Item: (FUmbraInventorySlot)      │
         │         │                                        │
         │         ▼                                        │
         │  [Break Umbra Inventory Slot]                   │
         │    ├─ Inventory ID: (int)                       │
         │    └─ Item Template ID: (int)                   │
         │         │                                        │
         │         ▼                                        │
         │  [Greater (Int Int)]                             │
         │    ├─ A: Item Template ID                       │
         │    ├─ B: 0                                      │
         │    └─ Return Value: (bool)                       │
         │         │                                        │
         │         ├─ TRUE ──────────────────────────────┐ │
         │         │                                      │ │
         │         │  [Get Game Instance]                │ │
         │         │    └─ Game Instance                 │ │
         │         │         │                            │ │
         │         │         ▼                            │ │
         │         │  [Cast to Umbra Game Instance]       │ │
         │         │    └─ Success                        │ │
         │         │         │                            │ │
         │         │         ▼                            │ │
         │         │  [Unequip Item]                      │ │
         │         │    ├─ Target: Game Instance         │ │
         │         │    └─ Inventory ID: Inventory ID    │ │
         │         │         │                            │ │
         │         │         ▼                            │ │
         │         │  [Handled]                            │ │
         │         │                                      │ │
         │         └─ FALSE: [Unhandled]                 │ │
         │                                                  │
         └─ FALSE: [Unhandled] ← Não é double click
```

**📘 Veja o guia simples:** `GUIA_SIMPLES_DOUBLE_CLICK_EQUIPAR.md`

```
[OnMouseButtonDown]
  ├─ Geometry: (FGeometry)
  ├─ MouseEvent: (FPointerEvent)
  ↓
[Get Mouse Button] ← Do MouseEvent
  └─ Button: Left Mouse Button
  ↓
[Is Valid] ← Verificar Equipped Item > Item Template ID > 0
  ├─ True: [Get Click Count] ← Do MouseEvent
  │         └─ Click Count: (Integer)
  │         ↓
  │         [Equal] ← Click Count == 2
  │         ├─ True: [Unequip Item] ← Game Instance
  │         │         └─ Inventory ID: Equipped Item > Inventory ID
  │         │         ↓
  │         │         [Handled]
  │         │
  │         └─ False: [Unhandled]
  │
  └─ False: [Unhandled]
```

**✅ RECOMENDAÇÃO:** Use `OnMouseButtonDoubleClick` - é mais simples e direto!

---

## 🔧 **PARTE 4: Atualizar WBP_CharacterInfo**

### **4.1 Adicionar Slots de Equipamento**

**NO LAYOUT DO WBP_CharacterInfo:**

1. **Criar Container para Slots:**
   - **Uniform Grid Panel** ou **Wrap Box**
   - Nome: `Grid_EquipmentSlots`
   - Columns: 3 ou 4 (conforme design)
   - **Criar variável:** `Grid_EquipmentSlots` (tipo: Uniform Grid Panel)
   - **Conectar no Designer:** Arraste o UniformGridPanel do Hierarchy para a variável

2. **Criar Função `CreateEquipmentSlots`:**
   - No Event Graph, criar função: `CreateEquipmentSlots`
   - Para cada slot (11 slots):
     - `Create Widget` (Class: WBP_EquipmentSlot)
     - `Set Slot Type` (com o tipo correto: Head, Chest, etc.)
     - `Add Child to Uniform Grid` (Target: Grid_EquipmentSlots, Column/Row corretos)
     - `Set Variable` (opcional, para armazenar referência)

3. **Chamar no Event Construct:**
   - No `Event Construct`, chamar `CreateEquipmentSlots`

**📘 Veja o guia detalhado:** `GUIA_CRIAR_EQUIPMENT_SLOTS_WBP_CHARACTERINFO.md`

### **4.2 Função Update Equipment Slots**

**CRIAR FUNÇÃO:**
```
[Update Equipment Slots]
  ├─ Character Info: (FUmbraCharacterInfo)
  ↓
[Break Umbra Character Info]
  └─ Character Info: Character Info
  ↓
[Get Equipped Items] ← TMap de Equipped Items
  └─ Equipped Items: (TMap)
  ↓
[ForEach Equipped Items]
  ├─ Key: Equipment Slot (EUmbraEquipmentSlot)
  ├─ Value: Inventory Slot (FUmbraInventorySlot)
  ↓
[Switch on EUmbraEquipmentSlot]
  ├─ Head: [Update Slot Visual] → Slot_Head
  │         └─ Item Slot: Inventory Slot
  │
  ├─ Chest: [Update Slot Visual] → Slot_Chest
  │          └─ Item Slot: Inventory Slot
  │
  ├─ Hands: [Update Slot Visual] → Slot_Hands
  │         └─ Item Slot: Inventory Slot
  │
  ├─ Feet: [Update Slot Visual] → Slot_Feet
  │        └─ Item Slot: Inventory Slot
  │
  ├─ MainHand: [Update Slot Visual] → Slot_MainHand
  │             └─ Item Slot: Inventory Slot
  │
  ├─ OffHand: [Update Slot Visual] → Slot_OffHand
  │            └─ Item Slot: Inventory Slot
  │
  ├─ Ring: [Update Slot Visual] → Slot_Ring
  │        └─ Item Slot: Inventory Slot
  │
  ├─ Amulet: [Update Slot Visual] → Slot_Amulet
  │           └─ Item Slot: Inventory Slot
  │
  ├─ Earring: [Update Slot Visual] → Slot_Earring
  │            └─ Item Slot: Inventory Slot
  │
  ├─ Bracelet: [Update Slot Visual] → Slot_Bracelet
  │             └─ Item Slot: Inventory Slot
  │
  └─ Mount: [Update Slot Visual] → Slot_Mount
            └─ Item Slot: Inventory Slot
```

### **4.3 Conectar Delegates**

**NO EVENT CONSTRUCT:**
```
[Event Construct]
  ↓
[Get Game Instance]
  └─ Class: Umbra Game Instance
  ↓
[Cast to Umbra Game Instance]
  ├─ Success: [Assign On Item Equipped]
  │             └─ Target: Umbra Game Instance
  │             └─ Event: OnItemEquipped_Event
  │             ↓
  │             [Assign On Item Unequipped]
  │             └─ Target: Umbra Game Instance
  │             └─ Event: OnItemUnequipped_Event
  │             ↓
  │             [Load Character Info] ← Recarregar após equipar
  │
  └─ Fail: [Log Error]
```

### **4.4 Eventos OnItemEquipped e OnItemUnequipped**

**CRIAR EVENTOS:**
```
[OnItemEquipped_Event]
  ├─ Inventory ID: (Integer)
  ├─ Equipment Slot: (EUmbraEquipmentSlot)
  ↓
[Load Character Info] ← Recarregar informações
  └─ Target: Game Instance

[OnItemUnequipped_Event]
  ├─ Inventory ID: (Integer)
  ├─ Equipment Slot: (EUmbraEquipmentSlot)
  ↓
[Load Character Info] ← Recarregar informações
  └─ Target: Game Instance
```

---

## 🔧 **PARTE 5: Atualizar WBP_InventorySlot para Double Click**

### **5.1 OnMouseButtonDoubleClick (WBP_InventorySlot) - RECOMENDADO!**

**✅ VANTAGEM:** O `OnMouseButtonDoubleClick` é disparado automaticamente quando há um double click, não precisa verificar `Click Count` manualmente!

**ADICIONAR LÓGICA:**
```
[OnMouseButtonDoubleClick]
  ├─ Geometry: (FGeometry)
  ├─ MouseEvent: (FPointerEvent)
  ├─ Button: (EKeys)
  ↓
[Equal (Key Key)] ← Verificar se é Left Mouse Button
  ├─ A: Button
  ├─ B: Left Mouse Button
  └─ Return Value: (bool)
       │
       ├─ TRUE ───────────────────────────────────────────────┐
       │                                                      │
       │  [Get Slot Data] ← Variável                          │
       │    └─ Slot Data: (FUmbraInventorySlot)                │
       │         │                                            │
       │         ▼                                            │
       │  [Break Umbra Inventory Slot]                        │
       │    ├─ Inventory ID: (int)                            │
       │    ├─ Item Template ID: (int)                        │
       │    └─ Item Template: (FUmbraItemTemplate)            │
       │         │                                            │
       │         ├─→ [Greater] → Item Template ID > 0?       │
       │         │    │                                       │
       │         │    ├─ TRUE ────────────────────────────┐  │
       │         │    │                                   │  │
       │         │    │  [Break Umbra Item Template]      │  │
       │         │    │    └─ Equipment Slot: (Enum)      │  │
       │         │    │         │                         │  │
       │         │    │         ▼                         │  │
       │         │    │  [Not Equal (Enum Enum)]          │  │
       │         │    │    ├─ A: Equipment Slot            │  │
       │         │    │    ├─ B: None                      │  │
       │         │    │    └─ Return Value: (bool)         │  │
       │         │    │         │                         │  │
       │         │    │         ├─ TRUE ────────────────┐  │  │
       │         │    │         │                     │  │  │
       │         │    │         │  [Get Game Instance] │  │  │
       │         │    │         │    └─ Game Instance  │  │  │
       │         │    │         │         │            │  │  │
       │         │    │         │         ▼            │  │  │
       │         │    │         │  [Cast to Umbra Game Instance] │  │
       │         │    │         │    └─ Success        │  │  │
       │         │    │         │         │            │  │  │
       │         │    │         │         ▼            │  │  │
       │         │    │         │  [Equip Item]        │  │  │
       │         │    │         │    ├─ Target: Game Instance│  │
       │         │    │         │    └─ Inventory ID   │  │  │
       │         │    │         │         │            │  │  │
       │         │    │         │         ▼            │  │  │
       │         │    │         │  [Handled]           │  │  │
       │         │    │         │                     │  │  │
       │         │    │         └─────────────────────┘  │  │
       │         │    │                                  │  │
       │         │    │  [Unhandled] ← Item não equipável│  │
       │         │    │                                  │  │
       │         │    └──────────────────────────────────┘  │
       │         │                                           │
       │         └─ FALSE                                     │
       │              │                                       │
       │              ▼                                       │
       │           [Unhandled] ← Item inválido                │
       │                                                      │
       └──────────────────────────────────────────────────────┘
```

**📘 Veja o guia detalhado:** `GUIA_ONMOUSEBUTTONDOUBLECLICK.md`

### **5.2 OnMouseButtonDown (WBP_InventorySlot) - ALTERNATIVA**

**⚠️ NOTA:** Se preferir usar `OnMouseButtonDown`, você precisa verificar `Click Count` manualmente (código anterior).

---

## 📋 **CHECKLIST DE IMPLEMENTAÇÃO:**

### **WBP_EquipmentSlot:**
- [x ] Criar widget com Border, Image e TextBlock
- [x ] Criar variáveis EquipmentSlot e EquippedItem
- [x ] Criar função Set Slot Type
- [ x] Criar função Update Slot Visual
- [ x] Criar função Clear Slot
- [x ] Implementar OnDragDetected
- [x ] Implementar OnDrop
- [ x] Implementar OnMouseButtonDown (double click)

### **WBP_CharacterInfo:**
- [x ] Adicionar Grid_EquipmentSlots ao layout
- [ x] Criar 11 instâncias de WBP_EquipmentSlot
- [ x] Configurar cada slot com Set Slot Type
- [ x] Criar função Update Equipment Slots
- [ ] Conectar delegates OnItemEquipped e OnItemUnequipped
- [ x] Chamar Update Equipment Slots no OnCharacterInfoLoaded

### **WBP_InventorySlot:**
- [x ] Adicionar lógica de double click no OnMouseButtonDown
- [ ] Verificar se item é equipável antes de equipar

---

## ✅ **PRONTO!**

Após implementar, o sistema de equipar itens estará completamente funcional!

**TESTE:**
1. Abra o inventário (I)
2. Abra o Character Info (C)
3. Arraste um item equipável para um slot de equipamento
4. Ou dê duplo clique em um item equipável no inventário
5. O item deve aparecer no slot correto

