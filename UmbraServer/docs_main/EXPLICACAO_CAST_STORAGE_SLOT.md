# 🔧 EXPLICAÇÃO: Cast to WBP Inventory Slot

## 📋 **CONTEXTO:**

Na linha 64 do `GUIA_SIMPLES_LOADSTORAGE_BLUEPRINT.md`, você precisa fazer um `Cast to WBP Inventory Slot`.

---

## 🎯 **RESPOSTA DIRETA:**

**O `Object` do `Cast to WBP Inventory Slot` é o `Output` do `Get Array Item (StorageSlots, Index: Slot Index)`.**

---

## 📋 **CONEXÃO COMPLETA:**

```
ForEachLoop (Get All Storage Slots result)
  Loop Body:
    ├─ Get Array Element  ← Elemento do GetAllStorageSlots (FUmbraInventorySlot struct)
    ├─ Break Umbra Inventory Slot
    │     └─ Slot Index
    │
    ├─ Get Array Item (StorageSlots, Index: Slot Index)
    │     └─ Output (Object)  ← ESTE É O OBJECT DO CAST!
    │
    └─ Cast to WBP Inventory Slot
          └─ Object: Output (do Get Array Item acima)
          └─ then:
              ├─ Set Slot Data (do Cast)
              └─ Update Slot Visual (do Cast)
```

---

## 🔧 **PASSO A PASSO:**

### **PASSO 1: Obter o Widget do Array**

```
Get Array Item
  ├─ Array: StorageSlots (Array of WBP Inventory Slot)
  └─ Dimension 1: Slot Index (do Break Umbra Inventory Slot)
  └─ Output: Object (genérico)
```

### **PASSO 2: Fazer o Cast**

```
Cast to WBP Inventory Slot
  ├─ Object: Output (do Get Array Item)
  └─ then:
      └─ As WBP Inventory Slot (agora você tem o tipo correto)
```

### **PASSO 3: Usar o Widget**

```
Set Slot Data (do Cast)
  ├─ Target: As WBP Inventory Slot (do Cast)
  └─ New Slot Data: Array Element (do ForEachLoop)
```

---

## ⚠️ **IMPORTANTE:**

1. **`StorageSlots` é um Array de Widgets:**
   - Tipo: `Array of WBP Inventory Slot`
   - Criado pelo `CreateStorageSlots`
   - Contém os widgets visuais dos slots

2. **`Get Array Item` retorna Object genérico:**
   - Por isso precisa fazer `Cast`
   - O cast confirma que é realmente um `WBP_InventorySlot`

3. **`Get All Storage Slots` retorna Structs:**
   - Tipo: `Array of FUmbraInventorySlot` (dados)
   - Não são widgets, são dados
   - Use `Get Array Element` (não `Get Array Item`)

---

## 📋 **ESTRUTURA COMPLETA CORRIGIDA:**

```
OnStorageLoaded_Event
  ↓
Get All Storage Slots (MyGameInstance)
  ↓
For Loop (First: 0, Last: 99)
  Loop Body:
    ├─ Get Array Item (GetAllStorageSlots result, Index: Index)
    │     └─ Output: FUmbraInventorySlot (struct)
    ├─ Break Umbra Inventory Slot
    │     └─ Slot Index, Inventory ID
    │
    ├─ Get Array Item (StorageSlots, Index: Index)
    │     └─ Output: Object (genérico)  ← WIDGET DO SLOT
    │
    └─ Cast to WBP Inventory Slot
          └─ Object: Output (do Get Array Item acima)
          └─ then:
              ├─ Branch (Inventory ID > 0)
              │   ├─ TRUE:
              │   │   ├─ Set Slot Data
              │   │   │     └─ Target: As WBP Inventory Slot
              │   │   │     └─ New Slot Data: Array Element (do GetAllStorageSlots)
              │   │   └─ Update Slot Visual
              │   │
              │   └─ FALSE:
              │       ├─ Clear Slot
              │       └─ Update Slot Visual
```

---

## 🎯 **RESUMO:**

- **`Get Array Item (StorageSlots)`** → Retorna o **widget** do slot (Object genérico)
- **`Cast to WBP Inventory Slot`** → Converte para o tipo correto (WBP_InventorySlot)
- **`As WBP Inventory Slot`** → Use este para `Set Slot Data` e `Update Slot Visual`

