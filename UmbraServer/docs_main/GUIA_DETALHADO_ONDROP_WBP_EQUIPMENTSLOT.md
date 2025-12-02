# 📘 GUIA DETALHADO: OnDrop no WBP_EquipmentSlot

## 🎯 **OBJETIVO:**

Quando um item é arrastado e solto em um slot de equipamento, precisamos:
1. Verificar se o item pode ser equipado neste slot
2. Verificar se o tipo de equipamento do item corresponde ao slot
3. Equipar o item chamando a API

---

## 🔍 **ENTENDENDO O PAYLOAD:**

### ⚠️ **IMPORTANTE: NÃO EXISTE "Get Payload"!**

O `UmbraItemDragDropOperation` **NÃO tem** um método `GetPayload()`.

**O QUE EXISTE:**
- ✅ `GetDraggedItemData()` - Retorna `FUmbraInventorySlot` (os dados do item)
- ✅ `GetSourceSlotWidget()` - Retorna o widget de origem
- ✅ `GetSourceSlotIndex()` - Retorna o índice do slot de origem

**O "PAYLOAD" É:**
- O `DraggedItemData` (FUmbraInventorySlot) que contém todas as informações do item

---

## 📋 **ESTRUTURA COMPLETA DO ONDROP:**

### **PASSO 1: Cast para UmbraItemDragDropOperation**

```
[OnDrop]
  ├─ Geometry: (FGeometry)
  ├─ Operation: (Drag Drop Operation)
  ↓
[Cast to Umbra Item Drag Drop Operation]
  ├─ Object: Operation
  └─ Success: As Umbra Item Drag Drop Operation
```

**POR QUÊ?**
- O `OnDrop` recebe um `Drag Drop Operation` genérico
- Precisamos fazer Cast para `UmbraItemDragDropOperation` para acessar os dados específicos

---

### **PASSO 2: Obter os Dados do Item (O "PAYLOAD")**

```
[Get Dragged Item Data] ← NÃO É "Get Payload"!
  ├─ Target: As Umbra Item Drag Drop Operation
  └─ Return Value: (FUmbraInventorySlot) ← ESTE É O "PAYLOAD"!
```

**O QUE É O "PAYLOAD"?**
- É o `FUmbraInventorySlot` que contém:
  - `InventoryID` - ID do item no inventário
  - `ItemTemplateID` - ID do template do item
  - `ItemTemplate` - Template completo (com `EquipmentSlot`, `ItemName`, etc.)
  - `Quantity` - Quantidade
  - `Durability` - Durabilidade
  - E outros dados...

---

### **PASSO 3: Validar se o Item é Válido**

```
[Break Umbra Inventory Slot]
  ├─ Umbra Inventory Slot: Return Value (do Get Dragged Item Data)
  └─ Item Template ID: (int)
  ↓
[Greater (Int Int)]
  ├─ A: Item Template ID
  ├─ B: 0
  └─ Return Value: (bool) ← Item Template ID > 0?
```

**POR QUÊ?**
- Se `ItemTemplateID` for 0 ou negativo, o item não é válido
- Não podemos equipar um item inválido

---

### **PASSO 4: Obter o Equipment Slot do Item**

```
[Break Umbra Inventory Slot]
  └─ Item Template: (FUmbraItemTemplate)
  ↓
[Break Umbra Item Template]
  └─ Equipment Slot: (EUmbraEquipmentSlot)
```

**O QUE É?**
- Cada item tem um `EquipmentSlot` que define onde ele pode ser equipado
- Exemplos: `Head`, `Chest`, `MainHand`, `Ring`, etc.

---

### **PASSO 5: Verificar se o Slot do Item Corresponde ao Slot do Widget**

```
[Equal (Enum Enum)]
  ├─ A: Equipment Slot (do Item Template)
  ├─ B: Equipment Slot Type (variável do WBP_EquipmentSlot)
  └─ Return Value: (bool) ← São iguais?
```

**POR QUÊ?**
- Um item de `Head` só pode ser equipado em um slot de `Head`
- Um item de `Ring` só pode ser equipado em um slot de `Ring`
- Precisamos verificar se o tipo do slot do item corresponde ao tipo do slot do widget

---

### **PASSO 6: Equipar o Item**

```
[Get Game Instance]
  └─ Return Value: (Game Instance)
  ↓
[Cast to Umbra Game Instance]
  └─ Success: As Umbra Game Instance
  ↓
[Equip Item]
  ├─ Target: As Umbra Game Instance
  └─ Inventory ID: Inventory ID (do Break Umbra Inventory Slot)
  ↓
[Handled]
```

**O QUE ACONTECE?**
- Chama `EquipItem(InventoryID)` no Game Instance
- O Game Instance faz a requisição para a API PHP
- A API valida e equipa o item no banco de dados
- O delegate `OnItemEquipped` é disparado
- O widget é atualizado automaticamente

---

## 📊 **ESTRUTURA VISUAL COMPLETA:**

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
  │  [Get Dragged Item Data] ← O "PAYLOAD"!                 │
  │    ├─ Target: As Umbra Item Drag Drop Operation         │
  │    └─ Return Value: (FUmbraInventorySlot)               │
  │         │                                                │
  │         ▼                                                │
  │  [Break Umbra Inventory Slot]                            │
  │    ├─ Inventory ID: (int)                               │
  │    ├─ Item Template ID: (int)                           │
  │    └─ Item Template: (FUmbraItemTemplate)               │
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
  │         │    │    ├─ B: Equipment Slot Type (widget) │   │
  │         │    │    └─ Return Value: (bool)            │   │
  │         │    │         │                             │   │
  │         │    │         ├─ TRUE ───────────────────┐  │   │
  │         │    │         │                          │  │   │
  │         │    │         │  [Get Game Instance]     │  │   │
  │         │    │         │    └─ Game Instance       │  │   │
  │         │    │         │         │                 │  │   │
  │         │    │         │         ▼                 │  │   │
  │         │    │         │  [Cast to Umbra Game Instance] │   │
  │         │    │         │    └─ Success              │  │   │
  │         │    │         │         │                 │  │   │
  │         │    │         │         ▼                 │  │   │
  │         │    │         │  [Equip Item]             │  │   │
  │         │    │         │    ├─ Target: Game Instance│  │   │
  │         │    │         │    └─ Inventory ID        │  │   │
  │         │    │         │         │                 │  │   │
  │         │    │         │         ▼                 │  │   │
  │         │    │         │  [Handled]                │  │   │
  │         │    │         │                          │  │   │
  │         │    │         └──────────────────────────┘  │   │
  │         │    │                                        │   │
  │         │    │  [Unhandled] ← Slot incorreto        │   │
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

---

## 🔧 **PASSO A PASSO DETALHADO:**

### **PASSO 1: Criar o Evento OnDrop**

1. No `WBP_EquipmentSlot`, Event Graph
2. Clique direito → **Override** → `OnDrop`
3. Você terá:
   - `Geometry` (FGeometry)
   - `Operation` (Drag Drop Operation)

---

### **PASSO 2: Cast para UmbraItemDragDropOperation**

1. Arraste o pin `Operation`
2. Digite: `Cast to Umbra Item Drag Drop Operation`
3. Conecte `Operation` ao pin `Object` do Cast

---

### **PASSO 3: Get Dragged Item Data (O "PAYLOAD")**

1. Arraste o pin `As Umbra Item Drag Drop Operation` (saída do Cast)
2. Digite: `Get Dragged Item Data`
3. **Target:** Conecte ao `As Umbra Item Drag Drop Operation`
4. **Return Value:** Este é o "PAYLOAD"! (FUmbraInventorySlot)

---

### **PASSO 4: Break Umbra Inventory Slot**

1. Arraste o pin `Return Value` do `Get Dragged Item Data`
2. Digite: `Break Umbra Inventory Slot`
3. Conecte o `Return Value` ao pin de entrada do Break
4. Expanda e mostre os pins:
   - `Inventory ID` ← Vamos usar para equipar
   - `Item Template ID` ← Para validar
   - `Item Template` ← Para obter o Equipment Slot

---

### **PASSO 5: Validar Item Template ID**

1. Arraste o pin `Item Template ID` do Break
2. Digite: `Greater (Int Int)`
3. Conecte:
   - `A`: `Item Template ID`
   - `B`: `0` (Make Literal Int)
4. Conecte o `Return Value` do `Greater` a um `Branch`

---

### **PASSO 6: Obter Equipment Slot do Item**

1. Arraste o pin `Item Template` do Break
2. Digite: `Break Umbra Item Template`
3. Conecte `Item Template` ao pin de entrada
4. Expanda e mostre o pin `Equipment Slot`

---

### **PASSO 7: Comparar Equipment Slots**

1. **Para o PIN A (Equipment Slot do Item):**
   - Arraste o pin `Equipment Slot` do `Break Umbra Item Template`
   - Este é o slot do item que está sendo arrastado

2. **Para o PIN B (Equipment Slot Type do Widget):**
   - Clique direito → Digite: `Get Equipment Slot Type` (ou o nome da sua variável)
   - Esta é a variável do `WBP_EquipmentSlot` que define qual tipo de slot este widget representa
   - **⚠️ IMPORTANTE:** Se a variável não existir, você precisa criá-la primeiro! (Veja `COMO_OBTER_EQUIPMENT_SLOT_ONDROP.md`)

3. **Criar o Equal:**
   - Digite: `Equal (Enum Enum)` ou `Equal (Byte Byte)`
   - Conecte:
     - `A`: `Equipment Slot` (do item - PIN A)
     - `B`: `Equipment Slot Type` (do widget - PIN B)
   - Conecte o `Return Value` a um `Branch`

**📘 Veja o guia detalhado:** `COMO_OBTER_EQUIPMENT_SLOT_ONDROP.md`

---

### **PASSO 8: Equipar o Item**

1. Se o `Equal` for `True`:
   - `Get Game Instance`
   - `Cast to Umbra Game Instance`
   - `Equip Item`
     - Target: Game Instance
     - Inventory ID: `Inventory ID` (do Break Umbra Inventory Slot)
   - `Handled`

2. Se o `Equal` for `False`:
   - `Unhandled` (slot incorreto)

---

## ⚠️ **IMPORTANTE:**

### **NÃO USE:**
- ❌ `Get Payload` - Não existe!
- ❌ `Payload` - Não é um método

### **USE:**
- ✅ `Get Dragged Item Data` - Retorna o `FUmbraInventorySlot` (o "PAYLOAD")
- ✅ `Break Umbra Inventory Slot` - Para acessar os dados do item
- ✅ `Break Umbra Item Template` - Para acessar o `EquipmentSlot`

---

## ✅ **RESUMO:**

1. ✅ `Cast to Umbra Item Drag Drop Operation`
2. ✅ `Get Dragged Item Data` ← Este é o "PAYLOAD"!
3. ✅ `Break Umbra Inventory Slot` → Obter `InventoryID` e `ItemTemplate`
4. ✅ `Break Umbra Item Template` → Obter `EquipmentSlot`
5. ✅ `Equal` → Comparar `EquipmentSlot` do item com o do widget
6. ✅ `Equip Item` → Se forem iguais, equipar usando `InventoryID`
7. ✅ `Handled` → Se equipou com sucesso
8. ✅ `Unhandled` → Se não pode equipar

---

## 🎯 **PRONTO!**

Agora você entende o que fazer com o "PAYLOAD" (que na verdade é o `Get Dragged Item Data`)!

