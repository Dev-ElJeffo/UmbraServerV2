# 🔍 COMO OBTER EQUIPMENT SLOT NO ONDROP

## 🎯 **OBJETIVO:**

No `Equal (Enum Enum)`, você precisa comparar:
- **A:** `Equipment Slot` do **ITEM** (que está sendo arrastado)
- **B:** `Equipment Slot Type` do **WIDGET** (o slot de equipamento onde está sendo solto)

---

## 📋 **ONDE OBTER CADA UM:**

### **1️⃣ EQUIPMENT SLOT DO ITEM (Pin A do Equal)**

**ORIGEM:** Vem do `ItemTemplate` dentro do `FUmbraInventorySlot` que você obteve com `Get Dragged Item Data`.

**FLUXO:**
```
[Get Dragged Item Data]
  └─ Return Value: (FUmbraInventorySlot)
       ↓
[Break Umbra Inventory Slot]
  └─ Item Template: (FUmbraItemTemplate) ← AQUI!
       ↓
[Break Umbra Item Template]
  └─ Equipment Slot: (EUmbraEquipmentSlot) ← ESTE É O PIN A!
```

**PASSO A PASSO:**
1. **Get Dragged Item Data** → Retorna `FUmbraInventorySlot`
2. **Break Umbra Inventory Slot** → Expanda e mostre o pin `Item Template`
3. **Break Umbra Item Template** → Expanda e mostre o pin `Equipment Slot`
4. **Conecte** o pin `Equipment Slot` ao pin **A** do `Equal (Enum Enum)`

---

### **2️⃣ EQUIPMENT SLOT TYPE DO WIDGET (Pin B do Equal)**

**ORIGEM:** É uma **variável** do próprio `WBP_EquipmentSlot` que você criou.

**FLUXO:**
```
[Get Equipment Slot] ← Variável do WBP_EquipmentSlot
  └─ Equipment Slot: (EUmbraEquipmentSlot) ← ESTE É O PIN B!
```

**PASSO A PASSO:**
1. No Event Graph do `WBP_EquipmentSlot`
2. Clique direito → Digite: `Get Equipment Slot` (ou o nome da sua variável)
3. Se você criou a variável como `EquipmentSlotType`, digite: `Get Equipment Slot Type`
4. **Conecte** o pin de saída ao pin **B** do `Equal (Enum Enum)`

**⚠️ IMPORTANTE:** Se a variável não existir, você precisa criá-la primeiro!

---

## 🔧 **COMO CRIAR A VARIÁVEL `EquipmentSlotType` (SE NÃO EXISTIR):**

### **PASSO 1: Criar a Variável**

1. No `WBP_EquipmentSlot`, vá para a aba **Variables**
2. Clique em **+ Variable**
3. Configure:
   - **Nome:** `EquipmentSlotType` (ou `EquipmentSlot`)
   - **Tipo:** `EUmbraEquipmentSlot` (Enum)
   - **Editable:** ✅ (marcado)
   - **Instance Editable:** ✅ (marcado)
4. Pressione **Enter**

### **PASSO 2: Definir o Valor (na Função Set Slot Type)**

Quando você chama `Set Slot Type` no widget, você deve setar essa variável:

```
[Set Slot Type]
  ├─ Equipment Slot: (EUmbraEquipmentSlot)
  ↓
[Set Equipment Slot Type] ← Variável
  └─ Equipment Slot Type: Equipment Slot
```

---

## 📊 **ESTRUTURA VISUAL COMPLETA:**

```
[OnDrop]
  ├─ Geometry: (FGeometry)
  ├─ Operation: (Drag Drop Operation)
  ↓
[Cast to Umbra Item Drag Drop Operation]
  └─ Success: As Umbra Item Drag Drop Operation
       ↓
[Get Dragged Item Data]
  └─ Return Value: (FUmbraInventorySlot)
       ↓
[Break Umbra Inventory Slot]
  ├─ Inventory ID: (int)
  ├─ Item Template ID: (int)
  └─ Item Template: (FUmbraItemTemplate) ← AQUI!
       ↓
[Break Umbra Item Template]
  ├─ Item Name: (FString)
  ├─ Item Type: (EUmbraItemType)
  ├─ Equipment Slot: (EUmbraEquipmentSlot) ← PIN A!
  └─ ... (outros campos)
       │
       ▼
[Equal (Enum Enum)]
  ├─ A: Equipment Slot (do Item Template) ← DO ITEM!
  ├─ B: Equipment Slot Type (variável do widget) ← DO WIDGET!
  └─ Return Value: (bool)
       │
       ├─ TRUE → [Equip Item]
       └─ FALSE → [Unhandled]
```

---

## 🔍 **ONDE OBTER CADA UM (RESUMO):**

### **PIN A (Equipment Slot do Item):**
```
Get Dragged Item Data
  → Break Umbra Inventory Slot → Item Template
    → Break Umbra Item Template → Equipment Slot
```

### **PIN B (Equipment Slot Type do Widget):**
```
Get Equipment Slot Type (variável do WBP_EquipmentSlot)
```

---

## ✅ **EXEMPLO PRÁTICO:**

### **CENÁRIO:**
- Você tem um item de `Head` sendo arrastado
- Você está soltando em um slot de `Head` no widget

### **FLUXO:**
1. **Get Dragged Item Data** → Retorna o item
2. **Break Umbra Inventory Slot** → `Item Template`
3. **Break Umbra Item Template** → `Equipment Slot = Head` ← **PIN A**
4. **Get Equipment Slot Type** → `Equipment Slot Type = Head` ← **PIN B**
5. **Equal** → `Head == Head` → **TRUE** ✅
6. **Equip Item** → Item é equipado!

---

## ⚠️ **PROBLEMAS COMUNS:**

### **PROBLEMA 1: "Equipment Slot Type não encontrado"**

**SOLUÇÃO:**
- Verifique se a variável `EquipmentSlotType` existe no `WBP_EquipmentSlot`
- Se não existir, crie-a (veja acima)
- Certifique-se de que ela está sendo setada na função `Set Slot Type`

### **PROBLEMA 2: "Equipment Slot do Item Template retorna None"**

**SOLUÇÃO:**
- Verifique se o item realmente tem um `EquipmentSlot` definido
- Verifique se o `ItemTemplate` está preenchido corretamente
- Adicione um `Is Valid` antes do `Break Umbra Item Template`

### **PROBLEMA 3: "Equal não conecta"**

**SOLUÇÃO:**
- Certifique-se de que ambos os pins são do tipo `EUmbraEquipmentSlot`
- Se o `Equal` não aceitar Enum, use `Equal (Byte Byte)` e converta os Enums para Byte

---

## 🎯 **RESUMO:**

1. ✅ **Equipment Slot do Item (PIN A):**
   - `Get Dragged Item Data` → `Break Umbra Inventory Slot` → `Item Template` → `Break Umbra Item Template` → `Equipment Slot`

2. ✅ **Equipment Slot Type do Widget (PIN B):**
   - `Get Equipment Slot Type` (variável do `WBP_EquipmentSlot`)

3. ✅ **Comparar:**
   - `Equal (Enum Enum)` → `A: Equipment Slot (item)` → `B: Equipment Slot Type (widget)`

---

## ✅ **PRONTO!**

Agora você sabe exatamente onde obter cada um dos valores para o `Equal`!

