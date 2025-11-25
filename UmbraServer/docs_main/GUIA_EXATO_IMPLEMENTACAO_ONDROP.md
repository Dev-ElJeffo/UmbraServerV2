# 🎯 GUIA EXATO: O Que Implementar em Cada Lugar do OnDrop

## 📊 **ANÁLISE DO CÓDIGO ATUAL:**

Analisando seu código, vejo que você já tem:
- ✅ Verificação de origem (`K2Node_IfThenElse_8`) conectada ao `then` do `K2Node_IfThenElse_7`
- ✅ Verificação de destino (`K2Node_IfThenElse_7` e `K2Node_IfThenElse_9`)
- ❌ **FALTAM as implementações das funções de movimento**

---

## 🔧 **IMPLEMENTAÇÃO 1: Storage → Inventário**

### **Localização:** `K2Node_IfThenElse_8` [then] → `K2Node_IfThenElse_7` [else]

**Cenário:** Origem = Storage, Destino = Inventário

**O que fazer:**
1. O `then` do `K2Node_IfThenElse_8` está conectado ao `execute` do `K2Node_IfThenElse_7` ✅
2. O `else` do `K2Node_IfThenElse_7` está conectado ao `K2Node_Knot_7` ✅
3. **IMPLEMENTAR AQUI:** Após o `K2Node_Knot_7` (que vem do `else` do `K2Node_IfThenElse_7`)

### **PASSO A PASSO:**

#### **1. Get Parent Storage Widget (Source) - Para obter WBP_Storage**

**Como obter:**
- Arraste o `ReturnValue` (azul) do `K2Node_VariableGet_3` (Get Parent Storage Widget - Source)
- Ou crie um novo: Arraste o `ReturnValue` (azul) do `K2Node_CallFunction_12` (GetSourceSlotWidget) → Digite "Get Parent Storage Widget"

**Conectar:**
- **`Target` (azul):** Conecte ao `ReturnValue` (azul) do `K2Node_CallFunction_12` (GetSourceSlotWidget)
- **`execute` (branco):** Conecte ao `OutputPin` (branco) do `K2Node_Knot_7`

**Nome do nó:** `GetParentStorageWidget_Source`

---

#### **2. IsValid (WBP_Storage - Source)**

**Como obter:**
- Arraste o `ReturnValue` (azul) do `GetParentStorageWidget_Source`
- Solte no espaço vazio
- Digite "Is Valid?"
- Selecione o nó

**Conectar:**
- **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `GetParentStorageWidget_Source`
- **`execute` (branco):** Conecte ao `then` (branco) do `GetParentStorageWidget_Source`

**Nome do nó:** `IsValid_StorageSource`

---

#### **3. Get Game Instance**

**Como obter:**
- Clique com botão direito no espaço vazio
- Digite "Get Game Instance"
- Selecione o nó

**Conectar:**
- **`execute` (branco):** Conecte ao `then` (branco) do `IsValid_StorageSource`

**Nome do nó:** `GetGameInstance`

---

#### **4. Cast to Umbra Game Instance**

**Como obter:**
- Arraste o `ReturnValue` (azul) do `GetGameInstance`
- Solte no espaço vazio
- Digite "Cast to Umbra Game Instance"
- Selecione o nó

**Conectar:**
- **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `GetGameInstance`
- **`execute` (branco):** Conecte ao `then` (branco) do `GetGameInstance`

**Nome do nó:** `CastToUmbraGameInstance`

---

#### **5. Get Storage ID By Inventory ID**

**Como obter:**
- Arraste o `As Umbra Game Instance` (azul) do `CastToUmbraGameInstance`
- Solte no espaço vazio
- Digite "Get Storage ID By Inventory ID"
- Selecione o nó

**Conectar:**
- **`Target` (azul):** Conecte ao `As Umbra Game Instance` (azul) do `CastToUmbraGameInstance`
- **`Inventory ID` (int):** Conecte ao `InventoryID` (int) do `K2Node_BreakStruct_0` (Break Umbra Inventory Slot)
- **`execute` (branco):** Conecte ao `then` (branco) do `CastToUmbraGameInstance`

**Nome do nó:** `GetStorageIDByInventoryID`

---

#### **6. Greater (Storage ID > 0?)**

**Como obter:**
- Arraste o `ReturnValue` (int, verde) do `GetStorageIDByInventoryID`
- Solte no espaço vazio
- Digite ">" (Greater)
- Selecione o nó

**Conectar:**
- **`A` (int):** Conecte ao `ReturnValue` (int) do `GetStorageIDByInventoryID`
- **`B` (int):** Clique com botão direito → "Make Literal Int" → Digite `0`

**Nome do nó:** `Greater_StorageID`

---

#### **7. Branch (Storage ID > 0?)**

**Como obter:**
- Arraste o `ReturnValue` (bool, verde) do `Greater_StorageID`
- Solte no espaço vazio
- Digite "Branch"
- Selecione o nó

**Conectar:**
- **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `Greater_StorageID`
- **`execute` (branco):** Conecte ao `then` (branco) do `GetStorageIDByInventoryID`

**Nome do nó:** `Branch_StorageIDGreaterThanZero`

---

#### **8. Get Slot Index (Destino)**

**Como obter:**
- Arraste um nó "Self" (clique com botão direito → "Self")
- Arraste o pino azul do `Self`
- Solte no espaço vazio
- Digite "Get Slot Index"
- Selecione o nó

**Conectar:**
- **`Target` (azul):** Conecte ao `Self` (azul)
- **`execute` (branco):** Conecte ao `TRUE` (branco) do `Branch_StorageIDGreaterThanZero`

**Nome do nó:** `GetSlotIndex_Destino`

---

#### **9. Move Item From Storage**

**Como obter:**
- Arraste o `ReturnValue` (azul) do `GetParentStorageWidget_Source`
- Solte no espaço vazio
- Digite "Move Item From Storage"
- Selecione o nó

**Conectar:**
- **`Target` (azul):** Conecte ao `ReturnValue` (azul) do `GetParentStorageWidget_Source`
- **`Storage Item ID` (int):** Conecte ao `ReturnValue` (int) do `GetStorageIDByInventoryID`
- **`Target Slot Index` (int):** Conecte ao `ReturnValue` (int) do `GetSlotIndex_Destino`
- **`execute` (branco):** Conecte ao `then` (branco) do `GetSlotIndex_Destino`

**Nome do nó:** `MoveItemFromStorage`

---

#### **10. Branch (Return Value do Move Item From Storage)**

**Como obter:**
- Arraste o `ReturnValue` (bool, verde) do `MoveItemFromStorage`
- Solte no espaço vazio
- Digite "Branch"
- Selecione o nó

**Conectar:**
- **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `MoveItemFromStorage`
- **`execute` (branco):** Conecte ao `then` (branco) do `MoveItemFromStorage`

**Nome do nó:** `Branch_MoveItemFromStorageResult`

---

#### **11. Return Handled / Return Unhandled**

**Como obter:**
- Clique com botão direito → Digite "Return Handled"
- Selecione o nó

**Conectar:**
- **`execute` (branco):** Conecte ao `TRUE` (branco) do `Branch_MoveItemFromStorageResult`

**Nome do nó:** `ReturnHandled_StorageToInventory`

---

**Como obter:**
- Clique com botão direito → Digite "Return Unhandled"
- Selecione o nó

**Conectar:**
- **`execute` (branco):** Conecte ao `FALSE` (branco) do `Branch_MoveItemFromStorageResult`

**Nome do nó:** `ReturnUnhandled_StorageToInventory`

---

## 🔧 **IMPLEMENTAÇÃO 2: Inventário → Storage**

### **Localização:** `K2Node_IfThenElse_8` [else] → Novo `Branch (Destino é Storage?)` [TRUE]

**Cenário:** Origem = Inventário, Destino = Storage

**O que fazer:**
1. O `else` do `K2Node_IfThenElse_8` não está conectado ❌
2. **IMPLEMENTAR AQUI:** Após o `else` do `K2Node_IfThenElse_8`

### **PASSO A PASSO:**

#### **1. Get Parent Storage Widget (Destino)**

**Como obter:**
- Arraste um nó "Self" (clique com botão direito → "Self")
- Arraste o pino azul do `Self`
- Solte no espaço vazio
- Digite "Get Parent Storage Widget"
- Selecione o nó

**Conectar:**
- **`Target` (azul):** Conecte ao `Self` (azul)
- **`execute` (branco):** Conecte ao `else` (branco) do `K2Node_IfThenElse_8`

**Nome do nó:** `GetParentStorageWidget_Destino`

---

#### **2. IsValid (Parent Storage Widget do Destino)**

**Como obter:**
- Arraste o `ReturnValue` (azul) do `GetParentStorageWidget_Destino`
- Solte no espaço vazio
- Digite "Is Valid?"
- Selecione o nó

**Conectar:**
- **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `GetParentStorageWidget_Destino`
- **`execute` (branco):** Conecte ao `then` (branco) do `GetParentStorageWidget_Destino`

**Nome do nó:** `IsValid_Destino`

---

#### **3. Branch (Destino é Storage?)**

**Como obter:**
- Arraste o `ReturnValue` (bool, verde) do `IsValid_Destino`
- Solte no espaço vazio
- Digite "Branch"
- Selecione o nó

**Conectar:**
- **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `IsValid_Destino`
- **`execute` (branco):** Conecte ao `then` (branco) do `IsValid_Destino`

**Nome do nó:** `Branch_DestinoIsStorage`

---

#### **4. Get Parent Storage Widget (Destino) - Para obter WBP_Storage**

**Como obter:**
- Arraste um nó "Self" (clique com botão direito → "Self")
- Arraste o pino azul do `Self`
- Solte no espaço vazio
- Digite "Get Parent Storage Widget"
- Selecione o nó

**Conectar:**
- **`Target` (azul):** Conecte ao `Self` (azul)
- **`execute` (branco):** Conecte ao `TRUE` (branco) do `Branch_DestinoIsStorage`

**Nome do nó:** `GetParentStorageWidget_DestinoForStorage`

---

#### **5. IsValid (WBP_Storage - Destino)**

**Como obter:**
- Arraste o `ReturnValue` (azul) do `GetParentStorageWidget_DestinoForStorage`
- Solte no espaço vazio
- Digite "Is Valid?"
- Selecione o nó

**Conectar:**
- **`Object` (azul):** Conecte ao `ReturnValue` (azul) do `GetParentStorageWidget_DestinoForStorage`
- **`execute` (branco):** Conecte ao `then` (branco) do `GetParentStorageWidget_DestinoForStorage`

**Nome do nó:** `IsValid_StorageDestino`

---

#### **6. Get Slot Index (Destino)**

**Como obter:**
- Arraste um nó "Self" (clique com botão direito → "Self")
- Arraste o pino azul do `Self`
- Solte no espaço vazio
- Digite "Get Slot Index"
- Selecione o nó

**Conectar:**
- **`Target` (azul):** Conecte ao `Self` (azul)
- **`execute` (branco):** Conecte ao `then` (branco) do `IsValid_StorageDestino`

**Nome do nó:** `GetSlotIndex_DestinoForStorage`

---

#### **7. Move Item To Storage**

**Como obter:**
- Arraste o `ReturnValue` (azul) do `GetParentStorageWidget_DestinoForStorage`
- Solte no espaço vazio
- Digite "Move Item To Storage"
- Selecione o nó

**Conectar:**
- **`Target` (azul):** Conecte ao `ReturnValue` (azul) do `GetParentStorageWidget_DestinoForStorage`
- **`Inventory Item ID` (int):** Conecte ao `InventoryID` (int) do `K2Node_BreakStruct_0` (Break Umbra Inventory Slot)
- **`Target Slot Index` (int):** Conecte ao `ReturnValue` (int) do `GetSlotIndex_DestinoForStorage`
- **`execute` (branco):** Conecte ao `then` (branco) do `GetSlotIndex_DestinoForStorage`

**Nome do nó:** `MoveItemToStorage`

---

#### **8. Branch (Return Value do Move Item To Storage)**

**Como obter:**
- Arraste o `ReturnValue` (bool, verde) do `MoveItemToStorage`
- Solte no espaço vazio
- Digite "Branch"
- Selecione o nó

**Conectar:**
- **`Condition` (bool):** Conecte ao `ReturnValue` (bool) do `MoveItemToStorage`
- **`execute` (branco):** Conecte ao `then` (branco) do `MoveItemToStorage`

**Nome do nó:** `Branch_MoveItemToStorageResult`

---

#### **9. Return Handled / Return Unhandled**

**Como obter:**
- Clique com botão direito → Digite "Return Handled"
- Selecione o nó

**Conectar:**
- **`execute` (branco):** Conecte ao `TRUE` (branco) do `Branch_MoveItemToStorageResult`

**Nome do nó:** `ReturnHandled_InventoryToStorage`

---

**Como obter:**
- Clique com botão direito → Digite "Return Unhandled"
- Selecione o nó

**Conectar:**
- **`execute` (branco):** Conecte ao `FALSE` (branco) do `Branch_MoveItemToStorageResult`

**Nome do nó:** `ReturnUnhandled_InventoryToStorage`

---

## 🔧 **IMPLEMENTAÇÃO 3: Inventário → Inventário**

### **Localização:** `K2Node_IfThenElse_8` [else] → Novo `Branch (Destino é Storage?)` [FALSE]

**Cenário:** Origem = Inventário, Destino = Inventário

**O que fazer:**
1. Use o mesmo `Branch_DestinoIsStorage` criado na Implementação 2
2. **IMPLEMENTAR AQUI:** Após o `FALSE` do `Branch_DestinoIsStorage`

### **PASSO A PASSO:**

#### **1. Process Item Drop**

**Como obter:**
- Arraste um nó "Self" (clique com botão direito → "Self")
- Arraste o pino azul do `Self`
- Solte no espaço vazio
- Digite "Process Item Drop"
- Selecione o nó

**Conectar:**
- **`Target` (azul):** Conecte ao `Self` (azul)
- **`Dragged Slot Widget` (azul):** Conecte ao `ReturnValue` (azul) do `K2Node_CallFunction_12` (GetSourceSlotWidget)
- **`execute` (branco):** Conecte ao `FALSE` (branco) do `Branch_DestinoIsStorage`

**Nome do nó:** `ProcessItemDrop`

---

#### **2. Return Handled**

**Como obter:**
- Clique com botão direito → Digite "Return Handled"
- Selecione o nó

**Conectar:**
- **`execute` (branco):** Conecte ao `then` (branco) do `ProcessItemDrop`

**Nome do nó:** `ReturnHandled_InventoryToInventory`

---

## 📊 **RESUMO DAS CONEXÕES:**

### **FLUXO COMPLETO:**

```
K2Node_IfThenElse_6 (Branch Inventory ID > 0?) [then]
  └─ K2Node_Knot_6
      └─ K2Node_IfThenElse_7 (Branch Destino é Storage?) [execute]
          ├─ [then] → K2Node_IfThenElse_8 (Branch Origem é Storage?) [execute]
          │   ├─ [then] → (Origem = Storage)
          │   │   └─ K2Node_IfThenElse_7 [else] → K2Node_Knot_7
          │   │       └─ [IMPLEMENTAÇÃO 1: Storage → Inventário]
          │   │
          │   └─ [else] → (Origem = Inventário)
          │       └─ GetParentStorageWidget_Destino
          │           └─ IsValid_Destino
          │               └─ Branch_DestinoIsStorage
          │                   ├─ [TRUE] → [IMPLEMENTAÇÃO 2: Inventário → Storage]
          │                   └─ [FALSE] → [IMPLEMENTAÇÃO 3: Inventário → Inventário]
          │
          └─ [else] → (Não usado neste cenário)
```

---

## ✅ **CHECKLIST FINAL:**

### **IMPLEMENTAÇÃO 1: Storage → Inventário**
- [ ] GetParentStorageWidget_Source
- [ ] IsValid_StorageSource
- [ ] GetGameInstance
- [ ] CastToUmbraGameInstance
- [ ] GetStorageIDByInventoryID
- [ ] Greater_StorageID
- [ ] Branch_StorageIDGreaterThanZero
- [ ] GetSlotIndex_Destino
- [ ] MoveItemFromStorage
- [ ] Branch_MoveItemFromStorageResult
- [ ] ReturnHandled_StorageToInventory
- [ ] ReturnUnhandled_StorageToInventory

### **IMPLEMENTAÇÃO 2: Inventário → Storage**
- [ ] GetParentStorageWidget_Destino
- [ ] IsValid_Destino
- [ ] Branch_DestinoIsStorage
- [ ] GetParentStorageWidget_DestinoForStorage
- [ ] IsValid_StorageDestino
- [ ] GetSlotIndex_DestinoForStorage
- [ ] MoveItemToStorage
- [ ] Branch_MoveItemToStorageResult
- [ ] ReturnHandled_InventoryToStorage
- [ ] ReturnUnhandled_InventoryToStorage

### **IMPLEMENTAÇÃO 3: Inventário → Inventário**
- [ ] ProcessItemDrop
- [ ] ReturnHandled_InventoryToInventory

---

**Siga este guia exato para implementar cada parte!** 🚀

