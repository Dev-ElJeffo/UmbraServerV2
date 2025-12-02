# 🔧 CORREÇÃO: Drag and Drop no WBP_EquipmentSlot

## 🐛 **PROBLEMA:**

No `OnDragDetected` do `WBP_EquipmentSlot`, você está tentando conectar `Equipped Item` (que é uma struct `FUmbraInventorySlot`) ao pin "Payload" do `Create Drag Drop Operation`.

**O PROBLEMA:** O pin "Payload" espera um `UObject`, mas `Equipped Item` é uma **struct**, não um objeto!

---

## ✅ **SOLUÇÃO:**

1. **Deixe o pin "Payload" como None** (ou não conecte nada)
2. **Após criar o Drag Drop Operation**, faça **Cast** para `UmbraItemDragDropOperation`
3. **Use `Set Dragged Item Data`** para definir o `EquippedItem` (struct)

---

## 📋 **ESTRUTURA CORRETA DO ONDRAGDETECTED:**

### **OnDragDetected (WBP_EquipmentSlot):**

```
[OnDragDetected]
  ├─ Geometry: (FGeometry)
  ├─ MouseEvent: (FPointerEvent)
  ↓
[Is Valid] ← Verificar Equipped Item > Item Template ID > 0
  ├─ True: [Create Drag Drop Operation]
  │         └─ Operation Class: Umbra Item Drag Drop Operation
  │         └─ Return Value: (Drag Drop Operation)
  │         ↓
  │         [Cast to Umbra Item Drag Drop Operation]
  │         ├─ Success: [Set Dragged Item Data] ← USAR ESTE!
  │         │             └─ Target: Cast Result
  │         │             └─ Dragged Item Data: Equipped Item
  │         │             ↓
  │         │             [Set Source Slot Widget] ← Opcional
  │         │             └─ Target: Cast Result
  │         │             └─ Source Slot Widget: self (ou None)
  │         │             ↓
  │         │             [Set Source Slot Index] ← Opcional
  │         │             └─ Target: Cast Result
  │         │             └─ Source Slot Index: -1 (ou slot index se tiver)
  │         │             ↓
  │         │             [Return Drag Drop Operation]
  │         │             └─ Drag Drop Operation: Cast Result
  │         │
  │         └─ Fail: [Return None]
  │
  └─ False: [Return None]
```

---

## 🔧 **PASSO A PASSO DETALHADO:**

### **PASSO 1: Criar Drag Drop Operation**

1. Clique com botão direito → `Create Drag Drop Operation`
2. **Operation Class:** `Umbra Item Drag Drop Operation`
3. **Payload:** Deixe como **None** (ou não conecte nada) ← **IMPORTANTE!**
4. Conecte ao pin de saída do `Is Valid` (True)

---

### **PASSO 2: Cast para Umbra Item Drag Drop Operation**

1. Arraste o pin **Return Value** do `Create Drag Drop Operation`
2. Digite: `Cast to Umbra Item Drag Drop Operation`
3. Conecte o **Return Value** ao pin de entrada do Cast

---

### **PASSO 3: Set Dragged Item Data**

**IMPORTANTE:** Este é o passo que você estava tentando fazer!

1. Arraste o pin **As Umbra Item Drag Drop Operation** (saída do Cast)
2. Digite: `Set Dragged Item Data`
3. **Target:** Conecte ao pin "As Umbra Item Drag Drop Operation"
4. **Dragged Item Data:** Conecte ao `Equipped Item` (variável)

**COMO OBTER:**
- Arraste a variável `Equipped Item` → `Get Equipped Item`
- Conecte ao pin **Dragged Item Data**

---

### **PASSO 4: Configurar Outras Propriedades (Opcional)**

Você pode configurar outras propriedades:

1. **Set Source Slot Widget:**
   - Target: Cast Result
   - Source Slot Widget: `self` (ou None, já que é equipment slot)

2. **Set Source Slot Index:**
   - Target: Cast Result
   - Source Slot Index: `-1` (ou índice se tiver)

---

### **PASSO 5: Return Drag Drop Operation**

1. Conecte o pin **As Umbra Item Drag Drop Operation** ao pin de saída **Return Drag Drop Operation**

---

## 📊 **ESTRUTURA VISUAL COMPLETA:**

```
[OnDragDetected]
  ├─ Geometry: (FGeometry)
  ├─ MouseEvent: (FPointerEvent)
  ↓
[Is Valid] ← Equipped Item > Item Template ID > 0
  │
  ├─ TRUE ───────────────────────────────────────────────────────────┐
  │                                                                  │
  │  [Create Drag Drop Operation]                                    │
  │    └─ Operation Class: Umbra Item Drag Drop Operation            │
  │    └─ Return Value: (Drag Drop Operation)                        │
  │         │                                                         │
  │         ▼                                                         │
  │  [Cast to Umbra Item Drag Drop Operation]                        │
  │    └─ Object: Return Value                                       │
  │         │                                                         │
  │         ├─ SUCCESS ────────────────────────────────────────────┐  │
  │         │                                                      │  │
  │         │  [Get Equipped Item] ← Variável                     │  │
  │         │    └─ Equipped Item: (FUmbraInventorySlot)           │  │
  │         │         │                                            │  │
  │         │         ▼                                            │  │
  │         │  [Set Dragged Item Data] ← USAR ESTE!                │  │
  │         │    ├─ Target: As Umbra Item Drag Drop Operation      │  │
  │         │    └─ Dragged Item Data: Equipped Item               │  │
  │         │         │                                            │  │
  │         │         ▼                                            │  │
  │         │  [Return Drag Drop Operation]                        │  │
  │         │    └─ Drag Drop Operation: As Umbra Item Drag...      │  │
  │         │                                                      │  │
  │         └──────────────────────────────────────────────────────┘  │
  │                                                                  │
  └──────────────────────────────────────────────────────────────────┘
  │
  └─ FALSE
       │
       ▼
    [Return None]
```

---

## 🔍 **PROPRIEDADES DO UMBRAITEMDRAGDROPOPERATION:**

O `UmbraItemDragDropOperation` tem estas propriedades:

1. **DraggedItemData** (FUmbraInventorySlot) ← **ESTE É O "PAYLOAD"!**
2. **SourceSlotWidget** (UUmbraInventorySlotWidget*) - Opcional
3. **SourceSlotIndex** (int32) - Opcional
4. **OperationType** (FString) - Opcional

**NO BLUEPRINT, USE:**
- `Set Dragged Item Data` ← Para definir o item
- `Set Source Slot Widget` ← Opcional
- `Set Source Slot Index` ← Opcional

---

## ⚠️ **IMPORTANTE:**

### **SOBRE O PIN "PAYLOAD":**
- ✅ O pin "Payload" **EXISTE** no `Create Drag Drop Operation`
- ❌ Mas ele espera um **`UObject`**, não uma struct
- ❌ `Equipped Item` é uma **struct** (`FUmbraInventorySlot`), não um objeto
- ✅ **SOLUÇÃO:** Deixe "Payload" como **None** e use `Set Dragged Item Data` depois

### **EXISTE:**
- ✅ Propriedade `DraggedItemData` (FUmbraInventorySlot)
- ✅ Função `Set Dragged Item Data` ← **USE ESTA!**
- ✅ Função `Get Dragged Item Data` (para ler depois)

---

## ✅ **RESUMO:**

1. ✅ `Create Drag Drop Operation` → Class: Umbra Item Drag Drop Operation
2. ✅ `Cast to Umbra Item Drag Drop Operation`
3. ✅ `Set Dragged Item Data` → Target: Cast Result, Dragged Item Data: Equipped Item
4. ✅ `Return Drag Drop Operation` → Drag Drop Operation: Cast Result

---

## ✅ **PRONTO!**

Use `Set Dragged Item Data` ao invés de "Payload"!

