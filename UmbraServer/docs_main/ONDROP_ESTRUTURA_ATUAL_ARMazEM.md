# ✅ OnDrop - Adicionar Suporte ao Armazém na Estrutura Atual

## 📋 **ESTRUTURA ATUAL (FUNCIONA PERFEITAMENTE):**

```
OnDrop (Function Entry)
  ↓
Cast to Umbra Item Drag Drop Operation
  ├─ then → Branch (IsValid Cast)
  └─ CastFailed → Return false
  ↓
IsValid (Cast result)
  ↓
Branch
  ├─ TRUE → Get Source Slot Widget
  └─ FALSE → Return false
  ↓
IsValid (Source Slot Widget)
  ↓
Branch
  ├─ TRUE → Process Item Drop → Return true
  └─ FALSE → Return false
```

---

## 🎯 **ONDE ADICIONAR A LÓGICA DO ARMAZÉM:**

**ADICIONE APÓS `IsValid (Source Slot Widget)` e ANTES de `Process Item Drop`!**

### **PASSO 1: Obter SlotIndex de Origem e Destino**

Após `IsValid (Source Slot Widget)` = TRUE:

```
Get Source Slot Widget (Return Value)
  ↓
Get Slot Data
  ├─ Target: Get Source Slot Widget (Return Value)
  └─ Return Value: FUmbraInventorySlot
  ↓
Break Umbra Inventory Slot
  └─ SlotIndex: int32 (ORIGEM)
```

E também:

```
Get Slot Data (self)
  ├─ Target: self
  └─ Return Value: FUmbraInventorySlot
  ↓
Break Umbra Inventory Slot
  └─ SlotIndex: int32 (DESTINO)
```

---

### **PASSO 2: Verificar se Destino é Inventário (< 50) ou Armazém (>= 50)**

```
Branch (Destino SlotIndex < 50?)
  ├─ TRUE: Destino é INVENTÁRIO
  └─ FALSE: Destino é ARMAZÉM
```

---

### **PASSO 3: Se Destino é INVENTÁRIO (< 50):**

```
Branch (Origem SlotIndex < 50?)
  ├─ TRUE: Origem é INVENTÁRIO
  │   └─ Process Item Drop (FUNCIONA COMO ANTES!)
  │       ├─ Target: self
  │       └─ Dragged Slot Widget: Get Source Slot Widget
  │
  └─ FALSE: Origem é ARMAZÉM (>= 50)
      └─ Move Item From Storage
          ├─ Target: Get ParentStorageWidget (Source Slot Widget)
          ├─ StorageItemID: Break (InventoryID da origem)
          └─ TargetSlotIndex: Break (SlotIndex do destino/self)
```

---

### **PASSO 4: Se Destino é ARMAZÉM (>= 50):**

```
Branch (Origem SlotIndex < 50?)
  ├─ TRUE: Origem é INVENTÁRIO
  │   └─ Move Item To Storage
  │       ├─ Target: Get ParentStorageWidget (self)
  │       ├─ InventoryItemID: Break (InventoryID da origem)
  │       └─ TargetSlotIndex: Break (SlotIndex do destino/self)
  │
  └─ FALSE: Origem é ARMAZÉM (>= 50)
      └─ Process Item Drop (reorganizar dentro do armazém)
          ├─ Target: self
          └─ Dragged Slot Widget: Get Source Slot Widget
```

---

## 🔧 **ESTRUTURA COMPLETA MODIFICADA:**

```
OnDrop (Function Entry)
  ↓
Cast to Umbra Item Drag Drop Operation
  ├─ then → Branch (IsValid Cast)
  └─ CastFailed → Return false
  ↓
IsValid (Cast result)
  ↓
Branch
  ├─ TRUE → Get Source Slot Widget
  └─ FALSE → Return false
  ↓
IsValid (Source Slot Widget)
  ↓
Branch
  ├─ TRUE → [NOVA LÓGICA AQUI]
  │   ↓
  │   Get Slot Data (Source)
  │   ├─ Target: Get Source Slot Widget
  │   └─ Break → SlotIndex (ORIGEM)
  │   ↓
  │   Get Slot Data (self)
  │   ├─ Target: self
  │   └─ Break → SlotIndex (DESTINO)
  │   ↓
  │   Branch (Destino < 50?)
  │   ├─ TRUE: (Destino = Inventário)
  │   │   ↓
  │   │   Branch (Origem < 50?)
  │   │   ├─ TRUE: Process Item Drop (INVENTÁRIO → INVENTÁRIO)
  │   │   └─ FALSE: Move Item From Storage (ARMAZÉM → INVENTÁRIO)
  │   │
  │   └─ FALSE: (Destino = Armazém)
  │       ↓
  │       Branch (Origem < 50?)
  │       ├─ TRUE: Move Item To Storage (INVENTÁRIO → ARMAZÉM)
  │       └─ FALSE: Process Item Drop (ARMAZÉM → ARMAZÉM)
  │
  └─ FALSE → Return false
```

---

## ⚠️ **IMPORTANTE:**

1. **NÃO REMOVA NENHUMA CONEXÃO EXISTENTE!**
2. **ADICIONE a nova lógica ANTES de `Process Item Drop`**
3. **Use `Branch` para decidir qual função chamar**
4. **Todas as funções devem conectar ao mesmo `Return Node` com `ReturnValue = true`**

---

## 📝 **NÓS NECESSÁRIOS:**

1. **Get Slot Data** (2x: Source e self)
2. **Break Umbra Inventory Slot** (2x: Source e self) - extrair `SlotIndex` e `InventoryID`
3. **Greater (Int Int)** ou **Less (Int Int)** (2x: comparar SlotIndex com 50)
4. **Branch** (3x: Destino < 50?, Origem < 50? dentro de cada)
5. **Get ParentStorageWidget** (2x: Source e self) - **PRECISA EXISTIR NO C++!**
6. **Move Item To Storage** (se destino >= 50 e origem < 50)
7. **Move Item From Storage** (se destino < 50 e origem >= 50)
8. **Process Item Drop** (mantém como está para inventário → inventário e armazém → armazém)

---

## ⚠️ **IMPORTANTE: ParentStorageWidget**

**Se `ParentStorageWidget` não existir no C++, você precisa:**
1. Adicionar a propriedade `UPROPERTY(BlueprintReadWrite, Category = "Storage") class UWBP_Storage* ParentStorageWidget;` em `UmbraInventorySlotWidget.h`
2. Ou usar uma abordagem alternativa (obter o Storage Widget de outra forma)

**Alternativa sem ParentStorageWidget:**
- Usar `Get Parent` duas vezes para obter o `WBP_Storage` (mas você disse que isso não funciona)
- Ou passar o Storage Widget como parâmetro durante a criação dos slots

---

**A lógica atual de `Process Item Drop` continua funcionando para inventário → inventário!** 🎯

