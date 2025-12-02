# ✅ SOLUÇÃO: Payload no Drag Drop do WBP_EquipmentSlot

## 🎯 **PROBLEMA IDENTIFICADO:**

Você está tentando conectar `Equipped Item` (struct `FUmbraInventorySlot`) ao pin **"Payload"** do `Create Drag Drop Operation`.

**O PROBLEMA:**
- O pin "Payload" espera um **`UObject`** (objeto)
- `Equipped Item` é uma **struct** (`FUmbraInventorySlot`), não um objeto
- **Por isso não conecta!**

---

## ✅ **SOLUÇÃO CORRETA:**

### **PASSO 1: Create Drag Drop Operation**

```
[Create Drag Drop Operation]
  ├─ Class: Umbra Item Drag Drop Operation
  ├─ Payload: None ← DEIXE COMO NONE!
  ├─ Default Drag Visual: None (ou self)
  └─ Return Value: (Drag Drop Operation)
```

**IMPORTANTE:** Deixe o pin "Payload" como **None** ou não conecte nada!

---

### **PASSO 2: Cast para Umbra Item Drag Drop Operation**

```
[Cast to Umbra Item Drag Drop Operation]
  ├─ Object: Return Value (do Create Drag Drop Operation)
  └─ Success: As Umbra Item Drag Drop Operation
```

---

### **PASSO 3: Set Dragged Item Data**

```
[Set Dragged Item Data]
  ├─ Target: As Umbra Item Drag Drop Operation (do Cast)
  ├─ Dragged Item Data: Equipped Item ← AQUI VOCÊ CONECTA!
  └─ (exec) → Return Drag Drop Operation
```

**ESTE É O PASSO CORRETO!** Use `Set Dragged Item Data` para definir a struct!

---

### **PASSO 4: Return Drag Drop Operation**

```
[Return Drag Drop Operation]
  └─ Drag Drop Operation: As Umbra Item Drag Drop Operation
```

---

## 📊 **ESTRUTURA COMPLETA:**

```
[OnDragDetected]
  ├─ Geometry: (FGeometry)
  ├─ MouseEvent: (FPointerEvent)
  ↓
[Get Equipped Item] ← Variável
  └─ Equipped Item: (FUmbraInventorySlot)
  ↓
[Break Umbra Inventory Slot]
  └─ Item Template ID: (int)
  ↓
[Greater (Int Int)]
  ├─ A: Item Template ID
  ├─ B: 0
  └─ Return Value: (bool)
  ↓
[Branch]
  ├─ Condition: Return Value (do Greater)
  │
  ├─ TRUE ───────────────────────────────────────────────┐
  │                                                      │
  │  [Create Drag Drop Operation]                        │
  │    ├─ Class: Umbra Item Drag Drop Operation          │
  │    ├─ Payload: None ← DEIXE COMO NONE!              │
  │    └─ Return Value: (Drag Drop Operation)            │
  │         │                                            │
  │         ▼                                            │
  │  [Cast to Umbra Item Drag Drop Operation]            │
  │    ├─ Object: Return Value                           │
  │    └─ Success: As Umbra Item Drag Drop Operation    │
  │         │                                            │
  │         ▼                                            │
  │  [Get Equipped Item] ← Variável novamente            │
  │    └─ Equipped Item: (FUmbraInventorySlot)           │
  │         │                                            │
  │         ▼                                            │
  │  [Set Dragged Item Data] ← AQUI CONECTA!            │
  │    ├─ Target: As Umbra Item Drag Drop Operation      │
  │    └─ Dragged Item Data: Equipped Item              │
  │         │                                            │
  │         ▼                                            │
  │  [Return Drag Drop Operation]                        │
  │    └─ Drag Drop Operation: As Umbra Item Drag...  │
  │                                                      │
  └──────────────────────────────────────────────────────┘
  │
  └─ FALSE
       │
       ▼
    [Return None]
```

---

## 🔍 **POR QUE ISSO FUNCIONA?**

1. **"Payload"** é para objetos (`UObject`), não para structs
2. **`DraggedItemData`** é uma propriedade do `UmbraItemDragDropOperation` que aceita structs
3. **`Set Dragged Item Data`** é a função que define essa propriedade

---

## ✅ **RESUMO:**

1. ✅ `Create Drag Drop Operation` → **Payload: None**
2. ✅ `Cast to Umbra Item Drag Drop Operation`
3. ✅ `Set Dragged Item Data` → **Dragged Item Data: Equipped Item**
4. ✅ `Return Drag Drop Operation`

---

## 🎯 **PRONTO!**

Agora o `Equipped Item` será corretamente passado para o drag drop operation através de `Set Dragged Item Data`!

