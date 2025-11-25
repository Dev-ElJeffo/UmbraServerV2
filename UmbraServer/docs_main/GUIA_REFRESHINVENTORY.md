# 🔄 GUIA: RefreshInventory - Limpar Todos os Slots

## 🎯 **FUNÇÃO CRIADA:**

Foi criada a função C++ `RefreshInventory` em `UUmbraInventoryWidget` que limpa **TODOS** os slots do inventário visualmente.

---

## 📋 **COMO USAR:**

### **PASSO 1: No `OnItemMoved_Event` do `WBP_Inventory`**

1. **Adicione `Refresh Inventory` ANTES de `Load Inventory`:**

```
Custom Event: On Item Moved Event
  ├─ Input: InventoryID (Integer)
  ├─ Input: FromSlot (Integer)
  └─ Input: ToSlot (Integer)
  ↓
Refresh Inventory  ← ADICIONE AQUI!
  └─ Slot Widgets: SlotWidgets (Array de WBP_InventorySlot)
  └─ then → Load Inventory (execute)
```

### **PASSO 2: Como obter o nó `Refresh Inventory`**

1. **No Event Graph do `WBP_Inventory`:**
   - Clique com botão direito → **"Refresh Inventory"**
   - Ou procure por **"Refresh Inventory"** na busca

2. **Conecte o array `SlotWidgets`:**
   - Arraste a variável `SlotWidgets` para o Event Graph
   - Conecte ao pin `Slot Widgets` do nó `Refresh Inventory`

3. **Conecte a execução:**
   - Conecte o `then` de `On Item Moved Event` ao `execute` de `Refresh Inventory`
   - Conecte o `then` de `Refresh Inventory` ao `execute` de `Load Inventory`

---

## ✅ **O QUE A FUNÇÃO FAZ:**

1. **Itera sobre TODOS os slots** do array `SlotWidgets`
2. **Chama `ClearSlot()`** em cada slot (limpa dados: InventoryID=0, ItemTemplateID=0, etc.)
3. **Chama `UpdateSlotVisual()`** do Blueprint em cada slot (remove ícones, barras, textos visualmente)

---

## ⚠️ **IMPORTANTE:**

1. **A função `UpdateSlotVisual` DEVE existir no Blueprint `WBP_InventorySlot`:**
   - Se não existir, a função mostrará um warning no log
   - Mas ainda limpará os dados (`ClearSlot`)

2. **A ordem é CRÍTICA:**
   - `Refresh Inventory` → `Load Inventory`
   - Isso garante que todos os slots sejam limpos ANTES de recarregar da API

3. **A função limpa TODOS os slots:**
   - Não apenas o `FromSlot`
   - Isso garante que não haja cópias visuais em nenhum slot

---

## 🧪 **TESTE:**

1. Abra o inventário
2. Mova um item de um slot para outro
3. **Todos os slots devem ser limpos IMEDIATAMENTE** (antes da API responder)
4. **Todos os slots devem ficar BRANCOS e VAZIOS** (sem ícones, barras, textos)
5. Após a API responder, apenas os slots com itens devem ser preenchidos

---

## 📝 **ESTRUTURA COMPLETA DO `OnItemMoved_Event`:**

```
Custom Event: On Item Moved Event
  ├─ Input: InventoryID (Integer)
  ├─ Input: FromSlot (Integer)
  └─ Input: ToSlot (Integer)
  ↓
Refresh Inventory
  └─ Slot Widgets: SlotWidgets (Array)
  └─ then → Load Inventory
      └─ Target: MyGameInstance
```

---

## 🔍 **LOGS:**

A função gera logs no Unreal Engine:
- `[UmbraInventoryWidget] RefreshInventory - Limpando X slots`
- `[UmbraInventoryWidget] RefreshInventory - Concluído! X slots limpos, Y slots atualizados visualmente`

Se algum slot não tiver a função `UpdateSlotVisual`, você verá:
- `[UmbraInventoryWidget] RefreshInventory - Função UpdateSlotVisual não encontrada no Blueprint para slot X!`

---

## ✅ **RESUMO:**

1. ✅ Adicione `Refresh Inventory` no `OnItemMoved_Event`
2. ✅ Conecte o array `SlotWidgets` ao pin `Slot Widgets`
3. ✅ Conecte `Refresh Inventory` ANTES de `Load Inventory`
4. ✅ Garanta que `UpdateSlotVisual` existe no Blueprint `WBP_InventorySlot`

