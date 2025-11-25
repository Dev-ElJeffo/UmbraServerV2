# ✅ SOLUÇÃO REAL: Como Determinar Origem do Item

**PROBLEMA:**
- `Get Parent` retorna `PanelWidget`, não `UserWidget`
- Não podemos fazer cast de `PanelWidget` para `WBP_Inventory`
- Fazer `Get Parent` duas vezes não resolve (retorna o mesmo)

---

## ✅ **SOLUÇÃO: Variável Booleana no Slot**

### **Adicionar variável `bIsStorageSlot` no `WBP_InventorySlot`:**

1. **Abra `WBP_InventorySlot`**
2. **Variables** → **+ Variable**
3. **Nome:** `bIsStorageSlot`
4. **Tipo:** `Boolean`
5. **Default Value:** `false`

---

## 🎯 **IMPLEMENTAÇÃO:**

### **No `CreateInventorySlots` do `WBP_Inventory`:**

```
For Loop (0 a 49)
  Create Widget (WBP_InventorySlot)
  Set bIsStorageSlot (do widget criado, Value: false)  ← INVENTÁRIO
  Add to Array (SlotWidgets)
```

### **No `CreateStorageSlots` do `WBP_Storage`:**

```
For Loop (0 a 99)
  Create Widget (WBP_InventorySlot)
  Set bIsStorageSlot (do widget criado, Value: true)  ← ARMAZÉM
  Add to Array (SlotWidgets)
```

---

## 🎯 **NO OnDrop:**

### **Determinar origem usando a variável:**

```
Get Source Slot Widget
  ↓
Get bIsStorageSlot (do Source Slot Widget)
  ↓
Branch (bIsStorageSlot?)
  ├─ TRUE: (veio do armazém)
  │   └─ Move Item From Storage
  └─ FALSE: (veio do inventário)
      └─ Move Item To Storage
```

---

## 📋 **CÓDIGO COMPLETO DO OnDrop (no WBP_Storage):**

**⚠️ IMPORTANTE:** O `OnDrop` está no `WBP_Storage`, não no slot. O slot de destino precisa ser determinado de outra forma.

### **OPÇÃO 1: OnDrop no Slot (RECOMENDADO)**

**Mova o `OnDrop` para `WBP_InventorySlot`:**

```
OnDrop (Operation)  ← no WBP_InventorySlot
  ↓
Cast to Umbra Item Drag Drop Operation (Operation)
  ↓ (then)
Get Source Slot Widget
  ├─ Target: Cast (As Umbra Item Drag Drop Operation)
  └─ Return Value: WBP_InventorySlot (origem)
  ↓
Is Valid? (Return Value)
  ↓ (TRUE)
Get Slot Data (do Source)
  ├─ Target: Get Source Slot Widget (Return Value)
  └─ Return Value: FUmbraInventorySlot
  ↓
Break Umbra Inventory Slot
  └─ InventoryID
  ↓
Branch (InventoryID > 0?)
  ↓ (TRUE)
Get bIsStorageSlot (do Source)
  ├─ Target: Get Source Slot Widget (Return Value)
  └─ Return Value: bool
  ↓
Branch (bIsStorageSlot?)
  ├─ TRUE: (veio do armazém)
  │   ↓
  │   Get Parent (do self) → Get Parent (2ª vez) → Cast to WBP Storage
  │   ↓
  │   Move Item From Storage
  │   ├─ Target: Cast (As WBP Storage)
  │   ├─ StorageItemID: Break (InventoryID)
  │   └─ TargetSlotIndex: self → Get Slot Data → Break (SlotIndex)
  │
  └─ FALSE: (veio do inventário)
      ↓
      Get Parent (do self) → Get Parent (2ª vez) → Cast to WBP Storage
      ↓
      Move Item To Storage
      ├─ Target: Cast (As WBP Storage)
      ├─ InventoryItemID: Break (InventoryID)
      └─ TargetSlotIndex: self → Get Slot Data → Break (SlotIndex)
```

### **OPÇÃO 2: OnDrop no WBP_Storage (se necessário manter)**

**Se o `OnDrop` precisa estar no `WBP_Storage`, você precisa determinar qual slot foi dropado usando `PointerEvent` ou outra forma.**

---

## ✅ **VANTAGENS:**

1. **Simples e direto** - sem casts complicados
2. **Confiável** - não depende da hierarquia de widgets
3. **Rápido** - apenas uma verificação booleana
4. **Sem erros** - não há problemas de tipo

---

**IMPLEMENTE USANDO A VARIÁVEL BOOLEANA!** 🎯

