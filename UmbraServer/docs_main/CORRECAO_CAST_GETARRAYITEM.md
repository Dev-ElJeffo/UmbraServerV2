# 🔧 CORREÇÃO: Cast do GetArrayItem para WBP_InventorySlot

**PROBLEMA:**
O `GetArrayItem` retorna um tipo genérico `Object`, mas `SetSlotData` e `UpdateSlotVisual` esperam `WBP_InventorySlot` como Target.

---

## ✅ **SOLUÇÃO: Fazer Cast**

Após o `GetArrayItem`, adicione um **Cast to WBP_InventorySlot** antes de usar como Target.

---

## 📝 **CÓDIGO CORRIGIDO:**

```
ForEachLoop (GetStorageData)
  Loop Body:
    ├─ Break Umbra Inventory Slot
    │     └─ Slot Index
    ├─ Get Array Item (SlotWidgets, Index: Slot Index)
    │     └─ Output: Object (genérico)
    ├─ Cast to WBP Inventory Slot  ← ADICIONAR!
    │     ├─ Object: (Output do GetArrayItem)
    │     └─ As WBP Inventory Slot: (resultado do cast)
    ├─ Is Valid? (As WBP Inventory Slot)
    │   └─ TRUE:
    │       ├─ Set Slot Data
    │       │     ├─ Target: As WBP Inventory Slot  ← USAR AQUI!
    │       │     └─ New Slot Data: Array Element
    │       └─ Update Slot Visual
    │             └─ Target: As WBP Inventory Slot  ← USAR AQUI!
```

---

## 🔧 **PASSO A PASSO:**

### **1. Adicionar Cast:**

1. Após o `GetArrayItem_0`, adicione um nó **Cast to WBP Inventory Slot**
2. Conecte o **Output** do `GetArrayItem_0` ao pin **Object** do Cast
3. O Cast terá um pin **As WBP Inventory Slot** (saída do cast bem-sucedido)

### **2. Usar o Cast no IsValid:**

1. **Desconecte** o `GetArrayItem_0` (Output) do `IsValid`
2. Conecte o **As WBP Inventory Slot** (do Cast) ao `IsValid` (Object)

### **3. Usar o Cast no SetSlotData:**

1. Conecte o **As WBP Inventory Slot** (do Cast) ao pin **Target** (ou **self**) do `SetSlotData`

### **4. Usar o Cast no UpdateSlotVisual:**

1. Conecte o **As WBP Inventory Slot** (do Cast) ao pin **Target** (ou **self**) do `UpdateSlotVisual`

---

## 📋 **ESTRUTURA COMPLETA:**

```
GetArrayItem_0 (Output: Object)
  ↓
Cast to WBP Inventory Slot
  ├─ Object: (do GetArrayItem_0)
  └─ As WBP Inventory Slot: (resultado)
      ├─ → IsValid (Object)
      ├─ → SetSlotData (Target)
      └─ → UpdateSlotVisual (Target)
```

---

## ⚠️ **IMPORTANTE:**

- O **Cast** pode falhar se o objeto não for do tipo correto
- Use o **As WBP Inventory Slot** (não o **Object** original)
- O pin **Is Valid** do Cast indica se o cast foi bem-sucedido (opcional, mas recomendado)

---

## 🎯 **ALTERNATIVA (SE O CAST NÃO FUNCIONAR):**

Se o Cast não aparecer ou não funcionar, verifique:

1. **Tipo do Array:** O array `SlotWidgets` deve ser do tipo `Array of WBP_InventorySlot`, não `Array of Object`
2. **Variável:** No `WBP_Storage`, a variável `SlotWidgets` deve ter o tipo correto:
   - **Variable Type:** `WBP_InventorySlot` (ou `UmbraInventorySlotWidget`)
   - **Container Type:** Array

Se o tipo do array estiver correto, o `GetArrayItem` deve retornar o tipo específico automaticamente.

---

**ADICIONE O CAST E CONECTE OS TARGETS!** 🚀

