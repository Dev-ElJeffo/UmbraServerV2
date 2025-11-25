# ✅ SOLUÇÃO DEFINITIVA: Limpar TODOS os Slots Primeiro

## 🎯 **PROBLEMA REAL:**

O `Clear Slot` no `OnItemMoved_Event` está funcionando, mas o `OnInventoryLoaded_Event` está sendo executado DEPOIS e pode estar sobrescrevendo a limpeza ou não está limpando slots vazios corretamente.

**Solução:** Modificar o `OnInventoryLoaded_Event` para **limpar TODOS os 50 slots PRIMEIRO**, e DEPOIS atualizar apenas os que têm itens.

---

## ✅ **SOLUÇÃO:**

### **MODIFIQUE O `OnInventoryLoaded_Event` NO `WBP_Inventory`:**

O `OnInventoryLoaded_Event` deve ter **DOIS LOOPS**:

1. **PRIMEIRO LOOP:** Limpar TODOS os 50 slots (0-49)
2. **SEGUNDO LOOP:** Atualizar apenas os slots que têm itens

---

## 📋 **ESTRUTURA COMPLETA DO `OnInventoryLoaded_Event`:**

```
Custom Event: On Inventory Loaded Event
  ↓
For Loop (First: 0, Last: 49)  ← PRIMEIRO: Limpar TODOS os slots
  Loop Body:
    ├─ Index (do For Loop)
    ├─ Get Slot Widgets (Array)
    ├─ Get Array Item (SlotWidgets, Index: Index)
    ├─ Is Valid? (do Array Item)
    │   └─ TRUE:
    │       ├─ Make Umbra Inventory Slot
    │       │     ├─ Inventory ID: 0
    │       │     ├─ Slot Index: Index (do For Loop)
    │       │     ├─ Player ID: 0
    │       │     ├─ Item Template ID: 0
    │       │     ├─ Quantity: 0
    │       │     ├─ Is Equipped: false
    │       │     ├─ Durability: 0.0
    │       │     └─ Item Template: (struct vazio)
    │       ├─ Set Slot Data
    │       │     └─ Target: Output (Get Array Item)
    │       │     └─ New Slot Data: (struct acima)
    │       └─ Update Slot Visual
    │             └─ Target: Output (Get Array Item)
  ↓
For Loop Completed
  ↓
Get Game Instance (Cast to Umbra Game Instance)
  ↓
Get Current Inventory (do Game Instance)
  ↓
ForEachLoop (Current Inventory)  ← SEGUNDO: Atualizar apenas os que têm itens
  Loop Body:
    ├─ Array Element: ItemSlot
    ├─ Break Umbra Inventory Slot
    │     └─ Slot Index
    ├─ Get Slot Widgets (Array)
    ├─ Get Array Item (SlotWidgets, Index: Slot Index)
    ├─ Is Valid? (do Array Item)
    │   └─ TRUE:
    │       ├─ Set Slot Data
    │       │     └─ Target: Output (Get Array Item)
    │       │     └─ New Slot Data: ItemSlot (do ForEachLoop)
    │       └─ Update Slot Visual
    │             └─ Target: Output (Get Array Item)
```

---

## 🔧 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: Adicionar o Primeiro Loop (Limpar Todos os Slots)**

**No `OnInventoryLoaded_Event`, ANTES de qualquer outra lógica:**

1. **Adicione um `For Loop`:**
   - **First Index:** `0`
   - **Last Index:** `49`

2. **No `Loop Body` do `For Loop`:**

   a. **Adicione `Get Slot Widgets`:**
      - Arraste a variável `SlotWidgets` para o Event Graph

   b. **Adicione `Get Array Item`:**
      - Conecte `SlotWidgets` ao `Array` pin
      - Conecte o `Index` do `For Loop` ao `Dimension 1` pin

   c. **Adicione `Is Valid?`:**
      - Conecte o `Output` de `Get Array Item` ao `Object` pin

   d. **No caminho `TRUE` do `Is Valid?`:**

      - **Adicione `Make Umbra Inventory Slot`:**
        - **Inventory ID:** `0` (Make Literal Int)
        - **Slot Index:** `Index` (do For Loop) ← **IMPORTANTE!**
        - **Player ID:** `0`
        - **Item Template ID:** `0`
        - **Quantity:** `0`
        - **Is Equipped:** `false`
        - **Durability:** `0.0`
        - **Item Template:** (deixe vazio ou use valores padrão)

      - **Adicione `Set Slot Data`:**
        - **Target:** `Output` de `Get Array Item` (use `Cast to WBP Inventory Slot` se necessário)
        - **New Slot Data:** `UmbraInventorySlot` (do `Make Umbra Inventory Slot`)

      - **Adicione `Update Slot Visual`:**
        - **Target:** Mesmo `Output` de `Get Array Item`

3. **Conecte o `then` do `OnInventoryLoaded_Event` ao `execute` do `For Loop`**

---

### **PASSO 2: Manter o Segundo Loop (Atualizar Slots com Itens)**

**O segundo loop (que atualiza slots com itens) já existe e está correto. Mantenha-o como está.**

**Conecte o `Completed` do primeiro `For Loop` ao início do segundo loop (Get Game Instance).**

---

## ⚠️ **IMPORTANTE:**

1. **O primeiro loop limpa TODOS os slots:**
   - Garante que não há dados antigos
   - Todos os slots ficam com `InventoryID = 0`

2. **O segundo loop atualiza apenas slots com itens:**
   - Sobrescreve apenas os slots que têm itens
   - Slots vazios permanecem limpos

3. **A ordem é CRÍTICA:**
   - Primeiro: Limpar tudo
   - Depois: Atualizar apenas os que têm itens

4. **O `OnItemMoved_Event` ainda deve limpar o `FromSlot`:**
   - Isso garante limpeza imediata antes do `OnInventoryLoaded_Event`
   - Mas o `OnInventoryLoaded_Event` também limpa tudo, então é redundante mas seguro

---

## 🧪 **TESTE:**

1. Mova um item do slot 0 para o slot 5
2. O slot 0 deve ser limpo **imediatamente** (via `OnItemMoved_Event`)
3. Quando `OnInventoryLoaded_Event` executar:
   - Primeiro loop limpa TODOS os 50 slots
   - Segundo loop atualiza apenas o slot 5 (com o item)
4. Resultado: Slot 0 vazio (branco), Slot 5 com item, nenhuma cópia visual

---

## 📝 **RESUMO:**

- **Adicione um `For Loop` (0-49) no início do `OnInventoryLoaded_Event`**
- **Este loop limpa TODOS os 50 slots primeiro**
- **Depois, o loop existente atualiza apenas os slots com itens**
- **Isso garante que não há dados antigos em nenhum slot**

