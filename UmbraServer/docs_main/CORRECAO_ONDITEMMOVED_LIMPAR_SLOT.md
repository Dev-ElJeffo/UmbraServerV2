# 🔧 CORREÇÃO: OnItemMoved_Event - Limpar Slot Anterior

## 🎯 **PROBLEMA:**

Todos os itens aparecem corretamente, mas quando um item é movido, o slot anterior não é removido visualmente.

**Causa:** O `OnItemMoved_Event` não está limpando o slot `FromSlot` imediatamente.

---

## ✅ **SOLUÇÃO:**

No `OnItemMoved_Event`, limpe o slot `FromSlot` **IMEDIATAMENTE** usando `Clear Slot` e `Update Slot Visual`.

---

## 📋 **IMPLEMENTAÇÃO:**

### **ESTRUTURA DO `OnItemMoved_Event`:**

```
Custom Event: On Item Moved Event
  ├─ Input: InventoryID (Integer)
  ├─ Input: FromSlot (Integer)
  └─ Input: ToSlot (Integer)
  ↓
Get Slot Widgets (Array)
  ↓
Get Array Item (SlotWidgets, Index: FromSlot)  ← SLOT QUE FICOU VAZIO
  ↓
Cast to WBP Inventory Slot
  └─ Object: Output (Get Array Item)
  ↓
Is Valid? (As WBP Inventory Slot)
  ├─ TRUE:
  │   ├─ Clear Slot  ← LIMPAR IMEDIATAMENTE!
  │   │     └─ Target: As WBP Inventory Slot
  │   └─ Update Slot Visual  ← ATUALIZAR VISUAL!
  │         └─ Target: As WBP Inventory Slot
  │
  └─ FALSE:
      (nada)
  ↓
Load Inventory (GameInstance)  ← JÁ EXISTE, MANTER
```

---

## 🛠️ **PASSO A PASSO:**

### **PASSO 1: Verificar se `OnItemMoved_Event` existe**

1. **No `WBP_Inventory` - Event Graph:**
   - Procure por `OnItemMoved_Event` (Custom Event)
   - Se não existir, crie:
     - Clique com botão direito → **Add Custom Event**
     - Nome: `OnItemMoved_Event`
     - Adicione 3 parâmetros:
       - `InventoryID` (Integer)
       - `FromSlot` (Integer)
       - `ToSlot` (Integer)

### **PASSO 2: Verificar se está conectado ao Delegate**

**No `Event Construct` do `WBP_Inventory`:**

1. **Verifique se existe:**
   ```
   Event Construct
     ↓
   Get Game Instance (Cast to Umbra Game Instance)
     ↓
   Assign On Item Moved
       └─ Event: On Item Moved Event
   ```

2. **Se não existir, adicione:**
   - Arraste `MyGameInstance` (ou `Get Game Instance`)
   - Procure por **"Assign On Item Moved"**
   - Conecte o `Event` pin ao `OnItemMoved_Event` (Custom Event)

### **PASSO 3: Implementar a limpeza do slot anterior**

**No `OnItemMoved_Event`:**

1. **Adicione `Get Slot Widgets`:**
   - Arraste a variável `SlotWidgets` para o Event Graph
   - Conecte ao `Array` pin de `Get Array Item`

2. **Adicione `Get Array Item`:**
   - Procure por **"Get Array Item"**
   - Conecte `SlotWidgets` ao `Array` pin
   - Conecte `FromSlot` (input do Custom Event) ao `Dimension 1` pin

3. **Adicione `Cast to WBP Inventory Slot`:**
   - Procure por **"Cast to WBP Inventory Slot"**
   - Conecte o `Output` de `Get Array Item` ao `Object` pin

4. **Adicione `Is Valid?`:**
   - Procure por **"Is Valid?"**
   - Conecte o `As WBP Inventory Slot` ao `Object` pin

5. **Adicione `Clear Slot`:**
   - No caminho `TRUE` do `Is Valid?`
   - Procure por **"Clear Slot"** (função do `UmbraInventorySlotWidget`)
   - Conecte o `As WBP Inventory Slot` ao `Target` pin

6. **Adicione `Update Slot Visual`:**
   - Após `Clear Slot` (conecte o `then` de `Clear Slot` ao `execute` de `Update Slot Visual`)
   - Procure por **"Update Slot Visual"** (função do `UmbraInventorySlotWidget`)
   - Conecte o mesmo `As WBP Inventory Slot` ao `Target` pin

7. **Mantenha `Load Inventory`:**
   - Se já existir, mantenha
   - Se não existir, adicione após o `Is Valid?` (conecte o `then` de `Is Valid?` ou após `Update Slot Visual`)

---

## ⚠️ **IMPORTANTE:**

1. **O `Clear Slot` deve ser chamado ANTES de `Load Inventory`:**
   - Isso garante que o slot anterior seja limpo imediatamente
   - O `Load Inventory` atualizará o `ToSlot` automaticamente

2. **NÃO use `Get Inventory Slot By Index` para o `FromSlot`:**
   - O inventário já foi limpo no C++ antes de `LoadInventory()` ser chamado
   - `Get Inventory Slot By Index` sempre retornará `false` para o `FromSlot`

3. **O `Update Slot Visual` é essencial:**
   - `Clear Slot` apenas limpa os dados
   - `Update Slot Visual` atualiza a aparência visual do slot

---

## 🧪 **TESTE:**

1. Abra o inventário
2. Mova um item do slot 0 para o slot 5
3. **O slot 0 deve ser limpo IMEDIATAMENTE** (antes da API responder)
4. O slot 5 deve mostrar o item após a API responder
5. Não deve haver cópias visuais nos slots intermediários

---

## 📝 **RESUMO:**

- **Adicione `Clear Slot` e `Update Slot Visual` no `OnItemMoved_Event`**
- **Use o `FromSlot` para obter o widget correto do array `SlotWidgets`**
- **Limpe o slot ANTES de `Load Inventory` ser chamado**

