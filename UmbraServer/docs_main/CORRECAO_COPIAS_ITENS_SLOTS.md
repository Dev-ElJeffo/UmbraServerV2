# 🔧 CORREÇÃO: Cópias de itens aparecendo em múltiplos slots

**PROBLEMA:** Após mover um item, cópias aparecem em todos os slots pelos quais o item passou. As cópias só desaparecem ao fechar e reabrir o inventário.

---

## 🔍 **CAUSA RAIZ:**

O `OnInventoryLoaded_Event` está iterando apenas sobre `CurrentInventory`, que contém **apenas os slots COM itens**. Os slots vazios não estão no array, então eles **não são limpos/atualizados**.

**Exemplo:**
- Slot 5 tem um item
- Você move para slot 10
- `CurrentInventory` agora tem o item no slot 10
- Mas o slot 5 **não está no array**, então ele não é limpo
- Resultado: o item aparece nos slots 5 E 10

---

## ✅ **SOLUÇÃO:**

O `OnInventoryLoaded_Event` deve iterar sobre **TODOS os 50 slots** (0-49), não apenas sobre `CurrentInventory`.

### **CORREÇÃO NO `OnInventoryLoaded_Event`:**

**ANTES (ERRADO):**
```
On Inventory Loaded Event
  ↓
Get Current Inventory (do Game Instance)
  ↓
ForEachLoop (Current Inventory)  ← SÓ ITERA SOBRE SLOTS COM ITENS!
  ├─ Array Element: ItemSlot
  └─ Break Umbra Inventory Slot
      └─ Slot Index
  ↓
Get Array Item (Slot Widgets, Index: Slot Index)
  ↓
Set Slot Data
  ↓
Update Slot Visual
```

**DEPOIS (CORRETO):**
```
On Inventory Loaded Event
  ↓
For Loop (First: 0, Last: 49)  ← ITERA SOBRE TODOS OS 50 SLOTS!
  ├─ Loop Index: SlotIndex
  ↓
Get Game Instance (Cast to Umbra Game Instance)
  ↓
Get Inventory Slot By Index (Slot Index: SlotIndex)
  ├─ Out Slot: SlotData
  └─ Return Value: bFound
  ↓
Branch (bFound?)
  ├─ TRUE: (Slot tem item)
  │   ├─ Get Array Item (Slot Widgets, Index: SlotIndex)
  │   ├─ Set Slot Data (do Array Item)
  │   │     └─ New Slot Data: SlotData
  │   └─ Update Slot Visual (do Array Item)
  │
  └─ FALSE: (Slot está vazio)
      ├─ Get Array Item (Slot Widgets, Index: SlotIndex)
      ├─ Clear Slot (do Array Item)  ← LIMPA O SLOT!
      └─ Update Slot Visual (do Array Item)  ← ATUALIZA PARA MOSTRAR VAZIO!
```

---

## 🛠️ **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: Abrir `WBP_Inventory` → `OnInventoryLoaded_Event`**

### **PASSO 2: REMOVER o `ForEachLoop` sobre `CurrentInventory`**

### **PASSO 3: ADICIONAR `For Loop` (0 a 49):**

1. Clique direito → `For Loop`
2. **First Index:** `0`
3. **Last Index:** `49`

### **PASSO 4: Dentro do `Loop Body`:**

```
Loop Body (exec)
  ↓
Get Game Instance (Cast to Umbra Game Instance)
  ↓
Get Inventory Slot By Index
  ├─ Target: Game Instance
  ├─ Slot Index: Index (do For Loop)
  ├─ Out Slot: SlotData (variável local)
  └─ Return Value: bFound (variável local)
  ↓
Branch (bFound?)
  ├─ TRUE:
  │   ├─ Get Array Item (Slot Widgets, Index: Index)
  │   ├─ Set Slot Data (do Array Item)
  │   │     └─ New Slot Data: SlotData
  │   └─ Update Slot Visual (do Array Item)
  │
  └─ FALSE:
      ├─ Get Array Item (Slot Widgets, Index: Index)
      ├─ Clear Slot (do Array Item)  ← IMPORTANTE!
      └─ Update Slot Visual (do Array Item)
```

### **PASSO 5: Verificar se `ClearSlot` existe no `WBP_InventorySlot`**

Se não existir, você pode criar um slot vazio manualmente:

```
FALSE Branch:
  ├─ Get Array Item (Slot Widgets, Index: Index)
  ├─ Make Umbra Inventory Slot
  │     ├─ Inventory ID: 0
  │     ├─ Item Template ID: 0
  │     ├─ Quantity: 0
  │     ├─ Slot Index: Index
  │     └─ (outros campos: 0 ou vazio)
  ├─ Set Slot Data (do Array Item)
  │     └─ New Slot Data: (struct criado acima)
  └─ Update Slot Visual (do Array Item)
```

---

## 📋 **CÓDIGO COMPLETO DO `OnInventoryLoaded_Event`:**

```
Custom Event: On Inventory Loaded Event
  ↓
For Loop (First: 0, Last: 49)
  Loop Body:
    ├─ Get Game Instance (Cast to Umbra Game Instance)
    ├─ Get Inventory Slot By Index
    │     ├─ Slot Index: Index (do For Loop)
    │     ├─ Out Slot: SlotData
    │     └─ Return Value: bFound
    ├─ Branch (bFound?)
    │   ├─ TRUE:
    │   │   ├─ Get Array Item (Slot Widgets, Index: Index)
    │   │   ├─ Set Slot Data (do Array Item)
    │   │   │     └─ New Slot Data: SlotData
    │   │   └─ Update Slot Visual (do Array Item)
    │   │
    │   └─ FALSE:
    │       ├─ Get Array Item (Slot Widgets, Index: Index)
    │       ├─ Make Umbra Inventory Slot
    │       │     ├─ Inventory ID: 0
    │       │     ├─ Item Template ID: 0
    │       │     ├─ Quantity: 0
    │       │     ├─ Slot Index: Index
    │       │     └─ Durability: 0.0
    │       ├─ Set Slot Data (do Array Item)
    │       │     └─ New Slot Data: (struct criado acima)
    │       └─ Update Slot Visual (do Array Item)
```

---

## 🔍 **VERIFICAÇÃO:**

### **1. Adicione logs de debug:**

No `OnInventoryLoaded_Event`, após o `Branch`:

```
Branch (bFound?)
  ├─ TRUE:
  │   └─ Print String: "Slot X tem item: ID Y"
  │
  └─ FALSE:
      └─ Print String: "Slot X está vazio - limpando"
```

Isso vai mostrar quais slots estão sendo atualizados e quais estão sendo limpos.

### **2. Teste o movimento:**

1. Mova um item do slot 5 para o slot 10
2. Verifique os logs:
   - Slot 5 deve aparecer como "vazio - limpando"
   - Slot 10 deve aparecer como "tem item"
3. Verifique visualmente:
   - Slot 5 deve estar vazio
   - Slot 10 deve ter o item
   - **NÃO deve haver cópias em outros slots**

---

## 📋 **CHECKLIST:**

- [ ] `OnInventoryLoaded_Event` usa `For Loop` (0-49) em vez de `ForEachLoop` sobre `CurrentInventory`
- [ ] Para cada slot, chama `Get Inventory Slot By Index`
- [ ] Se `bFound = true`, atualiza o slot com os dados do item
- [ ] Se `bFound = false`, limpa o slot (cria um slot vazio)
- [ ] Chama `Update Slot Visual` em ambos os casos
- [ ] Compilou sem erros
- [ ] Testou e verificou que não há mais cópias

---

## 🎯 **RESULTADO ESPERADO:**

✅ **Mover item** → Item aparece apenas no slot de destino  
✅ **Slot de origem** → Fica vazio imediatamente  
✅ **Sem cópias** → Item não aparece em múltiplos slots  
✅ **Atualização imediata** → Não precisa fechar/abrir inventário

---

## ⚠️ **IMPORTANTE:**

Se você ainda tiver o `OnItemMoved_Event` atualizando slots manualmente, **REMOVA essa lógica**! O `OnInventoryLoaded_Event` já atualiza todos os slots corretamente.

**Simplifique o `OnItemMoved_Event`:**

```
Custom Event: On Item Moved Event (InventoryID, FromSlot, ToSlot)
  ↓
Print String: "Item movido! From: X, To: Y"  ← DEBUG (opcional)
  ↓
(Remover tudo - o OnInventoryLoaded já atualiza tudo)
```

**OU** apenas chame `LoadInventory`:

```
Custom Event: On Item Moved Event (InventoryID, FromSlot, ToSlot)
  ↓
Get Game Instance (Cast to Umbra Game Instance)
  ↓
Load Inventory (do Game Instance)
  ↓
(O OnInventoryLoaded será disparado automaticamente)
```

---

**IMPLEMENTE A CORREÇÃO E TESTE!** 🚀

