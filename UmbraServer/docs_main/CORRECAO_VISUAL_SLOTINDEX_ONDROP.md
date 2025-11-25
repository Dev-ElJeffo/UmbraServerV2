# 🎯 CORREÇÃO VISUAL: SlotIndex no OnInventoryLoaded_Event

## 🔴 **PROBLEMA:**

Nos logs, quando `GetInventorySlotByIndex` retorna `false` (slot vazio), o `SlotIndex` no `Break` está sempre `0`:

```
LogBlueprintUserMessages: [WBP_Inventory_C_0] OnInventory Loaded - BREAK UMBRA INV SLOT - SLOT INDEX  = 0
```

Isso acontece porque o C++ limpa o `OutSlot` quando não encontra um item, então o `SlotIndex` fica `0`.

**No Blueprint, você está usando esse `SlotIndex = 0` para criar slots vazios, o que está ERRADO!**

---

## ✅ **SOLUÇÃO:**

**Use o `Index` do `For Loop` em vez do `SlotIndex` do `Break` quando criar slots vazios.**

---

## 📋 **CORREÇÃO PASSO A PASSO:**

### **1. Localize o `For Loop` no `OnInventoryLoaded_Event`**

O loop deve ser algo como:
```
For Loop (First: 0, Last: 49)
  └─ Index (saída do loop)
```

### **2. Localize o `Branch` que verifica `Get Inventory Slot By Index`**

```
Get Inventory Slot By Index
  └─ Return Value → Branch
      ├─ TRUE: (tem item)
      └─ FALSE: (slot vazio)
```

### **3. No caminho FALSE (slot vazio):**

**ENCONTRE o nó `Make Umbra Inventory Slot`**

**VERIFIQUE qual pin está conectado ao `SlotIndex`:**

- ❌ **ERRADO:** `Break Umbra Inventory Slot` → `Slot Index` → `Make Umbra Inventory Slot (SlotIndex)`
- ✅ **CORRETO:** `For Loop` → `Index` → `Make Umbra Inventory Slot (SlotIndex)`

### **4. CORREÇÃO:**

1. **Desconecte** a conexão entre `Break Umbra Inventory Slot (Slot Index)` e `Make Umbra Inventory Slot (SlotIndex)`
2. **Conecte** o `Index` do `For Loop` diretamente ao `SlotIndex` do `Make Umbra Inventory Slot`

---

## 🔍 **VERIFICAÇÃO:**

Após a correção, quando você executar o jogo e abrir o inventário, os logs devem mostrar:

```
LogBlueprintUserMessages: [WBP_Inventory_C_0] OnInventory Loaded - FOR LOOP INDEX - INDEX = 6
LogBlueprintUserMessages: [WBP_Inventory_C_0] OnInventory Loaded - GET INV SLOT BY INDEX -RETURN VALUE= falso
LogBlueprintUserMessages: [WBP_Inventory_C_0] OnInventory Loaded - GET INV SLOT BY INDEX INVENTORY ID = 0
LogBlueprintUserMessages: [WBP_Inventory_C_0] OnInventory Loaded - BREAK UMBRA INV SLOT - SLOT INDEX  = 0  ← Isso é normal (do Break)
```

Mas o `Make Umbra Inventory Slot` deve ter `SlotIndex = 6` (do Index do For Loop), não `0`!

---

## 📊 **ESTRUTURA CORRIGIDA (Caminho FALSE):**

```
For Loop (First: 0, Last: 49)
  └─ Index  ← ✅ USE ESTE!
      │
      └─ (quando GetInventorySlotByIndex retorna false)
          │
          ├─ Get Array Item (SlotWidgets, Index: Index)  ← ✅ USA INDEX
          ├─ Is Valid? (do Array Item)
          └─ TRUE:
              ├─ Make Umbra Inventory Slot
              │     ├─ Inventory ID: 0
              │     ├─ Slot Index: Index  ← ✅ CONECTE O INDEX DO FOR LOOP AQUI!
              │     ├─ Player ID: 0
              │     ├─ Item Template ID: 0
              │     ├─ Quantity: 0
              │     ├─ Is Equipped: false
              │     └─ Durability: 0.0
              ├─ Set Slot Data (do Array Item)
              └─ Update Slot Visual (do Array Item)
```

---

## ⚠️ **IMPORTANTE:**

- O `Break Umbra Inventory Slot (Slot Index)` sempre será `0` quando o slot está vazio (isso é normal)
- **NÃO use esse valor!** Use sempre o `Index` do `For Loop`
- O `Index` do `For Loop` é o índice correto do slot (0-49)

---

## ✅ **RESULTADO:**

Após essa correção:
- Slots vazios terão `SlotIndex` correto
- Quando um item é movido, o slot anterior será limpo corretamente
- Não haverá mais "slots fantasma" após mover itens
- O `UpdateSlotVisual` receberá `InventoryID = 0` mas `SlotIndex` correto

