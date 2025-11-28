# 📦 GUIA COMPLETO: Implementar Junção de Itens (Stack Item)

## 🎯 **OBJETIVO:**

Permitir que o jogador junte dois itens do mesmo tipo em uma única pilha, arrastando um item sobre outro.

---

## ✅ **O QUE JÁ ESTÁ PRONTO:**

1. ✅ **API PHP:** `www/umbra_api/api/inventory/stack_item.php`
2. ✅ **Função C++:** `UUmbraGameInstance::StackItem()`
3. ✅ **Função C++:** `UUmbraInventorySlotWidget::RequestStackItem()`
4. ✅ **Delegates:** `OnItemStacked` e `OnItemStackFailed`

---

## 🔧 **PARTE 1: Implementar no Blueprint - WBP_InventorySlot**

### **1.1 Modificar OnDrop para Detectar Stack**

**NO `WBP_InventorySlot` → `Event Graph` → `OnDrop`:**

**ADICIONE LÓGICA PARA DETECTAR SE PODE FAZER STACK:**

```
[OnDrop]
  ↓
[Get Source Slot Widget] ← Do DragDropOperation
  └─ Return Value: Source Slot Widget
  ↓
[Is Valid?]
  └─ Object: Source Slot Widget
  ↓
[Branch]
  ├─ TRUE: (Source Slot válido)
  │   ├─ [Get Slot Data] ← Source Slot Widget
  │   │     └─ Return Value: Source Slot Data
  │   ├─ [Is Slot Empty?] ← self (slot de destino)
  │   │     └─ Return Value: (Boolean)
  │   ├─ [Branch] ← Verificar se slot de destino está vazio
  │   │     ├─ TRUE: (Destino vazio - MOVER NORMALMENTE)
  │   │     │   └─ [Request Move Item] ← Lógica normal de movimento
  │   │     │
  │   │     └─ FALSE: (Destino ocupado - VERIFICAR STACK)
  │   │         ├─ [Get Slot Data] ← self (slot de destino)
  │   │         │     └─ Return Value: Target Slot Data
  │   │         ├─ [Can Stack With] ← Source Slot Widget
  │   │         │     ├─ Target: Source Slot Widget
  │   │         │     └─ Other Item Data: Target Slot Data
  │   │         │     └─ Return Value: (Boolean)
  │   │         ├─ [Branch] ← Verificar se pode fazer stack
  │   │         │     ├─ TRUE: (PODE FAZER STACK)
  │   │         │     │   └─ [Request Stack Item] ← Source Slot Widget
  │   │         │     │         ├─ Target: Source Slot Widget
  │   │         │     │         └─ Target Slot Widget: self
  │   │         │     │
  │   │         │     └─ FALSE: (NÃO PODE FAZER STACK - TENTAR SWAP)
  │   │         │         └─ [Request Move Item] ← Lógica normal de swap
  │   │         │
  │   │         └─ [Handled]
  │   │
  │   └─ FALSE: (Source Slot inválido)
  │       └─ [Unhandled]
```

---

## 📝 **COMO OBTER OS NÓS:**

### **1. Get Source Slot Widget:**
- Já deve existir no seu `OnDrop`
- Se não existir, veja o guia anterior de drag and drop

### **2. Is Slot Empty?:**
- Right Click → "Is Slot Empty"
- Ou use a função do C++ `UUmbraInventorySlotWidget::IsSlotEmpty()`

### **3. Get Slot Data:**
- Right Click → "Get Slot Data"
- Ou use a função do C++ `UUmbraInventorySlotWidget::GetSlotData()`

### **4. Can Stack With:**
- Right Click → "Can Stack With"
- Ou use a função do C++ `UUmbraInventorySlotWidget::CanStackWith()`
- **Target:** Source Slot Widget
- **Other Item Data:** Target Slot Data (do slot de destino)

### **5. Request Stack Item:**
- Right Click → "Request Stack Item"
- Ou use a função do C++ `UUmbraInventorySlotWidget::RequestStackItem()`
- **Target:** Source Slot Widget (o item que está sendo arrastado)
- **Target Slot Widget:** self (o slot de destino)

---

## 🔧 **PARTE 2: Conectar Delegates no WBP_Inventory**

### **2.1 Conectar OnItemStacked**

**NO `WBP_Inventory` → `Event Graph` → `Event Construct`:**

**ADICIONE:**

```
[Event Construct]
  ↓
[Get Game Instance]
  └─ Return Value: Game Instance
  ↓
[Cast to Umbra Game Instance]
  └─ Object: Game Instance
  └─ Return Value: Umbra Game Instance
  ↓
[Bind Event to OnItemStacked] ← ✅ ADICIONAR ESTE!
  ├─ Target: Umbra Game Instance
  └─ Event: OnItemStacked_Event (criar evento customizado)
```

**CRIAR EVENTO CUSTOMIZADO `OnItemStacked_Event`:**

```
[OnItemStacked_Event]
  ├─ Source Inventory ID: (Int32)
  ├─ Target Inventory ID: (Int32)
  ├─ Amount Stacked: (Int32)
  └─ Source Removed: (Boolean)
  ↓
[Refresh Inventory] ← Recarregar inventário
```

---

### **2.2 Conectar OnItemStackFailed**

**NO `WBP_Inventory` → `Event Graph` → `Event Construct`:**

**ADICIONAR:**

```
[Bind Event to OnItemStackFailed] ← ✅ ADICIONAR ESTE!
  ├─ Target: Umbra Game Instance
  └─ Event: OnItemStackFailed_Event (criar evento customizado)
```

**CRIAR EVENTO CUSTOMIZADO `OnItemStackFailed_Event`:**

```
[OnItemStackFailed_Event]
  └─ Error Message: (String)
  ↓
[Print String] ← Mostrar erro ao jogador
  └─ In String: Error Message
```

---

## 🔧 **PARTE 3: Conectar Delegates no WBP_Storage**

### **3.1 Conectar OnItemStacked**

**NO `WBP_Storage` → `Event Graph` → `Event Construct`:**

**ADICIONAR:**

```
[Bind Event to OnItemStacked] ← ✅ ADICIONAR ESTE!
  ├─ Target: Umbra Game Instance
  └─ Event: OnItemStacked_Event (criar evento customizado)
```

**CRIAR EVENTO CUSTOMIZADO `OnItemStacked_Event`:**

```
[OnItemStacked_Event]
  ├─ Source Inventory ID: (Int32)
  ├─ Target Inventory ID: (Int32)
  ├─ Amount Stacked: (Int32)
  └─ Source Removed: (Boolean)
  ↓
[Load Storage] ← Recarregar storage
```

---

## 🎯 **PARTE 4: Lógica Completa do Fluxo**

### **4.1 Fluxo Completo:**

1. **Jogador arrasta item sobre outro item**
   - `OnDrop` é chamado no slot de destino
   - Verifica se o slot de destino está vazio
   - Se vazio: Move normalmente
   - Se ocupado: Verifica se pode fazer stack

2. **Verificação de Stack:**
   - Verifica se são do mesmo tipo (`item_template_id`)
   - Verifica se o item pode ser empilhado (`max_stack_size > 1`)
   - Verifica se o slot de destino não está cheio

3. **Se pode fazer stack:**
   - Chama `RequestStackItem` no slot de origem
   - C++ chama API `stack_item.php`
   - API junta os itens no banco de dados

4. **C++ processa resposta:**
   - Chama `OnItemStacked` delegate
   - Recarrega inventário e storage
   - Blueprint atualiza visual

5. **Se não pode fazer stack:**
   - Tenta fazer swap (mover normalmente)

---

## ⚠️ **IMPORTANTE - DETALHES CRÍTICOS:**

### **1. Ordem de Verificação no OnDrop:**

**A ORDEM É CRÍTICA:**

1. ✅ Verificar se Source Slot é válido
2. ✅ Verificar se Destino está vazio → **MOVER**
3. ✅ Verificar se pode fazer stack → **STACK**
4. ✅ Caso contrário → **SWAP**

### **2. Validações no RequestStackItem:**

O C++ já faz as seguintes validações:
- ✅ Slot de origem não está vazio
- ✅ Slot de destino não está vazio
- ✅ Itens são do mesmo tipo
- ✅ Item pode ser empilhado
- ✅ Slot de destino não está cheio

### **3. Recarregamento Automático:**

O C++ já recarrega automaticamente:
- ✅ `LoadInventory()` se envolver slots 0-49
- ✅ `LoadStorage()` se envolver slots 50-149

**VOCÊ SÓ PRECISA:**
- ✅ Conectar os delegates `OnItemStacked` e `OnItemStackFailed`
- ✅ Chamar `Refresh Inventory` ou `Load Storage` nos eventos

---

## 📋 **CHECKLIST DE IMPLEMENTAÇÃO:**

### **WBP_InventorySlot:**
- [ ] Modificar `OnDrop` para detectar stack
- [ ] Adicionar verificação `Is Slot Empty?`
- [ ] Adicionar verificação `Can Stack With`
- [ ] Adicionar chamada `Request Stack Item`

### **WBP_Inventory:**
- [ ] Conectar `OnItemStacked` delegate
- [ ] Conectar `OnItemStackFailed` delegate
- [ ] Criar evento `OnItemStacked_Event`
- [ ] Criar evento `OnItemStackFailed_Event`
- [ ] Chamar `Refresh Inventory` no `OnItemStacked_Event`

### **WBP_Storage:**
- [ ] Conectar `OnItemStacked` delegate
- [ ] Conectar `OnItemStackFailed` delegate
- [ ] Criar evento `OnItemStacked_Event`
- [ ] Criar evento `OnItemStackFailed_Event`
- [ ] Chamar `Load Storage` no `OnItemStacked_Event`

---

## 🎉 **PRONTO!**

Após implementar, o sistema de junção de itens estará funcionando completamente!

**TESTE:**
1. Arraste um item empilhável sobre outro do mesmo tipo
2. Os itens devem se juntar até o limite de `max_stack_size`
3. Se não couber tudo, o item de origem deve manter a quantidade restante

---

**DÚVIDAS?** Consulte os arquivos:
- `www/umbra_api/api/inventory/stack_item.php` - API PHP
- `UmbraGameInstance.cpp` - Implementação C++
- `UmbraInventorySlotWidget.cpp` - Função Blueprint

