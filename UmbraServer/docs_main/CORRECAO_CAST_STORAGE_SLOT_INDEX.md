# 🔧 CORREÇÃO: Cast to WBP Inventory Slot - Usar SlotIndex

## ❌ **PROBLEMA:**

O `Cast to WBP_InventorySlot` está recebendo uma **struct** (`FUmbraInventorySlot`) em vez de um **widget**.

Na sua implementação atual:
- `Get Array Item` está usando `Array Index` do `ForEachLoop`
- Mas o `Array` conectado é do `GetAllStorageSlots` (structs), não do `StorageSlots` (widgets)

---

## ✅ **SOLUÇÃO:**

Você precisa usar o **`SlotIndex`** (do `Break Umbra Inventory Slot`) para obter o widget correto do array `StorageSlots`.

---

## 📋 **ESTRUTURA CORRIGIDA:**

```
OnStorageLoaded_Event
  ↓
Get All Storage Slots (MyGameInstance)
  ↓
ForEachLoop (GetAllStorageSlots result)
  Loop Body:
    ├─ Get Array Element  ← Struct FUmbraInventorySlot (dados)
    ├─ Break Umbra Inventory Slot
    │     └─ Slot Index  ← USE ESTE!
    │
    ├─ Get Array Item (StorageSlots, Index: Slot Index)  ← WIDGET!
    │     └─ Output (Object)
    │
    └─ Cast to WBP Inventory Slot
          └─ Object: Output (do Get Array Item acima)
          └─ then:
              ├─ Set Slot Data
              │     └─ Target: As WBP Inventory Slot
              │     └─ New Slot Data: Array Element (do ForEachLoop)
              └─ Update Slot Visual
```

---

## 🔧 **PASSO A PASSO:**

### **PASSO 0: Criar a Variável `StorageSlots` (SE AINDA NÃO EXISTIR)**

**⚠️ IMPORTANTE:** Se você não tem a variável `StorageSlots`, você precisa criá-la primeiro!

**Veja o guia completo:** `COMO_CRIAR_ARRAY_STORAGESLOTS.md`

**Resumo rápido:**
1. No `WBP_Storage`, vá para a aba **"Variables"**
2. Clique em **"+ Variable"**
3. Nome: `StorageSlots`
4. Tipo: **Array** → **WBP Inventory Slot**
5. Certifique-se de preencher o array na função `CreateStorageSlots` usando `Add to Array`

---

### **PASSO 1: Remover a Conexão Errada**

1. **Desconecte** o `Output` do `Get Array Item` que está usando `Array Index` do `ForEachLoop`
2. **Delete** esse `Get Array Item` se ele estiver acessando o array de structs

### **PASSO 2: Obter o Array `StorageSlots`**

**Como obter o nó "Get StorageSlots":**

1. **No Event Graph**, **clique com botão direito**
2. **Digite** "StorageSlots" ou "Get StorageSlots"
3. **Selecione** a variável `StorageSlots` da lista
4. **Isso criará** o nó **"Get StorageSlots"** (ou **"StorageSlots"**)

**Alternativa:**
- **Arraste** a variável `StorageSlots` do painel **"Variables"** para o Event Graph

### **PASSO 3: Criar o Get Array Item Correto**

1. **Crie** um novo `Get Array Item`
2. **Conecte:**
   - `Array`: **Get StorageSlots** (do passo 2 acima)
   - `Dimension 1`: `Slot Index` (do `Break Umbra Inventory Slot`)
3. **Output**: `Object` (genérico)

### **PASSO 3: Conectar ao Cast**

1. **Conecte** o `Output` do `Get Array Item (StorageSlots)` ao `Object` do `Cast to WBP Inventory Slot`
2. **Conecte** o `then` do `Cast` ao `Set Slot Data`

---

## ⚠️ **IMPORTANTE:**

### **Dois Arrays Diferentes:**

1. **`GetAllStorageSlots()`** → Retorna `Array of FUmbraInventorySlot` (structs/dados)
   - Use `ForEachLoop` para iterar
   - Use `Get Array Element` para obter cada struct
   - Use `Break Umbra Inventory Slot` para extrair `SlotIndex`

2. **`StorageSlots`** (variável local) → `Array of WBP Inventory Slot` (widgets)
   - Criado por `CreateStorageSlots`
   - Use `Get Array Item` com `SlotIndex` para obter o widget
   - Use `Cast` para confirmar o tipo

### **NÃO Use Array Index do ForEachLoop:**

- O `Array Index` do `ForEachLoop` é o índice no array de structs (0, 1, 2, ...)
- O `SlotIndex` do struct é o índice real do slot (0-99 para storage)
- **Sempre use `SlotIndex`** para acessar o array de widgets!

---

## 📋 **ESTRUTURA COMPLETA CORRIGIDA:**

```
OnStorageLoaded_Event
  ↓
Get All Storage Slots (MyGameInstance)
  ↓
ForEachLoop (GetAllStorageSlots result)
  Loop Body:
    ├─ Get Array Element  ← Struct (dados)
    ├─ Break Umbra Inventory Slot
    │     └─ Slot Index  ← EXTRAIR ESTE!
    │
    ├─ Get Array Item (StorageSlots, Index: Slot Index)  ← WIDGET!
    │     └─ Array: StorageSlots (variável local)
    │     └─ Dimension 1: Slot Index (do Break)
    │     └─ Output: Object
    │
    └─ Cast to WBP Inventory Slot
          └─ Object: Output (do Get Array Item)
          └─ then:
              ├─ Branch (Inventory ID > 0)
              │   ├─ TRUE:
              │   │   ├─ Set Slot Data
              │   │   │     └─ Target: As WBP Inventory Slot
              │   │   │     └─ New Slot Data: Array Element (do ForEachLoop)
              │   │   └─ Update Slot Visual
              │   │
              │   └─ FALSE:
              │       ├─ Clear Slot
              │       └─ Update Slot Visual
```

---

## 🎯 **RESUMO:**

1. **`ForEachLoop`** itera sobre **structs** (`GetAllStorageSlots`)
2. **`Break Umbra Inventory Slot`** extrai o **`SlotIndex`**
3. **`Get Array Item (StorageSlots, Index: Slot Index)`** obtém o **widget**
4. **`Cast to WBP Inventory Slot`** confirma o tipo
5. **`Set Slot Data`** atualiza o widget com os dados da struct

**NÃO use `Array Index` do `ForEachLoop` para acessar widgets!**

