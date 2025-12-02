# ✅ CONECTAR PINS: Setup Split Input

## 🎯 **PROBLEMA:**

Os pins `SourceSlot` e `ItemQuantity` do `Setup Split Input` estão desconectados.

---

## ✅ **SOLUÇÃO:**

### **PASSO 1: Conectar SourceSlot**

**ENCONTRE:**
- O nó `Setup Split Input` (`K2Node_CallFunction_4`)
- O pin `SourceSlot` (está desconectado)

**CONECTE:**
- `self` (do `WBP_InventorySlot`) → `SourceSlot` do `Setup Split Input`

**COMO FAZER:**
1. **Right Click no Event Graph**
2. **Digite "self"**
3. **Selecione "self"** (do WBP_InventorySlot)
4. **Arraste do pin `self`**
5. **Conecte ao pin `SourceSlot` do `Setup Split Input`**

---

### **PASSO 2: Conectar ItemQuantity**

**ENCONTRE:**
- O nó `Break Umbra Inventory Slot` (`K2Node_BreakStruct_0`)
- O pin `Quantity` (já está sendo usado no `Greater`)

**CONECTE:**
- `Quantity` (do `Break Umbra Inventory Slot`) → `ItemQuantity` do `Setup Split Input`

**COMO FAZER:**
1. **Encontre o pin `Quantity` do `Break Umbra Inventory Slot`**
2. **Arraste do pin `Quantity`**
3. **Conecte ao pin `ItemQuantity` do `Setup Split Input`**

**IMPORTANTE:** Você pode usar o mesmo `Quantity` que já está conectado ao `Greater`. Basta arrastar uma nova conexão dele.

---

## 📝 **ESTRUTURA CORRETA:**

```
[OnDragDetected]
  ↓
[Branch] (Is Shift Down?)
  ├─ TRUE:
  │   ├─ [Get Slot Data] (self)
  │   │     └─ Return Value: SlotData
  │   ├─ [Break Umbra Inventory Slot]
  │   │     └─ Input: SlotData
  │   │     └─ Quantity ← ✅ USAR ESTE VALOR!
  │   ├─ [Branch] (Quantity > 1)
  │   │   ├─ TRUE:
  │   │   │   ├─ [Create Widget] (WBP_SplitInput)
  │   │   │   │     └─ Return Value: SplitInputWidget
  │   │   │   ├─ [Setup Split Input] ← ✅ CONECTAR AQUI!
  │   │   │   │     └─ Target: SplitInputWidget
  │   │   │   │     └─ Source Slot: self ← ✅ CONECTAR self AQUI!
  │   │   │   │     └─ Item Quantity: Quantity ← ✅ CONECTAR Quantity AQUI!
```

---

## 🎯 **RESUMO:**

1. ✅ **Conectar `self` ao pin `SourceSlot` do `Setup Split Input`**
2. ✅ **Conectar `Quantity` (do Break) ao pin `ItemQuantity` do `Setup Split Input`**

---

## ⚠️ **OBSERVAÇÃO:**

- `self` sempre existe em qualquer widget Blueprint
- `Quantity` já está sendo extraído do `Break Umbra Inventory Slot`
- Você só precisa conectar esses valores aos pins corretos

---

**PRONTO! 🎉**

