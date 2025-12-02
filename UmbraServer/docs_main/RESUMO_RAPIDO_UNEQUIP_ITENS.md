# ⚡ RESUMO RÁPIDO: Unequip de Itens

## 🎯 **O QUE FAZER:**

### **1. WBP_EquipmentSlot - Double Click**

**Event:** `OnMouseButtonDoubleClick`

```
[OnMouseButtonDoubleClick]
  ↓
[Get Equipped Item]
  ↓
[Break Umbra Inventory Slot] → Inventory ID
  ↓
[Branch] (Inventory ID > 0?)
  ├─ True:
  │    ↓
  │  [Get Game Instance] → [Cast to Umbra Game Instance]
  │    ↓
  │  [Unequip Item] (Inventory ID)
  │
  └─ False: (nada)
```

---

### **2. WBP_InventorySlot - OnDrop (Aceitar Equipment Slots)**

**Event:** `OnDrop`

**Adicionar no início (antes da lógica normal):**

```
[OnDrop]
  ↓
[Cast to Umbra Item Drag Drop Operation]
  ↓
[Get Source Slot Widget]
  ↓
[Is Valid] (Source Slot Widget)
  ├─ True:
  │    ↓
  │  [Cast to WBP Equipment Slot]
  │    ├─ Success:
  │    │    ↓
  │    │  [Get Dragged Item Data] → [Break Umbra Inventory Slot] → Inventory ID
  │    │    ↓
  │    │  [Branch] (Inventory ID > 0?)
  │    │    ├─ True:
  │    │    │    ↓
  │    │    │  [Get Game Instance] → [Cast to Umbra Game Instance]
  │    │    │    ↓
  │    │    │  [Unequip Item] (Inventory ID)
  │    │    │    ↓
  │    │    │  [Make Literal Bool] (true) → Return
  │    │    │
  │    │    └─ False: [Make Literal Bool] (false) → Return
  │    │
  │    └─ Fail: (continua para lógica normal)
  │
  └─ False: (continua para lógica normal)
```

---

### **3. WBP_EquipmentSlot - Tooltip**

**Event:** `OnMouseEnter`

```
[OnMouseEnter]
  ↓
[Get Equipped Item]
  ↓
[Break Umbra Inventory Slot] → Item Template ID
  ↓
[Branch] (Item Template ID > 0?)
  ├─ True:
  │    ↓
  │  [Create Widget] (WBP_ItemTooltip)
  │    ↓
  │  [Set Tooltip Widget] (self, Return Value)
  │    ↓
  │  [Set Tooltip Item Data] (Return Value, Equipped Item)
  │
  └─ False: (nada)
```

**Event:** `OnMouseLeave`

```
[OnMouseLeave]
  ↓
[Set Tooltip Widget] (self, None)
```

---

### **4. WBP_CharacterInfo - Conectar Delegate**

**Event:** `Event Construct`

```
[Event Construct]
  ↓
[Get Game Instance] → [Cast to Umbra Game Instance]
  ↓
[Bind Event to OnItemUnequipped]
  ├─ Target: Cast (As Umbra Game Instance)
  └─ Event: OnItemUnequipped_Event (Custom Event)
```

**Custom Event:** `OnItemUnequipped_Event`

```
[OnItemUnequipped_Event]
  ├─ Inventory ID: (int32)
  ├─ Equipment Slot: (EUmbraEquipmentSlot)
  ↓
[Get Game Instance] → [Cast to Umbra Game Instance]
  ↓
[Load Character Info]
```

---

## ✅ **CHECKLIST:**

- [ ] `WBP_EquipmentSlot::OnMouseButtonDoubleClick` implementado
- [ ] `WBP_InventorySlot::OnDrop` modificado para aceitar equipment slots
- [ ] `WBP_EquipmentSlot::OnMouseEnter` implementado (tooltip)
- [ ] `WBP_EquipmentSlot::OnMouseLeave` implementado
- [ ] `WBP_CharacterInfo::OnItemUnequipped_Event` criado e conectado

---

## 🔍 **NOTAS IMPORTANTES:**

1. **Não precisa chamar `Move Item` após `Unequip Item`** - O PHP já coloca o item no primeiro slot vazio automaticamente
2. **C++ já chama `LoadInventory()` e `LoadCharacterInfo()`** após `UnequipItem` - Não precisa chamar manualmente
3. **Tooltip usa `WBP_ItemTooltip`** - O mesmo widget usado no inventário

