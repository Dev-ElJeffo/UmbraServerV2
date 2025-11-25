# 🔴 CORREÇÃO CRÍTICA: UpdateAllSlotsVisual - Índices Negativos

## 🎯 **PROBLEMA IDENTIFICADO:**

O `UpdateAllSlotsVisual` está tentando acessar índices negativos (-50 até -1), causando erros:

```
Attempted to access index -50 from array 'SlotWidgets' of length 100
WBP_STORAGE - UPDATE ALL SLOTS VISUAL - GET SLOT WIDGETS IS NOT VALID - SLOT INDEX = -50
```

**Causa:** O `UpdateAllSlotsVisual` está recebendo slots do inventário (0-49) quando deveria receber apenas slots do storage (50-149, convertidos para 0-99).

---

## ✅ **SOLUÇÃO:**

### **PROBLEMA 1: UpdateAllSlotsVisual está usando GetStorageData() incorretamente**

O `GetStorageData()` retorna `StorageSlots` que pode conter slots do inventário. Use `GetAllStorageSlots()` do `GameInstance` em vez disso.

### **PROBLEMA 2: Conversão de índices incorreta**

O `UpdateAllSlotsVisual` está subtraindo 50 de TODOS os slots, mas `GetAllStorageSlots()` já retorna slots com índices 0-99 (já convertidos).

---

## 📋 **CORREÇÃO DO UpdateAllSlotsVisual:**

### **ESTRUTURA CORRETA:**

```
Function: Update All Slots Visual
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓
then:
  └─ Get All Storage Slots  ← Use esta função, não GetStorageData()!
      ↓
  ForEachLoop (Storage Slots array)
    Loop Body:
      ├─ Break Umbra Inventory Slot
      │     └─ UmbraInventorySlot: Array Element
      ├─ Get Slot Index (do Break)
      ├─ Branch (Slot Index >= 0 AND Slot Index < 100)  ← FILTRAR APENAS STORAGE!
      │   ├─ TRUE:  ← Slot válido do storage (0-99)
      │   │   ├─ Get Array Item (StorageSlotWidgets, Index: Slot Index)  ← SEM SUBTRAIR 50!
      │   │   ├─ Is Valid? (do Array Item)
      │   │   │   ├─ TRUE:
      │   │   │   │   ├─ Cast to WBP Inventory Slot
      │   │   │   │   │     └─ Object: Output (Get Array Item)
      │   │   │   │   ├─ then (Cast bem-sucedido):
      │   │   │   │   │   ├─ Set Slot Data
      │   │   │   │   │   │     └─ New Slot Data: Array Element (do ForEachLoop)
      │   │   │   │   │   └─ Update Slot Visual
      │   │   │   │   │
      │   │   │   │   └─ CastFailed: (nada)
      │   │   │   │
      │   │   │   └─ FALSE: (nada)
      │   │   │
      │   └─ FALSE:  ← Slot do inventário (ignorar)
      │       (nada - não processar slots do inventário)
```

---

## 🛠️ **PASSO A PASSO:**

### **PASSO 1: Verificar qual função está sendo usada**

**No `UpdateAllSlotsVisual` do `WBP_Storage`:**

1. **Se estiver usando `GetStorageData()`:**
   - ❌ **REMOVA** `GetStorageData()`
   - ✅ **ADICIONE** `Get All Storage Slots` (do `GameInstance`)

2. **Se já estiver usando `GetAllStorageSlots()`:**
   - Verifique se está filtrando corretamente (próximo passo)

### **PASSO 2: Adicionar filtro para slots do storage**

**Adicione um `Branch` antes de processar o slot:**

1. **Após `Get Slot Index` (do `Break Umbra Inventory Slot`):**
   - Adicione `Branch` node
   - Adicione `AND` node
   - Adicione `Greater or Equal (Int Int)` node
   - Adicione `Less (Int Int)` node
   - Conecte:
     - `Slot Index` → `A` do `Greater or Equal`
     - `0` → `B` do `Greater or Equal`
     - `Slot Index` → `A` do `Less`
     - `100` → `B` do `Less`
     - Saídas dos `Greater or Equal` e `Less` → `A` e `B` do `AND`
     - Saída do `AND` → `Condition` do `Branch`

2. **No caminho `TRUE` do `Branch`:**
   - Processe o slot normalmente
   - **NÃO subtraia 50** do `Slot Index` - `GetAllStorageSlots()` já retorna índices 0-99!

3. **No caminho `FALSE` do `Branch`:**
   - Não faça nada (ignora slots do inventário)

### **PASSO 3: Remover subtração de 50**

**Se houver um `Subtract` (Slot Index - 50):**
- ❌ **REMOVA** este nó
- ✅ O `GetAllStorageSlots()` já retorna índices 0-99 (não precisa subtrair)

### **PASSO 4: Verificar nome do array**

**Certifique-se de usar o array correto:**
- ✅ `StorageSlotWidgets` (Array de `WBP_InventorySlot`)
- ❌ NÃO use `SlotWidgets` (esse é do inventário)

---

## 🔍 **VERIFICAÇÃO:**

### **Checklist:**

- [ ] `UpdateAllSlotsVisual` usa `Get All Storage Slots` (do `GameInstance`), não `GetStorageData()`
- [ ] Há um `Branch` que filtra apenas slots com `Slot Index >= 0 AND Slot Index < 100`
- [ ] **NÃO há** `Subtract` (Slot Index - 50) - `GetAllStorageSlots()` já retorna 0-99
- [ ] Usa `StorageSlotWidgets` (não `SlotWidgets`)
- [ ] `Get Array Item` usa `Slot Index` diretamente (sem subtrair 50)

---

## 📝 **RESUMO:**

**Problema:** `UpdateAllSlotsVisual` está processando slots do inventário (0-49) além dos slots do storage (50-149).

**Solução:**
1. Use `GetAllStorageSlots()` do `GameInstance` (não `GetStorageData()`)
2. Filtre apenas slots com `SlotIndex >= 0 AND SlotIndex < 100`
3. **NÃO subtraia 50** - `GetAllStorageSlots()` já retorna índices 0-99
4. Use `StorageSlotWidgets` (não `SlotWidgets`)

---

## ⚠️ **NOTA IMPORTANTE:**

O `GetAllStorageSlots()` do `GameInstance` já retorna slots com índices **0-99** (já convertidos do banco 50-149). Portanto, **NÃO é necessário subtrair 50** ao acessar o array `StorageSlotWidgets`.

