# ✅ SOLUÇÃO FINAL: OnDrop Correto

**PROBLEMA REAL:**
- `Get Parent` retorna `PanelWidget`, não `UserWidget`
- Não podemos determinar origem pelo parent
- Solução: usar variável booleana

---

## 🎯 **SOLUÇÃO:**

### **1. Adicionar variável no `WBP_InventorySlot`:**

**Variables:**
- `bIsStorageSlot` (Boolean, Default: false)

### **2. Setar a variável ao criar slots:**

**No `WBP_Inventory::CreateInventorySlots`:**
```
Set bIsStorageSlot (Value: false)
```

**No `WBP_Storage::CreateStorageSlots`:**
```
Set bIsStorageSlot (Value: true)
```

### **3. OnDrop no `WBP_InventorySlot` (NÃO no WBP_Storage):**

```
OnDrop (Operation)
  ↓
Cast to Umbra Item Drag Drop Operation
  ↓ (then)
Get Source Slot Widget
  ├─ Target: Cast (As Umbra Item Drag Drop Operation)
  └─ Return Value: WBP_InventorySlot
  ↓
Is Valid?
  ↓ (TRUE)
Get Slot Data (Source)
  ├─ Target: Get Source Slot Widget (Return Value)
  └─ Return Value: FUmbraInventorySlot
  ↓
Break Umbra Inventory Slot
  └─ InventoryID
  ↓
Branch (InventoryID > 0?)
  ↓ (TRUE)
Get bIsStorageSlot (Source)
  ├─ Target: Get Source Slot Widget (Return Value)
  └─ Return Value: bool
  ↓
Branch (bIsStorageSlot?)
  ├─ TRUE: (veio do armazém)
  │   ↓
  │   Move Item From Storage
  │   ├─ Target: (precisa obter referência ao WBP_Storage - ver abaixo)
  │   ├─ StorageItemID: Break (InventoryID)
  │   └─ TargetSlotIndex: self → Get Slot Data → Break (SlotIndex)
  │
  └─ FALSE: (veio do inventário)
      ↓
      Move Item To Storage
      ├─ Target: (precisa obter referência ao WBP_Storage - ver abaixo)
      ├─ InventoryItemID: Break (InventoryID)
      └─ TargetSlotIndex: self → Get Slot Data → Break (SlotIndex)
```

---

## ⚠️ **PROBLEMA: Como obter referência ao WBP_Storage?**

**O `Get Parent` não funciona porque retorna `PanelWidget`, não `UserWidget`.**

### **SOLUÇÃO: Adicionar variável no slot**

**No `WBP_InventorySlot`, adicione:**
- `ParentStorageWidget` (WBP_Storage, Object Reference, Default: None)
- `ParentInventoryWidget` (WBP_Inventory, Object Reference, Default: None)

**Ao criar slots:**

**No `WBP_Storage::CreateStorageSlots`:**
```
Create Widget (WBP_InventorySlot)
Set ParentStorageWidget (do widget criado, Value: self)
```

**No `WBP_Inventory::CreateInventorySlots`:**
```
Create Widget (WBP_InventorySlot)
Set ParentInventoryWidget (do widget criado, Value: self)
```

**No OnDrop (no slot do armazém):**
```
Get bIsStorageSlot (Source)
  ↓
Branch (bIsStorageSlot?)
  ├─ TRUE: (veio do armazém - reorganizar)
  │   ↓
  │   (usar MoveItem do GameInstance para reorganizar dentro do armazém)
  │
  └─ FALSE: (veio do inventário - mover para armazém)
      ↓
      Get ParentStorageWidget (self) → Move Item To Storage (Target)
```

**No OnDrop (no slot do inventário):**
```
Get bIsStorageSlot (Source)
  ↓
Branch (bIsStorageSlot?)
  ├─ TRUE: (veio do armazém - mover para inventário)
  │   ↓
  │   (precisa obter referência ao WBP_Storage do Source e chamar Move Item From Storage)
  │
  └─ FALSE: (veio do inventário - reorganizar)
      ↓
      (usar MoveItem do GameInstance para reorganizar dentro do inventário)
```

---

**IMPLEMENTE ASSIM!** 🎯

