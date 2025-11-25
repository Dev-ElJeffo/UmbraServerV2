# 🔧 IMPLEMENTAÇÃO: Funções de Movimento Storage no Blueprint

## 🎯 **PROBLEMA:**

As funções `MoveItemToStorage` e `MoveItemFromStorage` no C++ estão marcadas como `TODO` e não fazem chamadas à API.

## ✅ **SOLUÇÃO:**

Criar funções Blueprint no `WBP_Storage` que chamam a API diretamente usando VaRest.

---

## 📋 **PARTE 1: Criar Função MoveItemToStorageBlueprint**

### **No `WBP_Storage` → Functions → Add Function:**

**Nome:** `MoveItemToStorageBlueprint`

**Inputs:**
- `InventoryItemID` (Integer)
- `TargetSlotIndex` (Integer) ← Índice 0-99 do storage

**Implementação:**

```
MoveItemToStorageBlueprint (Function)
  ├─ Input: InventoryItemID (Integer)
  └─ Input: TargetSlotIndex (Integer)
  ↓
Get MyGameInstance
  ↓
Is Valid? (MyGameInstance)
  ├─ TRUE:
  │   ├─ Get Current Token (MyGameInstance)
  │   ├─ Get Va Rest Subsystem
  │   ├─ Construct Va Rest Request JSON
  │   │     └─ URL: "http://localhost/umbra_api/api/storage/move_to_storage.php"
  │   │     └─ Verb: "POST"
  │   ├─ Construct Va Rest Json Object
  │   ├─ Set String Field
  │   │     └─ Field Name: "token"
  │   │     └─ String Value: Current Token
  │   ├─ Set Number Field
  │   │     └─ Field Name: "inventory_id"
  │   │     └─ Number Value: InventoryItemID
  │   ├─ Add (TargetSlotIndex + 50)  ← Converte 0-99 para 50-149
  │   ├─ Set Number Field
  │   │     └─ Field Name: "target_slot_index"
  │   │     └─ Number Value: Resultado do Add
  │   ├─ Set Request Object (VaRest Request)
  │   │     └─ Request Object: JSON Object criado
  │   ├─ Bind Event to OnRequestComplete
  │   │     └─ Custom Event: OnMoveToStorageComplete
  │   ├─ Bind Event to OnRequestFail
  │   │     └─ Custom Event: OnMoveToStorageFail
  │   └─ Execute Process Request
  │
  └─ FALSE:
      Print String "GameInstance inválido"
```

### **Criar Custom Event: OnMoveToStorageComplete**

```
OnMoveToStorageComplete (Custom Event)
  ├─ Input: Request (VaRest Request JSON)
  ↓
Get Response Object (Request)
  ↓
Get Bool Field ("success")
  ↓
Branch (success)
  ├─ TRUE:
  │   ├─ Load Storage (self)
  │   ├─ Get Game Instance
  │   ├─ Cast to Umbra Game Instance
  │   └─ Load Inventory
  │
  └─ FALSE:
      Get String Field ("message")
      Print String (message)
```

### **Criar Custom Event: OnMoveToStorageFail**

```
OnMoveToStorageFail (Custom Event)
  ├─ Input: Request (VaRest Request JSON)
  ↓
Print String "Falha ao mover item para storage"
```

---

## 📋 **PARTE 2: Criar Função MoveItemFromStorageBlueprint**

### **No `WBP_Storage` → Functions → Add Function:**

**Nome:** `MoveItemFromStorageBlueprint`

**Inputs:**
- `StorageItemID` (Integer)
- `TargetSlotIndex` (Integer) ← Índice 0-49 do inventário

**Implementação:**

```
MoveItemFromStorageBlueprint (Function)
  ├─ Input: StorageItemID (Integer)
  └─ Input: TargetSlotIndex (Integer)
  ↓
Get MyGameInstance
  ↓
Is Valid? (MyGameInstance)
  ├─ TRUE:
  │   ├─ Get Current Token (MyGameInstance)
  │   ├─ Get Va Rest Subsystem
  │   ├─ Construct Va Rest Request JSON
  │   │     └─ URL: "http://localhost/umbra_api/api/storage/move_from_storage.php"
  │   │     └─ Verb: "POST"
  │   ├─ Construct Va Rest Json Object
  │   ├─ Set String Field
  │   │     └─ Field Name: "token"
  │   │     └─ String Value: Current Token
  │   ├─ Set Number Field
  │   │     └─ Field Name: "storage_item_id"
  │   │     └─ Number Value: StorageItemID
  │   ├─ Set Number Field
  │   │     └─ Field Name: "target_slot_index"
  │   │     └─ Number Value: TargetSlotIndex  ← Já está no range 0-49
  │   ├─ Set Request Object (VaRest Request)
  │   │     └─ Request Object: JSON Object criado
  │   ├─ Bind Event to OnRequestComplete
  │   │     └─ Custom Event: OnMoveFromStorageComplete
  │   ├─ Bind Event to OnRequestFail
  │   │     └─ Custom Event: OnMoveFromStorageFail
  │   └─ Execute Process Request
  │
  └─ FALSE:
      Print String "GameInstance inválido"
```

### **Criar Custom Event: OnMoveFromStorageComplete**

```
OnMoveFromStorageComplete (Custom Event)
  ├─ Input: Request (VaRest Request JSON)
  ↓
Get Response Object (Request)
  ↓
Get Bool Field ("success")
  ↓
Branch (success)
  ├─ TRUE:
  │   ├─ Load Storage (self)
  │   ├─ Get Game Instance
  │   ├─ Cast to Umbra Game Instance
  │   └─ Load Inventory
  │
  └─ FALSE:
      Get String Field ("message")
      Print String (message)
```

### **Criar Custom Event: OnMoveFromStorageFail**

```
OnMoveFromStorageFail (Custom Event)
  ├─ Input: Request (VaRest Request JSON)
  ↓
Print String "Falha ao mover item do storage"
```

---

## 📋 **PARTE 3: Modificar OnDrop no WBP_InventorySlot**

**Agora você pode usar essas funções no `OnDrop`:**

### **CENÁRIO 1: Origem = STORAGE, Destino = INVENTÁRIO**

```
Get Parent Storage Widget (Source Slot Widget)  ← WBP_Storage
  ↓
Is Valid? (WBP_Storage)
  ├─ TRUE:
  │   ├─ Get Slot Data (Source Slot Widget)
  │   ├─ Break Umbra Inventory Slot (Source)
  │   │     └─ Inventory ID
  │   ├─ Get Slot Data (self)
  │   ├─ Break Umbra Inventory Slot (self)
  │   │     └─ Slot Index
  │   └─ Move Item From Storage Blueprint (WBP_Storage)
  │         └─ Storage Item ID: InventoryID (do Source)
  │         └─ Target Slot Index: SlotIndex (do self, 0-49)
  │         └─ Return Handled
  │
  └─ FALSE:
      Return Unhandled
```

### **CENÁRIO 2: Origem = INVENTÁRIO, Destino = STORAGE**

```
Get Parent Storage Widget (self)  ← WBP_Storage
  ↓
Is Valid? (WBP_Storage)
  ├─ TRUE:
  │   ├─ Get Slot Data (Source Slot Widget)
  │   ├─ Break Umbra Inventory Slot (Source)
  │   │     └─ Inventory ID
  │   ├─ Get Slot Index (self)
  │   ├─ Subtract (SlotIndex - 50)  ← Converte 50-149 para 0-99
  │   └─ Move Item To Storage Blueprint (WBP_Storage)
  │         └─ Inventory Item ID: InventoryID (do Source)
  │         └─ Target Slot Index: Resultado do Subtract (0-99)
  │         └─ Return Handled
  │
  └─ FALSE:
      Return Unhandled
```

---

## ⚠️ **IMPORTANTE:**

1. **URLs das APIs:**
   - `move_to_storage.php` - Mover do inventário para storage
   - `move_from_storage.php` - Mover do storage para inventário
   - **Se essas APIs não existirem, você precisa criá-las no PHP**

2. **Estrutura do JSON:**
   - `move_to_storage.php` espera: `token`, `inventory_id`, `target_slot_index` (50-149)
   - `move_from_storage.php` espera: `token`, `storage_item_id`, `target_slot_index` (0-49)

3. **Recarregar após mover:**
   - As funções Blueprint já chamam `Load Storage` e `Load Inventory` nos callbacks
   - Isso garante que ambos os widgets sejam atualizados

---

## 🧪 **TESTE:**

1. Implemente as funções Blueprint no `WBP_Storage`
2. Modifique o `OnDrop` no `WBP_InventorySlot`
3. Teste arrastar um item do inventário para o storage
4. Teste arrastar um item do storage para o inventário
5. Verifique se ambos os widgets são atualizados corretamente

---

## 📝 **RESUMO:**

1. ✅ Criar `MoveItemToStorageBlueprint` no `WBP_Storage` (chama API)
2. ✅ Criar `MoveItemFromStorageBlueprint` no `WBP_Storage` (chama API)
3. ✅ Criar Custom Events para callbacks (`OnMoveToStorageComplete`, etc.)
4. ✅ Modificar `OnDrop` no `WBP_InventorySlot` para usar essas funções
5. ✅ Criar APIs PHP se não existirem (`move_to_storage.php`, `move_from_storage.php`)

