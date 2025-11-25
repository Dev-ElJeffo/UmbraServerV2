# 🔴 CORREÇÃO: OnItemMoved_Event - Usar Clear Slot em vez de Get Inventory Slot By Index

## 🎯 **PROBLEMA IDENTIFICADO:**

No `OnItemMoved_Event`, você está tentando atualizar o slot de origem (`FromSlot`) usando:
1. `Get Inventory Slot By Index` (FromSlot)
2. `Set Slot Data` com os dados retornados
3. `Update Slot Visual`

**PROBLEMA:** O `GameInstance` já limpou o inventário (`CurrentInventory.Empty()`) antes de `LoadInventory()` ser chamado, então `Get Inventory Slot By Index` retorna `false` e o slot não é limpo.

---

## ✅ **SOLUÇÃO:**

**Use `Clear Slot` diretamente no slot de origem, sem buscar dados do `GameInstance`.**

---

## 📋 **CORREÇÃO PASSO A PASSO:**

### **PASSO 1: Remover a lógica atual do `FromSlot`**

**No `OnItemMoved_Event`, remova ou desconecte:**
- `Get Inventory Slot By Index` (que usa `FromSlot`)
- `Set Slot Data` (que usa `OutSlot` do `Get Inventory Slot By Index`)
- `Update Slot Visual` (que vem após o `Set Slot Data` do `FromSlot`)

### **PASSO 2: Adicionar `Clear Slot` imediatamente após obter o widget**

**Após `Get Array Item` (que obtém o slot widget do `FromSlot`):**

```
Get Array Item (SlotWidgets, FromSlot)
  ↓
Cast to WBP Inventory Slot
  └─ Object: Output (Get Array Item)
  ↓
Is Valid? (As WBP Inventory Slot)
  ├─ TRUE:
  │   └─ Clear Slot
  │       └─ Target: As WBP Inventory Slot
  │   └─ Update Slot Visual
  │       └─ Target: As WBP Inventory Slot
  │
  └─ FALSE:
      (nada)
```

**Como fazer:**

1. **Adicione `Cast to WBP Inventory Slot`:**
   - Após `Get Array Item_2` (que obtém o slot do `FromSlot`)
   - Conecte o `Output` de `Get Array Item_2` ao `Object` pin do `Cast`

2. **Adicione `Is Valid?`:**
   - Após o `Cast`
   - Conecte o `As WBP Inventory Slot` ao `Object` pin do `Is Valid?`

3. **Adicione `Clear Slot`:**
   - No caminho `TRUE` do `Is Valid?`
   - Procure por **"Clear Slot"** (função do `UmbraInventorySlotWidget`)
   - Conecte o `As WBP Inventory Slot` ao `Target` pin

4. **Adicione `Update Slot Visual`:**
   - Após `Clear Slot`
   - Conecte o mesmo `As WBP Inventory Slot` ao `Target` pin

---

## 🔧 **ESTRUTURA CORRIGIDA DO `OnItemMoved_Event`:**

```
Custom Event: On Item Moved Event
  ├─ Input: InventoryID (Integer)
  ├─ Input: FromSlot (Integer)
  └─ Input: ToSlot (Integer)
  ↓
Get Slot Widgets (Array)
  ↓
Get Array Item_2 (SlotWidgets, FromSlot)  ← SLOT DE ORIGEM
  ↓
Cast to WBP Inventory Slot
  └─ Object: Output (Get Array Item_2)
  ↓
Is Valid? (As WBP Inventory Slot)
  ├─ TRUE:
  │   └─ Clear Slot  ← LIMPAR IMEDIATAMENTE!
  │       └─ Target: As WBP Inventory Slot
  │   └─ Update Slot Visual  ← ATUALIZAR VISUAL!
  │       └─ Target: As WBP Inventory Slot
  │
  └─ FALSE:
      (nada)
  ↓
Load Inventory (GameInstance)  ← JÁ EXISTE, MANTER
  ↓
(Atualização do ToSlot pode ser removida, pois será feita pelo OnInventoryLoaded_Event)
```

---

## ⚠️ **IMPORTANTE:**

1. **NÃO use `Get Inventory Slot By Index` para o `FromSlot`:**
   - O inventário já foi limpo no C++ antes de `LoadInventory()` ser chamado
   - `Get Inventory Slot By Index` sempre retornará `false` para o `FromSlot`

2. **Use `Clear Slot` diretamente:**
   - `Clear Slot` limpa o `SlotData` do widget sem precisar buscar dados do `GameInstance`
   - Define `InventoryID = 0` e preserva o `SlotIndex`

3. **A atualização do `ToSlot` será feita pelo `OnInventoryLoaded_Event`:**
   - Quando `LoadInventory()` completar, o `OnInventoryLoaded_Event` será executado
   - Ele atualizará TODOS os slots, incluindo o `ToSlot`

4. **Remova a lógica duplicada do `ToSlot`:**
   - A atualização do `ToSlot` no `OnItemMoved_Event` é desnecessária
   - O `OnInventoryLoaded_Event` já faz isso corretamente

---

## 🧪 **TESTE:**

1. Mova um item do slot 0 para o slot 5
2. O slot 0 deve ser limpo **imediatamente** (antes da API responder)
3. O slot 5 deve mostrar o item após a API responder (via `OnInventoryLoaded_Event`)
4. Não deve haver cópias visuais nos slots intermediários

---

## 📝 **RESUMO DAS MUDANÇAS:**

### **REMOVER:**
- `Get Inventory Slot By Index` (que usa `FromSlot`)
- `Set Slot Data` (que usa `OutSlot` do `Get Inventory Slot By Index` para `FromSlot`)
- `Update Slot Visual` (que vem após o `Set Slot Data` do `FromSlot`)
- Toda a lógica de atualização do `ToSlot` (será feita pelo `OnInventoryLoaded_Event`)

### **ADICIONAR:**
- `Cast to WBP Inventory Slot` (após `Get Array Item_2`)
- `Is Valid?` (após o `Cast`)
- `Clear Slot` (no caminho `TRUE` do `Is Valid?`)
- `Update Slot Visual` (após `Clear Slot`)

### **MANTER:**
- `Load Inventory` (já existe e está correto)

