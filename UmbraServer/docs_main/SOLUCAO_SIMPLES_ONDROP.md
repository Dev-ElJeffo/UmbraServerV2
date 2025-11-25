# ✅ SOLUÇÃO SIMPLES: OnDrop Correto

**PROBLEMA:**
- `Get Parent` retorna `PanelWidget`, não `UserWidget`
- Não podemos usar Get Parent para obter WBP_Inventory ou WBP_Storage

---

## ✅ **SOLUÇÃO:**

### **1. Adicionar variável no `WBP_InventorySlot`:**

**Variables:**
- `ParentStorageWidget` (WBP_Storage, Object Reference, Default: None) - Referência ao WBP_Storage (setar apenas se estiver no armazém)

---

### **2. Setar variáveis ao criar slots:**

**No `WBP_Storage::CreateStorageSlots`:**
```
Create Widget (WBP_InventorySlot)
Set ParentStorageWidget (Value: self)  ← self é o WBP_Storage
```

**No `WBP_Inventory::CreateInventorySlots`:**
```
Create Widget (WBP_InventorySlot)
(Não setar ParentStorageWidget - deixar None)
```

---

### **3. OnDrop no `WBP_InventorySlot`:**

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
Get ParentStorageWidget (Source)
  ├─ Target: Get Source Slot Widget (Return Value)
  └─ Return Value: WBP_Storage ou None
  ↓
Is Valid? (ParentStorageWidget do Source)
  ↓
Branch (Is Valid?)
  ├─ TRUE: (Source veio do armazém)
  │   ↓
  │   Get ParentStorageWidget (self)
  │   ↓
  │   Is Valid? (ParentStorageWidget do self)
  │   ↓
  │   Branch (Is Valid?)
  │   ├─ TRUE: (destino é armazém - reorganizar)
  │   │   └─ (usar MoveItem do GameInstance)
  │   └─ FALSE: (destino é inventário - mover para inventário)
  │       ↓
  │       Move Item From Storage
  │       ├─ Target: Get ParentStorageWidget (Source)
  │       ├─ StorageItemID: Break (InventoryID)
  │       └─ TargetSlotIndex: self → Get Slot Data → Break (SlotIndex)
  │
  └─ FALSE: (Source veio do inventário)
      ↓
      Get ParentStorageWidget (self)
      ↓
      Is Valid? (ParentStorageWidget do self)
      ↓
      Branch (Is Valid?)
      ├─ TRUE: (destino é armazém - mover para armazém)
      │   ↓
      │   Move Item To Storage
      │   ├─ Target: Get ParentStorageWidget (self)
      │   ├─ InventoryItemID: Break (InventoryID)
      │   └─ TargetSlotIndex: self → Get Slot Data → Break (SlotIndex)
      │
      └─ FALSE: (destino é inventário - reorganizar)
          └─ (usar MoveItem do GameInstance)
```

---

## 🎯 **RESUMO:**

1. **ParentStorageWidget** → Se != None, o slot está no armazém. Se == None, está no inventário.
2. **Is Valid?** → Usar para verificar se Source ou self estão no armazém
3. **NÃO use Get Parent** → Não funciona

---

**IMPLEMENTE ASSIM!** 🎯

