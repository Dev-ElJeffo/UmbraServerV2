# 🎯 GUIA VISUAL: OnDrop para Storage

## 📋 **ESTRUTURA COMPLETA DO OnDrop**

Este guia mostra exatamente como implementar o `OnDrop` no `WBP_InventorySlot` para suportar drag and drop entre inventário e storage.

---

## 🔧 **PASSO A PASSO COMPLETO:**

### **PASSO 1: Estrutura Base**

```
OnDrop (Override Function)
  ├─ Input: MyGeometry, PointerEvent, Operation
  ↓
Cast to Umbra Item Drag Drop Operation
  └─ Object: Operation
  ↓
then:
  ├─ Get Source Slot Widget
  └─ Is Valid? (Source Slot Widget)
```

---

### **PASSO 2: Obter Dados do Source e Self**

**Após `Is Valid? (Source Slot Widget) = TRUE`:**

```
TRUE:
  ├─ Get Slot Data (Source Slot Widget)
  ├─ Break Umbra Inventory Slot (Source)
  │     └─ Inventory ID → Variável Local: SourceInventoryID
  │     └─ Slot Index → Variável Local: SourceSlotIndex
  │
  ├─ Get Slot Data (self)
  ├─ Break Umbra Inventory Slot (self)
  │     └─ Slot Index → Variável Local: DestSlotIndex
  │
  └─ VERIFICAÇÃO DE ORIGEM:
      Get Parent Storage Widget (Source Slot Widget)
      ↓
      Is Valid? (Parent Storage Widget da origem)
      └─ Resultado → Variável Local: bSourceIsStorage
```

---

### **PASSO 3: Verificar Destino**

**Após obter `bSourceIsStorage`:**

```
Get Parent Storage Widget (self)
  ↓
Is Valid? (Parent Storage Widget do destino)
  └─ Resultado → Variável Local: bDestIsStorage
```

---

### **PASSO 4: Lógica Principal - Branch Principal**

**Criar um Branch com 4 caminhos:**

```
Branch Principal:
  ├─ Condição 1: bSourceIsStorage = TRUE AND bDestIsStorage = FALSE
  │   └─ CAMINHO 1: Storage → Inventário
  │
  ├─ Condição 2: bSourceIsStorage = FALSE AND bDestIsStorage = TRUE
  │   └─ CAMINHO 2: Inventário → Storage
  │
  ├─ Condição 3: bSourceIsStorage = FALSE AND bDestIsStorage = FALSE
  │   └─ CAMINHO 3: Inventário → Inventário (lógica existente)
  │
  └─ Condição 4: bSourceIsStorage = TRUE AND bDestIsStorage = TRUE
      └─ CAMINHO 4: Storage → Storage (não implementar)
```

---

### **CAMINHO 1: Storage → Inventário**

```
CAMINHO 1:
  ├─ Get Parent Storage Widget (Source Slot Widget)
  ├─ Is Valid? (WBP_Storage)
  ├─ TRUE:
  │   ├─ Move Item From Storage (WBP_Storage)
  │   │     └─ Storage Item ID: SourceInventoryID
  │   │     └─ Target Slot Index: DestSlotIndex (0-49)
  │   │
  │   ├─ Branch (Return Value)
  │   │   ├─ TRUE:
  │   │   │   ├─ Load Storage Blueprint (WBP_Storage)
  │   │   │   ├─ Get Game Instance
  │   │   │   ├─ Cast to Umbra Game Instance
  │   │   │   ├─ Load Inventory
  │   │   │   └─ Return Handled
  │   │   │
  │   │   └─ FALSE:
  │   │       └─ Return Unhandled
  │   │
  │   └─ FALSE:
  │       └─ Return Unhandled
```

---

### **CAMINHO 2: Inventário → Storage**

```
CAMINHO 2:
  ├─ Get Parent Storage Widget (self)
  ├─ Is Valid? (WBP_Storage)
  ├─ TRUE:
  │   ├─ Get Slot Index (self)
  │   ├─ Subtract (SlotIndex - 50)  ← Converte 50-149 para 0-99
  │   │     └─ Resultado → Variável Local: StorageSlotIndex
  │   │
  │   ├─ Move Item To Storage (WBP_Storage)
  │   │     └─ Inventory Item ID: SourceInventoryID
  │   │     └─ Target Slot Index: StorageSlotIndex (0-99)
  │   │
  │   ├─ Branch (Return Value)
  │   │   ├─ TRUE:
  │   │   │   ├─ Load Storage Blueprint (WBP_Storage)
  │   │   │   ├─ Get Game Instance
  │   │   │   ├─ Cast to Umbra Game Instance
  │   │   │   ├─ Load Inventory
  │   │   │   └─ Return Handled
  │   │   │
  │   │   └─ FALSE:
  │   │       └─ Return Unhandled
  │   │
  │   └─ FALSE:
  │       └─ Return Unhandled
```

---

### **CAMINHO 3: Inventário → Inventário**

```
CAMINHO 3:
  └─ (Lógica existente - manter como está)
  └─ Process Item Drop (Source Slot Widget)
  └─ Return Handled
```

---

### **CAMINHO 4: Storage → Storage**

```
CAMINHO 4:
  └─ Return Unhandled  ← Não implementar mover dentro do storage
```

---

## 🔧 **COMO CRIAR O BRANCH PRINCIPAL:**

### **OPÇÃO 1: Usar Múltiplos Branches Aninhados**

```
Branch (bSourceIsStorage)
  ├─ TRUE:
  │   └─ Branch (bDestIsStorage)
  │       ├─ TRUE: CAMINHO 4 (Storage → Storage)
  │       └─ FALSE: CAMINHO 1 (Storage → Inventário)
  │
  └─ FALSE:
      └─ Branch (bDestIsStorage)
          ├─ TRUE: CAMINHO 2 (Inventário → Storage)
          └─ FALSE: CAMINHO 3 (Inventário → Inventário)
```

### **OPÇÃO 2: Usar Switch on Int (Mais Limpo)**

**Criar uma variável `DropType` (Integer):**

```
Calculate DropType:
  ├─ If bSourceIsStorage = TRUE AND bDestIsStorage = FALSE → DropType = 1
  ├─ If bSourceIsStorage = FALSE AND bDestIsStorage = TRUE → DropType = 2
  ├─ If bSourceIsStorage = FALSE AND bDestIsStorage = FALSE → DropType = 3
  └─ If bSourceIsStorage = TRUE AND bDestIsStorage = TRUE → DropType = 4

Switch on Int (DropType)
  ├─ Case 1: CAMINHO 1 (Storage → Inventário)
  ├─ Case 2: CAMINHO 2 (Inventário → Storage)
  ├─ Case 3: CAMINHO 3 (Inventário → Inventário)
  └─ Case 4: CAMINHO 4 (Storage → Storage)
```

---

## 📋 **VARIÁVEIS LOCAIS NECESSÁRIAS:**

1. **`SourceInventoryID`** (Integer)
2. **`SourceSlotIndex`** (Integer)
3. **`DestSlotIndex`** (Integer)
4. **`bSourceIsStorage`** (Boolean)
5. **`bDestIsStorage`** (Boolean)
6. **`StorageSlotIndex`** (Integer) - apenas para CAMINHO 2
7. **`WBP_Storage_Ref`** (Object Reference → WBP Storage) - para armazenar referência do storage

---

## ⚠️ **IMPORTANTE:**

1. **Sempre valide:**
   - `Is Valid?` antes de usar widgets
   - `Return Value` das funções C++
   - `bSourceIsStorage` e `bDestIsStorage` antes de decidir o caminho

2. **Conversão de índices:**
   - Storage Blueprint: 0-99
   - Storage Banco: 50-149
   - Inventário: 0-49
   - Use `Subtract (SlotIndex - 50)` para converter

3. **Recarregar após mover:**
   - Sempre chame `LoadStorageBlueprint` e `LoadInventory`
   - Isso garante sincronização

4. **Return Handled vs Unhandled:**
   - `Return Handled` = operação bem-sucedida
   - `Return Unhandled` = operação falhou ou não aplicável

---

## 🧪 **TESTE:**

1. Teste cada caminho individualmente
2. Verifique logs para erros
3. Confirme que ambos os widgets são atualizados
4. Teste casos extremos (slots vazios, itens equipados, etc.)

