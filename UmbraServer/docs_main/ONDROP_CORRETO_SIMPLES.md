# ✅ OnDrop - Guia Direto e Correto

## 🔴 **PROBLEMA IDENTIFICADO:**

O cast para `WBP_Inventory` do parent do Source Slot Widget **SEMPRE vai falhar se o item veio do armazém** - isso é CORRETO e é assim que determinamos a origem!

---

## ✅ **SOLUÇÃO CORRETA:**

### **Fluxo Completo:**

```
OnDrop (Operation)
  ↓
Cast to Umbra Item Drag Drop Operation (Operation)
  ↓ (then)
Get Source Slot Widget
  ├─ Target: Cast (As Umbra Item Drag Drop Operation)
  └─ Return Value: WBP_InventorySlot (slot de origem)
  ↓
Is Valid? (Return Value)
  ↓ (TRUE)
Get Slot Data
  ├─ Target: Get Source Slot Widget (Return Value)
  └─ Return Value: FUmbraInventorySlot
  ↓
Break Umbra Inventory Slot
  └─ InventoryID
  ↓
Branch (InventoryID > 0?)
  ↓ (TRUE)
Get Parent (1ª vez)
  ├─ Target: Get Source Slot Widget (Return Value)
  └─ Return Value: PanelWidget (o grid)
  ↓
Get Parent (2ª vez)
  ├─ Target: Get Parent (Return Value) ← do primeiro
  └─ Return Value: UserWidget (WBP_Inventory OU WBP_Storage)
  ↓
Cast to WBP Inventory
  ├─ Object: Get Parent (Return Value) ← do segundo
  ├─ then: (SUCESSO = veio do inventário)
  └─ CastFailed: (FALHA = veio do armazém)
```

---

## 🎯 **CAMINHO TRUE: Item veio do Inventário**

```
Cast to WBP Inventory (then)
  ↓
Get Parent (1ª vez)
  ├─ Target: self
  └─ Return Value: PanelWidget (grid)
  ↓
Get Parent (2ª vez)
  ├─ Target: Get Parent (Return Value) ← do primeiro
  └─ Return Value: UserWidget (WBP_Storage)
  ↓
Cast to WBP Storage
  ├─ Object: Get Parent (Return Value) ← do segundo
  └─ As WBP Storage
  ↓
Move Item To Storage
  ├─ Target: Cast (As WBP Storage)
  ├─ InventoryItemID: Break (InventoryID)
  └─ TargetSlotIndex: self → Get Slot Data → Break (SlotIndex)
```

---

## 🎯 **CAMINHO FALSE: Item veio do Armazém**

```
Cast to WBP Inventory (CastFailed)
  ↓
Get Parent (1ª vez - do self)
  ├─ Target: self
  └─ Return Value: PanelWidget (grid)
  ↓
Get Parent (2ª vez - do self)
  ├─ Target: Get Parent (Return Value) ← do primeiro do self
  └─ Return Value: UserWidget (WBP_Inventory)
  ↓
Cast to WBP Inventory
  ├─ Object: Get Parent (Return Value) ← do segundo do self
  └─ As WBP Inventory
  ↓
Get Parent (1ª vez - do Source)
  ├─ Target: Get Source Slot Widget (Return Value)
  └─ Return Value: PanelWidget (grid)
  ↓
Get Parent (2ª vez - do Source)
  ├─ Target: Get Parent (Return Value) ← do primeiro do Source
  └─ Return Value: UserWidget (WBP_Storage)
  ↓
Cast to WBP Storage
  ├─ Object: Get Parent (Return Value) ← do segundo do Source
  └─ As WBP Storage
  ↓
Move Item From Storage
  ├─ Target: Cast (As WBP Storage)
  ├─ StorageItemID: Break (InventoryID)
  └─ TargetSlotIndex: self → Get Slot Data → Break (SlotIndex)
```

---

## 📋 **CONEXÕES ESSENCIAIS:**

### **1. Get Source Slot Widget:**
- **Target:** `Cast (As Umbra Item Drag Drop Operation)`
- **Como obter:** Arraste `Cast (As Umbra Item Drag Drop Operation)` → Menu → `Get Source Slot Widget`

### **2. Get Slot Data (do Source):**
- **Target:** `Get Source Slot Widget (Return Value)`
- **Como obter:** Arraste `Get Source Slot Widget (Return Value)` → Menu → `Get Slot Data`

### **3. Get Parent (1ª vez - do Source Slot Widget):**
- **Target:** `Get Source Slot Widget (Return Value)`
- **Return Value:** `PanelWidget` (o grid)
- **⚠️ NÃO é o UserWidget ainda!**

### **4. Get Parent (2ª vez - do PanelWidget):**
- **Target:** `Get Parent (Return Value)` ← do primeiro Get Parent
- **Return Value:** `UserWidget` (WBP_Inventory ou WBP_Storage)
- **✅ Agora sim é o UserWidget!**

### **5. Cast to WBP Inventory (do UserWidget):**
- **Object:** `Get Parent (Return Value)` ← do segundo Get Parent
- **then:** Se sucesso = veio do inventário
- **CastFailed:** Se falha = veio do armazém

### **6. Get Parent (1ª vez - do self):**
- **Target:** `self`
- **Return Value:** `PanelWidget` (o grid)
- **⚠️ NÃO é o UserWidget ainda!**

### **7. Get Parent (2ª vez - do PanelWidget do self):**
- **Target:** `Get Parent (Return Value)` ← do primeiro Get Parent do self
- **Return Value:** `UserWidget` (WBP_Storage ou WBP_Inventory)
- **✅ Agora sim é o UserWidget!**

### **6. Move Item To Storage:**
- **Target:** `Cast to WBP Storage (As WBP Storage)` ← do parent do self
- **Como obter:** Arraste `Cast (As WBP Storage)` → Menu → `Move Item To Storage`

### **7. Move Item From Storage:**
- **Target:** `Cast to WBP Storage (As WBP Storage)` ← do parent do Source Slot Widget
- **Como obter:** Arraste `Cast (As WBP Storage)` → Menu → `Move Item From Storage`

---

## ⚠️ **PONTOS CRÍTICOS:**

1. **Get Parent do Source Slot Widget** → Retorna o parent do slot de ORIGEM
2. **Get Parent do self** → Retorna o parent do slot de DESTINO
3. **Cast to WBP Inventory do parent do Source** → Se falhar, o item veio do armazém (isso é CORRETO!)

---

## 🎯 **RESUMO:**

- **Source Slot Widget** → Parent = origem (inventário ou armazém)
- **self** → Parent = destino (inventário ou armazém)
- **Cast falha?** = Item veio do armazém (use CastFailed)
- **Cast sucesso?** = Item veio do inventário (use then)

---

**FIM. Sem enrolação.**

