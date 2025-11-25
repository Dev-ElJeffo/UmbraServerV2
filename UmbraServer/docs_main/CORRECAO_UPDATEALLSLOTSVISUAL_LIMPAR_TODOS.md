# 🔴 CORREÇÃO CRÍTICA: UpdateAllSlotsVisual - Limpar TODOS os Slots

## 🎯 **PROBLEMA IDENTIFICADO:**

O `UpdateAllSlotsVisual` está iterando sobre todos os 100 slots retornados por `GetAllStorageSlots()`, mas:

1. **Slots vazios não estão sendo limpos:**
   - Quando um item é movido de slot 10 para slot 20, o slot 10 fica vazio
   - O código só atualiza slots com `InventoryID > 0`, então o slot 10 não é limpo
   - O item antigo permanece visualmente no slot 10

2. **O `Get Array Item` pode estar usando índice errado:**
   - O `SlotIndex` do `BreakStruct` é usado para acessar `SlotWidgets[SlotIndex]`
   - Mas precisa garantir que `SlotWidgets` tem 100 elementos e os índices estão alinhados

---

## ✅ **SOLUÇÃO:**

### **OPÇÃO 1: Limpar TODOS os slots primeiro, depois atualizar os ocupados**

**Modificar `UpdateAllSlotsVisual` para:**

1. **Primeiro loop: Limpar TODOS os 100 slots**
2. **Segundo loop: Atualizar apenas os slots ocupados**

**Como fazer:**

1. **Após o `ForEachLoop` atual (que itera sobre `GetAllStorageSlots()`):**
   - Adicione um **novo `For Loop`** que vai de **0 a 99**
   - Para cada índice, acesse `SlotWidgets[Index]`
   - Se o widget for válido, chame `ClearSlot` → `UpdateSlotVisual`

2. **No loop atual (que itera sobre `GetAllStorageSlots()`):**
   - Mantenha a lógica atual (filtro `SlotIndex >= 0 AND SlotIndex < 100`)
   - Mantenha o `Branch (InventoryID > 0)`
   - No `TRUE`: `SetSlotData` → `UpdateSlotVisual`
   - No `FALSE`: **NÃO fazer nada** (já foi limpo no primeiro loop)

---

### **OPÇÃO 2: Garantir que slots vazios sejam limpos no loop atual**

**Modificar o `Branch (InventoryID > 0)`:**

**No `FALSE` (slot vazio):**
- **ADICIONAR:** `ClearSlot` → `UpdateSlotVisual`

**Estrutura:**

```
ForEachLoop (GetAllStorageSlots)
  LoopBody:
    ├─ Break Umbra Inventory Slot
    ├─ Branch (SlotIndex >= 0 AND SlotIndex < 100)
    │   ├─ TRUE:
    │   │   ├─ Get Array Item (SlotWidgets, SlotIndex)
    │   │   ├─ Is Valid? (Widget)
    │   │   │   ├─ TRUE:
    │   │   │   │   ├─ Branch (InventoryID > 0)
    │   │   │   │   │   ├─ TRUE:  ← Slot ocupado
    │   │   │   │   │   │   ├─ Set Slot Data
    │   │   │   │   │   │   └─ Update Slot Visual
    │   │   │   │   │   │
    │   │   │   │   │   └─ FALSE:  ← Slot vazio
    │   │   │   │   │       ├─ Clear Slot  ← ADICIONAR!
    │   │   │   │   │       └─ Update Slot Visual  ← ADICIONAR!
    │   │   │   │   │
    │   │   │   └─ FALSE: (nada)
    │   │   │
    │   └─ FALSE: (nada)
```

---

## 📝 **RECOMENDAÇÃO:**

**Usar OPÇÃO 2** (mais simples e eficiente):

1. **No `FALSE` do `Branch (InventoryID > 0)`:**
   - Conecte ao `Target` do `ClearSlot` (mesmo widget do `Get Array Item`)
   - Conecte o `then` do `ClearSlot` ao `execute` do `UpdateSlotVisual`
   - Conecte o `self` do `UpdateSlotVisual` ao mesmo widget

**Como fazer:**

1. **No `FALSE` do `Branch (InventoryID > 0)` (K2Node_IfThenElse_2):**
   - Adicione `Clear Slot` (K2Node_CallFunction_10) - **JÁ EXISTE!**
   - Conecte o `execute` do `ClearSlot` ao `else` do `Branch (InventoryID > 0)`
   - Conecte o `Target` do `ClearSlot` ao mesmo widget do `Get Array Item` (use `K2Node_Knot_4` ou `K2Node_GetArrayItem_0`)
   - Conecte o `then` do `ClearSlot` ao `execute` do `UpdateSlotVisual` (K2Node_CallFunction_11) - **JÁ EXISTE!**
   - Conecte o `self` do `UpdateSlotVisual` ao mesmo widget

**Verificação:**
- O `ClearSlot` e `UpdateSlotVisual` já existem no código (K2Node_CallFunction_10 e K2Node_CallFunction_11)
- Eles estão conectados ao `else` do `Branch (InventoryID > 0)`, mas precisam estar conectados corretamente ao widget

---

## ⚠️ **IMPORTANTE:**

- **Slots vazios (`InventoryID = 0`) DEVEM chamar `ClearSlot`** para remover o item visual antigo
- **Slots ocupados (`InventoryID > 0`) DEVEM chamar `SetSlotData`** para atualizar o item
- **Ambos devem chamar `UpdateSlotVisual`** no final para atualizar a UI

