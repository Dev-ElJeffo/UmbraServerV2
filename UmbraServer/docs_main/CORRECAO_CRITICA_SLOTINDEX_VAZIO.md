# 🔴 CORREÇÃO CRÍTICA: SlotIndex Incorreto em Slots Vazios

## 🎯 **PROBLEMA IDENTIFICADO:**

Quando `GetInventorySlotByIndex` retorna `false` (slot vazio), o `SlotIndex` no `Break` está sempre `0` porque o `OutSlot` foi limpo no C++.

**No `OnInventoryLoaded_Event`, quando você cria um slot vazio, está usando o `SlotIndex` do `Break` (que é sempre 0) em vez do `Index` do `For Loop`.**

Isso faz com que:
- Slots vazios sejam criados com `SlotIndex = 0`
- Quando um item é movido, o slot anterior não é limpo corretamente
- O `UpdateSlotVisual` recebe `InventoryID = 0` mas o `SlotIndex` está errado

---

## ✅ **SOLUÇÃO:**

**No `OnInventoryLoaded_Event` do `WBP_Inventory`, no caminho `FALSE` (slot vazio):**

### **ERRO ATUAL:**
```
Break Umbra Inventory Slot
  └─ Slot Index → Make Umbra Inventory Slot (SlotIndex)  ❌ SEMPRE 0!
```

### **CORREÇÃO:**
```
For Loop
  └─ Index → Make Umbra Inventory Slot (SlotIndex)  ✅ ÍNDICE CORRETO!
```

---

## 📋 **IMPLEMENTAÇÃO DETALHADA:**

### **PASSO 1: Localizar o caminho FALSE no `OnInventoryLoaded_Event`**

Encontre o `Branch` que verifica se `Get Inventory Slot By Index` retornou `true` ou `false`.

### **PASSO 2: No caminho FALSE (slot vazio):**

**ANTES (ERRADO):**
```
Break Umbra Inventory Slot
  └─ Slot Index → Make Umbra Inventory Slot (SlotIndex)
```

**DEPOIS (CORRETO):**
```
For Loop
  └─ Index → Make Umbra Inventory Slot (SlotIndex)
```

**OU:**

Se você já tem o `Index` do `For Loop` disponível, use-o diretamente:

```
For Loop
  └─ Index → Make Umbra Inventory Slot (SlotIndex: Index)  ✅
```

---

## 🔧 **ESTRUTURA COMPLETA CORRIGIDA:**

```
For Loop (First: 0, Last: 49)
  Loop Body:
    ├─ Get Game Instance (Cast to Umbra Game Instance)
    ├─ Get Inventory Slot By Index (Slot Index: Index)
    │     └─ Return Value: bHasItem
    ├─ Branch (bHasItem?)
    │   │
    │   ├─ TRUE: (Slot tem item)
    │   │   ├─ Break Umbra Inventory Slot (do OutSlot)
    │   │   ├─ Get Array Item (SlotWidgets, Index: SlotIndex)  ← Usa SlotIndex do Break
    │   │   ├─ Is Valid? (do Array Item)
    │   │   └─ TRUE:
    │   │       ├─ Set Slot Data (do Array Item)
    │   │       └─ Update Slot Visual (do Array Item)
    │   │
    │   └─ FALSE: (Slot está vazio)
    │       ├─ Get Array Item (SlotWidgets, Index: Index)  ← USA INDEX DO FOR LOOP!
    │       ├─ Is Valid? (do Array Item)
    │       └─ TRUE:
    │           ├─ Make Umbra Inventory Slot
    │           │     ├─ Inventory ID: 0
    │           │     ├─ Slot Index: Index  ← ✅ USA INDEX DO FOR LOOP!
    │           │     ├─ Player ID: 0
    │           │     ├─ Item Template ID: 0
    │           │     ├─ Quantity: 0
    │           │     ├─ Is Equipped: false
    │           │     └─ Durability: 0.0
    │           ├─ Set Slot Data (do Array Item)
    │           │     └─ New Slot Data: (struct vazio acima)
    │           └─ Update Slot Visual (do Array Item)
```

---

## ⚠️ **PONTOS CRÍTICOS:**

1. **NÃO use `SlotIndex` do `Break` no caminho FALSE** - Ele sempre será `0` porque o `OutSlot` foi limpo
2. **SEMPRE use `Index` do `For Loop`** - Este é o índice correto do slot (0-49)
3. **Use o mesmo `Index` para `Get Array Item`** - Para obter o widget correto do array `SlotWidgets`

---

## ✅ **RESULTADO ESPERADO:**

Após a correção:
- Slots vazios terão `SlotIndex` correto (0-49)
- Quando um item é movido, o slot anterior será limpo corretamente
- O `UpdateSlotVisual` receberá `InventoryID = 0` e `SlotIndex` correto
- Os slots não ficarão "fantasma" após mover itens

---

## 🧪 **TESTE:**

1. Abra o inventário
2. Mova um item do slot 0 para o slot 10
3. Verifique nos logs que:
   - O slot 0 tem `InventoryID = 0` e `SlotIndex = 0` ✅
   - O slot 10 tem `InventoryID = X` e `SlotIndex = 10` ✅
   - Não há cópias visuais nos slots intermediários ✅

