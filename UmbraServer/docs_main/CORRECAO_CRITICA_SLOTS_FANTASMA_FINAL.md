# 🔴 CORREÇÃO CRÍTICA: Slots Fantasma Após Mover Item

## 🎯 **PROBLEMA IDENTIFICADO:**

Quando um item é movido:
1. O slot de origem ainda mostra o item visualmente
2. O `UpdateSlotVisual` recebe `InventoryID = 0` mas o slot não é limpo corretamente
3. Cópias visuais ficam nos slots anteriores até o inventário ser recarregado

**Causa Raiz:** O `OnInventoryLoaded_Event` só é executado após a API responder, então há um delay onde o slot anterior ainda tem dados antigos no `SlotData` do widget.

---

## ✅ **SOLUÇÃO:**

### **OPÇÃO 1: Limpar Slot Imediatamente no `OnItemMoved_Event` (RECOMENDADO)**

Quando o delegate `OnItemMoved` é disparado, limpe imediatamente o slot de origem antes de aguardar o `OnInventoryLoaded`.

---

## 📋 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: Verificar se `OnItemMoved_Event` existe no `WBP_Inventory`**

Se não existir, crie:

1. **No `WBP_Inventory` - Event Graph:**
   - Clique com botão direito → **Add Custom Event**
   - Nome: `OnItemMoved_Event`
   - Adicione 3 parâmetros:
     - `InventoryID` (Integer)
     - `FromSlot` (Integer)
     - `ToSlot` (Integer)

### **PASSO 2: Conectar o Delegate no `Event Construct`**

**No `WBP_Inventory` - Event Construct:**

```
Event Construct
  ↓
Get Game Instance (Cast to Umbra Game Instance)
  ↓
Assign On Item Moved (do Game Instance)
  │   └─ Event: On Item Moved Event (Custom Event)
```

**Como fazer:**
1. Arraste o `MyGameInstance` (ou `Get Game Instance`)
2. Procure por **"Assign On Item Moved"**
3. Conecte o `Event` pin ao `OnItemMoved_Event` (Custom Event)

### **PASSO 3: Implementar a Limpeza Imediata no `OnItemMoved_Event`**

**No `WBP_Inventory` - `OnItemMoved_Event`:**

```
Custom Event: On Item Moved Event (InventoryID, FromSlot, ToSlot)
  ↓
Print String: "Item movido! From: X, To: Y"  ← DEBUG (opcional)
  ↓
Get Slot Widgets (Array)
  ↓
Get Array Item (SlotWidgets, Index = FromSlot)
  ↓
Is Valid? (SlotWidget)
  ├─ TRUE:
  │   └─ Clear Slot (SlotWidget)  ← LIMPAR IMEDIATAMENTE!
  │       └─ Update Slot Visual (SlotWidget)  ← ATUALIZAR VISUAL!
  │
  └─ FALSE:
      (nada)
```

**Como fazer:**

1. **Adicione `Get Slot Widgets`:**
   - Arraste a variável `SlotWidgets` para o Event Graph
   - Conecte ao `Array` pin de `Get Array Item`

2. **Adicione `Get Array Item`:**
   - Procure por **"Get Array Item"**
   - Conecte `SlotWidgets` ao `Array` pin
   - Conecte `FromSlot` ao `Dimension 1` pin

3. **Adicione `Is Valid?`:**
   - Procure por **"Is Valid?"**
   - Conecte o `Output` de `Get Array Item` ao `Object` pin

4. **Adicione `Clear Slot`:**
   - No caminho `TRUE` do `Is Valid?`
   - Procure por **"Clear Slot"** (função do `UmbraInventorySlotWidget`)
   - Conecte o `Output` de `Get Array Item` ao `Target` pin
   - **IMPORTANTE:** Use `Cast to WBP Inventory Slot` se necessário

5. **Adicione `Update Slot Visual`:**
   - Após `Clear Slot`
   - Procure por **"Update Slot Visual"** (função do `UmbraInventorySlotWidget`)
   - Conecte o mesmo `Output` de `Get Array Item` ao `Target` pin

---

## 🔧 **ESTRUTURA COMPLETA DO `OnItemMoved_Event`:**

```
Custom Event: On Item Moved Event
  ├─ Input: InventoryID (Integer)
  ├─ Input: FromSlot (Integer)
  └─ Input: ToSlot (Integer)
  ↓
Print String: "Item movido! From: {FromSlot}, To: {ToSlot}"  ← DEBUG
  ↓
Get Slot Widgets (Array)
  ↓
Get Array Item
  ├─ Array: SlotWidgets
  └─ Dimension 1: FromSlot
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

---

## ⚠️ **IMPORTANTE:**

1. **`Clear Slot` limpa o `SlotData` do widget:**
   - Define `InventoryID = 0`
   - Define `SlotIndex` (preserva o índice)
   - Limpa outros campos

2. **`Update Slot Visual` atualiza a visualização:**
   - Esconde o ícone do item
   - Esconde a quantidade
   - Esconde a barra de durabilidade
   - Define `bIsEmpty = true`

3. **O `OnInventoryLoaded_Event` ainda será executado:**
   - Ele atualizará TODOS os slots com os dados corretos da API
   - Mas o slot de origem já estará limpo visualmente

---

## 🧪 **TESTE:**

1. Mova um item do slot 0 para o slot 5
2. O slot 0 deve ser limpo **imediatamente** (antes da API responder)
3. O slot 5 deve mostrar o item após a API responder
4. Não deve haver cópias visuais nos slots intermediários

---

## 📝 **NOTAS:**

- Se `Clear Slot` não existir, use `Set Slot Data` com um `Make Umbra Inventory Slot` vazio (todos os campos = 0)
- Se `Update Slot Visual` não existir, verifique se a função foi criada no Blueprint `WBP_InventorySlot`
- O `Cast to WBP Inventory Slot` é necessário porque `Get Array Item` retorna um `Object` genérico

