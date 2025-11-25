# 🔴 CORREÇÃO: UpdateAllSlotsVisual - Target do ClearSlot

## 🎯 **PROBLEMA IDENTIFICADO:**

O `ClearSlot` (K2Node_CallFunction_10) está conectado ao `else` do `Branch (InventoryID > 0)`, mas o `Target` pode não estar conectado corretamente ao widget.

**Análise do código atual:**
- `ClearSlot` está no `else` do `Branch (InventoryID > 0)` ✅
- `UpdateSlotVisual` está conectado ao `then` do `ClearSlot` ✅
- Mas o `Target` do `ClearSlot` precisa estar conectado ao **mesmo widget** usado no `SetSlotData`

---

## ✅ **CORREÇÃO:**

### **Verificar conexões do `Target`:**

**O `Target` do `ClearSlot` (K2Node_CallFunction_10) deve estar conectado ao mesmo widget usado no `SetSlotData`:**

1. **No `SetSlotData` (K2Node_CallFunction_1):**
   - `Target` está conectado a `K2Node_GetArrayItem_0` (Output) ✅

2. **No `ClearSlot` (K2Node_CallFunction_10):**
   - `Target` deve estar conectado ao **mesmo** `K2Node_GetArrayItem_0` (Output)
   - **OU** usar o `K2Node_Knot_4` que já está conectado ao `GetArrayItem`

**Como verificar:**

1. **Abra o `UpdateAllSlotsVisual` no Blueprint**
2. **Localize o `ClearSlot` (K2Node_CallFunction_10)**
3. **Verifique o `Target` pin:**
   - Deve estar conectado a `K2Node_GetArrayItem_0` (Output)
   - **OU** a `K2Node_Knot_4` (OutputPin)

**Se não estiver conectado:**

1. **Conecte o `Target` do `ClearSlot` ao `Output` do `Get Array Item` (K2Node_GetArrayItem_0)**
2. **OU** conecte ao `OutputPin` do `K2Node_Knot_4` (que já está conectado ao `GetArrayItem`)

---

## 📝 **ESTRUTURA CORRETA:**

```
ForEachLoop (GetAllStorageSlots)
  LoopBody:
    ├─ Break Umbra Inventory Slot
    ├─ Branch (SlotIndex >= 0 AND SlotIndex < 100)
    │   ├─ TRUE:
    │   │   ├─ Get Array Item (SlotWidgets, SlotIndex)  ← Widget obtido aqui
    │   │   ├─ Is Valid? (Widget)
    │   │   │   ├─ TRUE:
    │   │   │   │   ├─ Branch (InventoryID > 0)
    │   │   │   │   │   ├─ TRUE:  ← Slot ocupado
    │   │   │   │   │   │   ├─ Set Slot Data (Target = Widget)
    │   │   │   │   │   │   └─ Update Slot Visual (Target = Widget)
    │   │   │   │   │   │
    │   │   │   │   │   └─ FALSE:  ← Slot vazio
    │   │   │   │   │       ├─ Clear Slot (Target = Widget)  ← MESMO WIDGET!
    │   │   │   │   │       └─ Update Slot Visual (Target = Widget)  ← MESMO WIDGET!
    │   │   │   │   │
    │   │   │   └─ FALSE: (nada)
    │   │   │
    │   └─ FALSE: (nada)
```

---

## ⚠️ **IMPORTANTE:**

- **O `Target` do `ClearSlot` e `UpdateSlotVisual` DEVE ser o mesmo widget do `Get Array Item`**
- **Use `K2Node_Knot_4` para reutilizar a conexão do widget** (já existe no código)
- **O `ClearSlot` só será executado se o widget for válido** (dentro do `Is Valid?` check)

---

## 🔍 **VERIFICAÇÃO ADICIONAL:**

Se o problema persistir, verifique:

1. **O `SlotWidgets` tem 100 elementos?**
   - Verifique no `CreateStorageSlots` se está criando 100 widgets

2. **O `SlotIndex` está correto?**
   - O `SlotIndex` do `BreakStruct` deve ser 0-99 (já convertido pelo `GetAllStorageSlots()`)
   - O `Get Array Item` usa esse `SlotIndex` para acessar `SlotWidgets[SlotIndex]`

3. **O `GetAllStorageSlots()` está retornando slots vazios?**
   - Deve retornar 100 slots, onde slots vazios têm `InventoryID = 0`

