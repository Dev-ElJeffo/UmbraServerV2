# ✅ OnDrop - Como Realmente Funciona

**VERDADE:**
- `ProcessItemDrop` funciona automaticamente **DENTRO do inventário** (slots 0-49)
- **NÃO funciona automaticamente** entre inventário e armazém porque:
  - `GameInstance->MoveItem` valida `TargetSlotIndex < 50`
  - Armazém tem slots 0-99

---

## ✅ **SOLUÇÃO REAL:**

### **No OnDrop do `WBP_InventorySlot`:**

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
  └─ InventoryID, SlotIndex
  ↓
Branch (InventoryID > 0?)
  ↓ (TRUE)
Get Slot Index (self)
  └─ Return Value: int32 (slot de destino)
  ↓
Branch (SlotIndex do self < 50?)
  ├─ TRUE: (destino é inventário)
  │   ↓
  │   Branch (SlotIndex do Source < 50?)
  │   ├─ TRUE: (origem é inventário - usar ProcessItemDrop)
  │   │   └─ Process Item Drop
  │   │       ├─ Target: self
  │   │       └─ Dragged Slot Widget: Get Source Slot Widget (Return Value)
  │   │
  │   └─ FALSE: (origem é armazém - usar MoveItemFromStorage)
  │       ↓
  │       (precisa obter WBP_Storage do Source - usar variável ParentStorageWidget)
  │       ↓
  │       Move Item From Storage
  │       ├─ Target: Get ParentStorageWidget (Source)
  │       ├─ StorageItemID: Break (InventoryID)
  │       └─ TargetSlotIndex: Get Slot Index (self)
  │
  └─ FALSE: (destino é armazém)
      ↓
      Branch (SlotIndex do Source < 50?)
      ├─ TRUE: (origem é inventário - usar MoveItemToStorage)
      │   ↓
      │   Move Item To Storage
      │   ├─ Target: Get ParentStorageWidget (self)
      │   ├─ InventoryItemID: Break (InventoryID)
      │   └─ TargetSlotIndex: Get Slot Index (self)
      │
      └─ FALSE: (origem é armazém - usar ProcessItemDrop ou reorganizar)
          └─ (precisa implementar lógica de reorganização no armazém)
```

---

## 🎯 **RESUMO:**

1. **Destino < 50 e Origem < 50:** Usar `ProcessItemDrop` (automático)
2. **Destino < 50 e Origem >= 50:** Usar `MoveItemFromStorage`
3. **Destino >= 50 e Origem < 50:** Usar `MoveItemToStorage`
4. **Destino >= 50 e Origem >= 50:** Reorganizar dentro do armazém (precisa implementar)

**A lógica é baseada no SlotIndex:**
- **< 50 = Inventário**
- **>= 50 = Armazém**

---

**IMPLEMENTE USANDO SlotIndex PARA DETERMINAR ORIGEM E DESTINO!** 🎯

