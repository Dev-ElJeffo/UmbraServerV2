# ✅ OnDrop - Adicionar Armazém na Estrutura Atual (Passo a Passo)

## 📋 **ESTRUTURA ATUAL (NÃO MEXER NISSO!):**

```
OnDrop (Function Entry)
  ↓
Cast to Umbra Item Drag Drop Operation
  ├─ then → IsValid (Cast) → Branch
  └─ CastFailed → Return false
  ↓
IsValid (Source Slot Widget) → Branch
  ├─ TRUE → [AQUI VAMOS ADICIONAR A NOVA LÓGICA]
  └─ FALSE → Return false
```

---

## 🎯 **PASSO 1: Obter SlotIndex de Origem e Destino**

**LOCAL:** Após `IsValid (Source Slot Widget)` = TRUE, ANTES de `Process Item Drop`

### **1.1: Obter SlotIndex da ORIGEM (Source):**

```
Get Source Slot Widget (Return Value)
  ↓
Get Slot Data
  ├─ Target: Get Source Slot Widget (Return Value)
  └─ Return Value: FUmbraInventorySlot
  ↓
Break Umbra Inventory Slot
  ├─ SlotIndex: int32 (ORIGEM) → [GUARDAR PARA USAR DEPOIS]
  └─ InventoryID: int32 (ORIGEM) → [GUARDAR PARA USAR DEPOIS]
```

**COMO OBTER:**
- Arraste `Get Source Slot Widget (Return Value)` para o canvas
- Arraste `Get Slot Data` e conecte `Target` = `Get Source Slot Widget (Return Value)`
- Arraste `Break Umbra Inventory Slot` e conecte `Struct` = `Get Slot Data (Return Value)`

---

### **1.2: Obter SlotIndex do DESTINO (self):**

```
Get Slot Data (self)
  ├─ Target: self
  └─ Return Value: FUmbraInventorySlot
  ↓
Break Umbra Inventory Slot
  ├─ SlotIndex: int32 (DESTINO) → [GUARDAR PARA USAR DEPOIS]
  └─ InventoryID: int32 (DESTINO) → [GUARDAR PARA USAR DEPOIS]
```

**COMO OBTER:**
- Arraste `Get Slot Data` e conecte `Target` = `self`
- Arraste `Break Umbra Inventory Slot` e conecte `Struct` = `Get Slot Data (Return Value)`

---

## 🎯 **PASSO 2: Verificar se Destino é Inventário ou Armazém**

**LOCAL:** Após obter ambos os SlotIndex

### **2.1: Comparar Destino com 50:**

```
Less (Int Int)
  ├─ A: Break (SlotIndex DESTINO)
  ├─ B: Make Literal Int (50)
  └─ Return Value: bool
  ↓
Branch
  ├─ Condition: Less (Return Value)
  ├─ TRUE: Destino é INVENTÁRIO (< 50)
  └─ FALSE: Destino é ARMAZÉM (>= 50)
```

**COMO OBTER:**
- Arraste `Less (Int Int)` e conecte `A` = `Break (SlotIndex DESTINO)`
- Arraste `Make Literal Int` e defina `Value` = `50`
- Conecte `B` = `Make Literal Int (Return Value)`
- Arraste `Branch` e conecte `Condition` = `Less (Return Value)`

---

## 🎯 **PASSO 3: Se Destino é INVENTÁRIO (< 50)**

**LOCAL:** No `then` do Branch (Destino < 50?)

### **3.1: Verificar se Origem é Inventário ou Armazém:**

```
Less (Int Int)
  ├─ A: Break (SlotIndex ORIGEM)
  ├─ B: Make Literal Int (50)
  └─ Return Value: bool
  ↓
Branch
  ├─ Condition: Less (Return Value)
  ├─ TRUE: Origem é INVENTÁRIO (< 50)
  │   └─ Process Item Drop (FUNCIONA COMO ANTES!)
  │       ├─ Target: self
  │       ├─ Dragged Slot Widget: Get Source Slot Widget (Return Value)
  │       └─ Return Value: bool → Return Node (ReturnValue)
  │
  └─ FALSE: Origem é ARMAZÉM (>= 50)
      └─ Move Item From Storage
          ├─ Target: Get ParentStorageWidget (Source Slot Widget)
          ├─ StorageItemID: Break (InventoryID ORIGEM)
          ├─ TargetSlotIndex: Break (SlotIndex DESTINO)
          └─ Return Value: bool → Return Node (ReturnValue)
```

**COMO OBTER:**
- Arraste `Less (Int Int)` e conecte `A` = `Break (SlotIndex ORIGEM)`
- Conecte `B` = `Make Literal Int (50)` (reutilize o mesmo nó)
- Arraste `Branch` e conecte `Condition` = `Less (Return Value)`
- **TRUE:** Conecte `Process Item Drop` (já existe, apenas mova para cá)
- **FALSE:** Arraste `Move Item From Storage` e conecte:
  - `Target` = `Get ParentStorageWidget` → `Target` = `Get Source Slot Widget (Return Value)`
  - `StorageItemID` = `Break (InventoryID ORIGEM)`
  - `TargetSlotIndex` = `Break (SlotIndex DESTINO)`

---

## 🎯 **PASSO 4: Se Destino é ARMAZÉM (>= 50)**

**LOCAL:** No `else` do Branch (Destino < 50?)

### **4.1: Verificar se Origem é Inventário ou Armazém:**

```
Less (Int Int)
  ├─ A: Break (SlotIndex ORIGEM)
  ├─ B: Make Literal Int (50)
  └─ Return Value: bool
  ↓
Branch
  ├─ Condition: Less (Return Value)
  ├─ TRUE: Origem é INVENTÁRIO (< 50)
  │   └─ Move Item To Storage
  │       ├─ Target: Get ParentStorageWidget (self)
  │       ├─ InventoryItemID: Break (InventoryID ORIGEM)
  │       ├─ TargetSlotIndex: Break (SlotIndex DESTINO)
  │       └─ Return Value: bool → Return Node (ReturnValue)
  │
  └─ FALSE: Origem é ARMAZÉM (>= 50)
      └─ Process Item Drop (reorganizar dentro do armazém)
          ├─ Target: self
          ├─ Dragged Slot Widget: Get Source Slot Widget (Return Value)
          └─ Return Value: bool → Return Node (ReturnValue)
```

**COMO OBTER:**
- Arraste `Less (Int Int)` e conecte `A` = `Break (SlotIndex ORIGEM)`
- Conecte `B` = `Make Literal Int (50)` (reutilize o mesmo nó)
- Arraste `Branch` e conecte `Condition` = `Less (Return Value)`
- **TRUE:** Arraste `Move Item To Storage` e conecte:
  - `Target` = `Get ParentStorageWidget` → `Target` = `self`
  - `InventoryItemID` = `Break (InventoryID ORIGEM)`
  - `TargetSlotIndex` = `Break (SlotIndex DESTINO)`
- **FALSE:** Conecte `Process Item Drop` (já existe, apenas mova para cá)

---

## ⚠️ **IMPORTANTE:**

1. **NÃO REMOVA `Process Item Drop` existente!** Apenas mova-o para dentro dos Branches corretos.
2. **Todas as funções (`Process Item Drop`, `Move Item To Storage`, `Move Item From Storage`) devem conectar ao mesmo `Return Node` com `ReturnValue = true`.**
3. **Se `ParentStorageWidget` não existir no C++, você precisa adicioná-lo primeiro!**

---

## 📝 **RESUMO DA ESTRUTURA FINAL:**

```
OnDrop (Function Entry)
  ↓
Cast to Umbra Item Drag Drop Operation
  ├─ then → IsValid (Cast) → Branch
  └─ CastFailed → Return false
  ↓
IsValid (Source Slot Widget) → Branch
  ├─ TRUE → [NOVA LÓGICA]
  │   ├─ Get Slot Data (Source) → Break → SlotIndex ORIGEM, InventoryID ORIGEM
  │   ├─ Get Slot Data (self) → Break → SlotIndex DESTINO, InventoryID DESTINO
  │   ├─ Less (Destino < 50?) → Branch
  │   │   ├─ TRUE: (Destino = Inventário)
  │   │   │   ├─ Less (Origem < 50?) → Branch
  │   │   │   │   ├─ TRUE: Process Item Drop (INV → INV)
  │   │   │   │   └─ FALSE: Move Item From Storage (ARMAZÉM → INV)
  │   │   │
  │   │   └─ FALSE: (Destino = Armazém)
  │   │       ├─ Less (Origem < 50?) → Branch
  │   │       │   ├─ TRUE: Move Item To Storage (INV → ARMAZÉM)
  │   │       │   └─ FALSE: Process Item Drop (ARMAZÉM → ARMAZÉM)
  │   │
  │   └─ Todas conectam ao Return Node (ReturnValue = true)
  │
  └─ FALSE → Return false
```

---

**A lógica atual de `Process Item Drop` continua funcionando para inventário → inventário!** 🎯

