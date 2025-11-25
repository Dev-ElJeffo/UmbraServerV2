# 🔴 CORREÇÃO: WBP_Inventory - Tentando Acessar Slot 137

## 🎯 **PROBLEMA IDENTIFICADO:**

O `WBP_Inventory` está tentando acessar `SlotWidgets[137]`, mas o array só tem 50 elementos (slots 0-49).

**Erro:**
```
Attempted to access index 137 from array 'SlotWidgets' of length 50 in '/Game/Widgets/UI/Inventory/WBP_Inventory.WBP_Inventory_C'
```

**Causa:**
- O `OnItemMoved` do `WBP_Inventory` está sendo disparado quando um item é movido no storage
- O `FromSlot: 137` e `ToSlot: 149` são slots do storage (50-149)
- O `UpdateAllSlotsVisual` ou `RefreshInventory` do inventário está tentando processar esses slots

---

## ✅ **SOLUÇÃO:**

### **1. Filtrar movimentos do storage no `OnItemMoved` do inventário:**

**No `WBP_Inventory` → `Event Graph` → `OnItemMoved_Event`:**

Adicione um filtro para ignorar movimentos que envolvem slots do storage:

```
OnItemMoved (FromSlot, ToSlot)
  ├─ Branch (FromSlot < 50 AND ToSlot < 50)  ← ADICIONAR ESTE FILTRO!
  │   ├─ TRUE:  ← Movimento dentro do inventário (0-49)
  │   │   └─ Refresh Inventory  ← Processar normalmente
  │   │
  │   └─ FALSE:  ← Movimento envolve storage (50-149)
  │       └─ (nada - ignorar)
```

**Como fazer:**

1. **No `OnItemMoved_Event` do `WBP_Inventory`:**
   - Adicione `Branch` node antes de `Refresh Inventory`
   - Adicione `AND` node
   - Adicione `Less (Int Int)` node (2x)
   - Conecte:
     - `FromSlot` → `A` do primeiro `Less`
     - `50` → `B` do primeiro `Less`
     - `ToSlot` → `A` do segundo `Less`
     - `50` → `B` do segundo `Less`
     - Saídas dos dois `Less` → `A` e `B` do `AND`
     - Saída do `AND` → `Condition` do `Branch`

2. **No caminho `TRUE` do `Branch`:**
   - Mantenha `Refresh Inventory` (movimento dentro do inventário)

3. **No caminho `FALSE` do `Branch`:**
   - Não faça nada (ignora movimentos do storage)

---

### **2. Verificar `UpdateAllSlotsVisual` do inventário:**

**No `WBP_Inventory` → `Functions` → `UpdateAllSlotsVisual`:**

Certifique-se de que está filtrando apenas slots 0-49:

```
ForEachLoop (GetAllInventorySlots)
  LoopBody:
    ├─ Break Umbra Inventory Slot
    ├─ Get Slot Index
    ├─ Branch (Slot Index >= 0 AND Slot Index < 50)  ← FILTRAR APENAS INVENTÁRIO!
    │   ├─ TRUE:  ← Slot válido do inventário (0-49)
    │   │   └─ Processar slot normalmente
    │   │
    │   └─ FALSE:  ← Slot do storage (ignorar)
    │       └─ (nada)
```

---

## 📝 **RESUMO:**

1. ✅ **Correção do PHP aplicada** - `get_storage.php` agora retorna `slot_index` correto
2. ⚠️ **Adicionar filtro no `OnItemMoved` do inventário** - Ignorar movimentos do storage
3. ⚠️ **Verificar `UpdateAllSlotsVisual` do inventário** - Filtrar apenas slots 0-49

---

## 🔍 **VERIFICAÇÃO:**

Após aplicar as correções:

1. **Mova um item no storage**
2. **Verifique os logs:**
   - Não deve aparecer erro de `SlotWidgets[137]`
   - O `OnItemMoved` do inventário não deve processar o movimento
3. **Verifique a UI:**
   - O item deve aparecer no slot correto no storage
   - O inventário não deve ser afetado

